#pragma once

#include "qidi_tag.h"
#include <lib/nfc/nfc.h>

typedef enum {
    NfcWorkerEventReadSuccess,
    NfcWorkerEventReadFail,
    NfcWorkerEventWriteSuccess,
    NfcWorkerEventWriteFail,
} NfcWorkerEvent;

typedef struct {
    NfcWorkerEvent event;
    QidiTagData    tag_data; // valid on ReadSuccess
} NfcWorkerResult;

typedef void (*NfcWorkerCallback)(NfcWorkerResult result, void* context);

typedef struct NfcWorker NfcWorker;

NfcWorker* nfc_worker_alloc(void);
void       nfc_worker_free(NfcWorker* worker);

void nfc_worker_start(NfcWorker* worker, NfcWorkerCallback cb, void* context);
void nfc_worker_stop(NfcWorker* worker);

void nfc_worker_read(NfcWorker* worker);
void nfc_worker_write(NfcWorker* worker, const QidiTagData* tag_data);
