#pragma once

//#define LEDEFFECT_DEBUG

#ifndef ARDUINO
#define max(a,b) ((a)>(b)?(a):(b))
#define min(a,b) ((a)<(b)?(a):(b))
#endif

#define JSON_NODE_SIZE \
  (sizeof(JsonObject::node_type))

#ifdef LEDEFFECT_DEBUG
 #ifndef LEDEFFECT_DEBUG_PRINTER
 #define LEDEFFECT_DEBUG_PRINTER Serial
 #endif
 #define LEDEFFECT_DEBUG_WRITE(...)    LEDEFFECT_DEBUG_PRINTER.write(__VA_ARGS__)
 #define LEDEFFECT_DEBUG_PRINT(...)    LEDEFFECT_DEBUG_PRINTER.print(__VA_ARGS__)
 #define LEDEFFECT_DEBUG_PRINTLN(...)  LEDEFFECT_DEBUG_PRINTER.println(__VA_ARGS__)
 #define LEDEFFECT_DEBUG_PRINTF(...)   LEDEFFECT_DEBUG_PRINTER.printf(__VA_ARGS__)
#else
 #define LEDEFFECT_DEBUG_WRITE(...)
 #define LEDEFFECT_DEBUG_PRINT(...)
 #define LEDEFFECT_DEBUG_PRINTLN(...)
 #define LEDEFFECT_DEBUG_PRINTF(...)
#endif
