#include "../flipbox.h"
#include <stdio.h>

static void start_write(FlipBoxApp* app) {
    QidiTagData data = {
        .material_code     = qidi_materials[app->mat_idx].code,
        .color_code        = qidi_colors[app->col_idx].code,
        .manufacturer_code = qidi_manufacturers[app->mfr_idx].code,
    };
    nfc_worker_write(app->worker, &data);
}

void flipbox_scene_write_on_enter(void* context) {
    FlipBoxApp* app = context;

    popup_reset(app->popup);
    popup_set_header(app->popup, "Write Tag", 64, 10, AlignCenter, AlignTop);
    popup_set_text(app->popup, "Hold tag\nnear Flipper...", 64, 36, AlignCenter, AlignCenter);
    view_dispatcher_switch_to_view(app->view_dispatcher, FlipBoxViewPopup);

    start_write(app);
}

bool flipbox_scene_write_on_event(void* context, SceneManagerEvent event) {
    FlipBoxApp* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == FlipBoxCustomEventWriteSuccess) {
            snprintf(
                app->text_buf, sizeof(app->text_buf),
                "Written!\n%s / %s",
                qidi_material_name(qidi_materials[app->mat_idx].code),
                qidi_color_name(qidi_colors[app->col_idx].code));
            popup_set_header(app->popup, "Success!", 64, 4, AlignCenter, AlignTop);
            popup_set_text(app->popup, app->text_buf, 64, 36, AlignCenter, AlignCenter);
            notification_message(app->notification, &sequence_success);
            consumed = true;
        } else if(event.event == FlipBoxCustomEventWriteFail) {
            popup_set_header(app->popup, "Write Tag", 64, 10, AlignCenter, AlignTop);
            popup_set_text(app->popup, "Failed!\nRetrying...", 64, 36, AlignCenter, AlignCenter);
            start_write(app);
            consumed = true;
        }
    } else if(event.type == SceneManagerEventTypeBack) {
        scene_manager_previous_scene(app->scene_manager);
        consumed = true;
    }

    return consumed;
}

void flipbox_scene_write_on_exit(void* context) {
    FlipBoxApp* app = context;
    popup_reset(app->popup);
}
