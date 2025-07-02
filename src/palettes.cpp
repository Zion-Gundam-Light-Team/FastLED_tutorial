#include <FastLED.h>
#include "../include/palettes.h"

DEFINE_GRADIENT_PALETTE(blue_wave_gp){
    0, 0, 255, 245,
    46, 0, 21, 255,
    179, 12, 250, 0,
    230, 0, 255, 245,
    255, 250, 255, 245};

DEFINE_GRADIENT_PALETTE(sunrise_sunset_gp){
    0, 0, 0, 0,
    58, 0, 0, 0,
    65, 76, 0, 0,
    69, 78, 0, 0,
    70, 80, 0, 0,
    71, 82, 0, 0,
    72, 84, 0, 0,
    73, 86, 0, 0,
    74, 88, 0, 0,
    75, 90, 0, 0,
    76, 92, 0, 0,
    77, 94, 0, 0,
    78, 96, 0, 0,
    79, 98, 0, 0,
    80, 100, 0, 0,
    100, 240, 0, 0,
    120, 255, 44, 0,
    135, 255, 208, 60,
    155, 255, 255, 255,
    185, 200, 160, 180,
    200, 150, 20, 100,
    240, 100, 20, 0,
    255, 10, 0, 0};

CRGBPalette16 blue_wave_p = blue_wave_gp;
CRGBPalette16 sunrise_sunset_p = sunrise_sunset_gp;
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
