#pragma once

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

typedef struct {
    uint8_t  code;
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

extern const QidiMaterialInfo qidi_materials[];
extern const QidiColorInfo    qidi_colors[];
extern const QidiManufacturerInfo qidi_manufacturers[];

#define QIDI_MATERIAL_COUNT    22
#define QIDI_COLOR_COUNT       24
#define QIDI_MFR_COUNT         2

// Lookup helpers — return array index, or -1 if not found
int qidi_material_idx_by_code(uint8_t code);
int qidi_color_idx_by_code(uint8_t code);
int qidi_mfr_idx_by_code(uint8_t code);

const char* qidi_material_name(uint8_t code);
const char* qidi_color_name(uint8_t code);
const char* qidi_mfr_name(uint8_t code);
