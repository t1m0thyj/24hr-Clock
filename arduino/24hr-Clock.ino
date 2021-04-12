#include <Adafruit_IS31FL3731.h>
#include <ezTime.h>
#include <WiFiEsp.h>
#include "arduino_secrets.h"

// Emulate Serial1 on pins 6/7 if not present
#ifndef HAVE_HWSERIAL1
#include "SoftwareSerial.h"
SoftwareSerial Serial1(6, 7); // RX, TX
#endif

char ssid[] = SECRET_SSID;            // your network SSID (name)
char pass[] = SECRET_PASS;        // your network password
int status = WL_IDLE_STATUS;     // the Wifi radio's status

const int ldrPin = A0;
const char timezone[] = "America/New_York";

const byte digits[][10] = {
  { B111, B001, B111, B111, B101, B111, B111, B111, B111, B111 },
  { B101, B001, B001, B001, B101, B100, B100, B001, B101, B101 },
  { B101, B001, B001, B001, B101, B100, B100, B001, B101, B101 },
  { B101, B001, B111, B111, B111, B111, B111, B001, B111, B111 },
  { B101, B001, B100, B001, B001, B001, B101, B001, B101, B001 },
  { B101, B001, B100, B001, B001, B001, B101, B001, B101, B001 },
  { B111, B001, B111, B111, B001, B111, B111, B001, B111, B111 }
};

const uint8_t PROGMEM gamma8[] = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,
    1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,
    2,  3,  3,  3,  3,  3,  3,  3,  4,  4,  4,  4,  4,  5,  5,  5,
    5,  6,  6,  6,  6,  7,  7,  7,  7,  8,  8,  8,  9,  9,  9, 10,
   10, 10, 11, 11, 11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16,
   17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 24, 24, 25,
   25, 26, 27, 27, 28, 29, 29, 30, 31, 32, 32, 33, 34, 35, 35, 36,
   37, 38, 39, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 50,
   51, 52, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 66, 67, 68,
   69, 70, 72, 73, 74, 75, 77, 78, 79, 81, 82, 83, 85, 86, 87, 89,
   90, 92, 93, 95, 96, 98, 99,101,102,104,105,107,109,110,112,114,
  115,117,119,120,122,124,126,127,129,131,133,135,137,138,140,142,
  144,146,148,150,152,154,156,158,160,162,164,167,169,171,173,175,
  177,180,182,184,186,189,191,193,196,198,200,203,205,208,210,213,
  215,218,220,223,225,228,231,233,236,239,241,244,247,249,252,255 };

Adafruit_IS31FL3731 matrix = Adafruit_IS31FL3731(17, 7);
Timezone myTz;

bool showColon = false;
int brightness = 0;
int ldrValue = 0;
int msPrev = 0;
int timePrev = -1;

int pixelAddr(int x, int y) {
  if (x > 8) {
    x = x - 8;
    y = 6 - (y + 8);
  } else {
    x = 8 - x;
  }
  return x * 16 + y;
}

int readPWM() {
  int tempLdrValue = analogRead(ldrPin) / 4;
  if (abs(tempLdrValue - ldrValue) >= (ldrValue / 16)) {
    ldrValue = tempLdrValue;
  }
  return max(1, pgm_read_byte(&gamma8[ldrValue]));
}

void drawDigit(int x, int y, int digit, int pwm) {
  for (int y2 = 0; y2 < 7; y2++) {
    for (int x2 = 0; x2 < 3; x2++) {
      bool isOn = (digits[y2][digit] >> x2) & 1;
      matrix.setLEDPWM(pixelAddr(x + 2 - x2, y + 6 - y2), pwm * isOn);
    }
  }
}

void setup()
{
  // turn off bright yellow light
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  // initialize serial for debugging
  Serial.begin(115200);
  // initialize serial for ESP module
  Serial1.begin(9600);
  // initialize ESP module
  WiFi.init(&Serial1);

  // check for the presence of the shield
  if (WiFi.status() == WL_NO_SHIELD) {
    digitalWrite(LED_BUILTIN, HIGH);
    Serial.println("WiFi shield not present");
    // don't continue
    while (true);
  }

  // attempt to connect to WiFi network
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network
    status = WiFi.begin(ssid, pass);
  }

  // you're connected now, so print out the data
  Serial.println("You're connected to the network");

  if (!matrix.begin()) {
    digitalWrite(LED_BUILTIN, HIGH);
    Serial.println("Failed to initialize LED matrix");
    while (true);
  }
  
  waitForSync();
  myTz.setLocation(timezone);
  Serial.println("Current local time: " + myTz.dateTime());
}

void loop()
{
  int msNow = (myTz.now() * 1000 + ms(LAST_READ)) % 1000;
  int timeNow = myTz.hour() * 60 + myTz.minute();
  bool needsDraw = timeNow != timePrev;

  if (needsDraw || ((msNow != msPrev) && (msNow % 10 == 0))) {
    int brightnessPrev = brightness;
    brightness = readPWM();
    if (brightness != brightnessPrev) {
      needsDraw = true;
    }
  }

  if (needsDraw) {
    int hourNow = myTz.hour();
    int minuteNow = myTz.minute();
    drawDigit(0, 0, hourNow / 10, brightness);
    drawDigit(4, 0, hourNow % 10, brightness);
    drawDigit(10, 0, minuteNow / 10, brightness);
    drawDigit(14, 0, minuteNow % 10, brightness);
  }

  bool tempShowColon = msNow < 500;
  if (needsDraw || (tempShowColon != showColon)) {
    showColon = tempShowColon;
    matrix.setLEDPWM(pixelAddr(8, 1), brightness * showColon);
    matrix.setLEDPWM(pixelAddr(8, 5), brightness * showColon);
  }

  if (minuteChanged() && (WiFi.status() != WL_CONNECTED)) {
    WiFi.disconnect();
    WiFi.begin(ssid, pass);
  }

  msPrev = msNow;
  timePrev = timeNow;
  events();
}
