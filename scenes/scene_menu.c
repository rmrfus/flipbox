#include "../flipbox.h"

typedef enum {
    MenuItemWrite,
    MenuItemRead,
} MenuItem;

static void menu_cb(void* context, uint32_t index) {
    FlipBoxApp* app = context;
    if(index == MenuItemWrite) {
        scene_manager_next_scene(app->scene_manager, FlipBoxSceneWriteCfg);
    } else {
        scene_manager_next_scene(app->scene_manager, FlipBoxSceneRead);
    }
}

void flipbox_scene_menu_on_enter(void* context) {
    FlipBoxApp* app = context;
    submenu_reset(app->submenu);
    submenu_set_header(app->submenu, "FlipBox: QIDI Box NFC");
    submenu_add_item(app->submenu, "Write Tag", MenuItemWrite, menu_cb, app);
    submenu_add_item(app->submenu, "Read Tag",  MenuItemRead,  menu_cb, app);
    view_dispatcher_switch_to_view(app->view_dispatcher, FlipBoxViewSubmenu);
}

bool flipbox_scene_menu_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    return false;
}

void flipbox_scene_menu_on_exit(void* context) {
    FlipBoxApp* app = context;
    submenu_reset(app->submenu);
}
