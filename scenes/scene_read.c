#include "../flipbox.h"
#include <stdio.h>

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
            snprintf(
                app->text_buf, sizeof(app->text_buf),
                "Material: %s\nColor: %s\nMfr: %s",
                qidi_material_name(d->material_code),
                qidi_color_name(d->color_code),
                qidi_mfr_name(d->manufacturer_code));

            widget_reset(app->widget);
            widget_add_string_multiline_element(
                app->widget, 64, 28, AlignCenter, AlignCenter, FontSecondary, app->text_buf);
            widget_add_button_element(
                app->widget, GuiButtonTypeLeft, "Retry", read_btn_cb, app);
            widget_add_button_element(
                app->widget, GuiButtonTypeRight, "Edit & Write", read_btn_cb, app);

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

            int mi = qidi_material_idx_by_code(d->material_code);
            int ci = qidi_color_idx_by_code(d->color_code);
            int ri = qidi_mfr_idx_by_code(d->manufacturer_code);
            app->mat_idx = (mi >= 0) ? (uint8_t)mi : 0;
            app->col_idx = (ci >= 0) ? (uint8_t)ci : 0;
            app->mfr_idx = (ri >= 0) ? (uint8_t)ri : 0;

            // find the group containing mat_idx so MaterialGroup opens at the right entry
            app->group_idx = 0;
            for(uint8_t g = 0; g < QIDI_GROUP_COUNT; g++) {
                for(uint8_t k = 0; k < qidi_material_groups[g].count; k++) {
                    if(qidi_material_groups[g].indices[k] == app->mat_idx) {
                        app->group_idx = g;
                        break;
                    }
                }
            }

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
    popup_reset(app->popup);
    widget_reset(app->widget);
}
