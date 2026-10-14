#include "../flipbox.h"
#include <stdio.h>

// Labels survive until on_exit clears the submenu
static char group_labels[QidiGroupCount][32];

static void group_cb(void* context, uint32_t index) {
    FlipBoxApp* app = context;
    app->group_idx = (uint8_t)index;
    scene_manager_next_scene(app->scene_manager, FlipBoxSceneMaterialSelect);
}

static bool group_on_printer(QidiGroup group, QidiPrinter printer) {
    for(uint8_t i = 0; i < QIDI_MATERIAL_COUNT; i++) {
        if(qidi_materials[i].group == group && qidi_material_on_printer(&qidi_materials[i], printer))
            return true;
    }
    return false;
}

void flipbox_scene_material_group_on_enter(void* context) {
    FlipBoxApp* app = context;
    submenu_reset(app->submenu);
    submenu_set_header(app->submenu, "Material");
    // Item index is the QidiGroup, so hidden groups don't shift the selection
    for(uint8_t g = 0; g < QidiGroupCount; g++) {
        if(!group_on_printer(g, app->printer)) continue;
        snprintf(group_labels[g], sizeof(group_labels[g]), "%s ...", qidi_group_names[g]);
        submenu_add_item(app->submenu, group_labels[g], g, group_cb, app);
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
