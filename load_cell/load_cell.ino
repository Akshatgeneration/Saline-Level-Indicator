#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include "HX711.h"

#define DT  5
#define SCK 4

HX711 scale;
const float calibration_factor = 205.0;

const char* ssid = "";         //Enter Wifi name
const char* password = "";     //Enter wifi password

#define BOT_TOKEN ""           //Enter Telegram Bot token
#define CHAT_ID ""             //Enter Telegram Chat ID

WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);

bool messageSent = false;
unsigned long lastCheckTime = 0;
const unsigned long checkInterval = 1000; // 1 second

void setup() {
  Serial.begin(9600);
  WiFi.begin(ssid, password);
  client.setInsecure();  // Skip certificate validation

  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");

  scale.begin(DT, SCK);
  scale.set_scale(calibration_factor);
  scale.tare();
}

void loop() {
  float weight = scale.get_units(5);
  Serial.print("Weight: ");
  Serial.print(weight, 2);
  Serial.println(" g");

  // Telegram alert if weight drops below threshold
  if (weight < 50.0 && !messageSent) {
    String msg = "⚠️ Alert: Weight dropped below 200g!\nCurrent weight: " + String(weight, 2) + " g";
    bot.sendMessage(CHAT_ID, msg, "");
    messageSent = true;
  }

  if (weight >= 200.0) {
    messageSent = false;
  }

  // Check Telegram messages every 1 second
  if (millis() - lastCheckTime > checkInterval) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while (numNewMessages) {
      for (int i = 0; i < numNewMessages; i++) {
        String text = bot.messages[i].text;
        String chat_id = bot.messages[i].chat_id;

        if (text.equalsIgnoreCase("Hi")) {
          String response = "✅ Current weight: " + String(weight, 2) + " g";
          bot.sendMessage(chat_id, response, "");
        }
      }
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }

    lastCheckTime = millis();
  }

  delay(500);
}
