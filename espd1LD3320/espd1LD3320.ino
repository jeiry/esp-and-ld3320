#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include "Ticker.h"
#include "config.h"
#define PIN_LED BUILTIN_LED
int relayInput = PIN_LED;
boolean light = false;
bool mqttConnected = false;
#include <PubSubClient.h>
WiFiClient espClient;
PubSubClient client(espClient);

#include <SoftwareSerial.h>
SoftwareSerial uart1(14, 12); //RX=d5,TX=d6
unsigned char incomingByte;
String value;

int pixelHue;
bool rainbowOn = false;

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif
#define LED_PIN   D3
#define LED_COUNT 12
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
String unionid = "";
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  uart1.listen();
  uart1.begin(9600);

  uint64_t chipid = ESP.getChipId();
  unionid = String((uint32_t)chipid);
  delay(1000);
  Serial.print("unionid:");
  Serial.println(unionid);

  strip.begin();
  strip.setBrightness(100);

  strip.clear();
  for (int i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, strip.Color(0, 0, 0));
  }
  strip.show();

  pinMode(D1, OUTPUT);

  //初始化LittleFS文件系统
  initFileSystem();
  delay(500);
  //启动wifi
  wifiInit();
  initMqtt();
}

int setDelay = 0;
long firstPixelHue = 0;
void rainbow() {

  if (firstPixelHue < 5 * 65536) {
    firstPixelHue += 256;
  } else {
    firstPixelHue = 0;
  }
  for (int i = 0; i < strip.numPixels(); i++) {
    pixelHue = firstPixelHue + (i * 65536L / strip.numPixels());
    strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(pixelHue)));
  }
  strip.show();
  //  Serial.println(setDelay);
  delay(setDelay);

}
bool initBool = false;
void loop() {
  while (WiFi.status() == WL_CONNECTED) {
    if (initBool == false) {
      initBool = true;
    }
    startMqtt();
    if (mqttConnected == true) {
      wifiLoop();
    }
    //////

    while (uart1.available() > 0) {
      String val = uart1.readString();
      Serial.println(val);
      if (val == "LISTENING") {
        rainbowOn = true;
      } else {
        if (val == "FSYD") {
          analogWrite(D1, 200);
        } else if (val == "FSED") {
          analogWrite(D1, 280);
        } else if (val == "FSSD") {
          analogWrite(D1, 320);
        } else if (val == "GFS") {
          analogWrite(D1, 0);
        }

        if (val == "KD") {
          client.publish("/home/r/espmoto", "70");
          delay(1000);
          client.publish("/home/r/espmoto", "50");
        } else if (val == "GD") {
          client.publish("/home/r/espmoto", "30");
          delay(1000);
          client.publish("/home/r/espmoto", "50");
        }

        if (val == "KFWD") {
          client.publish("/yoyo/rgbled/i/13926995", "{\"OptType\":\"rainbow\",\"Command\":\"on\",\"ExtData\":\"5\"}");
          client.publish("/yoyo/rgbled/i/1602838", "{\"OptType\":\"rainbow\",\"Command\":\"on\",\"ExtData\":\"5\"}");
        }else if (val == "GFWD") {
          client.publish("/yoyo/rgbled/i/13926995", "{\"OptType\":\"setrgb\",\"Command\":\"off\"}");
          client.publish("/yoyo/rgbled/i/1602838", "{\"OptType\":\"setrgb\",\"Command\":\"off\"}");
        }

        for (int i = 0; i < strip.numPixels(); i++) {
          strip.setPixelColor(i, strip.Color(0, 0, 0));
        }
        strip.show();
        rainbowOn = false;
      }
    }
    if (rainbowOn == true) {
      rainbow();
    }
    delay(10);
  }

}
