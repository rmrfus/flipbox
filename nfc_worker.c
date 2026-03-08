#include "nfc_worker.h"

#include <furi.h>
#include <lib/nfc/nfc.h>
#include <lib/nfc/protocols/mf_classic/mf_classic.h>
#include <lib/nfc/protocols/mf_classic/mf_classic_poller_sync.h>

#define TAG         "FlipBox_NFC"
#define STACK_SIZE  (2 * 1024)
#define POLL_MS     150

typedef enum {
    WorkerStateIdle,
    WorkerStateRead,
    WorkerStateWrite,
    WorkerStateStop,
} WorkerState;

struct NfcWorker {
    FuriThread*       thread;
    Nfc*              nfc;
    volatile WorkerState state;
    NfcWorkerCallback cb;
    void*             cb_ctx;
    QidiTagData       write_data;
};

// Try reading with two keys (primary then default)
static bool do_read(Nfc* nfc, QidiTagData* out) {
    MfClassicBlock block = {0};
    MfClassicKey key = {0};
    MfClassicError err;

    // 1. Try primary key D3:F7:D3:F7:D3:F7
    memcpy(key.data, QIDI_KEY_PRIMARY, 6);
    err = mf_classic_poller_sync_read_block(nfc, QIDI_ABSOLUTE_BLOCK, &key, MfClassicKeyTypeA, &block);
    if(err != MfClassicErrorNone) {
        // 2. Fallback to default key FF:FF:FF:FF:FF:FF
        memcpy(key.data, QIDI_KEY_DEFAULT, 6);
        err = mf_classic_poller_sync_read_block(nfc, QIDI_ABSOLUTE_BLOCK, &key, MfClassicKeyTypeA, &block);
    }

    if(err == MfClassicErrorNone) {
        out->material_code     = block.data[0];
        out->color_code        = block.data[1];
        out->manufacturer_code = block.data[2];
        FURI_LOG_I(TAG, "Read OK: mat=0x%02X col=0x%02X mfr=0x%02X",
            out->material_code, out->color_code, out->manufacturer_code);
        return true;
    }

    FURI_LOG_E(TAG, "Read failed: err=%d", err);
    return false;
}

static bool do_write(Nfc* nfc, const QidiTagData* data) {
    MfClassicBlock block = {0};
    block.data[0] = data->material_code;
    block.data[1] = data->color_code;
    block.data[2] = data->manufacturer_code;

    MfClassicKey key = {0};
    MfClassicError err;

    // 1. Try primary key
    memcpy(key.data, QIDI_KEY_PRIMARY, 6);
    err = mf_classic_poller_sync_write_block(nfc, QIDI_ABSOLUTE_BLOCK, &key, MfClassicKeyTypeA, &block);
    if(err != MfClassicErrorNone) {
        // 2. Try default key
        memcpy(key.data, QIDI_KEY_DEFAULT, 6);
        err = mf_classic_poller_sync_write_block(nfc, QIDI_ABSOLUTE_BLOCK, &key, MfClassicKeyTypeA, &block);
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

    while(w->state != WorkerStateStop) {
        WorkerState cur = w->state;

        if(cur == WorkerStateRead) {
            QidiTagData result = {0};
            bool ok = do_read(w->nfc, &result);
            if(w->state != WorkerStateStop) {
                NfcWorkerResult ev = {
                    .event    = ok ? NfcWorkerEventReadSuccess : NfcWorkerEventReadFail,
                    .tag_data = result,
                };
                w->state = WorkerStateIdle;
                if(w->cb) w->cb(ev, w->cb_ctx);
            }
        } else if(cur == WorkerStateWrite) {
            bool ok = do_write(w->nfc, &w->write_data);
            if(w->state != WorkerStateStop) {
                NfcWorkerResult ev = {
                    .event = ok ? NfcWorkerEventWriteSuccess : NfcWorkerEventWriteFail,
                };
                w->state = WorkerStateIdle;
                if(w->cb) w->cb(ev, w->cb_ctx);
            }
        } else {
            furi_delay_ms(POLL_MS);
        }
    }

    return 0;
}

NfcWorker* nfc_worker_alloc(void) {
    NfcWorker* w = malloc(sizeof(NfcWorker));
    w->nfc   = nfc_alloc();
    w->state = WorkerStateIdle;
    w->cb    = NULL;
    w->cb_ctx = NULL;
    w->thread = furi_thread_alloc_ex("FlipBoxNfc", STACK_SIZE, worker_thread, w);
    return w;
}

void nfc_worker_free(NfcWorker* w) {
    furi_assert(w);
    furi_thread_free(w->thread);
    nfc_free(w->nfc);
    free(w);
}

void nfc_worker_start(NfcWorker* w, NfcWorkerCallback cb, void* context) {
    furi_assert(w);
    w->cb     = cb;
    w->cb_ctx = context;
    w->state  = WorkerStateIdle;
    furi_thread_start(w->thread);
}

void nfc_worker_stop(NfcWorker* w) {
    furi_assert(w);
    w->state = WorkerStateStop;
    furi_thread_join(w->thread);
}

void nfc_worker_read(NfcWorker* w) {
    furi_assert(w);
    w->state = WorkerStateRead;
}

void nfc_worker_write(NfcWorker* w, const QidiTagData* data) {
    furi_assert(w);
    memcpy(&w->write_data, data, sizeof(QidiTagData));
    w->state = WorkerStateWrite;
}
