#include "scenes.h"

// Forward declarations for all scene handlers
void flipbox_scene_menu_on_enter(void*);
bool flipbox_scene_menu_on_event(void*, SceneManagerEvent);
void flipbox_scene_menu_on_exit(void*);

void flipbox_scene_read_on_enter(void*);
bool flipbox_scene_read_on_event(void*, SceneManagerEvent);
void flipbox_scene_read_on_exit(void*);

void flipbox_scene_write_cfg_on_enter(void*);
bool flipbox_scene_write_cfg_on_event(void*, SceneManagerEvent);
void flipbox_scene_write_cfg_on_exit(void*);

void flipbox_scene_material_group_on_enter(void*);
bool flipbox_scene_material_group_on_event(void*, SceneManagerEvent);
void flipbox_scene_material_group_on_exit(void*);

void flipbox_scene_material_select_on_enter(void*);
bool flipbox_scene_material_select_on_event(void*, SceneManagerEvent);
void flipbox_scene_material_select_on_exit(void*);

void flipbox_scene_color_select_on_enter(void*);
bool flipbox_scene_color_select_on_event(void*, SceneManagerEvent);
void flipbox_scene_color_select_on_exit(void*);

void flipbox_scene_write_on_enter(void*);
bool flipbox_scene_write_on_event(void*, SceneManagerEvent);
void flipbox_scene_write_on_exit(void*);

// Three separate arrays indexed by FlipBoxScene enum
static const AppSceneOnEnterCallback on_enter_handlers[FlipBoxSceneCount] = {
    [FlipBoxSceneMenu]           = flipbox_scene_menu_on_enter,
    [FlipBoxSceneRead]           = flipbox_scene_read_on_enter,
    [FlipBoxSceneWriteCfg]       = flipbox_scene_write_cfg_on_enter,
    [FlipBoxSceneMaterialGroup]  = flipbox_scene_material_group_on_enter,
    [FlipBoxSceneMaterialSelect] = flipbox_scene_material_select_on_enter,
    [FlipBoxSceneColorSelect]    = flipbox_scene_color_select_on_enter,
    [FlipBoxSceneWrite]          = flipbox_scene_write_on_enter,
};

static const AppSceneOnEventCallback on_event_handlers[FlipBoxSceneCount] = {
    [FlipBoxSceneMenu]           = flipbox_scene_menu_on_event,
    [FlipBoxSceneRead]           = flipbox_scene_read_on_event,
    [FlipBoxSceneWriteCfg]       = flipbox_scene_write_cfg_on_event,
    [FlipBoxSceneMaterialGroup]  = flipbox_scene_material_group_on_event,
    [FlipBoxSceneMaterialSelect] = flipbox_scene_material_select_on_event,
    [FlipBoxSceneColorSelect]    = flipbox_scene_color_select_on_event,
    [FlipBoxSceneWrite]          = flipbox_scene_write_on_event,
};

static const AppSceneOnExitCallback on_exit_handlers[FlipBoxSceneCount] = {
    [FlipBoxSceneMenu]           = flipbox_scene_menu_on_exit,
    [FlipBoxSceneRead]           = flipbox_scene_read_on_exit,
    [FlipBoxSceneWriteCfg]       = flipbox_scene_write_cfg_on_exit,
    [FlipBoxSceneMaterialGroup]  = flipbox_scene_material_group_on_exit,
    [FlipBoxSceneMaterialSelect] = flipbox_scene_material_select_on_exit,
    [FlipBoxSceneColorSelect]    = flipbox_scene_color_select_on_exit,
    [FlipBoxSceneWrite]          = flipbox_scene_write_on_exit,
};

const SceneManagerHandlers flipbox_scene_handlers = {
    .on_enter_handlers = on_enter_handlers,
    .on_event_handlers = on_event_handlers,
    .on_exit_handlers  = on_exit_handlers,
    .scene_num         = FlipBoxSceneCount,
};
