#include <ESP8266WiFi.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include "ThingSpeak.h"

 
LiquidCrystal_I2C lcd(0x27, 16, 2);

 
#define DHTPIN D4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

#define MQ135_PIN A0   

 
#define RED_LED D5
#define GREEN_LED D7
#define BUZZER D8

 
const char* ssid = "wifi-name";         
const char* password = "wifipassword";

 
WiFiClient client;
unsigned long myChannelNumber = 2914014;
const char* myWriteAPIKey = "FUKS7HFFE143YG2H";

String getAirQualityStatus(int value) {
  if (value < 300) return "Good";
  else if (value < 500) return "Moderate";
  else return "Poor";
}

void setup() {
  Serial.begin(115200);

  
  lcd.begin();
  lcd.backlight();

  
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  
  dht.begin();

  
  lcd.setCursor(0, 0);
  lcd.print("Project by:");
  lcd.setCursor(0, 1);
  lcd.print("20049 & 20105");
  delay(4000);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Connecting WiFi...");

  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");

  lcd.setCursor(0, 1);
  lcd.print("WiFi connected");
  delay(2000);

  ThingSpeak.begin(client);
}

void loop() {
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();
  int gas = analogRead(MQ135_PIN);

  
  if (isnan(temp) || isnan(hum)) {
    Serial.println("Failed to read from DHT sensor!");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Sensor Error");
    delay(2000);
    return;
  }

  String airStatus = getAirQualityStatus(gas);

  
  Serial.print("Temp: "); Serial.print(temp); Serial.print(" °C, ");
  Serial.print("Humidity: "); Serial.print(hum); Serial.print(" %, ");
  Serial.print("Gas: "); Serial.print(gas); Serial.print(" (");
  Serial.print(airStatus); Serial.println(")");

  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("T:"); lcd.print(temp); lcd.print("C ");
  lcd.print("H:"); lcd.print(hum); lcd.print("%");

  lcd.setCursor(0, 1);
  lcd.print("Air: "); lcd.print(airStatus);

  
  if (gas > 500) {
    digitalWrite(RED_LED, HIGH);
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(BUZZER, HIGH);
  } else {
    digitalWrite(RED_LED, LOW);
    digitalWrite(GREEN_LED, HIGH);
    digitalWrite(BUZZER, LOW);
  }

  
  ThingSpeak.setField(1, temp);
  ThingSpeak.setField(2, hum);
  ThingSpeak.setField(3, gas);

  int response = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  if (response == 200) {
    Serial.println("Data sent to ThingSpeak!");
    tone(BUZZER, 1000, 200);   
  } else {
    Serial.print("Error sending data. Code: ");
    Serial.println(response);
  }

  delay(20000); 
}
