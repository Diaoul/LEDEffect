#pragma once

#include "PaletteEffect.hpp"

// Random white flashes transforming into a color before fading to black
class ApplauseEffect final : public PaletteEffect
{
public:
  uint8_t fadeRate;

  ApplauseEffect(const char* name, uint8_t fadeRate = 32, const char* paletteName = "ocean", TBlendType blend = NOBLEND,
    const PaletteData* palettes = 0, const size_t paletteCount = 0) :
    PaletteEffect(name, paletteName, blend, palettes, paletteCount, JSON_NODE_SIZE), fadeRate(fadeRate) { };

  void deserialize(JsonObject& data) override {
    PaletteEffect::deserialize(data);

    // fadeRate
    if (data.containsKey("fade_rate")) {
      LEDEFFECT_DEBUG_PRINT(F("ApplauseEffect: fadeRate to "));
      LEDEFFECT_DEBUG_PRINTLN(data["fade_rate"].as<uint8_t>());
      fadeRate = data["fade_rate"].as<uint8_t>();
    }
  }

  void serialize(JsonObject& data) const override {
    PaletteEffect::serialize(data);

    data["fade_rate"] = fadeRate;
  }

  void loop() override {
    static uint16_t lastPixel = 0;

    fadeToBlackBy(_controller->leds(), _controller->size(), fadeRate);
    _controller->leds()[lastPixel] = ColorFromPalette(PaletteFromName(paletteName), random8(), 255, blend);
    // _controller->leds()[lastPixel] = CHSV(random8(hueStart, hueEnd), saturation, value);
    lastPixel = random16(_controller->size());
    _controller->leds()[lastPixel] = CRGB::White;
  }
};
