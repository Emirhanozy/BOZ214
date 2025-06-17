#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>

#define DHTPIN 2
#define DHTTYPE DHT11
#define LED_SICAKLIK 4
#define LED_NEM 5
#define LED_HAVA 6
#define BUZZER_PIN 9
#define BUTTON_PIN 7
#define MQ135_PIN A0

DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);

unsigned long lastSwitchTime = 0;
unsigned long lastBlinkTime = 0;
bool ledSicaklikState = false;
bool ledNemState = false;
bool ledHavaState = false;

int currentPage = 0;
int lastButtonState = HIGH;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;

void setup() {
  lcd.init();
  lcd.backlight();
  dht.begin();

  pinMode(LED_SICAKLIK, OUTPUT);
  pinMode(LED_NEM, OUTPUT);
  pinMode(LED_HAVA, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  lcd.setCursor(0, 0);
  lcd.print("Ortam Asistani");
  lcd.setCursor(0, 1);
  lcd.print("Basliyor...");
  delay(2000);
}

void loop() {
  float sicaklik = dht.readTemperature();
  float nem = dht.readHumidity();
  int havaKalite = analogRead(MQ135_PIN);

  // LED'leri 0.5 sn aralıkla yak/söndür
  if (millis() - lastBlinkTime >= 500) {
    lastBlinkTime = millis();

    // Sıcaklık LED
    if (!isnan(sicaklik) && sicaklik > 32) {
      ledSicaklikState = !ledSicaklikState;
      digitalWrite(LED_SICAKLIK, ledSicaklikState);
    } else {
      digitalWrite(LED_SICAKLIK, LOW);
    }

    // Nem LED
    if (!isnan(nem) && (nem < 40 || nem > 60)) {
      ledNemState = !ledNemState;
      digitalWrite(LED_NEM, ledNemState);
    } else {
      digitalWrite(LED_NEM, LOW);
    }

    // Hava Kalitesi LED
    if (havaKalite > 600) {
      ledHavaState = !ledHavaState;
      digitalWrite(LED_HAVA, ledHavaState);
    } else {
      digitalWrite(LED_HAVA, LOW);
    }
  }

  // Butonla sayfa geçişi (debounce ile)
  int reading = digitalRead(BUTTON_PIN);
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading == LOW) {
      currentPage = (currentPage + 1) % 3;
      lcd.clear();
    }
  }
  lastButtonState = reading;

  // Buzzer kontrolü
  if (!isnan(sicaklik) && sicaklik > 32) {
    tone(BUZZER_PIN, 1000);  // orta ses
  } else if (!isnan(nem) && nem > 60) {
    tone(BUZZER_PIN, 1000);  // orta ses
  } else if (havaKalite > 600) {
    tone(BUZZER_PIN, 2000);  // daha yüksek ses
  } else {
    noTone(BUZZER_PIN);
  }

  // LCD ekran içeriği
  lcd.setCursor(0, 0);
  if (currentPage == 0) {
    lcd.print("Sicaklik: ");
    if (isnan(sicaklik)) {
      lcd.setCursor(0, 1);
      lcd.print("Veri Hatasi");
    } else {
      lcd.print(sicaklik, 1);
      lcd.print((char)223);
      lcd.print("C");

      lcd.setCursor(0, 1);
      if (sicaklik <= 25) {
        lcd.print("Serin - Normal");
      } else if (sicaklik <= 32) {
        lcd.print("Sicak - Normal");
      } else {
        lcd.print("Cok Sicak! Tehlike");
      }
    }
  } else if (currentPage == 1) {
    lcd.print("Nem: ");
    if (isnan(nem)) {
      lcd.setCursor(0, 1);
      lcd.print("Veri Hatasi");
    } else {
      lcd.print(nem, 1);
      lcd.print("%");

      lcd.setCursor(0, 1);
      if (nem < 40) {
        lcd.print("Cok kuru");
      } else if (nem <= 60) {
        lcd.print("Nem ideal");
      } else {
        lcd.print("Cok nemli! Risk");
      }
    }
  } else if (currentPage == 2) {
    lcd.print("Hava Kalitesi");
    lcd.setCursor(0, 1);
    lcd.print("Deger: ");
    lcd.print(havaKalite);

    if (havaKalite < 300) {
      lcd.print(" (Temiz)");
    } else if (havaKalite < 600) {
      lcd.print(" (Orta)");
    } else {
      lcd.print(" (Kirli!)");
    }
  }

  delay(100);
}