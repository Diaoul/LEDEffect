#pragma once

#include "BaseEffect.hpp"

template<size_t NUM_LEDS>
class FireEffect : public BaseEffect
{
public:
  uint8_t cooling;
  uint8_t sparking;
  bool forward;

  FireEffect(const char* name, uint8_t cooling = 55, uint8_t sparking = 120, bool forward = true) :
    BaseEffect(name, 2 * JSON_NODE_SIZE), cooling(cooling), sparking(sparking), forward(forward) { };

  void deserialize(JsonObject& data) override {
    // cooling
    if (data.containsKey("cooling")) {
      LEDEFFECT_DEBUG_PRINT(F("FireEffect: cooling to "));
      LEDEFFECT_DEBUG_PRINTLN(data["cooling"].as<uint8_t>());
      cooling = data["cooling"].as<uint8_t>();
    }

    // sparking
    if (data.containsKey("sparking")) {
      LEDEFFECT_DEBUG_PRINT(F("FireEffect: sparking to "));
      LEDEFFECT_DEBUG_PRINTLN(data["sparking"].as<uint8_t>());
      sparking = data["sparking"].as<uint8_t>();
    }

    // forward
    if (data.containsKey("forward")) {
      LEDEFFECT_DEBUG_PRINT(F("FireEffect: forward to "));
      LEDEFFECT_DEBUG_PRINTLN(data["forward"].as<bool>());
      forward = data["forward"].as<bool>();
    }
  }

  void serialize(JsonObject& data) const override {
    data["cooling"] = cooling;
    data["sparking"] = sparking;
    data["forward"] = forward;
  }

  void loop() override {
    random16_add_entropy(random16());

    // Step 1.  Cool down every cell a little
    for (int i = 0; i < _controller->size(); i++) {
      _heat[i] = qsub8(_heat[i],  random8(0, ((cooling * 10) / _controller->size()) + 2));
    }

  // Step 2.  Heat from each cell drifts 'up' and diffuses a little
    for (int k= _controller->size() - 1; k >= 2; k--) {
      _heat[k] = (_heat[k - 1] + _heat[k - 2] + _heat[k - 2]) / 3;
    }

  // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
    if (random8() < sparking) {
      int y = random8(7);
      _heat[y] = qadd8(_heat[y], random8(160,255));
    }

      // Step 4.  Map from heat cells to LED colors
      for (int j = 0; j < _controller->size(); j++) {
        // Scale the heat value from 0-255 down to 0-240
        // for best results with color palettes.
        byte colorindex = scale8(_heat[j], 240);
        CRGB color = ColorFromPalette(HeatColors_p, colorindex);
        int pixelnumber;
        if (forward) {
          pixelnumber = j;
        } else {
          pixelnumber = (_controller->size()-1) - j;
        }
        _controller->leds()[pixelnumber] = color;
    }
  }

protected:
  byte _heat[NUM_LEDS];
};
