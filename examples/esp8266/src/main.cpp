/**
 * LEDEffect example
 * =================
 * This example is meant for direct use, search the code for `CHANGEME` and adjust to your setup.
 *
 * Features
 * --------
 * - LED Effects!
 * - OTA
 * - RESTful interface
 *   - /leds GET & POST: Control your LEDs!
 *   - /effects GET: List of available effects
 *   - /info GET: General informations about the ESP
 *   - /dht GET: Temperature & Humidity data with error reporting (optional)
 * - MQTT
 * - Easy debugging
 * - Temperature and humidity (optional)
 * - Home Assistant easy integration (see below)
 *
 * Hardware requirements
 * ---------------------
 * - ESP (with > 4MB chip if you want to use OTA)
 * - FastLED-compatible LED strip with correct hardware setup (e.g. power, voltage translator)
 * - DHT22 (optional)
 *
 * Home Assitant setup
 * -------------------
 * You can control your LEDs (including effects!) with Home Assitant with the mqtt_template light component.
 *
 light:
  - name: ledeffect
    platform: mqtt_template
    effect_list:
      - Rainbow
      - Solid
      - Twinkle
      - Applause
      - Juggle
    state_topic: home/ledeffect
    availability_topic: home/ledeffect/availability
    command_topic: home/ledeffect/set
    command_on_template: >
      {"state": "ON"
      {%- if brightness is defined -%}
      , "brightness": {{ brightness }}
      {%- endif -%}
      {%- if transition is defined -%}
      , "brightness_rate": {{ transition * 4 }}
      {%- endif -%}
      {%- if red is defined and green is defined and blue is defined -%}
      , "effect": {"name": "solid", "color_rgb": [{{ red }}, {{ green }}, {{ blue }}]}
      {%- elif effect is defined -%}
      , "effect": {"name": "{{ effect | lower }}"
        {%- if effect == 'Solid' and transition is defined -%}
        , "rate": {{ transition * 4 }}
        {%- endif -%}
      }
      {%- endif -%}
      }
    command_off_template: '{"state": "OFF"}'
    state_template: '{{ value_json.state | lower }}'
    brightness_template: '{{ value_json.brightness }}'
    red_template: '{{ value_json.effect.color_rgb[0] if value_json.effect.name == "solid" else value_json.brightness }}'
    green_template: '{{ value_json.effect.color_rgb[1] if value_json.effect.name == "solid" else value_json.brightness }}'
    blue_template: '{{ value_json.effect.color_rgb[2] if value_json.effect.name == "solid" else value_json.brightness }}'
    effect_template: '{{ value_json.effect.name | capitalize }}'
 *
 * Enjoy!
 *
 */
// Libraries
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <PubSubClient.h>
#include <Ticker.h>
#include <ArduinoJson.h>
#include <FastLED.h>
#include <LEDEffect.h>
#include <DHTNew.h>
#include <ArduinoOTA.h>

// Config
//#define DEBUG
#define CONFIG_WIFI  // CHANGEME (do this once then you can remove wifi password from the code)
ADC_MODE(ADC_VCC);
#define DHT_PIN   2  // CHANGEME (comment to disable DHT)
#define DATA_PIN  14  // CHANGEME
#define NUM_LEDS  90  // CHANGEME

// WiFi
const char* ssid = "";  // CHANGEME
const char* password = "";  // CHANGEME (if CONFIG_WIFI is defined)
WiFiClient wifiClient;
const char* hostname_ = "";  // CHANGEME
#ifdef DEBUG
bool previousIsConnected = false;
#endif

// OTA
const char* OTAPassword = "";  // CHANGEME

// Server
ESP8266WebServer server(80);

// MQTT
const char* mqttId = "ledeffect";  // CHANGEME (or not)
const char* mqttTopicState = "home/ledeffect";  // CHANGEME (or not)
const char* mqttTopicAvailability = "home/ledeffect/availability";  // CHANGEME (or not)
const char* mqttTopicSet = "home/ledeffect/set";  // CHANGEME (or not)
const char* mqttTopicTemperature = "home/ledeffect/temperature";  // CHANGEME (or not)
const char* mqttTopicHumidity = "home/ledeffect/humidity";  // CHANGEME (or not)
PubSubClient mqttClient("mqtt", 1883, wifiClient);  // CHANGEME

// Data buffer
const size_t dataSize = 800;
char data[dataSize];
size_t dataLength = 0;

// DHT
#ifdef DHT_PIN
DHT dht(DHT_PIN, DHT_MODEL_DHT22);
Ticker publishDHTTicker;
volatile bool publishDHT = false;
unsigned int dhtReadings = 0;
unsigned int dhtErrors = 0;
#endif

// Strip
CRGB leds[NUM_LEDS];
BaseEffect* effects[] = {  // CHANGEME (you can add as many effects as you want with a unique name)
  new RainbowEffect("rainbow"),
  new SolidEffect("solid"),
  new TwinkleEffect<NUM_LEDS>("twinkle"),
  new ApplauseEffect("applause"),
  new JuggleEffect("juggle")
};
const uint8_t effectCount = 5;  // CHANGEME
ColorStrip strip(effects, effectCount);

#ifdef DEBUG
 #ifndef DEBUG_PRINTER
 #define DEBUG_PRINTER Serial
 #endif
 #define DEBUG_PRINT(...)    DEBUG_PRINTER.print(__VA_ARGS__)
 #define DEBUG_PRINTLN(...)  DEBUG_PRINTER.println(__VA_ARGS__)
 #define DEBUG_PRINTF(...)   DEBUG_PRINTER.printf(__VA_ARGS__)
#else
 #define DEBUG_PRINT(...)
 #define DEBUG_PRINTLN(...)
 #define DEBUG_PRINTF(...)
#endif


#ifdef DHT_PIN
void readDHT() {
  if (dht.read()) {
    dhtReadings++;
    if (dht.getError() != DHT_ERROR_NONE) {
      DEBUG_PRINT(F("DHT: Error "));
      DEBUG_PRINTLN(dht.getErrorString());
      dhtErrors++;
      return;
    }

    DEBUG_PRINT(F("DHT: Temperature "));
    DEBUG_PRINT(dht.getTemperature());
    DEBUG_PRINTLN(F("°C"));
    DEBUG_PRINT(F("DHT: Humidity "));
    DEBUG_PRINT(dht.getHumidity());
    DEBUG_PRINTLN('%');
  }
}

void handleDHTGet() {
  const size_t bufferSize = JSON_OBJECT_SIZE(5);
  StaticJsonBuffer<bufferSize> jsonBuffer;
  char message[100];

  // get data
  readDHT();

  // create JSON
  JsonObject& root = jsonBuffer.createObject();
  if (isnan(dht.getTemperature()))
    root["temperature"] = (char*)0;
  else
    root["temperature"] = dht.getTemperature();
  if (isnan(dht.getHumidity()))
    root["humidity"] = (char*)0;
  else
    root["humidity"] = dht.getHumidity();
  root["error"] = dht.getErrorString();
  root["readings"] = dhtReadings;
  root["errors"] = dhtErrors;

  // send response
  root.printTo(message, sizeof(message));
  server.send(200, "application/json", message);
}
#endif

void handleInfoGet() {
  const size_t bufferSize = JSON_OBJECT_SIZE(11) + JSON_OBJECT_SIZE(5) + \
    JSON_OBJECT_SIZE(14) + JSON_ARRAY_SIZE(2);
  StaticJsonBuffer<bufferSize> jsonBuffer;

  // create JSON
  JsonObject& root = jsonBuffer.createObject();
  root["id"] = ESP.getChipId();
  root["free_heap"] = ESP.getFreeHeap();
  root["sdk_version"] = ESP.getSdkVersion();
  root["boot_version"] = ESP.getBootVersion();
  root["boot_mode"] = ESP.getBootMode();
  root["vcc"] = ESP.getVcc() / 1024.00;
  root["cpu_freq"] = ESP.getCpuFreqMHz();
  root["sketch_size"] = ESP.getSketchSize();
  root["sketch_free_space"] = ESP.getFreeSketchSpace();

  JsonObject& flash_chip = root.createNestedObject("flash_chip");
  flash_chip["id"] = ESP.getFlashChipId();
  flash_chip["size"] = ESP.getFlashChipSize();
  flash_chip["real_size"] = ESP.getFlashChipRealSize();
  flash_chip["speed"] = ESP.getFlashChipSpeed();
  FlashMode_t flashChipMode = ESP.getFlashChipMode();
  if (flashChipMode == FM_QIO)
    flash_chip["mode"] = "qio";
  else if (flashChipMode == FM_QOUT)
    flash_chip["mode"] = "qout";
  else if (flashChipMode == FM_DIO)
    flash_chip["mode"] = "dio";
  else if (flashChipMode == FM_DOUT)
    flash_chip["mode"] = "dout";
  else if (flashChipMode == FM_UNKNOWN)
    flash_chip["mode"] = "unknown";

  JsonObject& wifi = root.createNestedObject("wifi");
  wifi["mac"] = WiFi.macAddress();
  wifi["ssid"] = WiFi.SSID();
  wifi["bssid"] = WiFi.BSSIDstr();
  wifi["rssi"] = WiFi.RSSI();
  wifi["channel"] = WiFi.channel();
  WiFiMode_t wifiMode = WiFi.getMode();
  if (wifiMode == WIFI_OFF)
    wifi["mode"] = "off";
  else if (wifiMode == WIFI_STA)
    wifi["mode"] = "sta";
  else if (wifiMode == WIFI_AP)
    wifi["mode"] = "ap";
  else if (wifiMode == WIFI_AP_STA)
    wifi["mode"] = "ap_sta";
  WiFiPhyMode_t wifiPhyMode = WiFi.getPhyMode();
  if (wifiPhyMode == WIFI_PHY_MODE_11B)
    wifi["phy_mode"] = "11b";
  else if (wifiPhyMode == WIFI_PHY_MODE_11G)
    wifi["phy_mode"] = "11g";
  else if (wifiPhyMode == WIFI_PHY_MODE_11N)
    wifi["phy_mode"] = "11n";
  WiFiSleepType_t wifiSleepMode = WiFi.getSleepMode();
  if (wifiSleepMode == WIFI_NONE_SLEEP)
    wifi["sleep_mode"] = "none";
  else if (wifiSleepMode == WIFI_LIGHT_SLEEP)
    wifi["sleep_mode"] = "light";
  else if (wifiSleepMode == WIFI_MODEM_SLEEP)
    wifi["sleep_mode"] = "modem";
  wifi["persistent"] = WiFi.getPersistent();
  wifi["ip"] = WiFi.localIP().toString();
  wifi["hostname"] = WiFi.hostname();
  wifi["subnet_mask"] = WiFi.subnetMask().toString();
  wifi["gateway_ip"] = WiFi.gatewayIP().toString();
  JsonArray& dns = wifi.createNestedArray("dns");
  dns.add(WiFi.dnsIP(0).toString());
  dns.add(WiFi.dnsIP(1).toString());

  // send response
  String response;
  root.printTo(response);
  server.send(200, "application/json", response);
}

void handleEffectsGet() {
  const size_t bufferSize = JSON_ARRAY_SIZE(effectCount);
  StaticJsonBuffer<bufferSize> jsonBuffer;

  // create JSON
  JsonArray& root = jsonBuffer.createArray();
  for (size_t i = 0; i < effectCount; i++) {
    root.add(effects[i]->name);
  }

  // send response
  String response;
  root.printTo(response);
  server.send(200, "application/json", response);
}

void handleLedsGet() {
  // send response
  String response;
  strip.printTo(response);
  server.send(200, "application/json", response);
}

void handleLedsPost() {
  // check request
  if (!server.hasArg("plain")) {
    DEBUG_PRINTLN(F("REST: Missing body"));
    server.send(400, "text/plain", "Missing body");
    return;
  }

  // deserialize
  server.arg("plain").toCharArray(data, dataSize);
  if (!strip.deserialize(data)) {
    DEBUG_PRINTLN(F("REST: Deserialize failed"));
    server.send(400, "text/plain", "Could not parse the body");
    return;
  }

  // serialize
  dataLength = strip.printTo(data, dataSize);

  // send the response to the client
  server.send(201, "application/json", data);
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  // set
  if (strcmp(topic, mqttTopicSet) == 0) {
    if (!strip.deserialize((char*)payload)) {
      DEBUG_PRINTLN(F("MQTT: Deserialize failed"));
      return;
    }

    // serialize
    dataLength = strip.printTo(data, dataSize);

    DEBUG_PRINT(F("Strip: Data buffer "));
    DEBUG_PRINT(dataLength);
    DEBUG_PRINT(F("/"));
    DEBUG_PRINTLN(dataSize);
  }
}


void setup() {
  // Serial
  Serial.begin(115200);
  delay(2000);
  DEBUG_PRINTLN(F("Starting..."));

  // WiFi
  WiFi.persistent(true);
  WiFi.mode(WIFI_STA);
  WiFi.setOutputPower(0);
  WiFi.setSleepMode(WIFI_NONE_SLEEP);
  WiFi.setAutoConnect(true);
  WiFi.setAutoReconnect(true);
  WiFi.hostname(hostname_);
#ifdef CONFIG_WIFI
  WiFi.begin(ssid, password);
#endif

  // OTA
  ArduinoOTA.setHostname(hostname_);
  ArduinoOTA.setPassword(OTAPassword);
  ArduinoOTA.onStart([]() {
#ifdef DEBUG
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
      type = "sketch";
    else // U_SPIFFS
      type = "filesystem";
    DEBUG_PRINTLN("OTA: Start " + type);
#endif
  });
  ArduinoOTA.onEnd([]() {
    DEBUG_PRINTLN();
    DEBUG_PRINTLN(F("OTA: End"));
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    DEBUG_PRINTF("OTA: Progress %u%%\r", (progress / (total / 100)));
    // TODO: led status
  });
  ArduinoOTA.onError([](ota_error_t error) {
#ifdef DEBUG
    DEBUG_PRINTF("OTA: Error[%u] ", error);
    if (error == OTA_AUTH_ERROR) DEBUG_PRINTLN(F("Auth Failed"));
    else if (error == OTA_BEGIN_ERROR) DEBUG_PRINTLN(F("Begin Failed"));
    else if (error == OTA_CONNECT_ERROR) DEBUG_PRINTLN(F("Connect Failed"));
    else if (error == OTA_RECEIVE_ERROR) DEBUG_PRINTLN(F("Receive Failed"));
    else if (error == OTA_END_ERROR) DEBUG_PRINTLN(F("End Failed"));
#endif
    ESP.restart();
  });
  ArduinoOTA.begin();

  // Strip
  strip.begin(&FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS));  // CHANGEME

  // Server
#ifdef DHT_PIN
  server.on("/dht", HTTP_GET, handleDHTGet);
#endif
  server.on("/info", HTTP_GET, handleInfoGet);
  server.on("/effects", HTTP_GET, handleEffectsGet);
  server.on("/leds", HTTP_GET, handleLedsGet);
  server.on("/leds", HTTP_POST, handleLedsPost);
  server.begin();

  // DHT
#ifdef DHT_PIN
  dht.begin();
  publishDHTTicker.attach(60, [] () { publishDHT = true; });
#endif
}


void loop() {
  // WiFi
#ifdef DEBUG
  if (WiFi.isConnected() && !previousIsConnected) {
    DEBUG_PRINTLN(F("WiFi: Connected"));
    DEBUG_PRINT(F("WiFi: IP address "));
    DEBUG_PRINTLN(WiFi.localIP());
  } else if (previousIsConnected && !WiFi.isConnected()) {
    DEBUG_PRINTLN(F("WiFi: Disconnected"));
  }
  previousIsConnected = WiFi.isConnected();
#endif
  if (!WiFi.isConnected()) {
    return;
  }

  // OTA
  ArduinoOTA.handle();

  // Strip
  strip.loop();

  // Server
  server.handleClient();

  // MQTT
  if (!mqttClient.connected()) {
    DEBUG_PRINTLN(F("MQTT: Connecting..."));
    // TODO: led status
    if (mqttClient.connect(mqttId, mqttTopicAvailability, 1, true, "offline")) {
      mqttClient.publish(mqttTopicAvailability, "online", true);
      DEBUG_PRINTLN(F("MQTT: Connected"));
      // setup callback and subscriptions
      mqttClient.setCallback(mqttCallback);
      mqttClient.subscribe(mqttTopicSet);
    } else {
      DEBUG_PRINTLN(F("MQTT: Not connected"));
      return;
    }
  }
  mqttClient.loop();

  // publish
  if (dataLength) {
    mqttClient.publish(mqttTopicState, (byte*)data, dataLength, true);
    dataLength = 0;
    DEBUG_PRINTLN(F("MQTT: Published"));
  }

#ifdef DHT_PIN
  if (publishDHT) {
    unsigned int size = 50;
    char payload[size];

    // read sensor
    readDHT();

    if (dht.getError() == DHT_ERROR_NONE) {
      // Temperature
      String(dht.getTemperature(), 1).toCharArray(payload, size);
      mqttClient.publish(mqttTopicTemperature, payload);

      // Humidity
      String(dht.getHumidity(), 1).toCharArray(payload, size);
      mqttClient.publish(mqttTopicHumidity, payload);
    }

    DEBUG_PRINTLN(F("MQTT: Published"));
    publishDHT = false;
  }
#endif
}
