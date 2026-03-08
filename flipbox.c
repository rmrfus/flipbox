#include "flipbox.h"

// Single worker callback — routes NFC events to view_dispatcher as custom events
static void flipbox_worker_cb(NfcWorkerResult result, void* context) {
    FlipBoxApp* app = context;

    FlipBoxCustomEvent ev;
    switch(result.event) {
    case NfcWorkerEventReadSuccess:
        app->last_read       = result.tag_data;
        app->last_read_valid = true;
        ev = FlipBoxCustomEventReadSuccess;
        break;
    case NfcWorkerEventReadFail:
        ev = FlipBoxCustomEventReadFail;
        break;
    case NfcWorkerEventWriteSuccess:
        ev = FlipBoxCustomEventWriteSuccess;
        break;
    default:
        ev = FlipBoxCustomEventWriteFail;
        break;
    }

    view_dispatcher_send_custom_event(app->view_dispatcher, ev);
}

static bool flipbox_custom_event_cb(void* context, uint32_t event) {
    FlipBoxApp* app = context;
    return scene_manager_handle_custom_event(app->scene_manager, event);
}

static bool flipbox_back_event_cb(void* context) {
    FlipBoxApp* app = context;
    return scene_manager_handle_back_event(app->scene_manager);
}

static FlipBoxApp* flipbox_app_alloc(void) {
    FlipBoxApp* app = malloc(sizeof(FlipBoxApp));
    memset(app, 0, sizeof(FlipBoxApp));

    // Default config: PLA / White / QIDI
    app->mat_idx = 0;
    app->col_idx = 0;
    app->mfr_idx = 0;

    // GUI
    app->gui = furi_record_open(RECORD_GUI);
    app->notification = furi_record_open(RECORD_NOTIFICATION);

    app->view_dispatcher = view_dispatcher_alloc();
    view_dispatcher_set_event_callback_context(app->view_dispatcher, app);
    view_dispatcher_set_custom_event_callback(app->view_dispatcher, flipbox_custom_event_cb);
    view_dispatcher_set_navigation_event_callback(app->view_dispatcher, flipbox_back_event_cb);

    app->scene_manager = scene_manager_alloc(&flipbox_scene_handlers, app);

    // Views
    app->submenu = submenu_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, FlipBoxViewSubmenu, submenu_get_view(app->submenu));

    app->popup = popup_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, FlipBoxViewPopup, popup_get_view(app->popup));

    app->var_item_list = variable_item_list_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher,
        FlipBoxViewVariableItemList,
        variable_item_list_get_view(app->var_item_list));

    app->widget = widget_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, FlipBoxViewWidget, widget_get_view(app->widget));

    // NFC worker
    app->worker = nfc_worker_alloc();
    nfc_worker_start(app->worker, flipbox_worker_cb, app);

    return app;
}

static void flipbox_app_free(FlipBoxApp* app) {
    furi_assert(app);

    nfc_worker_stop(app->worker);
    nfc_worker_free(app->worker);

    view_dispatcher_remove_view(app->view_dispatcher, FlipBoxViewSubmenu);
    submenu_free(app->submenu);

    view_dispatcher_remove_view(app->view_dispatcher, FlipBoxViewPopup);
    popup_free(app->popup);

    view_dispatcher_remove_view(app->view_dispatcher, FlipBoxViewVariableItemList);
    variable_item_list_free(app->var_item_list);

    view_dispatcher_remove_view(app->view_dispatcher, FlipBoxViewWidget);
    widget_free(app->widget);

    scene_manager_free(app->scene_manager);
    view_dispatcher_free(app->view_dispatcher);

    furi_record_close(RECORD_NOTIFICATION);
    furi_record_close(RECORD_GUI);

    free(app);
}

int32_t flipbox_app(void* p) {
    UNUSED(p);
    FlipBoxApp* app = flipbox_app_alloc();

    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);
    scene_manager_next_scene(app->scene_manager, FlipBoxSceneMenu);
    view_dispatcher_run(app->view_dispatcher);

    flipbox_app_free(app);
    return 0;
}
