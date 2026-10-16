#include "../flipbox.h"
#include <stdio.h>

// Unknown codes are shown as "#<code>" so foreign tags still say something
static const char* name_or_code(const char* name, uint8_t code, char* buf, size_t size) {
    if(name) return name;
    snprintf(buf, size, "#%u", code);
    return buf;
}

// "A0 73 2C 36" as the NFC app shows it; longer UIDs drop the spaces to fit
// one 128 px line
static void uid_to_str(const NfcTagUid* uid, char* buf, size_t size) {
    size_t pos = 0;
    buf[0] = '\0';
    for(uint8_t i = 0; i < uid->len && pos < size; i++) {
        pos += snprintf(buf + pos, size - pos, (i && uid->len <= 4) ? " %02X" : "%02X", uid->data[i]);
    }
}

static void read_btn_cb(GuiButtonType type, InputType input_type, void* context) {
    if(input_type != InputTypeShort) return;
    FlipBoxApp* app = context;
    if(type == GuiButtonTypeRight) {
        view_dispatcher_send_custom_event(app->view_dispatcher, FlipBoxCustomEventReadEditTag);
    } else if(type == GuiButtonTypeLeft) {
        view_dispatcher_send_custom_event(app->view_dispatcher, FlipBoxCustomEventReadRetry);
    }
}

void flipbox_scene_read_on_enter(void* context) {
    FlipBoxApp* app = context;

    popup_reset(app->popup);
    popup_set_header(app->popup, "Read Tag", 64, 10, AlignCenter, AlignTop);
    popup_set_text(app->popup, "Hold tag\nnear Flipper...", 64, 36, AlignCenter, AlignCenter);
    view_dispatcher_switch_to_view(app->view_dispatcher, FlipBoxViewPopup);

    nfc_worker_read(app->worker);
}

bool flipbox_scene_read_on_event(void* context, SceneManagerEvent event) {
    FlipBoxApp* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == FlipBoxCustomEventReadSuccess) {
            const QidiTagData* d = &app->last_read;
            const char* mat_name = qidi_material_name(app->printer, d->material_code);
            const char* col_name = qidi_color_name(d->color_code);
            char mat[8], col[8], mfr[8], uid[3 * ISO14443_3A_MAX_UID_SIZE];
            uid_to_str(&app->last_uid, uid, sizeof(uid));
            // Short labels keep the longest names ("Support For PET/PA") on one line
            snprintf(
                app->text_buf, sizeof(app->text_buf),
                "\e#UID:\e# %s\n\e#Mat:\e# %s\n\e#Color:\e# %s\n\e#Mfr:\e# %s",
                uid,
                name_or_code(mat_name, d->material_code, mat, sizeof(mat)),
                name_or_code(col_name, d->color_code, col, sizeof(col)),
                name_or_code(
                    qidi_mfr_name(d->manufacturer_code), d->manufacturer_code, mfr, sizeof(mfr)));

            widget_reset(app->widget);
            // Text box, not multiline string: only it takes the \e# bold markup.
            // The height stops above the button row
            widget_add_text_box_element(
                app->widget, 0, 0, 128, 52, AlignCenter, AlignCenter, app->text_buf, true);
            widget_add_button_element(
                app->widget, GuiButtonTypeLeft, "Retry", read_btn_cb, app);
            // Editing would silently replace a code the selected printer's
            // table doesn't know, so only offer it when both are known
            if(mat_name && col_name) {
                widget_add_button_element(
                    app->widget, GuiButtonTypeRight, "Edit & Write", read_btn_cb, app);
            }

            view_dispatcher_switch_to_view(app->view_dispatcher, FlipBoxViewWidget);
            notification_message(app->notification, &sequence_success);
            consumed = true;
        } else if(event.event == FlipBoxCustomEventReadRetry) {
            popup_reset(app->popup);
            popup_set_header(app->popup, "Read Tag", 64, 10, AlignCenter, AlignTop);
            popup_set_text(app->popup, "Hold tag\nnear Flipper...", 64, 36, AlignCenter, AlignCenter);
            view_dispatcher_switch_to_view(app->view_dispatcher, FlipBoxViewPopup);
            nfc_worker_read(app->worker);
            consumed = true;
        } else if(event.event == FlipBoxCustomEventReadFail) {
            popup_set_header(app->popup, "Read Tag", 64, 10, AlignCenter, AlignTop);
            popup_set_text(app->popup, "No tag found.\nHold closer...", 64, 36, AlignCenter, AlignCenter);
            nfc_worker_read(app->worker);
            consumed = true;
        } else if(event.event == FlipBoxCustomEventReadEditTag) {
            const QidiTagData* d = &app->last_read;

            // The button exists only when material and color are known
            int mi = qidi_material_idx_by_code(app->printer, d->material_code);
            int ci = qidi_color_idx_by_code(d->color_code);
            int ri = qidi_mfr_idx_by_code(d->manufacturer_code);
            furi_check(mi >= 0 && ci >= 0);
            app->mat_idx = (uint8_t)mi;
            app->col_idx = (uint8_t)ci;
            app->mfr_idx = (ri >= 0) ? (uint8_t)ri : 0;

            // Fresh entry starts on the top row, never one OK away from writing
            scene_manager_set_scene_state(app->scene_manager, FlipBoxSceneWriteCfg, 0);
            scene_manager_next_scene(app->scene_manager, FlipBoxSceneWriteCfg);
            consumed = true;
        }
    } else if(event.type == SceneManagerEventTypeBack) {
        scene_manager_previous_scene(app->scene_manager);
        consumed = true;
    }

    return consumed;
}

void flipbox_scene_read_on_exit(void* context) {
    FlipBoxApp* app = context;
    nfc_worker_cancel(app->worker);
    popup_reset(app->popup);
    widget_reset(app->widget);
}
