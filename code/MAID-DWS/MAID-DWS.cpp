/**
 *        _____  ____    ____  _______          _
 *       |_   _||_   \  /   _||_   __ \        / \
 *         | |    |   \/   |    | |__) |      / _ \
 *     _   | |    | |\  /| |    |  __ /      / ___ \
 *    | |__' |   _| |_\/_| |_  _| |  \ \_  _/ /   \ \_
 *    `.____.'  |_____||_____||____| |___||____| |____|
 *
 * Project by Jorge Assunção
 *
 * See Github for instructions on how to use this code: https://github.com/jorgeassuncao/MAID-DWS
 *
 * This program is free software; you can redistribute it and/or modify it under the terms of the
 * GNU General Public License version 2 as published by the Free Software Foundation.
 *
 * You can change your personal user data (wifi access, MQTT server, etc) in the "config/userdata.h" file
 */

 //************* INCLUDE LIBRARIES ************************************************************************
 //********************************************************************************************************
#include <ESP8266WiFi.h>
#include <MQTTClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h>
#include <Breathe.h>
#include "RemoteDebug.h"
#include "../config/userdata_devel.h"
//#include "../config/pin_mapping.h"

//************* PROJECT AND VERSION **********************************************************************
//********************************************************************************************************
const char* proj_ver = "MAID - Doors and Windows Sensor v0.1.2 (18/10/2017)";   // Project name and version

//************* CONFIG DEBUG *****************************************************************************
//********************************************************************************************************
RemoteDebug Debug;                                                              // Remote debug type

//************* CONFIG BREATHE ***************************************************************************
//********************************************************************************************************
Breathe Breathe;                                                                // Initiate LED breathe

//************* CONFIG OTA *******************************************************************************
//********************************************************************************************************
const char* update_path = "/firmware";                                          // Path to firmware update page

//************* CONFIG PINS ******************************************************************************
//********************************************************************************************************
int sensorPin = D1;                                                             // Door sensor pin
int r_ledPin = D6;                                                              // Red LED pin - door opened
int g_ledPin = D7;                                                              // Green LED pin - door closed
int b_ledPin = D8;                                                              // Blue LED pin - External running indicator
int ledPin = D4;                                                                // Internal LED in NodeMCU

//int ledState = LOW;             // ledState used to set the LED

//********************************************************************************************************

//unsigned long previousMillis = 0;        // will store last time LED was updated
//const long interval = 1000;           // interval at which to blink (milliseconds)

long unsigned int lowIn;    //the time when the sensor outputs a low impulse

//the amount of milliseconds the sensor has to be low
//before we assume all detection has stopped
long unsigned int pause = 100;

byte mac[6];                                                                    // Variable - MAC address
char myBuffer[15];                                                              // Variable - MAC string buffer

//sensor variables
boolean lockLow = true;
boolean takeLowTime;

//webserver
ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;

//MQTT
WiFiClient net;
MQTTClient client;

unsigned long lastMillis = 0;                                                   // Time Variable

void connect();                                                                 // Connect to WiFI and MQTT

//************* SETUP ************************************************************************************
//********************************************************************************************************
void setup()
{

  pinMode(sensorPin, INPUT_PULLUP);
  digitalWrite(sensorPin, LOW);
  pinMode(ledPin, OUTPUT);
  pinMode(r_ledPin, OUTPUT);
  pinMode(g_ledPin, OUTPUT);
  pinMode(b_ledPin, OUTPUT);

  Serial.begin(115200);
  Serial.println("- - - - - - - - - - - - - - - - - - - - - - - - - - -");      // Block separator to serial interface
  Debug.println("- - - - - - - - - - - - - - - - - - - - - - - - - - -");       // Block separator to telnet debug interface
  Serial.println(proj_ver);                                                     // Send project name and version to serial interface
  Debug.println(proj_ver);                                                      // Send project name and version to telnet debug interface
  Serial.println("- - - - - - - - - - - - - - - - - - - - - - - - - - -");      // Block separator to serial interface
  Debug.println("- - - - - - - - - - - - - - - - - - - - - - - - - - -");       // Block separator to telnet debug interface
  Serial.println();                                                             // Send space to serial interface
  Debug.println();                                                              // Send space to telnet debug interface

  Serial.println();                                                             // Connecting to wifi network
  Serial.print("Connecting to "); Serial.println(ssid);                         // Send network name to serial interface
  Debug.printf("Connecting to "); Debug.println(ssid);                          // Send network name to telnet debug interface

  WiFi.config(ip, dns, gateway, subnet);                                        // Configure connection with IP, DNS, Gateway and subnet
  WiFi.mode(WIFI_STA);                                                          // Switch to STA mode
  WiFi.begin(ssid, password);                                                   // Start wifi connection with SSID and Password
  client.begin(mqtt_server, net);
  WiFi.macAddress(mac);                                                         // Get MAC address of the node

  Serial.println();                                                             // Block space to serial interface
  Debug.println();                                                              // Block space to telnet debug interface
  Serial.println("WiFi connected");                                             // Send successful connection to serial interface
  Debug.println("WiFi connected");                                              // Send successful connection to telnet debug interface

  Serial.print("IP address is "); Serial.println(WiFi.localIP());               // Send IP address to serial interface
  Debug.printf("IP address is "); Debug.println(WiFi.localIP());                // Send IP address to telnet debug interface

  sprintf(myBuffer,"%02X:%02X:%02X:%02X:%02X:%02X",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);    // Get MAC address
  Serial.print("MAC address is "); Serial.println(myBuffer);                                      // Send MAC address to serial interface
  Debug.printf("MAC address is "); Debug.println(myBuffer);                                       // Send MAC address to telnet debug interface

  connect();

  MDNS.begin(host_name);

  httpUpdater.setup(&httpServer, update_path, update_username, update_password);
  httpServer.begin();

  MDNS.addService("http", "tcp", 80);

}

//Connect to wifi and MQTT
void connect() {
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
  }

  while (!client.connect(host_name, mqtt_username, mqtt_password))
  {
    delay(1000);
  }
}

//************* LOOP *************************************************************************************
//********************************************************************************************************
void loop()
{

  httpServer.handleClient();

  Breathe.set(b_ledPin, HIGH, 1, 5 );    // https://github.com/kslstn/Breathe

  if (!client.connected()) {                                                    // If client disconnects...
    connect();                                                                  // ...connect again
  }

  client.loop();

  if (digitalRead(sensorPin) == HIGH) {                                         // Sensor Detection
    if (lockLow)
    {
      lockLow = false;    //makes sure we wait for a transition to LOW before any further output is made:
      client.publish(outTopic, doorOpen);
      Serial.print("Door state: << Opened >> ");                                // Send door state to serial interface
      digitalWrite(r_ledPin, HIGH);                                             // Red LED ON
      digitalWrite(g_ledPin, LOW);                                              // Green LED OFF
      delay(50);
    }
    takeLowTime = true;
  }

  if (digitalRead(sensorPin) == LOW)
  {
    if (takeLowTime)
    {
      lowIn = millis();                                                         //save the time of the transition from high to LOW
      takeLowTime = false;                                                      //make sure this is only done at the start of a LOW phase
    }
    //if the sensor is low for more than the given pause,
    //we assume that no more detection is going to happen
    if (!lockLow && millis() - lowIn > pause)
    {
      //makes sure this block of code is only executed again after
      //a new detection sequence has been detected
      lockLow = true;
      client.publish(outTopic, doorClosed);
      Serial.print("Door state: >> Closed << ");                                // Send door state to serial interface
      digitalWrite(g_ledPin, HIGH);                                             // Red LED OFF
      digitalWrite(r_ledPin, LOW);                                              // Green LED ON
      delay(50);
    }
  }

  // Debug.handle();                                                               // Remote debug over telnet

  yield();                                                                      // Yielding

}

// END
//********************************************************************************************************
