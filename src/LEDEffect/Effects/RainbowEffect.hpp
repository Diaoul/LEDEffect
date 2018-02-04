#pragma once

#include "BaseEffect.hpp"

// A rainbow that moves along the leds
class RainbowEffect final : public BaseEffect
{
public:
  uint8_t deltaHue = 2;  // hue difference between two leds
  int8_t rate = 1;       // rate of change of hue on each loop

  RainbowEffect(const char* name) : BaseEffect(name, 2 * JSON_NODE_SIZE) { };

  void deserialize(JsonObject& data) override {
    // deltaHue
    if (data.containsKey("delta_hue")) {
      LEDEFFECT_DEBUG_PRINT(F("RainbowEffect: deltaHue to "));
      LEDEFFECT_DEBUG_PRINTLN(data["delta_hue"].as<uint8_t>());
      deltaHue = data["delta_hue"].as<uint8_t>();
    }

    // rate
    if (data.containsKey("rate")) {
      LEDEFFECT_DEBUG_PRINT(F("RainbowEffect: rate to "));
      LEDEFFECT_DEBUG_PRINTLN(data["rate"].as<int8_t>());
      rate = data["rate"].as<int8_t>();
    }
  }

  void serialize(JsonObject& data) const override {
    data["delta_hue"] = deltaHue;
    data["rate"] = rate;
  }

  void loop() override {
    _hue += rate;
    fill_rainbow(_controller->leds(), _controller->size(), _hue, deltaHue);
  }

protected:
  uint8_t _hue = 0;
};
