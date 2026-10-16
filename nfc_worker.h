#pragma once

#include "qidi_tag.h"
#include <lib/nfc/nfc.h>
#include <lib/nfc/protocols/iso14443_3a/iso14443_3a.h>

typedef enum {
    NfcWorkerEventReadSuccess,
    NfcWorkerEventReadFail,
    NfcWorkerEventWriteSuccess,
    NfcWorkerEventWriteFail,
} NfcWorkerEvent;

typedef struct {
    uint8_t data[ISO14443_3A_MAX_UID_SIZE];
    uint8_t len;
} NfcTagUid;

typedef struct {
    NfcWorkerEvent event;
    QidiTagData    tag_data; // valid on ReadSuccess
    NfcTagUid      uid;      // valid on ReadSuccess
} NfcWorkerResult;

typedef void (*NfcWorkerCallback)(NfcWorkerResult result, void* context);

typedef struct NfcWorker NfcWorker;

NfcWorker* nfc_worker_alloc(void);
void       nfc_worker_free(NfcWorker* worker);

void nfc_worker_start(NfcWorker* worker, NfcWorkerCallback cb, void* context);
void nfc_worker_stop(NfcWorker* worker);

// A new request replaces any pending one; the result of a replaced or
// cancelled request is never delivered
void nfc_worker_read(NfcWorker* worker);
void nfc_worker_write(NfcWorker* worker, const QidiTagData* tag_data);
// Drop the pending request; an operation already on the air finishes, but its
// result is discarded and a write skips its fallback-key attempt
void nfc_worker_cancel(NfcWorker* worker);
