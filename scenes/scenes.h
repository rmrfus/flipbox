#pragma once

#include <gui/scene_manager.h>

typedef enum {
    FlipBoxSceneMenu,
    FlipBoxSceneRead,
    FlipBoxSceneWriteCfg,
    FlipBoxSceneWrite,
    FlipBoxSceneCount,
} FlipBoxScene;

extern const SceneManagerHandlers flipbox_scene_handlers;
