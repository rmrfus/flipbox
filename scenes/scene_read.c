#include "../flipbox.h"
#include <stdio.h>

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
            popup_set_header(app->popup, "Tag Found!", 64, 4, AlignCenter, AlignTop);
            popup_set_text(app->popup, app->text_buf, 64, 36, AlignCenter, AlignCenter);
            notification_message(app->notification, &sequence_success);
            consumed = true;
        } else if(event.event == FlipBoxCustomEventReadFail) {
            popup_set_header(app->popup, "Read Tag", 64, 10, AlignCenter, AlignTop);
            popup_set_text(app->popup, "No tag found.\nHold closer...", 64, 36, AlignCenter, AlignCenter);
            nfc_worker_read(app->worker);
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
}
