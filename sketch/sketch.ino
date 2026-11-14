// Климат-станция: Arduino Uno
// BME280 + OLED SSD1306 128x64 (общая шина I2C: A4 = SDA, A5 = SCL)
// Кнопка звука на D5 (к GND), активный зуммер через BC547 на D7

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_BME280.h>

#define PIN_SOUND_SW 5   // кнопка между D5 и GND: нажата = LOW = звук разрешён
#define PIN_BUZZER 7     // через резистор 1 кОм на базу BC547: HIGH = пищит

// Пороги тревоги
const float TEMP_MAX = 30.0;   // °C
const float HUM_MAX = 70.0;    // %

Adafruit_SSD1306 display(128, 64, &Wire, -1);
Adafruit_BME280 bme;

unsigned long lastRead = 0;
unsigned long lastBeep = 0;
bool alarmActive = false;

void setup() {
  Serial.begin(115200);
  pinMode(PIN_SOUND_SW, INPUT_PULLUP);
  pinMode(PIN_BUZZER, OUTPUT);
  digitalWrite(PIN_BUZZER, LOW);

  Wire.begin();

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("OLED not found (check SDA/SCL, address 0x3C)"));
    while (true) delay(1000);
  }
  // Адрес BME280 зависит от модуля: 0x76 или 0x77
  if (!bme.begin(0x76) && !bme.begin(0x77)) {
    Serial.println(F("BME280 not found"));
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println(F("BME280 not found"));
    display.display();
    while (true) delay(1000);
  }
}

void loop() {
  unsigned long now = millis();
  bool soundOn = (digitalRead(PIN_SOUND_SW) == LOW);

  if (now - lastRead >= 2000) {
    lastRead = now;
    float t = bme.readTemperature();
    float h = bme.readHumidity();
    float p = bme.readPressure() / 100.0F;   // гПа
    alarmActive = (t > TEMP_MAX) || (h > HUM_MAX);

    Serial.print(F("T="));  Serial.print(t, 1);
    Serial.print(F(" H=")); Serial.print(h, 0);
    Serial.print(F(" P=")); Serial.print(p, 0);
    Serial.print(F(" alarm=")); Serial.print(alarmActive);
    Serial.print(F(" sound=")); Serial.println(soundOn);

    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(2);
    display.setCursor(0, 0);
    display.print(t, 1);
    display.print(F(" C"));
    display.setCursor(0, 22);
    display.print(h, 0);
    display.print(F(" %"));
    display.setTextSize(1);
    display.setCursor(0, 44);
    display.print(p, 0);
    display.print(F(" hPa"));
    display.setCursor(0, 56);
    display.print(soundOn ? F("SOUND ON") : F("MUTE"));
    if (alarmActive) {
      display.setCursor(90, 56);
      display.print(F("ALARM!"));
    }
    display.display();
  }

  // Короткий писк раз в 2 секунды, только если тревога и звук разрешён
  if (alarmActive && soundOn) {
    if (now - lastBeep >= 2000) {
      lastBeep = now;
      digitalWrite(PIN_BUZZER, HIGH);
    } else if (now - lastBeep >= 200) {
      digitalWrite(PIN_BUZZER, LOW);
    }
  } else {
    digitalWrite(PIN_BUZZER, LOW);
  }
}
