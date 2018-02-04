#pragma once

#include "BaseEffect.hpp"

class SolidEffect final : public BaseEffect
{
public:
  CRGB color;
  uint8_t rate;

  SolidEffect(const char* name, CRGB color = CRGB::Blue, uint8_t rate = 4) :
    BaseEffect(name, JSON_NODE_SIZE + JSON_ARRAY_SIZE(3)), color(color), rate(rate) { };

  void deserialize(JsonObject& data) override {
    // color rgb
    if (data.containsKey("color_rgb")) {
      LEDEFFECT_DEBUG_PRINT(F("SolidEffect: color_rgb to ["));
      LEDEFFECT_DEBUG_PRINT(data["color_rgb"][0].as<uint8_t>());
      LEDEFFECT_DEBUG_PRINT(F(", "));
      LEDEFFECT_DEBUG_PRINT(data["color_rgb"][1].as<uint8_t>());
      LEDEFFECT_DEBUG_PRINT(F(", "));
      LEDEFFECT_DEBUG_PRINT(data["color_rgb"][2].as<uint8_t>());
      LEDEFFECT_DEBUG_PRINTLN(F("]"));

      _lastColor = _currentColor;
      _blend = 0;
      color = CRGB(data["color_rgb"][0], data["color_rgb"][1], data["color_rgb"][2]);
    // color hsv
    } else if (data.containsKey("color_hsv")) {
      LEDEFFECT_DEBUG_PRINT(F("SolidEffect: color_hsv to ["));
      LEDEFFECT_DEBUG_PRINT(data["color_hsv"][0].as<uint8_t>());
      LEDEFFECT_DEBUG_PRINT(F(", "));
      LEDEFFECT_DEBUG_PRINT(data["color_hsv"][1].as<uint8_t>());
      LEDEFFECT_DEBUG_PRINT(F(", "));
      LEDEFFECT_DEBUG_PRINT(data["color_hsv"][2].as<uint8_t>());
      LEDEFFECT_DEBUG_PRINTLN(F("]"));

      _lastColor = _currentColor;
      _blend = 0;
      color = CHSV(data["color_hsv"][0], data["color_hsv"][1], data["color_hsv"][2]);
    }
  }

  void serialize(JsonObject& data) const override {
    JsonArray& color_rgb = data.createNestedArray("color_rgb");
    color_rgb.add(color.r);
    color_rgb.add(color.g);
    color_rgb.add(color.b);
    data["rate"] = rate;
  }

  void loop() override {
    // compute new color and increment blend
    if (_blend < 255) {
      _currentColor = blend(_lastColor, color, _blend);
      _blend = _blend + min((int)rate, 255 - _blend);
    }

    // solid color
    fill_solid(_controller->leds(), _controller->size(), _currentColor);
  }

protected:
  CRGB _lastColor = CRGB::Black;
  CRGB _currentColor = CRGB::Black;
  uint8_t _blend = 0;
};
