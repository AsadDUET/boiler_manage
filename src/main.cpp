#include <Arduino.h>
#include <WiFi.h>
#include <LiquidCrystal_I2C.h>
#include <Firebase_ESP_Client.h>
#include "secrets.h"
#define WIFI_SSID "note5"
#define WIFI_PASSWORD "88888888"
#include <OneWire.h>
#include <DallasTemperature.h>
#define water_level_pin 34

//**********
//    Pressure
//***********
#include "HX711.h"
// HX711 circuit wiring
const int LOADCELL_DOUT_PIN = 33;
const int LOADCELL_SCK_PIN = 32;
HX711 scale;

//*********************
//  ONE WIRE
//*********************
#define ONE_WIRE_BUS 4
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
//*******************
//    FIREBASE
//******************

String path = "boilar_data/";
String Path = "";
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
//**********
//  LCD
//**********
LiquidCrystal_I2C lcd(0x27, 20, 4);
WiFiClient client;
float tempr, pressure = 0;
int water_level = 0;
int pump1 = 0;
int pump1_pin = 2;
int heater_pin = 18, heater = 0;
int buzz=25,green_m=26,green_r=13,green_l=27,red=14;
void setup()
{
  Serial.begin(115200);
  pinMode(water_level_pin, INPUT);
  pinMode(pump1_pin, OUTPUT);
  pinMode(buzz, OUTPUT);
  pinMode(green_m, OUTPUT);
  pinMode(green_l, OUTPUT);
  pinMode(green_r, OUTPUT);
  pinMode(red, OUTPUT);

  //*****Pressure
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);

  //*****LCD
  lcd.init();
  lcd.backlight();
  //******TEMPerature
  sensors.begin();
  WiFi.mode(WIFI_STA);
  if (WiFi.status() != WL_CONNECTED)
  {
    lcd.print("Attempting to connect to SSID: ");
    Serial.println(WIFI_SSID);
    while (WiFi.status() != WL_CONNECTED)
    {
      WiFi.begin(WIFI_SSID, WIFI_PASSWORD); // Connect to WPA/WPA2 network. Change this line if using open or WEP network
      Serial.print(".");
      delay(5000);
    }
    lcd.clear();
    lcd.println("Connected.");
    Serial.println("Connected.");
  }
  config.host = FIREBASE_HOST;
  config.api_key = API_KEY;
  config.signer.tokens.legacy_token = API_KEY;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
#if defined(ESP8266)
  //Set the size of WiFi rx/tx buffers in the case where we want to work with large data.
  fbdo.setBSSLBufferSize(1024, 1024);
#endif
  //Set the size of HTTP response buffers in the case where we want to work with large data.
  fbdo.setResponseSize(1024);
  //Set database read timeout to 1 minute (max 15 minutes)
  Firebase.RTDB.setReadTimeout(&fbdo, 1000 * 60);
  //tiny, small, medium, large and unlimited.
  //Size and its write timeout e.g. tiny (1s), small (10s), medium (30s) and large (60s).
  Firebase.RTDB.setwriteSizeLimit(&fbdo, "tiny");
}
// void loop()
// {
//   digitalWrite(25, HIGH);//buzz
//   delay(1000);
//   digitalWrite(26, HIGH);//green_m
//   delay(1000);
//   digitalWrite(27, HIGH);//green_l
//   delay(1000);
//   digitalWrite(14, HIGH);//red
//   delay(1000);
//   digitalWrite(13, HIGH);//green_r
//   delay(5000);
//   digitalWrite(25, LOW);
//   digitalWrite(26, LOW);
//   digitalWrite(27, LOW);
//   digitalWrite(14, LOW);
//   digitalWrite(13, LOW);
//   delay(1000);
// }


void loop()
{
  for (int i = 0; i < 5; i++)
  {
    sensors.requestTemperatures();
    tempr = sensors.getTempCByIndex(0);
    water_level = analogRead(water_level_pin);
    if (scale.wait_ready_timeout(1000))
    {
      pressure = scale.read();
      Serial.print("HX711 reading: ");
      Serial.println(pressure);
    }
    else
    {
      Serial.println("HX711 not found.");
    }
    if (tempr > 50 || pressure > 30)
    {
      heater = 0;
      digitalWrite(red,HIGH);
      digitalWrite(buzz, HIGH);
      digitalWrite(green_l,LOW);
      digitalWrite(green_m,LOW);
      
    }
    else
    {
      heater = 1;
      digitalWrite(red,LOW);
      digitalWrite(buzz, LOW);
      digitalWrite(green_l,HIGH);
      digitalWrite(green_m,HIGH);
    }
    digitalWrite(heater_pin, heater);

    if (water_level > 2000)
    {
      pump1 = 1;
      digitalWrite(red,LOW);
      digitalWrite(buzz, LOW);
      digitalWrite(green_r,HIGH);
    }
    else
    {
      pump1 = 0;
      digitalWrite(red,HIGH);
      digitalWrite(green_r,LOW);
    }
    digitalWrite(pump1_pin, pump1);
    lcd.clear();
    lcd.print("Temperature: ");
    lcd.print(tempr);
    lcd.setCursor(0, 1);
    lcd.print("Water Level: ");
    lcd.print(water_level);
    lcd.setCursor(0, 2);
    lcd.print("Pessure: ");
    lcd.print(pressure);
    delay(100);
  }

  Path = path + "water_level/";
  if (Firebase.RTDB.setInt(&fbdo, Path.c_str(), water_level))
  {
  }

  Path = path + "tempr/";
  if (Firebase.RTDB.setFloat(&fbdo, Path.c_str(), tempr))
  {
  }

  Path = path + "pressure/";
  if (Firebase.RTDB.setInt(&fbdo, Path.c_str(), pump1))
  {
  }
}


