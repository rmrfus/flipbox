#include "../flipbox.h"

static void material_cb(void* context, uint32_t index) {
    FlipBoxApp* app = context;
    app->mat_idx = (uint8_t)index;
    scene_manager_search_and_switch_to_previous_scene(app->scene_manager, FlipBoxSceneWriteCfg);
}

void flipbox_scene_material_select_on_enter(void* context) {
    FlipBoxApp* app = context;

    submenu_reset(app->submenu);
    submenu_set_header(app->submenu, qidi_group_names[app->group_idx]);

    // Item index is the qidi_materials[] index; selection falls back to the
    // first item when the current material is in another group
    for(uint8_t i = 0; i < QIDI_MATERIAL_COUNT; i++) {
        const QidiMaterialInfo* m = &qidi_materials[i];
        if(m->group != app->group_idx || !qidi_material_on_printer(m, app->printer)) continue;
        submenu_add_item(app->submenu, m->name, i, material_cb, app);
    }
    submenu_set_selected_item(app->submenu, app->mat_idx);
    view_dispatcher_switch_to_view(app->view_dispatcher, FlipBoxViewSubmenu);
}

bool flipbox_scene_material_select_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    return false;
}

void flipbox_scene_material_select_on_exit(void* context) {
    FlipBoxApp* app = context;
    submenu_reset(app->submenu);
}
