#include "../flipbox.h"
#include <stdio.h>

typedef enum {
    CfgItemMaterial,
    CfgItemColor,
    CfgItemManufacturer,
    CfgItemWriteNow,
} CfgItem;

// Called on left/right for Manufacturer
static void mfr_change_cb(VariableItem* item) {
    FlipBoxApp* app = variable_item_get_context(item);
    app->mfr_idx = variable_item_get_current_value_index(item);
    variable_item_set_current_value_text(item, qidi_manufacturers[app->mfr_idx].name);
}

// Called on OK press for any row
static void cfg_enter_cb(void* context, uint32_t index) {
    FlipBoxApp* app = context;
    if(index == CfgItemMaterial) {
        scene_manager_next_scene(app->scene_manager, FlipBoxSceneMaterialGroup);
    } else if(index == CfgItemColor) {
        scene_manager_next_scene(app->scene_manager, FlipBoxSceneColorSelect);
    } else if(index == CfgItemWriteNow) {
        scene_manager_next_scene(app->scene_manager, FlipBoxSceneWrite);
    }
}

void flipbox_scene_write_cfg_on_enter(void* context) {
    FlipBoxApp* app = context;
    VariableItemList* list = app->var_item_list;
    variable_item_list_reset(list);

    VariableItem* item;

    // Material — OK opens submenu, append ">" to signal that
    snprintf(app->text_buf, sizeof(app->text_buf), "%s ...", qidi_materials[app->mat_idx].name);
    item = variable_item_list_add(list, "Mat", 0, NULL, NULL);
    variable_item_set_current_value_text(item, app->text_buf);

    // Color — same, separate buffer so Material text isn't overwritten
    snprintf(app->text_buf2, sizeof(app->text_buf2), "%s ...", qidi_colors[app->col_idx].name);
    item = variable_item_list_add(list, "Color", 0, NULL, NULL);
    variable_item_set_current_value_text(item, app->text_buf2);

    // Manufacturer — left/right cycles, no submenu
    item = variable_item_list_add(list, "Maker", QIDI_MFR_COUNT, mfr_change_cb, app);
    variable_item_set_current_value_index(item, app->mfr_idx);
    variable_item_set_current_value_text(item, qidi_manufacturers[app->mfr_idx].name);

    // Write button row
    item = variable_item_list_add(list, "Write", 1, NULL, NULL);
    variable_item_set_current_value_text(item, ">");

    variable_item_list_set_enter_callback(list, cfg_enter_cb, app);

    view_dispatcher_switch_to_view(app->view_dispatcher, FlipBoxViewVariableItemList);
}

bool flipbox_scene_write_cfg_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    return false;
}

void flipbox_scene_write_cfg_on_exit(void* context) {
    FlipBoxApp* app = context;
    variable_item_list_reset(app->var_item_list);
}
