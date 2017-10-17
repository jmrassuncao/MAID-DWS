/*
 * Device Title: MK-DoorSensor
 * Device Description: MQTT Door Sensor
 * Device Explanation: When the magnetic switch sensor is broken then
 *                     the device sends an mqtt message to the defined server.
 * Device information: https://www.MK-SmartHouse.com/door-sensor
 *
 * Author: Matt Kaczynski
 * Website: http://www.MK-SmartHouse.com
 *
 * Code may only be distrbuted through http://www.MK-SmartHouse.com any other methods
 * of obtaining or distributing are prohibited
 * Copyright (c) 2016-2017
 *
 * Note: After flashing the code once you can remotely access your device by going to http://HOSTNAMEOFDEVICE.local/firmware
 * obviously replace HOSTNAMEOFDEVICE with whatever you defined below. The user name and password are also defined below.
 */

 //************* INCLUDE LIBRARIES ************************************************************************
 //********************************************************************************************************
#include <ESP8266WiFi.h>
#include <MQTTClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h>
#include "../config/userdata_devel.h"


//********************************************************************************************************
const char* update_path = "/firmware";                                          // Path to firmware update page

//the time when the sensor outputs a low impulse
long unsigned int lowIn;

//the amount of milliseconds the sensor has to be low
//before we assume all detection has stopped
long unsigned int pause = 100;

//sensor variables
boolean lockLow = true;
boolean takeLowTime;

int sensorPin = 13;                                                             //the digital pin connected to the door sensor's output

//webserver
ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;

//MQTT
WiFiClient net;
MQTTClient client;

unsigned long lastMillis = 0;                                                   //Time Variable

void connect();                                                                 // Connect to WiFI and MQTT


void setup()
{
  pinMode(sensorPin, INPUT_PULLUP);
  digitalWrite(sensorPin, LOW);

  WiFi.config(ip, dns, gateway, subnet);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  client.begin(mqtt_server, net);
  //client.onMessage(messageReceived);

  connect();

  MDNS.begin(host_name);

  httpUpdater.setup(&httpServer, update_path, update_username, update_password);
  httpServer.begin();

  MDNS.addService("http", "tcp", 80);
}

//Connect to wifi and MQTT
void connect()
{
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
  }

  while (!client.connect(host_name, mqtt_username, mqtt_password))
  {
    delay(1000);
  }
}

void loop()
{
  // MQTT Loop
  client.loop();
  delay(10);

  // Make sure device is connected
  if(!client.connected())
  {
    connect();
  }

  httpServer.handleClient();

  if(digitalRead(sensorPin) == HIGH)                                            //Sensor Detection
  {
    if(lockLow)
    {
      //makes sure we wait for a transition to LOW before any further output is made:
      lockLow = false;
      client.publish(outTopic, "OPEN");
      delay(50);
    }
    takeLowTime = true;
  }

  if(digitalRead(sensorPin) == LOW)
  {
    if(takeLowTime)
    {
      lowIn = millis();                                                         //save the time of the transition from high to LOW
      takeLowTime = false;                                                      //make sure this is only done at the start of a LOW phase
    }
    //if the sensor is low for more than the given pause,
    //we assume that no more detection is going to happen
    if(!lockLow && millis() - lowIn > pause)
    {
      //makes sure this block of code is only executed again after
      //a new detection sequence has been detected
      lockLow = true;
      client.publish(outTopic, "CLOSED");
      delay(50);
    }
  }

}

void messageReceived(String &topic, String &payload)
{
  //This sensor does not recieve anything from MQTT Server so this is blank
}
