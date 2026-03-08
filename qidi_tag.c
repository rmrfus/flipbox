#include "qidi_tag.h"
#include <string.h>

const uint8_t QIDI_KEY_PRIMARY[6] = {0xD3, 0xF7, 0xD3, 0xF7, 0xD3, 0xF7};
const uint8_t QIDI_KEY_DEFAULT[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// Codes are the actual hex values written to the tag.
// Wiki documents them in decimal; these are the converted hex equivalents.
// Source: https://wiki.qidi3d.com/en/QIDIBOX/RFID
// Codes 9,10,15-17,20-23,28-29,35-36,46,48 are unassigned in the wiki.
const QidiMaterialInfo qidi_materials[QIDI_MATERIAL_COUNT] = {
    {  1, "PLA Rapido"        },
    {  2, "PLA Matte"         },
    {  3, "PLA Metal"         },
    {  4, "PLA Silk"          },
    {  5, "PLA-CF"            },
    {  6, "PLA-Wood"          },
    {  7, "PLA Basic"         },
    {  8, "PLA Matte Basic"   },
    { 11, "ABS"               },
    { 12, "ABS-GF"            },
    { 13, "ABS-Metal"         },
    { 14, "ABS-Odorless"      },
    { 18, "ASA"               },
    { 19, "ASA-AERO"          },
    { 24, "UltraPA"           },
    { 25, "PA-CF"             },
    { 26, "UltraPA-CF25"      },
    { 27, "PA12-CF"           },
    { 30, "PAHT-CF"           },
    { 31, "PAHT-GF"           },
    { 32, "Support For PAHT"  },
    { 33, "Support For PET/PA"},
    { 34, "PC/ABS-FR"         },
    { 37, "PET-CF"            },
    { 38, "PET-GF"            },
    { 39, "PETG Basic"        },
    { 40, "PETG Tough"        },
    { 41, "PETG Rapido"       },
    { 42, "PETG-CF"           },
    { 43, "PETG-GF"           },
    { 44, "PPS-CF"            },
    { 45, "PETG Translucent"  },
    { 47, "PVA"               },
    { 49, "TPU-Aero"          },
    { 50, "TPU"               },
};

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

const QidiManufacturerInfo qidi_manufacturers[QIDI_MFR_COUNT] = {
    { 0x01, "QIDI"    },
    { 0x02, "Generic" },
};

// Material group index → indices into qidi_materials[]
// qidi_materials[] order: PLA(0-7) ABS(8-11) ASA(12-13) PA(14-19)
//   Support(20-21) PC(22) PET/PETG(23-29) PPS(30) PETGTrans(31) PVA(32) TPU(33-34)
const QidiMaterialGroup qidi_material_groups[QIDI_GROUP_COUNT] = {
    { "PLA",        { 0,  1,  2,  3,  4,  5,  6,  7           }, 8 },
    { "ABS",        { 8,  9, 10, 11                            }, 4 },
    { "ASA",        {12, 13                                    }, 2 },
    { "PA / Nylon", {14, 15, 16, 17, 18, 19                   }, 6 },
    { "PET / PETG", {23, 24, 25, 26, 27, 28, 29, 31           }, 8 },
    { "Support",    {20, 21                                    }, 2 },
    { "TPU",        {33, 34                                    }, 2 },
    { "Other",      {22, 30, 32                                }, 3 },
};

int qidi_material_idx_by_code(uint8_t code) {
    for(int i = 0; i < QIDI_MATERIAL_COUNT; i++) {
        if(qidi_materials[i].code == code) return i;
    }
    return -1;
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

const char* qidi_material_name(uint8_t code) {
    int idx = qidi_material_idx_by_code(code);
    return (idx >= 0) ? qidi_materials[idx].name : "Unknown";
}

const char* qidi_color_name(uint8_t code) {
    int idx = qidi_color_idx_by_code(code);
    return (idx >= 0) ? qidi_colors[idx].name : "Unknown";
}

const char* qidi_mfr_name(uint8_t code) {
    int idx = qidi_mfr_idx_by_code(code);
    return (idx >= 0) ? qidi_manufacturers[idx].name : "Unknown";
}
