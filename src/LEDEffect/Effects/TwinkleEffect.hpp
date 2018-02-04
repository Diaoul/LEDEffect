#pragma once

#include "PaletteEffect.hpp"

// light a random pixel which will get brighter (direction 1) then darker (direction 0) until black, bouncing if there is a minimal brightness
template<size_t NUM_LEDS>
class TwinkleEffect final : public PaletteEffect
{
public:
  uint8_t initialBrightness;
  uint8_t maxBrightness;
  fract8 brightenRate;
  fract8 fadeRate;
  fract8 density;

  TwinkleEffect(const char* name, uint8_t initialBrightness = 64, uint8_t maxBrightness = 255,
    fract8 brightenRate = 32, fract8 fadeRate = 16, fract8 density = 150,
    const char* paletteName = "rainbow", TBlendType blend = NOBLEND,
    const PaletteData* palettes = 0, const size_t paletteCount = 0) :
    PaletteEffect(name, paletteName, blend, palettes, paletteCount, 5 * JSON_NODE_SIZE),
    initialBrightness(initialBrightness), maxBrightness(maxBrightness), brightenRate(brightenRate),
    fadeRate(fadeRate), density(density) { };

  void deserialize(JsonObject& data) override {
    PaletteEffect::deserialize(data);

    // initialBrightness
    if (data.containsKey("initial_brightness")) {
      LEDEFFECT_DEBUG_PRINT(F("TwinkleEffect: initialBrightness to "));
      LEDEFFECT_DEBUG_PRINTLN(data["initial_brightness"].as<uint8_t>());
      initialBrightness = data["initial_brightness"].as<uint8_t>();
    }

    // maxBrightness
    if (data.containsKey("max_brightness")) {
      LEDEFFECT_DEBUG_PRINT(F("TwinkleEffect: maxBrightness to "));
      LEDEFFECT_DEBUG_PRINTLN(data["max_brightness"].as<uint8_t>());
      maxBrightness = data["max_brightness"].as<uint8_t>();
    }

    // brightenRate
    if (data.containsKey("brighten_rate")) {
      LEDEFFECT_DEBUG_PRINT(F("TwinkleEffect: brightenRate to "));
      LEDEFFECT_DEBUG_PRINTLN(data["brighten_rate"].as<fract8>());
      brightenRate = data["brighten_rate"].as<fract8>();
    }

    // fadeRate
    if (data.containsKey("fade_rate")) {
      LEDEFFECT_DEBUG_PRINT(F("TwinkleEffect: fadeRate to "));
      LEDEFFECT_DEBUG_PRINTLN(data["fade_rate"].as<fract8>());
      fadeRate = data["fade_rate"].as<fract8>();
    }

    // density
    if (data.containsKey("density")) {
      LEDEFFECT_DEBUG_PRINT(F("TwinkleEffect: density to "));
      LEDEFFECT_DEBUG_PRINTLN(data["density"].as<fract8>());
      density = data["density"].as<fract8>();
    }
  }

  void serialize(JsonObject& data) const override {
    PaletteEffect::serialize(data);
    data["initial_brightness"] = initialBrightness;
    data["max_brightness"] = maxBrightness;
    data["brighten_rate"] = brightenRate;
    data["fade_rate"] = fadeRate;
    data["density"] = density;
  }

  void loop() override {
    for (int i = 0; i < _controller->size(); i++) {
      if (_directions[i] == 1) {
        CRGB color = _controller->leds()[i];
        _controller->leds()[i] += color.nscale8(brightenRate);
        if (_controller->leds()[i].r >= maxBrightness || _controller->leds()[i].g >= maxBrightness || _controller->leds()[i].b >= maxBrightness) {
          _directions[i] = 0;
        }
      } else {
        _controller->leds()[i].nscale8(255 - fadeRate);
      }
    }
    if (random8() < density ) {
      uint16_t pos = random16(_controller->size());
      if (!_controller->leds()[pos]) {
        _controller->leds()[pos] = ColorFromPalette(PaletteFromName(paletteName), random8(), initialBrightness, NOBLEND);
        _directions[pos] = 1;
      }
    }
  }

protected:
  uint8_t _directions[NUM_LEDS];  // TODO: optimize with NUM_LEDS bits instead of bytes
};
