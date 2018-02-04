#pragma once

#include <FastLED.h>

#ifndef LEDEFFECT_PALETTE_NAME_MAX_LENGTH
#define LEDEFFECT_PALETTE_NAME_MAX_LENGTH 16
#endif

struct PaletteData {
  const TProgmemRGBPalette16& palette;
  const char name[LEDEFFECT_PALETTE_NAME_MAX_LENGTH];
};

const TProgmemRGBPalette16& PaletteFromName(const char* name, const PaletteData* palettes = 0, const size_t paletteCount = 0) {
  // custom
  for (size_t i = 0; i < paletteCount; i++) {
    if (strcmp(name, palettes[i].name) == 0) {
      return palettes[i].palette;
    }
  }

  // predefined
  if (strcmp(name, "rainbow") == 0) {
    return RainbowColors_p;
  } else if (strcmp(name, "rainbow_stripes") == 0) {
    return RainbowStripeColors_p;
  } else if (strcmp(name, "ocean") == 0) {
    return OceanColors_p;
  } else if (strcmp(name, "cloud") == 0) {
    return CloudColors_p;
  } else if (strcmp(name, "lava") == 0) {
    return LavaColors_p;
  } else if (strcmp(name, "forest") == 0) {
    return ForestColors_p;
  } else if (strcmp(name, "party") == 0) {
    return PartyColors_p;
  } else if (strcmp(name, "heat") == 0) {
    return HeatColors_p;
  }

  // unknown
  return RainbowColors_p;
}

 uint32_t CRGBToColorcode(CRGB color) {
   return ((uint32_t)color.r << 16) + ((uint32_t)color.g << 8) + color.b;
}
