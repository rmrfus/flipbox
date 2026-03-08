#pragma once

#include <furi.h>
#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/scene_manager.h>
#include <gui/modules/submenu.h>
#include <gui/modules/popup.h>
#include <gui/modules/variable_item_list.h>
#include <notification/notification.h>
#include <notification/notification_messages.h>

#include "qidi_tag.h"
#include "nfc_worker.h"
#include "scenes/scenes.h"

typedef enum {
    FlipBoxViewSubmenu,
    FlipBoxViewPopup,
    FlipBoxViewVariableItemList,
} FlipBoxView;

typedef enum {
    FlipBoxCustomEventReadSuccess,
    FlipBoxCustomEventReadFail,
    FlipBoxCustomEventWriteSuccess,
    FlipBoxCustomEventWriteFail,
} FlipBoxCustomEvent;

typedef struct {
    Gui*              gui;
    ViewDispatcher*   view_dispatcher;
    SceneManager*     scene_manager;
    NotificationApp*  notification;

    Submenu*          submenu;
    Popup*            popup;
    VariableItemList* var_item_list;

    NfcWorker*        worker;

    // write config state
    uint8_t mat_idx;   // index into qidi_materials[]
    uint8_t col_idx;   // index into qidi_colors[]
    uint8_t mfr_idx;   // index into qidi_manufacturers[]
    uint8_t group_idx; // index into qidi_material_groups[]

    // last read result (for display in popup)
    QidiTagData last_read;
    bool        last_read_valid;

    char text_buf[128];
    char text_buf2[128];
} FlipBoxApp;
