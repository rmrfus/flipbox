#include "nfc_worker.h"

#include <furi.h>
#include <lib/nfc/nfc.h>
#include <lib/nfc/protocols/mf_classic/mf_classic.h>
#include <lib/nfc/protocols/mf_classic/mf_classic_poller_sync.h>
#include <lib/nfc/protocols/iso14443_3a/iso14443_3a_poller_sync.h>

#define TAG        "FlipBox_NFC"
#define STACK_SIZE (2 * 1024)
#define POLL_MS    150

typedef enum {
    WorkerCmdIdle,
    WorkerCmdRead,
    WorkerCmdWrite,
} WorkerCmd;

struct NfcWorker {
    FuriThread*       thread;
    Nfc*              nfc;
    NfcWorkerCallback cb;
    void*             cb_ctx;

    // Shared with the GUI thread, guarded by mutex
    FuriMutex*  mutex;
    WorkerCmd   cmd;        // pending request; the worker takes it and resets to Idle
    uint32_t    generation; // bumped by every request, cancel and stop
    QidiTagData write_data;
    bool        stop;
};

static void worker_lock(NfcWorker* w) {
    furi_check(furi_mutex_acquire(w->mutex, FuriWaitForever) == FuriStatusOk);
}

static void worker_unlock(NfcWorker* w) {
    furi_check(furi_mutex_release(w->mutex) == FuriStatusOk);
}

// False once the request was cancelled, superseded or the worker is stopping
static bool worker_is_current(NfcWorker* w, uint32_t generation) {
    worker_lock(w);
    bool current = !w->stop && w->generation == generation;
    worker_unlock(w);
    return current;
}

static bool do_read(NfcWorker* w, uint32_t generation, NfcWorkerResult* out) {
    // Activation alone gives the UID, and without a tag it fails before any
    // login attempt
    Iso14443_3aData iso = {0};
    Iso14443_3aError iso_err = iso14443_3a_poller_sync_read(w->nfc, &iso);
    if(iso_err != Iso14443_3aErrorNone || !worker_is_current(w, generation)) {
        FURI_LOG_D(TAG, "No tag: err=%d", iso_err);
        return false;
    }
    out->uid.len = iso.uid_len;
    memcpy(out->uid.data, iso.uid, iso.uid_len);

    MfClassicBlock block = {0};
    MfClassicKey key = {0};
    MfClassicError err;

    memcpy(key.data, QIDI_KEY_PRIMARY, 6);
    err = mf_classic_poller_sync_read_block(w->nfc, QIDI_ABSOLUTE_BLOCK, &key, MfClassicKeyTypeA, &block);
    if(err != MfClassicErrorNone && worker_is_current(w, generation)) {
        memcpy(key.data, QIDI_KEY_DEFAULT, 6);
        err = mf_classic_poller_sync_read_block(w->nfc, QIDI_ABSOLUTE_BLOCK, &key, MfClassicKeyTypeA, &block);
    }

    if(err == MfClassicErrorNone) {
        out->tag_data.material_code     = block.data[0];
        out->tag_data.color_code        = block.data[1];
        out->tag_data.manufacturer_code = block.data[2];
        FURI_LOG_I(TAG, "Read OK: mat=0x%02X col=0x%02X mfr=0x%02X",
            block.data[0], block.data[1], block.data[2]);
        return true;
    }

    FURI_LOG_E(TAG, "Read failed: err=%d", err);
    return false;
}

static bool do_write(NfcWorker* w, uint32_t generation, const QidiTagData* data) {
    MfClassicBlock block = {0};
    block.data[0] = data->material_code;
    block.data[1] = data->color_code;
    block.data[2] = data->manufacturer_code;

    MfClassicKey key = {0};
    MfClassicError err;

    memcpy(key.data, QIDI_KEY_PRIMARY, 6);
    err = mf_classic_poller_sync_write_block(w->nfc, QIDI_ABSOLUTE_BLOCK, &key, MfClassicKeyTypeA, &block);
    // Don't start a second write attempt for a request the user already left
    if(err != MfClassicErrorNone && worker_is_current(w, generation)) {
        memcpy(key.data, QIDI_KEY_DEFAULT, 6);
        err = mf_classic_poller_sync_write_block(w->nfc, QIDI_ABSOLUTE_BLOCK, &key, MfClassicKeyTypeA, &block);
    }

    if(err == MfClassicErrorNone) {
        FURI_LOG_I(TAG, "Write OK: mat=0x%02X col=0x%02X mfr=0x%02X",
            data->material_code, data->color_code, data->manufacturer_code);
        return true;
    }

    FURI_LOG_E(TAG, "Write failed: err=%d", err);
    return false;
}

static int32_t worker_thread(void* arg) {
    NfcWorker* w = arg;

    for(;;) {
        // Take the pending request atomically
        worker_lock(w);
        bool        stop       = w->stop;
        WorkerCmd   cmd        = w->cmd;
        uint32_t    generation = w->generation;
        QidiTagData data       = w->write_data;
        w->cmd = WorkerCmdIdle;
        worker_unlock(w);

        if(stop) break;
        if(cmd == WorkerCmdIdle) {
            furi_delay_ms(POLL_MS);
            continue;
        }

        NfcWorkerResult ev = {0};
        if(cmd == WorkerCmdRead) {
            bool ok  = do_read(w, generation, &ev);
            ev.event = ok ? NfcWorkerEventReadSuccess : NfcWorkerEventReadFail;
        } else {
            bool ok  = do_write(w, generation, &data);
            ev.event = ok ? NfcWorkerEventWriteSuccess : NfcWorkerEventWriteFail;
        }

        // Results of cancelled or superseded requests are dropped
        if(worker_is_current(w, generation)) w->cb(ev, w->cb_ctx);
    }

    return 0;
}

static void worker_request(NfcWorker* w, WorkerCmd cmd, const QidiTagData* data) {
    worker_lock(w);
    if(data) w->write_data = *data;
    w->cmd = cmd;
    w->generation++;
    worker_unlock(w);
}

NfcWorker* nfc_worker_alloc(void) {
    NfcWorker* w = malloc(sizeof(NfcWorker));
    memset(w, 0, sizeof(NfcWorker));
    w->nfc    = nfc_alloc();
    w->mutex  = furi_mutex_alloc(FuriMutexTypeNormal);
    w->cmd    = WorkerCmdIdle;
    w->thread = furi_thread_alloc_ex("FlipBoxNfc", STACK_SIZE, worker_thread, w);
    return w;
}

void nfc_worker_free(NfcWorker* w) {
    furi_assert(w);
    furi_assert(w->stop); // nfc_worker_stop() must be called first
    furi_thread_free(w->thread);
    furi_mutex_free(w->mutex);
    nfc_free(w->nfc);
    free(w);
}

void nfc_worker_start(NfcWorker* w, NfcWorkerCallback cb, void* context) {
    furi_assert(w);
    furi_assert(cb);
    w->cb     = cb;
    w->cb_ctx = context;
    furi_thread_start(w->thread);
}

void nfc_worker_stop(NfcWorker* w) {
    furi_assert(w);
    worker_lock(w);
    w->stop = true;
    w->generation++;
    worker_unlock(w);
    furi_thread_join(w->thread);
}

void nfc_worker_read(NfcWorker* w) {
    furi_assert(w);
    worker_request(w, WorkerCmdRead, NULL);
}

void nfc_worker_write(NfcWorker* w, const QidiTagData* data) {
    furi_assert(w);
    worker_request(w, WorkerCmdWrite, data);
}

void nfc_worker_cancel(NfcWorker* w) {
    furi_assert(w);
    worker_request(w, WorkerCmdIdle, NULL);
}
