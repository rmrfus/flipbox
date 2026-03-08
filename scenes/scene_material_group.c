#include "../flipbox.h"
#include <stdio.h>

// Labels survive until on_exit clears the submenu
static char group_labels[QIDI_GROUP_COUNT][32];

static void group_cb(void* context, uint32_t index) {
    FlipBoxApp* app = context;
    app->group_idx = (uint8_t)index;
    scene_manager_next_scene(app->scene_manager, FlipBoxSceneMaterialSelect);
}

void flipbox_scene_material_group_on_enter(void* context) {
    FlipBoxApp* app = context;
    submenu_reset(app->submenu);
    submenu_set_header(app->submenu, "Material");
    for(uint8_t i = 0; i < QIDI_GROUP_COUNT; i++) {
        snprintf(group_labels[i], sizeof(group_labels[i]), "%s ...", qidi_material_groups[i].name);
        submenu_add_item(app->submenu, group_labels[i], i, group_cb, app);
    }
    submenu_set_selected_item(app->submenu, app->group_idx);
    view_dispatcher_switch_to_view(app->view_dispatcher, FlipBoxViewSubmenu);
}

bool flipbox_scene_material_group_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    return false;
}

void flipbox_scene_material_group_on_exit(void* context) {
    FlipBoxApp* app = context;
    submenu_reset(app->submenu);
}
