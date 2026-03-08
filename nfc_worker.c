#include "nfc_worker.h"

#include <furi.h>
#include <lib/nfc/nfc.h>
#include <lib/nfc/nfc_poller.h>
#include <lib/nfc/protocols/mf_classic/mf_classic.h>
#include <lib/nfc/protocols/mf_classic/mf_classic_poller.h>

#define TAG "FlipBox_NFC"

typedef enum {
    WorkerOpRead,
    WorkerOpWrite,
} WorkerOp;

struct NfcWorker {
    Nfc*              nfc;
    NfcPoller*        poller;
    NfcWorkerCallback cb;
    void*             cb_ctx;
    WorkerOp          op;
    QidiTagData       write_data;
};

// Runs in NFC stack context. On RequestMode we do auth+read/write ourselves,
// then stop the poller. No FuriThread needed.
static NfcCommand worker_poller_cb(NfcGenericEvent event, void* context) {
    NfcWorker* w = context;

    if(event.protocol != NfcProtocolMfClassic) {
        return NfcCommandStop;
    }

    MfClassicPollerEvent* mfc_event = event.event_data;

    // Only act at RequestMode — card is detected and activated, crypto not yet started.
    // Auth + read/write right here, then stop the poller.
    if(mfc_event->type != MfClassicPollerEventTypeRequestMode) {
        return NfcCommandContinue;
    }

    MfClassicPoller* mfc = event.instance;
    MfClassicKey key = {0};
    MfClassicAuthContext auth_ctx = {0};
    MfClassicError err;
    NfcWorkerResult result = {0};

    // Try primary key D3:F7:D3:F7:D3:F7, then factory default FF:FF:FF:FF:FF:FF
    memcpy(key.data, QIDI_KEY_PRIMARY, 6);
    err = mf_classic_poller_auth(mfc, QIDI_ABSOLUTE_BLOCK, &key, MfClassicKeyTypeA, &auth_ctx, false);
    if(err != MfClassicErrorNone) {
        memcpy(key.data, QIDI_KEY_DEFAULT, 6);
        err = mf_classic_poller_auth(mfc, QIDI_ABSOLUTE_BLOCK, &key, MfClassicKeyTypeA, &auth_ctx, false);
    }

    if(err != MfClassicErrorNone) {
        FURI_LOG_E(TAG, "Auth failed: %d", err);
        result.event = (w->op == WorkerOpRead) ? NfcWorkerEventReadFail : NfcWorkerEventWriteFail;
        goto done;
    }

    if(w->op == WorkerOpRead) {
        MfClassicBlock block = {0};
        err = mf_classic_poller_read_block(mfc, QIDI_ABSOLUTE_BLOCK, &block);
        if(err == MfClassicErrorNone) {
            result.event                      = NfcWorkerEventReadSuccess;
            result.tag_data.material_code     = block.data[0];
            result.tag_data.color_code        = block.data[1];
            result.tag_data.manufacturer_code = block.data[2];
            FURI_LOG_I(TAG, "Read OK: mat=0x%02X col=0x%02X mfr=0x%02X",
                result.tag_data.material_code,
                result.tag_data.color_code,
                result.tag_data.manufacturer_code);
        } else {
            FURI_LOG_E(TAG, "Read block failed: %d", err);
            result.event = NfcWorkerEventReadFail;
        }
    } else {
        MfClassicBlock block = {0};
        block.data[0] = w->write_data.material_code;
        block.data[1] = w->write_data.color_code;
        block.data[2] = w->write_data.manufacturer_code;
        err = mf_classic_poller_write_block(mfc, QIDI_ABSOLUTE_BLOCK, &block);
        if(err == MfClassicErrorNone) {
            FURI_LOG_I(TAG, "Write OK");
            result.event = NfcWorkerEventWriteSuccess;
        } else {
            FURI_LOG_E(TAG, "Write block failed: %d", err);
            result.event = NfcWorkerEventWriteFail;
        }
    }

done:
    if(w->cb) w->cb(result, w->cb_ctx);
    return NfcCommandStop;
}

static void worker_stop_poller(NfcWorker* w) {
    if(w->poller) {
        nfc_poller_stop(w->poller);
        nfc_poller_free(w->poller);
        w->poller = NULL;
    }
}

NfcWorker* nfc_worker_alloc(void) {
    NfcWorker* w = malloc(sizeof(NfcWorker));
    memset(w, 0, sizeof(NfcWorker));
    w->nfc = nfc_alloc();
    return w;
}

void nfc_worker_free(NfcWorker* w) {
    furi_assert(w);
    worker_stop_poller(w);
    nfc_free(w->nfc);
    free(w);
}

void nfc_worker_start(NfcWorker* w, NfcWorkerCallback cb, void* context) {
    furi_assert(w);
    w->cb     = cb;
    w->cb_ctx = context;
}

void nfc_worker_stop(NfcWorker* w) {
    furi_assert(w);
    worker_stop_poller(w);
}

void nfc_worker_read(NfcWorker* w) {
    furi_assert(w);
    worker_stop_poller(w);
    w->op     = WorkerOpRead;
    w->poller = nfc_poller_alloc(w->nfc, NfcProtocolMfClassic);
    nfc_poller_start(w->poller, worker_poller_cb, w);
}

void nfc_worker_write(NfcWorker* w, const QidiTagData* data) {
    furi_assert(w);
    memcpy(&w->write_data, data, sizeof(QidiTagData));
    worker_stop_poller(w);
    w->op     = WorkerOpWrite;
    w->poller = nfc_poller_alloc(w->nfc, NfcProtocolMfClassic);
    nfc_poller_start(w->poller, worker_poller_cb, w);
}
