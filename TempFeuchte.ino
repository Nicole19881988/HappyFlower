// Example testing sketch for various DHT humidity/temperature sensors
// Written by ladyada, public domain
// REQUIRES the following Arduino libraries:
// - DHT Sensor Library: https://github.com/adafruit/DHT-sensor-library
// - Adafruit Unified Sensor Lib: https://github.com/adafruit/Adafruit_Sensor


#include <ArduinoJson.h>
#include <ESP8266WiFi.h>      // ESP8266 WiFi library
#include <WiFiClientSecure.h>
#include <WiFiManager.h>
#include <NTPClient.h>
#include <ThingSpeak.h>       // ThingSpeak library
#include <time.h>             // for time() ctime()

/* Globals */
time_t now;                         // this are the seconds since Epoch (1970) - UTC
tm tm;                              // the structure tm holds time information in a more convenient way


  // Fill out the credentials of your local WiFi Access Point
  const char *  wifiSsid              = "A54 von Nicole"; // Your WiFi network SSID name
  const char *  wifiPassword          = "Murli12!"; // Your WiFi network password

  // Fill out the credentials of your ThingSpeak channel
  char thingSpeakAddress[] = "api.thingspeak.com";
  unsigned long thingspeakChannelId   = 2751313; // Your ThingSpeak Channel ID 
  const char *  thingspeakWriteApiKey = "O12G4EA21ZCC84MV"; // Your ThingSpeak write api key

/* Configuration of NTP */
#define MY_NTP_SERVER "at.pool.ntp.org"           
#define MY_TZ "CET-1CEST,M3.5.0/02,M10.5.0/03"   

WiFiClient  client;

#include "DHT.h"

//Umgebungssensor (Blau)
#define DHTPIN1 D4 // Digital pin connected to the DHT sensor
#define DHTTYPE DHT22 // DHT 11
DHT dht1(DHTPIN1, DHTTYPE); //Temperatur und Luftfeuchtigkeit
float UmgebungsTemperatur = 0;
float UmgebungsFeuchte = 0;

#define PUMPE_OUT D0

//Kapazitiver Feuchte Sensor (Blume)
#define SENSOR_PIN A0

void setup() 
{

    pinMode(PUMPE_OUT,OUTPUT);

    SPIFFS.begin();
    //Wifi::setup();

    configTime(MY_TZ, MY_NTP_SERVER); // --> Here is the IMPORTANT ONE LINER needed in your sketch!
    
    Serial.begin(9600);

    dht1.begin();

    // Enable WiFi
    //Serial.printf("init: MAC %s\n",WiFi.macAddress().c_str());
    Serial.print("init: WiFi '");
    Serial.print(wifiSsid);
    Serial.print("' ..");
    WiFi.mode(WIFI_STA);
    WiFi.begin(wifiSsid, wifiPassword);
    while( true ) {
      if( WiFi.status()==WL_CONNECTED ) break;
      delay(250);
      Serial.printf(".");
    }
    Serial.printf(" up (%s)\n",WiFi.localIP().toString().c_str());

    //https://werner.rothschopf.net/201802_arduino_esp8266_ntp.htm

    // Enable ThingSpeak
    ThingSpeak.begin(client);
    Serial.println("init: ThingSpeak up");
    digitalWrite(PUMPE_OUT, 1);

}

void loop() 
{
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  UmgebungsFeuchte  = dht1.readHumidity();
  // Read temperature as Celsius (the default)
  UmgebungsTemperatur  = dht1.readTemperature();
  // Wenn DHT1xx Werte ok verarbeiten
  if ((!isnan(UmgebungsFeuchte)) || !(isnan(UmgebungsTemperatur))) {
    ThingSpeak.setField(1,UmgebungsTemperatur);
    ThingSpeak.setField(2,UmgebungsFeuchte);
  } else {
     Serial.println(F("Failed to read from DHT1 sensor!"));
  }

    int measure = analogRead(SENSOR_PIN);
    ThingSpeak.setField(3,measure);

  int http= ThingSpeak.writeFields(thingspeakChannelId, thingspeakWriteApiKey);

  Serial.print(" UmgebungsTemperatur=");        Serial.print(UmgebungsTemperatur,1);
  Serial.println(" UmgebungsFeuchte=");         Serial.print(UmgebungsFeuchte,1);

  // Wait a few seconds between measurements.
  delay(60000); //1Minute


}

