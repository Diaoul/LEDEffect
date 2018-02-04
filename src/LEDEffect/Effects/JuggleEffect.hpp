#pragma once

#include "BaseEffect.hpp"

// Colored dots weaving out of sync with each other
class JuggleEffect final : public BaseEffect
{
public:
  uint8_t dots;
  uint8_t saturation;
  uint8_t value;
  uint8_t fadeRate;

  JuggleEffect(const char* name, uint8_t dots = 10, uint8_t saturation = 200, uint8_t value = 255, uint8_t fadeRate = 32) :
    BaseEffect(name, 4 * JSON_NODE_SIZE), dots(dots), saturation(saturation), value(value), fadeRate(fadeRate) { };

  void deserialize(JsonObject& data) override {
    // dots
    if (data.containsKey("dots")) {
      LEDEFFECT_DEBUG_PRINT(F("JuggleEffect: dots to "));
      LEDEFFECT_DEBUG_PRINTLN(data["dots"].as<uint8_t>());
      dots = data["dots"].as<uint8_t>();
    }

    // saturation
    if (data.containsKey("saturation")) {
      LEDEFFECT_DEBUG_PRINT(F("JuggleEffect: saturation to "));
      LEDEFFECT_DEBUG_PRINTLN(data["saturation"].as<uint8_t>());
      saturation = data["saturation"].as<uint8_t>();
    }

    // value
    if (data.containsKey("value")) {
      LEDEFFECT_DEBUG_PRINT(F("JuggleEffect: value to "));
      LEDEFFECT_DEBUG_PRINTLN(data["value"].as<uint8_t>());
      value = data["value"].as<uint8_t>();
    }

    // fadeRate
    if (data.containsKey("fade_rate")) {
      LEDEFFECT_DEBUG_PRINT(F("JuggleEffect: fadeRate to "));
      LEDEFFECT_DEBUG_PRINTLN(data["fade_rate"].as<uint8_t>());
      fadeRate = data["fade_rate"].as<uint8_t>();
    }
  }

  void serialize(JsonObject& data) const override {
    data["dots"] = dots;
    data["saturation"] = saturation;
    data["value"] = value;
    data["fade_rate"] = fadeRate;
  }

  void loop() override {
    fadeToBlackBy(_controller->leds(), _controller->size(), fadeRate);
    uint8_t dothue = 0;
    for (uint8_t i = 0; i < dots; i++) {
      _controller->leds()[beatsin16(i + 5, 0, _controller->size())] |= CHSV(dothue, saturation, value);
      dothue += 256 / dots;
    }
  }
};
