#include "../flipbox.h"
#include <stdio.h>

typedef enum {
    CfgItemPrinter,
    CfgItemMaterial,
    CfgItemColor,
    CfgItemManufacturer,
    CfgItemWriteNow,
} CfgItem;

// Valid between on_enter and on_exit; printer change rewrites its text
static VariableItem* material_item;

static void material_text_update(FlipBoxApp* app) {
    // OK opens submenu, append "..." to signal that
    snprintf(app->text_buf, sizeof(app->text_buf), "%s ...", qidi_materials[app->mat_idx].name);
    variable_item_set_current_value_text(material_item, app->text_buf);
}

// Called on left/right for Printer
static void printer_change_cb(VariableItem* item) {
    FlipBoxApp* app = variable_item_get_context(item);
    app->printer = variable_item_get_current_value_index(item);
    variable_item_set_current_value_text(item, qidi_printer_names[app->printer]);
    // Keep the material when the new printer has the same table row, else reset
    if(!qidi_material_on_printer(&qidi_materials[app->mat_idx], app->printer)) {
        app->mat_idx = qidi_material_first_idx(app->printer);
    }
    material_text_update(app);
}

// Called on left/right for Manufacturer
static void mfr_change_cb(VariableItem* item) {
    FlipBoxApp* app = variable_item_get_context(item);
    app->mfr_idx = variable_item_get_current_value_index(item);
    variable_item_set_current_value_text(item, qidi_manufacturers[app->mfr_idx].name);
}

// Called on OK press for any row
static void cfg_enter_cb(void* context, uint32_t index) {
    FlipBoxApp* app = context;
    // Return to this row when coming back from the scene it opens. Entries
    // from Menu and Read reset the state to the top row
    scene_manager_set_scene_state(app->scene_manager, FlipBoxSceneWriteCfg, index);
    if(index == CfgItemMaterial) {
        // Open the group picker on the current material's group
        app->group_idx = qidi_materials[app->mat_idx].group;
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

    // Printer — left/right cycles, picks the material code table
    item = variable_item_list_add(list, "Printer", QidiPrinterCount, printer_change_cb, app);
    variable_item_set_current_value_index(item, app->printer);
    variable_item_set_current_value_text(item, qidi_printer_names[app->printer]);

    // Material — OK opens group submenu
    material_item = variable_item_list_add(list, "Material", 0, NULL, NULL);
    material_text_update(app);

    // Color — OK opens submenu; value text is copied, so text_buf can be reused
    snprintf(app->text_buf, sizeof(app->text_buf), "%s ...", qidi_colors[app->col_idx].name);
    item = variable_item_list_add(list, "Color", 0, NULL, NULL);
    variable_item_set_current_value_text(item, app->text_buf);

    // Manufacturer — left/right cycles, no submenu
    item = variable_item_list_add(list, "Maker", QIDI_MFR_COUNT, mfr_change_cb, app);
    variable_item_set_current_value_index(item, app->mfr_idx);
    variable_item_set_current_value_text(item, qidi_manufacturers[app->mfr_idx].name);

    // Write button row
    item = variable_item_list_add(list, "Write Tag", 1, NULL, NULL);
    variable_item_set_current_value_text(item, ">");

    variable_item_list_set_enter_callback(list, cfg_enter_cb, app);
    variable_item_list_set_selected_item(
        list, scene_manager_get_scene_state(app->scene_manager, FlipBoxSceneWriteCfg));

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
    material_item = NULL;
}
