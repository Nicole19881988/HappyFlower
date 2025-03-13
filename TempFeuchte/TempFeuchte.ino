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
#include "DHT.h"

/* Globals */
time_t now;                         // this are the seconds since Epoch (1970) - UTC
tm tm;                              // the structure tm holds time information in a more convenient way

//Messwerte
float UmgebungsTemperatur = 0;
float UmgebungsFeuchte = 0;
int   BodenFeuchte = 0;

//Aktor
bool Pumpe = 0;

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

//Parametrierung
#define HUMIDITY_PUMP_ON 400.0
#define HUMIDITY_PUMP_OFF 700.0

//Umgebungssensor (Blau)
#define DHTPIN1 D4 // Digital pin connected to the DHT sensor
#define DHTTYPE DHT22 // DHT 11
DHT dht1(DHTPIN1, DHTTYPE); //Temperatur und Luftfeuchtigkeit

#define PUMPE_OUT D0

//Kapazitiver Feuchte Sensor (Blume)
#define SENSOR_BODEN_FEUCHTE_PIN A0

void setup() 
{
    //Setup ESP IOs
    pinMode(PUMPE_OUT,OUTPUT);
    digitalWrite(PUMPE_OUT, 0);

    //ESPXXXX FlashFileSystem init. (Benutzt von ThingSpeak)
    SPIFFS.begin();
    //Wifi::setup();

    //NTP (Zeit Server) Konfigurieren.
    configTime(MY_TZ, MY_NTP_SERVER);
    
    //Serielle Debug Schnitstelle konfigurieren.
    Serial.begin(9600);

    //Umgebungs Sensor DHT1xx init.
    dht1.begin();

    // WiFi konfigurieren und starten:
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

    // ThinkSpeak init:
    ThingSpeak.begin(client);
    Serial.println("init: ThingSpeak up");
}

void loop() 
{
  //Umgebungs Werte abfragen (DHT1XX Sensor)
  UmgebungsFeuchte  = dht1.readHumidity();
  UmgebungsTemperatur  = dht1.readTemperature();
  // Wenn DHT1xx Werte nicht ok -1.0 ersatz wert.
  if (isnan(UmgebungsFeuchte) || isnan(UmgebungsTemperatur)) {
    UmgebungsFeuchte  = -1.0;
    UmgebungsTemperatur  = -1.0;
    Serial.println(F("Failed to read from DHT1 sensor!"));
  }
  //Bodenfeuchte abfragen
  BodenFeuchte = analogRead(SENSOR_BODEN_FEUCHTE_PIN);

  //Bericht für ThingSpeak erstellen.
  ThingSpeak.setField(1,UmgebungsTemperatur);
  ThingSpeak.setField(2,UmgebungsFeuchte);
  ThingSpeak.setField(3,BodenFeuchte);

  //Bericht an ThingSpeak senden.
  int http= ThingSpeak.writeFields(thingspeakChannelId, thingspeakWriteApiKey);

  //Werte ausgeben (Debug)
  Serial.print(" UmgebungsTemperatur=");    Serial.println(UmgebungsTemperatur,1);
  Serial.print(" UmgebungsFeuchte=");     Serial.println(UmgebungsFeuchte,1);
  Serial.print(" BodenFeuchte=");         Serial.println(BodenFeuchte,1);

  if(BodenFeuchte >= HUMIDITY_PUMP_ON) {
    Pumpe = 1;
  } else if(BodenFeuchte <= HUMIDITY_PUMP_OFF) {
    Pumpe = 0;
  }
  digitalWrite(PUMPE_OUT, Pumpe);

  if(Pumpe) {
    delay(500);
  } else {
    delay(60000); //1Minute
  }


}

