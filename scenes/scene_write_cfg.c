#include "../flipbox.h"

typedef enum {
    CfgItemMaterial,
    CfgItemColor,
    CfgItemManufacturer,
    CfgItemWriteNow,
} CfgItem;

static void mat_change_cb(VariableItem* item) {
    FlipBoxApp* app = variable_item_get_context(item);
    app->mat_idx = variable_item_get_current_value_index(item);
    variable_item_set_current_value_text(item, qidi_materials[app->mat_idx].name);
}

static void col_change_cb(VariableItem* item) {
    FlipBoxApp* app = variable_item_get_context(item);
    app->col_idx = variable_item_get_current_value_index(item);
    variable_item_set_current_value_text(item, qidi_colors[app->col_idx].name);
}

static void mfr_change_cb(VariableItem* item) {
    FlipBoxApp* app = variable_item_get_context(item);
    app->mfr_idx = variable_item_get_current_value_index(item);
    variable_item_set_current_value_text(item, qidi_manufacturers[app->mfr_idx].name);
}

static void cfg_selected_cb(void* context, uint32_t index) {
    FlipBoxApp* app = context;
    if(index == CfgItemWriteNow) {
        scene_manager_next_scene(app->scene_manager, FlipBoxSceneWrite);
    }
}

void flipbox_scene_write_cfg_on_enter(void* context) {
    FlipBoxApp* app = context;
    VariableItemList* list = app->var_item_list;
    variable_item_list_reset(list);

    VariableItem* item;

    // Material
    item = variable_item_list_add(list, "Material", QIDI_MATERIAL_COUNT, mat_change_cb, app);
    variable_item_set_current_value_index(item, app->mat_idx);
    variable_item_set_current_value_text(item, qidi_materials[app->mat_idx].name);

    // Color
    item = variable_item_list_add(list, "Color", QIDI_COLOR_COUNT, col_change_cb, app);
    variable_item_set_current_value_index(item, app->col_idx);
    variable_item_set_current_value_text(item, qidi_colors[app->col_idx].name);

    // Manufacturer
    item = variable_item_list_add(list, "Maker", QIDI_MFR_COUNT, mfr_change_cb, app);
    variable_item_set_current_value_index(item, app->mfr_idx);
    variable_item_set_current_value_text(item, qidi_manufacturers[app->mfr_idx].name);

    // Write button row
    item = variable_item_list_add(list, "Write Tag", 1, NULL, NULL);
    variable_item_set_current_value_text(item, ">");

    variable_item_list_set_enter_callback(list, cfg_selected_cb, app);

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
