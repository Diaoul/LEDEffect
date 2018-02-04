#pragma once

#include "BaseEffect.hpp"
#include "../PaletteData.hpp"

#define LEDEFFECT_DEFAULT_PALETTE PaletteData { RainbowColors_p, "rainbow" }

class PaletteEffect : public BaseEffect
{
public:
  char paletteName[LEDEFFECT_PALETTE_NAME_MAX_LENGTH];
  TBlendType blend;

  PaletteEffect(const char* name, const char* paletteName, TBlendType blend = NOBLEND,
    const PaletteData* palettes = 0, const size_t paletteCount = 0, const size_t jsonBufferSize = 0) :
    BaseEffect(name, 2 * JSON_NODE_SIZE + jsonBufferSize),
    blend(blend), _palettes(palettes), _paletteCount(paletteCount) {
      strcpy(this->paletteName, paletteName);
    };

  void deserialize(JsonObject& data) override {
    // blend
    if (data.containsKey("blend")) {
      LEDEFFECT_DEBUG_PRINT(F("PaletteEffect: blend to "));
      LEDEFFECT_DEBUG_PRINTLN(data["blend"].as<bool>());
      blend = (TBlendType)data["blend"].as<bool>();
    }

    // palette
    if (data.containsKey("palette")) {
      LEDEFFECT_DEBUG_PRINT(F("PaletteEffect: palette to "));
      LEDEFFECT_DEBUG_PRINTLN(data["palette"].as<const char*>());
      strcpy(paletteName, data["palette"].as<const char*>());
    }
  }

  void serialize(JsonObject& data) const override {
    data["blend"] = (bool)blend;
    data["palette"] = paletteName;
  }

  void loop() override {
    fill_palette(_controller->leds(), _controller->size(), 0, 255 / _controller->size() + 1, PaletteFromName(paletteName), 255, blend);
  }

protected:
  const PaletteData* _palettes;
  const size_t _paletteCount;
};
