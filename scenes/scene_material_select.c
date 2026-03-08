#include "../flipbox.h"

static void material_cb(void* context, uint32_t index) {
    FlipBoxApp* app = context;
    const QidiMaterialGroup* g = &qidi_material_groups[app->group_idx];
    app->mat_idx = g->indices[index];
    scene_manager_search_and_switch_to_previous_scene(app->scene_manager, FlipBoxSceneWriteCfg);
}

void flipbox_scene_material_select_on_enter(void* context) {
    FlipBoxApp* app = context;
    const QidiMaterialGroup* g = &qidi_material_groups[app->group_idx];

    submenu_reset(app->submenu);
    submenu_set_header(app->submenu, g->name);

    // Find which entry in this group matches current mat_idx (for pre-selection)
    uint8_t selected = 0;
    for(uint8_t i = 0; i < g->count; i++) {
        submenu_add_item(
            app->submenu,
            qidi_materials[g->indices[i]].name,
            i,
            material_cb,
            app);
        if(g->indices[i] == app->mat_idx) selected = i;
    }
    submenu_set_selected_item(app->submenu, selected);
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
