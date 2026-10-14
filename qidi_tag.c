#include "qidi_tag.h"
#include <furi.h>

const uint8_t QIDI_KEY_PRIMARY[6] = {0xD3, 0xF7, 0xD3, 0xF7, 0xD3, 0xF7};
const uint8_t QIDI_KEY_DEFAULT[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

const char* const qidi_printer_names[QidiPrinterCount] = {
    [QidiPrinterQ2]    = "Q2",
    [QidiPrinterMax4]  = "Max4",
    [QidiPrinterPlus4] = "Plus4",
};

const char* const qidi_group_names[QidiGroupCount] = {
    [QidiGroupPla]     = "PLA",
    [QidiGroupAbs]     = "ABS",
    [QidiGroupAsa]     = "ASA",
    [QidiGroupPa]      = "PA / Nylon",
    [QidiGroupPet]     = "PET / PETG",
    [QidiGroupSupport] = "Support",
    [QidiGroupFlex]    = "TPU / Flex",
    [QidiGroupOther]   = "Other",
};

#define Q2    QIDI_PRINTER_BIT(QidiPrinterQ2)
#define MAX4  QIDI_PRINTER_BIT(QidiPrinterMax4)
#define PLUS4 QIDI_PRINTER_BIT(QidiPrinterPlus4)
#define ALL   (Q2 | MAX4 | PLUS4)

// Names and codes from officiall_filas_list.cfg in QIDI's firmware repos:
//   Q2    — github.com/QIDITECH/QIDI_Q2    (V1.1.1, 2026-01)
//   Max4  — github.com/QIDITECH/QIDI_MAX4  (2026-08)
//   Plus4 — github.com/QIDITECH/QIDI_PLUS4 (V1.7.1, 2025-08)
// A code may map to a different name per printer (e.g. 25 is PA-CF on Q2/Max4
// but PA12-CF on Plus4), hence one row per (code, name) with a printer mask.
// Plus4's "PETG-Though" (40) and "TPU-AERO" (49) are treated as spelling
// variants of the Q2/Max4 names. Sorted by code; menus keep this order.
const QidiMaterialInfo qidi_materials[] = {
    {  1, QidiGroupPla,     Q2 | MAX4, "PLA Rapido"         },
    {  1, QidiGroupPla,     PLUS4,     "PLA"                },
    {  2, QidiGroupPla,     ALL,       "PLA Matte"          },
    {  3, QidiGroupPla,     ALL,       "PLA Metal"          },
    {  4, QidiGroupPla,     ALL,       "PLA Silk"           },
    {  5, QidiGroupPla,     ALL,       "PLA-CF"             },
    {  6, QidiGroupPla,     ALL,       "PLA-Wood"           },
    {  7, QidiGroupPla,     ALL,       "PLA Basic"          },
    {  8, QidiGroupPla,     ALL,       "PLA Matte Basic"    },
    { 10, QidiGroupSupport, Q2 | MAX4, "Support For PLA"    },
    { 11, QidiGroupAbs,     Q2 | MAX4, "ABS Rapido"         },
    { 11, QidiGroupAbs,     PLUS4,     "ABS"                },
    { 12, QidiGroupAbs,     ALL,       "ABS-GF"             },
    { 13, QidiGroupAbs,     ALL,       "ABS-Metal"          },
    { 14, QidiGroupAbs,     ALL,       "ABS-Odorless"       },
    { 15, QidiGroupFlex,    MAX4,      "TPU-GF"             },
    { 18, QidiGroupAsa,     ALL,       "ASA"                },
    { 19, QidiGroupAsa,     ALL,       "ASA-Aero"           },
    { 20, QidiGroupAsa,     Q2 | MAX4, "ASA-CF"             },
    { 23, QidiGroupOther,   Q2 | MAX4, "PC"                 },
    { 24, QidiGroupPa,      ALL,       "UltraPA"            },
    { 25, QidiGroupPa,      Q2 | MAX4, "PA-CF"              },
    { 25, QidiGroupPa,      PLUS4,     "PA12-CF"            },
    { 26, QidiGroupPa,      ALL,       "UltraPA-CF25"       },
    { 27, QidiGroupPa,      Q2 | MAX4, "PA12-CF"            },
    { 30, QidiGroupPa,      ALL,       "PAHT-CF"            },
    { 31, QidiGroupPa,      ALL,       "PAHT-GF"            },
    { 32, QidiGroupSupport, ALL,       "Support For PAHT"   },
    { 33, QidiGroupSupport, ALL,       "Support For PET/PA" },
    { 34, QidiGroupOther,   ALL,       "PC/ABS-FR"          },
    { 35, QidiGroupFlex,    Q2 | MAX4, "TPEE"               },
    { 36, QidiGroupFlex,    Q2 | MAX4, "PEBA"               },
    { 37, QidiGroupPet,     ALL,       "PET-CF"             },
    { 38, QidiGroupPet,     ALL,       "PET-GF"             },
    { 39, QidiGroupPet,     ALL,       "PETG Basic"         },
    { 40, QidiGroupPet,     ALL,       "PETG-Tough"         },
    { 41, QidiGroupPet,     Q2 | MAX4, "PETG Rapido"        },
    { 41, QidiGroupPet,     PLUS4,     "PETG"               },
    { 42, QidiGroupPet,     Q2 | MAX4, "PETG-CF"            },
    { 43, QidiGroupPet,     Q2 | MAX4, "PETG-GF"            },
    { 44, QidiGroupOther,   ALL,       "PPS-CF"             },
    { 45, QidiGroupPet,     ALL,       "PETG Translucent"   },
    { 46, QidiGroupOther,   Q2 | MAX4, "PPS-GF"             },
    { 47, QidiGroupOther,   ALL,       "PVA"                },
    { 48, QidiGroupFlex,    Q2 | MAX4, "TPU-AERO 64D"       },
    { 49, QidiGroupFlex,    ALL,       "TPU-Aero"           },
    { 50, QidiGroupFlex,    Q2 | MAX4, "TPU 95A-HF"         },
    { 50, QidiGroupFlex,    PLUS4,     "TPU"                },
};
_Static_assert(
    sizeof(qidi_materials) / sizeof(qidi_materials[0]) == QIDI_MATERIAL_COUNT,
    "QIDI_MATERIAL_COUNT does not match qidi_materials[]");

#undef Q2
#undef MAX4
#undef PLUS4
#undef ALL

// Same 24-entry palette on every printer ([colordict] in the cfg)
const QidiColorInfo qidi_colors[QIDI_COLOR_COUNT] = {
    {  1, "White",       0xFAFAFA },
    {  2, "Black",       0x060606 },
    {  3, "Light Gray",  0xD9E3ED },
    {  4, "Lime Green",  0x5CF30F },
    {  5, "Mint Green",  0x63E492 },
    {  6, "Blue",        0x2850FF },
    {  7, "Pink",        0xFE98FE },
    {  8, "Yellow",      0xDFD628 },
    {  9, "Dark Green",  0x228332 },
    { 10, "Light Blue",  0x99DEFF },
    { 11, "Dark Blue",   0x1714B0 },
    { 12, "Lavender",    0xCEC0FE },
    { 13, "Yellow-Green",0xCADE4B },
    { 14, "Ocean Blue",  0x1353AB },
    { 15, "Sky Blue",    0x5EA9FD },
    { 16, "Purple",      0xA878FF },
    { 17, "Light Red",   0xFE717A },
    { 18, "Red",         0xFF362D },
    { 19, "Beige",       0xE2DFCD },
    { 20, "Gray",        0x898F9B },
    { 21, "Brown",       0x6E3812 },
    { 22, "Tan",         0xCAC59F },
    { 23, "Orange",      0xF28636 },
    { 24, "Bronze",      0xB87F2B },
};

// [vendor_list] in the cfg: 0 = Generic, 1 = QIDI
const QidiManufacturerInfo qidi_manufacturers[QIDI_MFR_COUNT] = {
    { 0x01, "QIDI"    },
    { 0x00, "Generic" },
};

int qidi_material_idx_by_code(QidiPrinter printer, uint8_t code) {
    for(int i = 0; i < QIDI_MATERIAL_COUNT; i++) {
        if(qidi_materials[i].code == code && qidi_material_on_printer(&qidi_materials[i], printer))
            return i;
    }
    return -1;
}

uint8_t qidi_material_first_idx(QidiPrinter printer) {
    for(uint8_t i = 0; i < QIDI_MATERIAL_COUNT; i++) {
        if(qidi_material_on_printer(&qidi_materials[i], printer)) return i;
    }
    // Every printer has at least one material in the table
    furi_crash("no materials for printer");
}

int qidi_color_idx_by_code(uint8_t code) {
    for(int i = 0; i < QIDI_COLOR_COUNT; i++) {
        if(qidi_colors[i].code == code) return i;
    }
    return -1;
}

int qidi_mfr_idx_by_code(uint8_t code) {
    for(int i = 0; i < QIDI_MFR_COUNT; i++) {
        if(qidi_manufacturers[i].code == code) return i;
    }
    return -1;
}

const char* qidi_material_name(QidiPrinter printer, uint8_t code) {
    int idx = qidi_material_idx_by_code(printer, code);
    return (idx >= 0) ? qidi_materials[idx].name : NULL;
}

const char* qidi_color_name(uint8_t code) {
    int idx = qidi_color_idx_by_code(code);
    return (idx >= 0) ? qidi_colors[idx].name : NULL;
}

const char* qidi_mfr_name(uint8_t code) {
    int idx = qidi_mfr_idx_by_code(code);
    return (idx >= 0) ? qidi_manufacturers[idx].name : NULL;
}
