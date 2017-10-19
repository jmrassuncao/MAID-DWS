/**
 *        _____  ____    ____  _______          _
 *       |_   _||_   \  /   _||_   __ \        / \
 *         | |    |   \/   |    | |__) |      / _ \
 *     _   | |    | |\  /| |    |  __ /      / ___ \
 *    | |__' |   _| |_\/_| |_  _| |  \ \_  _/ /   \ \_
 *    `.____.'  |_____||_____||____| |___||____| |____|
 *
 * Project by Jorge Assunção (2017)
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
#include <TimeLib.h>
#include <NtpClientLib.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h>
#include <Breathe.h>
#include "RemoteDebug.h"
#include "../config/userdata_devel.h"
//#include "../config/pin_mapping.h"

//************* PROJECT AND VERSION **********************************************************************
//********************************************************************************************************
const char* proj_ver = "MAID - Doors and Windows Sensor v0.1.4 (18/10/2017)";   // Project name and version

//************* CONFIG DEBUG *****************************************************************************
//********************************************************************************************************
RemoteDebug Debug;                                                              //Initiate Remote debug

//************* CONFIG BREATHE ***************************************************************************
//********************************************************************************************************
Breathe Breathe;                                                                // Initiate LED breathe

//************* CONFIG WEBSERVER *************************************************************************
//********************************************************************************************************
ESP8266WebServer server(80);                                                    // Webserver port

//************* CONFIG OTA *******************************************************************************
//********************************************************************************************************
const char* update_path = "/firmware";                                          // Path to OTA update page

//************* CONFIG UDP *******************************************************************************
//********************************************************************************************************


//************* CONFIG OTHER GLOBALS *********************************************************************
//********************************************************************************************************
const char* doorState;                                                          // Variable - The state of the door

long unsigned int lowIn;                                                        // Time when the sensor outputs a low impulse

//the amount of milliseconds the sensor has to be low
//before we assume all detection has stopped
long unsigned int pause = 100;

byte mac[6];                                                                    // Variable - MAC address
char myBuffer[15];                                                              // Variable - MAC string buffer

boolean lockLow = true;                                                         //sensor variables
boolean takeLowTime;

ESP8266WebServer httpServer(80);                                                // Initiate webserver
ESP8266HTTPUpdateServer httpUpdater;                                            // initiate HTTP updater

WiFiClient net;                                                                 // Initiate wifi
MQTTClient client;                                                              // initiate MQTT

unsigned long lastMillis = 0;                                                   // Time Variable

void connect();                                                                 // Connect to WiFI and MQTT

String getPage(){                                                               // Create webpage content
  String page = "<!DOCTYPE html>";
  page += "<html lang='en'>";
  page +=   "<head>";
  page +=     "<meta charset='utf-8'>";
  page +=     "<meta http-equiv='X-UA-Compatible' content='IE=edge'>";
  page +=     "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  page +=     "<meta http-equiv='refresh' content='15'/>";
  page +=     "<link rel='shortcut icon' href='data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAADAAAAAwCAYAAABXAvmHAAAABGdBTUEAALGPC/xhBQAAAAFzUkdCAK7OHOkAAAAgY0hSTQAAeiYAAICEAAD6AAAAgOgAAHUwAADqYAAAOpgAABdwnLpRPAAAAAZiS0dEAAAAAAAA+UO7fwAAAAlwSFlzAAAASAAAAEgARslrPgAAAyRJREFUaN7tmM1vDWEUxn/6QUhK4itxW6ErCQsSafEHSDStWAmxKhJKRCokysbdoHtCb22JpBo2SiyEv6FpoldYqK9ElbaKCnItZiamz7y9fWfuO23UfZJZnDnnPOc5M++8HwNllFHGf4Hqf7lmNdAHdM6i+CxwH6hyQZYDCv6VnSXxQb1rpZIdCJEF15EUxbcZ6u1LSrYS+Chkg/79tJAB3krNYWB5ErIuIZoANqYoPkAD8E1qX41LUgf8EJJ2Q1yjA8EmjtNSexKojUPaKQTPiM4Iu4HflPZhZ32OFrlfBeRFwyVb0krgjSS3SkwtME5ps1M2lD+GN/7DOCgahoAKG+IdkjgKLJaYG0Rf8foY4uuJDtGcxCyRh1TAcsiel6Sb4l8L/JKYtgRv4Lhw/CQ6zm9LTIeSmF7JVrGfir0Hb5gF6Ae6DU32+k9wHLgHbJCYHDAQsqt87jCezKDNiAHpukH8D8V/0iB+RGIKwCffF0a7xPSJf5v4+20a0OK6cOnsoGtDr0F8cPVI7CbxD4p/tfiHbRrQj2uh+PXDqpnBH77GJLZG/OPiXyT+SRVrNS05RME1oamBCbGXiv1ebB3Xj4vUeyT2OrHfib1M7C82DajAerFfir1T7A7gs4F3BG+LUCz3xQy1VZuxgbzYm8V+IPZh4cn7OXf4O432AFvwVvgAlX5uMW6trdqMOMfUD+eW+OvwFp1wzDEbYsEJ4bBZyM7aEG8nOnPoVqKb6OxQb0Puw7SV6JIY01aiwYa8Angtia0Sk/Ebc7WZGwXWiP+QaHhFjFnzMtEFRrfTLbjbTjfL/WrguWi4GIfYdKA5ZYgr9kp1EZsOJo4zRIdohpi4LiRf8ZZ+W9g2oGgEvkvulbjiAVYQPdTngVUpNlBL9FD/gYSHeoD9BiFHU2zA9Ftlb1LxAcJDKRsjL+kQyoZyYv+NMKESuAtciJmXtAHwFixnvxYh2Y/WUhqwrrnAVYfTNJB6LZfngWam/o4xNVTAW+Wb0mimVOj2o9g15Kqo6bU6PzU5xhTNs32kdI5yA/MJc/IRu0STZRNDwK65FltGGfMFfwB1P4cc2cai3gAAACV0RVh0ZGF0ZTpjcmVhdGUAMjAxNy0xMC0xM1QyMDo1OTo1MiswMDowMGf6eRAAAAAldEVYdGRhdGU6bW9kaWZ5ADIwMTctMTAtMTNUMjA6NTk6NTIrMDA6MDAWp8GsAAAAKHRFWHRzdmc6YmFzZS11cmkAZmlsZTovLy90bXAvbWFnaWNrLWZDN1pLU3MxVs1E8gAAAABJRU5ErkJggg=='>";
  page +=     "<title>";
  page +=       host_name;
  page +=     "</title>";
  page +=     "<link rel='stylesheet' href='//maxcdn.bootstrapcdn.com/bootstrap/3.3.7/css/bootstrap.min.css' integrity='sha384-BVYiiSIFeK1dGmJRAkycuHAHRg32OmUcww7on3RYdg4Va+PmSTsz/K68vbdEjh4u' crossorigin='anonymous'>";
  page +=     "<link rel='stylesheet' href='//maxcdn.bootstrapcdn.com/bootstrap/3.3.7/css/bootstrap-theme.min.css' integrity='sha384-rHyoN1iRsVXV4nD0JutlnGaslCJuC7uwjduW9SVrLvRYooPp2bWYgmgJQIXwl/Sp' crossorigin='anonymous'>";
  page +=     "<script src='//maxcdn.bootstrapcdn.com/bootstrap/3.3.7/js/bootstrap.min.js' integrity='sha384-Tc5IQib027qvyjSMfHjOMaLkfuWVxZxUPnCJA7l2mCWNIpG9mGCD8wGNIcPD7Txa' crossorigin='anonymous'></script>";
  page +=     "<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.0/jquery.min.js'></script>";
  page +=   "</head>";
  page +=   "<body>";
  page +=   "<p></p>";
  page +=     "<div class='container-fluid'>";
  page +=     "<div class='row'>";
  page +=       "<div class='col-md-12'>";
  page +=         "<div class='jumbotron'>";
  page +=           "<h2>";
  page +=             proj_ver;
  page +=           "</h2>";
  page +=           "<p>This project uses a sensor, mechanical or magnectic, to get the state of your doors or windows. The state is published to the topic <small><em style='color: #ababab;'>";
  page +=             outTopic;
  page +=           "</em></small></p>";
  page +=         "</div>";
  page +=         "<div class='alert alert-dismissable alert-info'>";
  page +=           "<button type='button' class='close' data-dismiss='alert' aria-hidden='true'>×</button>";
  page +=           "<strong>Attention!</strong> This page auto-refreshes every 15 seconds. For that fact, the state may not be correct";
  page +=         "</div>";
  page +=       "</div>";
  page +=     "</div>";
  page +=     "<div class='row'>";
  page +=       "<div class='col-md-4'>";
  page +=         "<h3 class='text-primary'>Door State</h3>";
  page +=           "<p class='lead'>";
  page +=             doorState;
  page +=           "</p>";
  page +=         "</div>";
  page +=       "<div class='col-md-4'>";
  page +=         "<h3 class='text-primary'> </h3>";
  page +=           "<p class='lead'>";
  page +=           " </p>";
  page +=         "</div>";
  page +=       "<div class='col-md-4'>";
  page +=         "<h3 class='text-primary'> </h3>";
  page +=           "<p class='lead'>";
  page +=           " </p>";
  page +=         "</div>";
  page +=       "</div>";
  page +=       "<div class='row'>";
  page +=         "<div class='col-md-12'>";
  page +=           "<div class='alert alert-dismissable alert-danger'>";
  page +=             "<button type='button' class='close' data-dismiss='alert' aria-hidden='true'>×</button>";
  page +=           "<h4>Warning!</h4>The OTA update page is protected and you need to use the username and password that was configured when the code was flashed to the board.&nbsp;&nbsp;";
  page +=           "<a href='/firmware'><button type='button' class='btn btn-warning btn-default'>OTA UPDATE</button></a>";
  page +=         "</div>";
  page +=       "<div class='row'>";
  page +=         "<div class='col-md-12'>";
  page +=           "<div class='alert alert-dismissable alert-danger'>";
  page +=             "<button type='button' class='close' data-dismiss='alert' aria-hidden='true'>×</button>";
  page +=           "<h4>Warning!</h4>Sometimes the board does not restart correctly. If you can't get back to this page after one minute, you need to reset on the board directly on the fisical reset button.&nbsp;&nbsp;";
  page +=           "<a href='/reset'><button type='button' class='btn btn-warning btn-default' >RESET</button></a>";
  page +=         "</div>";
  page +=     "</div>";
  page +=   "</div>";
  page +=     "<div class='col-md-12'>";
  page +=       "Jorge Assunção (2017) - <a href='https://github.com/jorgeassuncao/MAID-DWS' target='_blank' rel='noopener'>https://github.com/jorgeassuncao/MAID-DWS</a> <br/><br/>";
  page +=     "</div>";
  page += "</div>";
  page +=   "</body>";
  page += "</html>";
  return page;
}

//************* CONFIG WESERVER **************************************************************************
//********************************************************************************************************
void handleRoot() {                                                             // Handle "root" page
  httpServer.send ( 200, "text/html", getPage() );
}

void handleReset() {                                                             // Handle "root" page
  ESP.restart();
}

void handleNotFound(){                                                          // Handle "not found" page
  String message = "Page or File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  httpServer.send(404, "text/plain", message);
}

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

  Debug.begin(host_name);                                                       // Initiaze the telnet server
  Debug.setResetCmdEnabled(true);                                               // Enable/disable (true/false) the reset command (true/false)
  Debug.showTime(false);                                                        // Enable/disable (true/false) timestamps
  Debug.showProfiler(false);                                                    // Enable/disable (true/false) Profiler - time between messages of Debug
  Debug.showDebugLevel(false);                                                  // Enable/disable (true/false) debug levels
  Debug.showColors(true);                                                       // Enable/disable (true/false) colors

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

  Serial.println();                                                             // Block space to serial interface

  connect();

  httpServer.on("/", handleRoot);                                               // Serve root page
  httpServer.on("/reset", handleReset);                                         // Serve root page
  httpServer.onNotFound(handleNotFound);                                        // Serve page not found

  MDNS.begin(host_name);

  httpUpdater.setup(&httpServer, update_path, update_username, update_password);// Ask for user and pass for OTA page
  httpServer.begin();                                                           // Start webserver

  MDNS.addService("http", "tcp", 80);                                           // Open por 80 for HTTP and TCP
}

void connect() {
  while (WiFi.status() != WL_CONNECTED)                                         //Connect to wifi network
  {
    delay(1000);                                                                // Delay
  }

  while (!client.connect(host_name, mqtt_username, mqtt_password))              //Connect to MQTT server
  {
    delay(1000);                                                                // Delay
  }
}

//************* LOOP *************************************************************************************
//********************************************************************************************************
void loop()
{
  httpServer.handleClient();                                                    // Handle http requests

  Breathe.set(b_ledPin, HIGH, 1, 5 );                                           // Breathe the external blue LED
  Breathe.set(ledPin, HIGH, 0.5, 10 );                                          // Breathe the internal blue LED

  if (!client.connected()) {                                                    // If client disconnects...
    connect();                                                                  // ...connect again
  }

  client.loop();                                                                // Connect to wifi
  delay(10);                                                                    // Delay - Fixes stability issues with some wifi connections

  if (digitalRead(sensorPin) == HIGH) {                                         // Sensor Detection - High
    if (lockLow) {
      lockLow = false;                                                          // Wait for a transition to LOW before further output
      client.publish(outTopic, doorOpen);                                       // Publish door state to MQTT topic
      Serial.println("Door state: << Opened >> ");                              // Send door state to serial interface
      Debug.println("Door state: << Opened >> ");                               // Send door state to debug interface
      doorState = "Open";                                                     // Saves door state in variable
      digitalWrite(r_ledPin, HIGH);                                             // Red LED ON
      digitalWrite(g_ledPin, LOW);                                              // Green LED OFF
      delay(50);                                                                // Delay
    }
    takeLowTime = true;
  }

  if (digitalRead(sensorPin) == LOW) {                                          // Sensor Detection - Low
    if (takeLowTime) {
      lowIn = millis();                                                         // Save the time of transition from high to LOW
      takeLowTime = false;                                                      // Make sure this is only done at the start of a LOW phase
    }

    if (!lockLow && millis() - lowIn > pause) {                                 // If sensor is low for more than a given time no more detections will happen
      lockLow = true;                                                           // execute only if new sequence is detected
      client.publish(outTopic, doorClosed);                                     // Publish door state to MQTT topic
      Serial.println("Door state: >> Closed << ");                              // Send door state to serial interface
      Debug.println("Door state: >> Closed << ");                               // Send door state to debug interface
      doorState = "Closed";                                                   // Saves door state in variable
      digitalWrite(g_ledPin, HIGH);                                             // Red LED OFF
      digitalWrite(r_ledPin, LOW);                                              // Green LED ON
      delay(50);
    }
  }

  Debug.handle();                                                               // Remote debug over telnet

  yield();                                                                      // Yielding
}

// END
//********************************************************************************************************
