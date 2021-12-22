// DHT Temperature & Humidity Sensor
// Unified Sensor Library Example
// Written by Tony DiCola for Adafruit Industries
// Released under an MIT license.

// REQUIRES the following Arduino libraries:
// - DHT Sensor Library: https://github.com/adafruit/DHT-sensor-library
// - Adafruit Unified Sensor Lib: https://github.com/adafruit/Adafruit_Sensor

#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include "heltec.h"
#include <WiFi.h>
#include "InfluxArduino.hpp"

#define DHTPIN 17 // Digital pin connected to the DHT sensor
// Feather HUZZAH ESP8266 note: use pins 3, 4, 5, 12, 13 or 14 --
// Pin 15 can work but DHT must be disconnected during program upload.

// Uncomment the type of sensor in use:
#define DHTTYPE DHT11 // DHT 11
//#define DHTTYPE    DHT22     // DHT 22 (AM2302)
//#define DHTTYPE    DHT21     // DHT 21 (AM2301)

// See guide for details on sensor wiring and usage:
//   https://learn.adafruit.com/dht/overview

DHT_Unified dht(DHTPIN, DHTTYPE);

uint32_t delayMS;

InfluxArduino influx;
//connection/ database stuff that needs configuring
const char WIFI_NAME[] = "xxxxx";
const char WIFI_PASS[] = "xxxx";
const char INFLUX_DATABASE[] = "esp32_sensors";
const char INFLUX_IP[] = "xxx.xxx.xxx.xxx";
const char INFLUX_USER[] = "";            //username if authorization is enabled.
const char INFLUX_PASS[] = "";            //password for if authorization is enabled.
const char INFLUX_MEASUREMENT[] = "temp"; //measurement name for the database. (in practice, you can use several, this example just uses the one)

unsigned long DELAY_TIME_MS = 60 * 1000; //how frequently to send data, in milliseconds
unsigned long count = 0;                 //a variable that we gradually increase in the loop

void setup()
{
  Serial.begin(9600);

  // Initialize device.
  dht.begin();
  Heltec.begin(true /*DisplayEnable Enable*/, false /*LoRa Disable*/, false /*Serial Enable*/);
  Heltec.display->flipScreenVertically();
  Heltec.display->setFont(ArialMT_Plain_16);
  Heltec.display->display();
  Serial.println(F("DHTxx Unified Sensor Example"));
  // Print temperature sensor details.
  sensor_t sensor;
  dht.temperature().getSensor(&sensor);
  Serial.println(F("------------------------------------"));
  Serial.println(F("Temperature Sensor"));
  Serial.print(F("Sensor Type: "));
  Serial.println(sensor.name);
  Serial.print(F("Driver Ver:  "));
  Serial.println(sensor.version);
  Serial.print(F("Unique ID:   "));
  Serial.println(sensor.sensor_id);
  Serial.print(F("Max Value:   "));
  Serial.print(sensor.max_value);
  Serial.println(F("°C"));
  Serial.print(F("Min Value:   "));
  Serial.print(sensor.min_value);
  Serial.println(F("°C"));
  Serial.print(F("Resolution:  "));
  Serial.print(sensor.resolution);
  Serial.println(F("°C"));
  Serial.println(F("------------------------------------"));
  // Print humidity sensor details.
  dht.humidity().getSensor(&sensor);
  Serial.println(F("Humidity Sensor"));
  Serial.print(F("Sensor Type: "));
  Serial.println(sensor.name);
  Serial.print(F("Driver Ver:  "));
  Serial.println(sensor.version);
  Serial.print(F("Unique ID:   "));
  Serial.println(sensor.sensor_id);
  Serial.print(F("Max Value:   "));
  Serial.print(sensor.max_value);
  Serial.println(F("%"));
  Serial.print(F("Min Value:   "));
  Serial.print(sensor.min_value);
  Serial.println(F("%"));
  Serial.print(F("Resolution:  "));
  Serial.print(sensor.resolution);
  Serial.println(F("%"));
  Serial.println(F("------------------------------------"));
  // Set delay between sensor readings based on sensor details.
  delayMS = sensor.min_delay / 1000;

  WiFi.begin(WIFI_NAME, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Heltec.display->clear();
    Serial.print(".");
    Heltec.display->drawString(1, 1, "Connecting to Wifi");
    Heltec.display->display();
  }
  Heltec.display->clear();
  Heltec.display->drawString(1, 1, "Connection succeded");
  Heltec.display->display();
  delay(500);
  influx.configure(INFLUX_DATABASE, INFLUX_IP); //third argument (port number) defaults to 8086
  // influx.authorize(INFLUX_USER,INFLUX_PASS); //if you have set the Influxdb .conf variable auth-enabled to true, uncomment this
  // influx.addCertificate(ROOT_CERT); //uncomment if you have generated a CA cert and copied it into InfluxCert.hpp
  //Serial.print("Using HTTPS: ");
  //Serial.println(influx.isSecure()); //will be true if you've added the InfluxCert.hpp file.
}

char tags[32];
char fields[32];
bool tempWorking = false, humidWorking = false;
float temp = 0, humidiy = 0;
unsigned long elapsedTime = 0;

void loop()
{
  tempWorking = false;
  humidWorking = false;
  temp = 0;
  humidiy = 0;
  // Delay between measurements.
  elapsedTime += delayMS;
  delay(delayMS);
  Heltec.display->clear();
  // Get temperature event and print its value.
  sensors_event_t event;
  dht.temperature().getEvent(&event);
  if (isnan(event.temperature))
  {
    Serial.println(F("Error reading temperature!"));
  }
  else
  {
    tempWorking = true;
    temp = event.temperature;

    Heltec.display->drawString(1, 1, "Temp: " + String(temp, 1) + "°C");
  }
  // Get humidity event and print its value.
  dht.humidity().getEvent(&event);
  if (isnan(event.relative_humidity))
  {
    Serial.println(F("Error reading humidity!"));
  }
  else
  {
    humidWorking = true;
    humidiy = event.relative_humidity;
    Heltec.display->drawString(1, 20, "Humidity: " + String(humidiy, 1) + "%");
  }
  
  if (elapsedTime >= DELAY_TIME_MS)
  {
    elapsedTime = 0;
    if (humidWorking && tempWorking)
    {
      sprintf(fields, "temperature=%0.1f,humidity=%0.1f", temp, humidiy);
      sprintf(tags, "name=%s", "esp32"); //write a tag called new_tag
      Serial.println(fields);
      Serial.println("Tags:");
      Serial.println(tags);
      bool writeSuccessful = influx.write(INFLUX_MEASUREMENT, tags, fields);
      if (!writeSuccessful)
      {
        Heltec.display->clear();
        Serial.print("error: ");
        Serial.println(String(influx.getResponse()));
        Heltec.display->drawString(1, 1, "Error in Influx write: " + String(influx.getResponse()));
      }
    }
  }

  Heltec.display->display();
}
