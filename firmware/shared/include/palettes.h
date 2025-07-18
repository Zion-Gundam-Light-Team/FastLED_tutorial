#ifndef PALETTES_H
#define PALETTES_H

#include <FastLED.h>

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

extern const TProgmemRGBGradientPalette_byte lava_gp[];
extern const TProgmemRGBGradientPalette_byte fire_gp[];

extern CRGBPalette16 footplate_flash1_palette;
extern CRGBPalette16 footplate_flash2_palette;
extern CRGBPalette16 footplate_flash3_palette;
extern CRGBPalette16 footplate_flash4_palette;
extern CRGBPalette16 footplate_flash5_palette;
extern CRGBPalette16 footplate_flash6_palette;

struct NamedPalette
{
    const char *name;
    CRGBPalette16 palette;
};

extern const NamedPalette namedPalettes[];
extern uint8_t paletteCount;
#endif