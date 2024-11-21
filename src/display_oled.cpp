#include "display_oled.h"

Adafruit_SH1106 display(-1);

// Timer variables
unsigned long currentMillis_wifi = millis();
unsigned long previousMillis_wifi;
unsigned long notifStartTime = 0;

// Toggles
bool displayWiFiName = true;
bool isNotif = false;

std::queue<String> notificationQueue;
String notif_text;

// 8x8 Bitmap Icons
const uint8_t wifiIcon[] PROGMEM = {
    0b00011000,
    0b00100100,
    0b01000010,
    0b10000001,
    0b10011001,
    0b00000000,
    0b00011000,
    0b00000000
};
const uint8_t checkmarkIcon[] PROGMEM = {
    0b00000000,
    0b00000000,
    0b00000001,
    0b00000010,
    0b00000100,
    0b10001000,
    0b01010000,
    0b00100000
};
const uint8_t heaterIcon[] PROGMEM = {
    0b00101000, //   ░█░█░░░
    0b01010100, //  ░█░█░█░
    0b00101000, //   ░█░█░░░
    0b01010100, //  ░█░█░█░
    0b00101000, //   ░█░█░░░
    0b01010100, //  ░█░█░█░
    0b00101000, //   ░█░█░░░
    0b01010100  //  ░█░█░█░
};

const uint8_t fanIcon[] PROGMEM = {
    0b00011000, //   ░░██░░░░
    0b00111100, //   ░████░░░
    0b01100110, //  ░██░░██░░
    0b11000011, // ██░░░░░██░
    0b11000011, // ██░░░░░██░
    0b01100110, //  ░██░░██░░
    0b00111100, //   ░████░░░
    0b00011000  //   ░░██░░░░
};

void display_addNotification(String text) {
    notificationQueue.push(text);
    if (!isNotif) {
        notif_text = notificationQueue.front();
        notificationQueue.pop();
        isNotif = true;
        notifStartTime = millis();
    }
}

// Function to format the time
String formatTime(unsigned long duration) {
  unsigned long seconds = duration / 1000;
  unsigned long minutes = seconds / 60;
  unsigned long hours = minutes / 60;
  seconds %= 60;
  minutes %= 60;

  char timeStr[10];
  sprintf(timeStr, "%02lu:%02lu:%02lu", hours, minutes, seconds);
  return String(timeStr);
}


void display_begin() {
    display.begin(SH1106_I2C_ADDRESS, 0x3C); // Default I2C address
    display.display();
    delay(2000); // Show splash screen
    display.clearDisplay();
}

void display_topBar() {
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(10, 0);

    currentMillis_wifi = millis();
    if (currentMillis_wifi - previousMillis_wifi >= 5000) {
        previousMillis_wifi = currentMillis_wifi;
        displayWiFiName = !displayWiFiName;
    }

    // Left-side WiFi status
    display.drawBitmap(0, 0, wifiIcon, 8, 8, WHITE);
    if (displayWiFiName) {
        String limitedSSID = ssid.substring(0, 80); // Limit SSID to 80 characters
        display.print(limitedSSID);
    } else {
        display.print(ip);
    }

    // Right-side relay status
    if (digitalRead(RELAY_HEAT) == LOW) {
        display.drawBitmap(100, 0, heaterIcon, 8, 8, WHITE);
    }
    if (digitalRead(RELAY_FAN) == LOW) {
        display.drawBitmap(112, 0, fanIcon, 8, 8, WHITE);
    }
}

void display_bottomBar() {
    display.setTextSize(1); display.setTextColor(WHITE);
    display.setCursor(0, 40);
    display.print(temperature, 1); display.print("C");
    display.setCursor(0, 50);
    display.print("Temp");

    display.setCursor(40, 40);
    display.print(humidity, 1); display.print("%");
    display.setCursor(40, 50);
    display.print("Hum");

    display.setCursor(80, 40);
    display.print(weight, 1); display.print("g");
    display.setCursor(80, 50);
    display.print("Weight");
}

void display_main() {
    display.setTextSize(2);
    display.setTextColor(WHITE);

    // Check if 5 seconds have passed since the notification was set
    if (isNotif && (millis() - notifStartTime >= 5000)) {
        isNotif = false;
        if (!notificationQueue.empty()) {
            notif_text = notificationQueue.front();
            notificationQueue.pop();
            isNotif = true;
            notifStartTime = millis();
        }
    }

    String text;
    if (isNotif) {
        text = notif_text;
    } else if (!timerRunning && !isNotif) {
        text = "SYS IDLE";
    } else if (timerRunning && !isNotif) {
        unsigned long remainingTime = getRemainingTime();
        text = formatTime(remainingTime);
    }

    int16_t x1, y1;
    uint16_t w, h;
    display.getTextBounds(text, 0, 0, &x1, &y1, &w, &h); // Calculate text bounds
    int16_t x = (128 - w) / 2; // Center the text horizontally
    display.setCursor(x, 16); // Adjusted cursor position to align with the bitmap
    display.print(text);
}

void display_loop() {
    display_topBar();
    display_bottomBar();
    display_main();

    display.display();
    display.clearDisplay();
}