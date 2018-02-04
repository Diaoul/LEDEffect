#pragma once

#include <ArduinoJson.h>
#include <FastLED.h>

#include "Effects/BaseEffect.hpp"
#include "Configuration.hpp"

class LedEffect
{
public:
  bool state = true;
  uint8_t brightness = 50;
  uint8_t brightnessRate = 8;
  uint8_t fps = 30;

  LedEffect(BaseEffect** effects, uint8_t effectCount) : _effects(effects), _effectCount(effectCount) { };

  void begin(CLEDController* controller, CFastLED fastLed) {
    _fastLed = fastLed;
    _controller = controller;
    _fastLed.setBrightness(brightness);

    size_t maxEffectJsonBufferSize = 0;
    for (uint8_t i = 0; i < _effectCount; i++) {
      _effects[i]->begin(_controller);
      if (_effects[i]->jsonBufferSize > maxEffectJsonBufferSize)
        maxEffectJsonBufferSize = _effects[i]->jsonBufferSize;
    }

    LEDEFFECT_DEBUG_PRINT(F("LEDEffect: JSON buffer size is "));
    LEDEFFECT_DEBUG_PRINT(_jsonBufferSize);
    LEDEFFECT_DEBUG_PRINT(F(" (base) + "));
    LEDEFFECT_DEBUG_PRINT(maxEffectJsonBufferSize);
    LEDEFFECT_DEBUG_PRINT(F(" (effects) = "));
    LEDEFFECT_DEBUG_PRINTLN(_jsonBufferSize + maxEffectJsonBufferSize);

    _jsonBufferSize += maxEffectJsonBufferSize;
  }

  void begin(CLEDController* controller) {
    begin(controller, FastLED);
  }

  bool deserialize(JsonObject& root) {
    LEDEFFECT_DEBUG_PRINTLN(F("LED Effect: Deserializing..."));

    if (!root.success()) {
      LEDEFFECT_DEBUG_PRINTLN(F("LED Effect: JSON Parse failed"));
      return false;
    }

    // state
    if (root.containsKey("state")) {
      LEDEFFECT_DEBUG_PRINT(F("LED Effect: state to "));
      LEDEFFECT_DEBUG_PRINTLN(root["state"].as<char*>());
      if (strcmp(root["state"], "ON") == 0) {
        state = true;
      } else if (strcmp(root["state"], "OFF") == 0) {
        state = false;
      }
    }

    // brightness
    if (root.containsKey("brightness")) {
      LEDEFFECT_DEBUG_PRINT(F("LED Effect: brightness to "));
      LEDEFFECT_DEBUG_PRINTLN(root["brightness"].as<uint8_t>());
      brightness = root["brightness"].as<uint8_t>();
    }

    // brightness_rate
    if (root.containsKey("brightness_rate")) {
      LEDEFFECT_DEBUG_PRINT(F("LED Effect: brightnessRate to "));
      LEDEFFECT_DEBUG_PRINTLN(root["brightness_rate"].as<uint8_t>());
      brightnessRate = root["brightness_rate"].as<uint8_t>();
    }

    // fps
    if (root.containsKey("fps")) {
      LEDEFFECT_DEBUG_PRINT(F("LED Effect: fps to "));
      LEDEFFECT_DEBUG_PRINTLN(root["fps"].as<int>());
      fps = root["fps"].as<uint8_t>();
    }

    // effect
    if (root.containsKey("effect")) {
      JsonObject& effect = root["effect"];
      if (effect.containsKey("name")) {
        LEDEFFECT_DEBUG_PRINT(F("LED Effect: effect to "));
        LEDEFFECT_DEBUG_PRINTLN(effect["name"].as<char*>());
        for (uint8_t i = 0; i < _effectCount; i++) {
          if (strcmp(effect["name"], _effects[i]->name) == 0) {
            _currentEffect = i;
            LEDEFFECT_DEBUG_PRINT(F("LED Effect: Switch to effect "));
            LEDEFFECT_DEBUG_PRINTLN(_currentEffect);
            break;
          }
        }
      }
      _effects[_currentEffect]->deserialize(effect);
    }

    return true;
  }

  bool deserialize(char* data) {
    DynamicJsonBuffer jsonBuffer(_jsonBufferSize);
    JsonObject& root = jsonBuffer.parseObject(data);
    LEDEFFECT_DEBUG_PRINT(F("LED Effect: JSON Buffer "));
    LEDEFFECT_DEBUG_PRINT(jsonBuffer.size());
    LEDEFFECT_DEBUG_PRINT(F("/"));
    LEDEFFECT_DEBUG_PRINTLN(_jsonBufferSize);

    return deserialize(root);
  }

  void serialize(JsonObject& root) {
    LEDEFFECT_DEBUG_PRINTLN(F("LED Effect: Serializing..."));

    root["state"] = state ? "ON" : "OFF";
    root["brightness"] = brightness;
    root["brightness_rate"] = brightnessRate;
    root["fps"] = fps;
    JsonObject& effect = root.createNestedObject("effect");
    effect["name"] = _effects[_currentEffect]->name;
    _effects[_currentEffect]->serialize(effect);
  }

  size_t printTo(char* buffer, size_t bufferSize) {
    DynamicJsonBuffer jsonBuffer(_jsonBufferSize);
    JsonObject& root = jsonBuffer.createObject();
    serialize(root);

    LEDEFFECT_DEBUG_PRINT(F("LED Effect: JSON Buffer "));
    LEDEFFECT_DEBUG_PRINT(jsonBuffer.size());
    LEDEFFECT_DEBUG_PRINT(F("/"));
    LEDEFFECT_DEBUG_PRINTLN(_jsonBufferSize);

    return root.printTo(buffer, bufferSize);
  }

  size_t printTo(Print& print) {
    DynamicJsonBuffer jsonBuffer(_jsonBufferSize);
    JsonObject& root = jsonBuffer.createObject();
    serialize(root);

    LEDEFFECT_DEBUG_PRINT(F("LED Effect: JSON Buffer "));
    LEDEFFECT_DEBUG_PRINT(jsonBuffer.size());
    LEDEFFECT_DEBUG_PRINT(F("/"));
    LEDEFFECT_DEBUG_PRINTLN(_jsonBufferSize);

    return root.printTo(print);
  }

  size_t printTo(String& str) {
    DynamicJsonBuffer jsonBuffer(_jsonBufferSize);
    JsonObject& root = jsonBuffer.createObject();
    serialize(root);

    LEDEFFECT_DEBUG_PRINT(F("LED Effect: JSON Buffer "));
    LEDEFFECT_DEBUG_PRINT(jsonBuffer.size());
    LEDEFFECT_DEBUG_PRINT(F("/"));
    LEDEFFECT_DEBUG_PRINTLN(_jsonBufferSize);

    return root.printTo(str);
  }

  void loop() {
#ifdef LEDEFFECT_DEBUG
    auto startMillis = millis();
#endif

    // apply effect
    _effects[_currentEffect]->loop();

    // update strip
    if (state) {
      uint8_t currentBrightness = _fastLed.getBrightness();
      if (currentBrightness < brightness)
        _fastLed.setBrightness(currentBrightness + min((int)brightnessRate, brightness - currentBrightness));
      else if (currentBrightness > brightness)
        _fastLed.setBrightness(currentBrightness - min((int)brightnessRate, currentBrightness - brightness));
    } else {
      _fastLed.setBrightness(0);
    }
    _fastLed.show();
    if (fps > 0)
      _fastLed.delay(1000 / fps);


#ifdef LEDEFFECT_DEBUG
    EVERY_N_SECONDS(10) {
      LEDEFFECT_DEBUG_PRINT(F("LED Effect: Loop time is "));
      LEDEFFECT_DEBUG_PRINT(millis() - startMillis);
      LEDEFFECT_DEBUG_PRINTLN(F("ms"));
    }
#endif
  }

private:
  CLEDController* _controller;
  CFastLED _fastLed;
  BaseEffect** _effects;
  uint8_t _effectCount;
  uint8_t _currentEffect = 0;
  size_t _jsonBufferSize = JSON_OBJECT_SIZE(5) + JSON_OBJECT_SIZE(1);  // root + effect
};
