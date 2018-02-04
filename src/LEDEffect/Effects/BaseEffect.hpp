#pragma once

#include <ArduinoJson.h>
#include <FastLED.h>
#include "../Configuration.hpp"

#ifndef LEDEFFECT_EFFECT_NAME_MAX_LENGTH
#define LEDEFFECT_EFFECT_NAME_MAX_LENGTH 20
#endif

class BaseEffect
{
public:
  char name[LEDEFFECT_EFFECT_NAME_MAX_LENGTH];
  const size_t jsonBufferSize = 0;

  BaseEffect(const char* name, const size_t jsonBufferSize = 0) : jsonBufferSize(jsonBufferSize) {
    strncpy(this->name, name, LEDEFFECT_EFFECT_NAME_MAX_LENGTH);
  };

  virtual void begin(CLEDController* controller) {
    LEDEFFECT_DEBUG_PRINT(F("BaseEffect: Beginning effect "));
    LEDEFFECT_DEBUG_PRINTLN(name);
    _controller = controller;
  }

  virtual void deserialize(JsonObject& data) = 0;
  virtual void serialize(JsonObject& data) const = 0;
  virtual void loop() = 0;

protected:
  CLEDController* _controller;
};
