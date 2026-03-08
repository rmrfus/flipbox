#include "../flipbox.h"

static void color_cb(void* context, uint32_t index) {
    FlipBoxApp* app = context;
    app->col_idx = (uint8_t)index;
    scene_manager_previous_scene(app->scene_manager);
}

void flipbox_scene_color_select_on_enter(void* context) {
    FlipBoxApp* app = context;
    submenu_reset(app->submenu);
    submenu_set_header(app->submenu, "Color");
    for(uint8_t i = 0; i < QIDI_COLOR_COUNT; i++) {
        submenu_add_item(app->submenu, qidi_colors[i].name, i, color_cb, app);
    }
    submenu_set_selected_item(app->submenu, app->col_idx);
    view_dispatcher_switch_to_view(app->view_dispatcher, FlipBoxViewSubmenu);
}

bool flipbox_scene_color_select_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    return false;
}

void flipbox_scene_color_select_on_exit(void* context) {
    FlipBoxApp* app = context;
    submenu_reset(app->submenu);
}
