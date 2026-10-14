#pragma once

#include <stdbool.h>
#include <stdint.h>

// MIFARE Classic 1K layout for QIDI Box tags
// Data lives in Sector 1, Block 0 (absolute block 4)
// 3 bytes: material_code | color_code | manufacturer_code
#define QIDI_ABSOLUTE_BLOCK 4

// Auth keys to try in order (primary then fallback)
extern const uint8_t QIDI_KEY_PRIMARY[6]; // D3:F7:D3:F7:D3:F7
extern const uint8_t QIDI_KEY_DEFAULT[6]; // FF:FF:FF:FF:FF:FF

typedef struct {
    uint8_t material_code;
    uint8_t color_code;
    uint8_t manufacturer_code;
} QidiTagData;

// The tag stores only codes; each printer maps them to names through its own
// officiall_filas_list.cfg, and those tables differ between models.
// Append-only: the ordinal is persisted in settings.bin.
typedef enum {
    QidiPrinterQ2,
    QidiPrinterMax4,
    QidiPrinterPlus4,
    QidiPrinterCount,
} QidiPrinter;

#define QIDI_PRINTER_BIT(p) (1u << (p))

typedef enum {
    QidiGroupPla,
    QidiGroupAbs,
    QidiGroupAsa,
    QidiGroupPa,
    QidiGroupPet,
    QidiGroupSupport,
    QidiGroupFlex,
    QidiGroupOther,
    QidiGroupCount,
} QidiGroup;

typedef struct {
    uint8_t     code;
    uint8_t     group;    // QidiGroup
    uint8_t     printers; // QIDI_PRINTER_BIT mask
    const char* name;
} QidiMaterialInfo;

typedef struct {
    uint8_t  code;
    const char* name;
    uint32_t rgb;
} QidiColorInfo;

typedef struct {
    uint8_t  code;
    const char* name;
} QidiManufacturerInfo;

extern const QidiMaterialInfo     qidi_materials[];
extern const QidiColorInfo        qidi_colors[];
extern const QidiManufacturerInfo qidi_manufacturers[];
extern const char* const          qidi_printer_names[QidiPrinterCount];
extern const char* const          qidi_group_names[QidiGroupCount];

#define QIDI_MATERIAL_COUNT    48
#define QIDI_COLOR_COUNT       24
#define QIDI_MFR_COUNT         2

static inline bool qidi_material_on_printer(const QidiMaterialInfo* m, QidiPrinter printer) {
    return m->printers & QIDI_PRINTER_BIT(printer);
}

// Lookup helpers — return array index, or -1 if not found
int qidi_material_idx_by_code(QidiPrinter printer, uint8_t code);
int qidi_color_idx_by_code(uint8_t code);
int qidi_mfr_idx_by_code(uint8_t code);

// First material available on the printer (always exists)
uint8_t qidi_material_first_idx(QidiPrinter printer);

// Return NULL for codes the table does not know
const char* qidi_material_name(QidiPrinter printer, uint8_t code);
const char* qidi_color_name(uint8_t code);
const char* qidi_mfr_name(uint8_t code);
