#include <FastLED.h>
#include "../include/palettes.h"

DEFINE_GRADIENT_PALETTE(lava_gp){
    0, 0, 0, 0,
    46, 18, 0, 0,
    96, 113, 0, 0,
    108, 142, 3, 1,
    119, 175, 17, 1,
    146, 213, 44, 2,
    174, 255, 82, 4,
    188, 255, 115, 4,
    202, 255, 156, 4,
    218, 255, 203, 4,
    234, 255, 255, 4,
    244, 255, 255, 71,
    255, 255, 255, 255};

DEFINE_GRADIENT_PALETTE(fire_gp){
    0, 1, 1, 0,
    76, 32, 5, 0,
    146, 192, 24, 0,
    197, 220, 105, 5,
    240, 252, 255, 31,
    250, 252, 255, 111,
    255, 255, 255, 255};

CRGBPalette16 footplate_flash1_palette = CRGBPalette16(
    CRGB(100, 0, 0),
    CRGB(72, 26, 0));
CRGBPalette16 footplate_flash2_palette = CRGBPalette16(
    CRGB(72, 26, 0),
    CRGB(82, 20, 0),
    CRGB(82, 16, 0));
CRGBPalette16 footplate_flash3_palette = CRGBPalette16(
    CRGB(86, 28, 0),
    CRGB(86, 16, 40));
CRGBPalette16 footplate_flash4_palette = CRGBPalette16(
    CRGB(84, 16, 40),
    CRGB(24, 50, 60));
CRGBPalette16 footplate_flash5_palette = CRGBPalette16(
    CRGB(25, 30, 80),
    CRGB(82, 16, 40),
    CRGB(72, 10, 0),
    CRGB(84, 16, 40));
CRGBPalette16 footplate_flash6_palette = CRGBPalette16(
    CRGB(84, 16, 40),
    CRGB(50, 12, 76),
    CRGB(100, 0, 0));

    
const NamedPalette namedPalettes[] = {
    {"Heat", HeatColors_p},
    {"lava", lava_gp},
    {"fire", fire_gp},
};

uint8_t paletteCount = ARRAY_SIZE(namedPalettes);