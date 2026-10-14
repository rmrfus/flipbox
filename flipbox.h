#pragma once

#include <furi.h>
#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/scene_manager.h>
#include <gui/modules/submenu.h>
#include <gui/modules/popup.h>
#include <gui/modules/variable_item_list.h>
#include <gui/modules/widget.h>
#include <notification/notification.h>
#include <notification/notification_messages.h>

#include "qidi_tag.h"
#include "nfc_worker.h"
#include "scenes/scenes.h"

typedef enum {
    FlipBoxViewSubmenu,
    FlipBoxViewPopup,
    FlipBoxViewVariableItemList,
    FlipBoxViewWidget,
} FlipBoxView;

typedef enum {
    FlipBoxCustomEventReadSuccess,
    FlipBoxCustomEventReadFail,
    FlipBoxCustomEventReadRetry,
    FlipBoxCustomEventReadEditTag,
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
    Widget*           widget;

    NfcWorker*        worker;

    // Selects the code table for both read and write; saved on exit
    QidiPrinter printer;

    // write config state
    uint8_t mat_idx;   // index into qidi_materials[], always available on printer
    uint8_t col_idx;   // index into qidi_colors[]
    uint8_t mfr_idx;   // index into qidi_manufacturers[]
    uint8_t group_idx; // QidiGroup being browsed in material picker

    // last read result (for display in popup)
    QidiTagData last_read;
    bool        last_read_valid;

    char text_buf[128];
} FlipBoxApp;
