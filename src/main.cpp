#include "Adafruit_NeoPixel.h"
#include "ESPNowW.h"
#include "WiFi.h"
#include <esp_wifi.h>
#include "Preferences.h"

// NeoPixel attributes
#define NEOPIXEL_PIN 27    // Pin for NeoPixel strip
#define NUM_PIXELS 50

#define DEFAULT_RED       140
#define DEFAULT_GREEN     0
#define DEFAULT_BLUE      255
#define DEFAULT_POWER     1

void readMacAddress(){
  uint8_t baseMac[6];
  esp_err_t ret = esp_wifi_get_mac(WIFI_IF_STA, baseMac);
  if (ret == ESP_OK) {
    Serial.printf("%02x:%02x:%02x:%02x:%02x:%02x\n",
                  baseMac[0], baseMac[1], baseMac[2],
                  baseMac[3], baseMac[4], baseMac[5]);
  } else {
    Serial.println("Failed to read MAC address");
  }
}

// State management setup
Adafruit_NeoPixel LED(NUM_PIXELS, NEOPIXEL_PIN, NEO_BRG + NEO_KHZ800);
Preferences prefs;

int glow_red;
int glow_green;
int glow_blue;
int glow_power;

typedef struct message_struct {
  int red;
  int green;
  int blue;
  int power;
} message_struct;

message_struct myData;

void setLedColor() {
  Serial.print("Red: ");
  Serial.println(glow_red);
  Serial.print("Green: ");
  Serial.println(glow_green);
  Serial.print("Blue: ");
  Serial.println(glow_blue);
  for (int ledNumber = 0; ledNumber < NUM_PIXELS; ledNumber++) {
    if (glow_power == 1) {
      LED.setPixelColor(ledNumber, glow_red, glow_green, glow_blue);
    } else {
      LED.setPixelColor(ledNumber, 0, 0, 0);
    }
    
    LED.show();
  } 
}

void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len) {
  memcpy(&myData, incomingData, sizeof(myData));
  
  Serial.print("Data Recieved");

  glow_red = myData.red;
  glow_green = myData.green;
  glow_blue = myData.blue;
  glow_power = myData.power;

  prefs.putInt("glowRed", glow_red);
  prefs.putInt("glowGree", glow_green);
  prefs.putInt("glowBlue", glow_blue);
  prefs.putInt("glowPower", glow_power);
  
  setLedColor();
}

void setup() {
  Serial.begin(115200);
  
  WiFi.mode(WIFI_STA);

  readMacAddress();

  if (esp_now_init() == ESP_OK) {
    Serial.println("ESP Now Started");
  } else {
    Serial.println("Error starting ESP Now");
    return;
  }

  esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));

  // Initialize buttons
  pinMode(NEOPIXEL_PIN, OUTPUT);
  LED.begin();
  prefs.begin("glow-app", false);

  glow_red = prefs.getInt("glowRed", DEFAULT_RED);
  glow_green = prefs.getInt("glowGreen", DEFAULT_GREEN);
  glow_blue = prefs.getInt("glowBlue", DEFAULT_BLUE);
  glow_power = prefs.getInt("glowPower", DEFAULT_POWER);

  setLedColor();
}

void loop() {
  delay(100); //debounce
}