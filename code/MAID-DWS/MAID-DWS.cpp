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
#include "../config/userdata_prod.h"	                                          			// Load external userdata file
#include <Arduino.h>
#include <TimeLib.h>
#include <NtpClientLib.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h>
#include <ArduinoOTA.h>
#include <RemoteDebug.h>
#include <Breathe.h>
#include "../config/WifiConfig.h"                                               // Load wifi configuration file

//************* PROJECT AND VERSION **********************************************************************
//********************************************************************************************************
const char* PROJ_VER = "MAID - Doors and Windows Sensor v0.2.1 (27/10/2017)";   // Project name and version

//************* GLOBAL VARIABLES *************************************************************************
//********************************************************************************************************
byte mac[6];                                                                    // Variable - MAC address
char myBuffer[15];                                                              // Variable - MAC string buffer

const char* doorState;                                                          // Variable - The state of the door
String doorStateTimeDate;

long unsigned int lowIn;                                                        // Time when the sensor outputs a low impulse
long unsigned int pause = 200;																									// Millis the sensor has to be low to assume detection has stopped

boolean lockLow = false;                                                        // Sensor variables
boolean takeLowTime;

//************* CREATE DEBUG *****************************************************************************
//********************************************************************************************************
RemoteDebug Debug;                                                              // Create remote debug

//************* CREATE BREATHE ***************************************************************************
//********************************************************************************************************
Breathe Breathe;                                                                // Create LED breathe

//************* CREATE MQTT CLIENT ***********************************************************************
//********************************************************************************************************
WiFiClient espClient;                                                           // Create wifi
PubSubClient client(espClient);                                                 // Create MQTT client

//************* CONFIG WEBSERVER *************************************************************************
//********************************************************************************************************
ESP8266WebServer server(80);                                                    // Config webserver port

#include "webpages.h"																														// Include webpages file

//************* CONFIG OTA *******************************************************************************
//********************************************************************************************************
ESP8266HTTPUpdateServer httpUpdater;

//************* CONNECT TO WIFI AND NTP ******************************************************************
//********************************************************************************************************
void onSTAGotIP(WiFiEventStationModeGotIP ipInfo) {															// Start NTP only after IP network is connected
	int8_t myTimeZone = timeZone;
	Serial.printf("IP address is  %s\r\n", ipInfo.ip.toString().c_str());
	NTP.begin(ntpServerName, timeZone, true);
	NTP.setInterval(86400000); 																										// resync after 86400000 milliseconds (24 hours)
}

void onSTADisconnected(WiFiEventStationModeDisconnected event_info) {						// Manage network disconnection
	Serial.printf("Disconnected from %s\n", event_info.ssid.c_str());
	Serial.printf("Reason: %d\n", event_info.reason);
	digitalWrite(ONBOARD_LED, HIGH); 																							// Turn off internal LED
	//NTP.stop(); // NTP sync disabled to avoid sync errors if no wifi
}

void processSyncEvent(NTPSyncEvent_t ntpEvent) {																// Manage NTP disconnection
	if (ntpEvent) {
		Serial.print("Time Sync error: ");
		if (ntpEvent == noResponse)
			Serial.println("NTP server not reachable");																// Send text to serial interface
		else if (ntpEvent == invalidAddress)
			Serial.println("Invalid NTP server address");															// Send text to serial interface
	}
	else {
		Serial.print("NTP time is ");
		Serial.println(NTP.getTimeDateString(NTP.getLastNTPSync()));
	}
}

boolean syncEventTriggered = false;																							// True if time event has been triggered
NTPSyncEvent_t ntpEvent;																												// Last triggered event

//************* RECONNECT MQTT ***************************************************************************
//********************************************************************************************************
void mqttConnect() {

  while (!client.connected()) {                                                 // Loop until reconnected
    Serial.print("Starting MQTT Client... ");                   								// Send text to serial interface
    Debug.printf("Starting MQTT Client... ");                   								// Send text to telnet debug interface
    if (client.connect(
			DEVICE_HOSTNAME, MQTT_USERNAME, MQTT_PASSWORD,
			MQTT_WILL_TOPIC, MQTT_WILL_QOS, MQTT_WILL_RETAIN, MQTT_WILL_MESSAGE)) {		// Connect to MQTT broker
			delay(1000);																															// Wait 1 second
			digitalWrite(ONBOARD_LED, LOW); delay(250);																// Blink internal LED
			digitalWrite(ONBOARD_LED, HIGH);																					// ...
			delay(1000);																															// Wait 1 second
      Serial.println(" Started!");                                              // Send text to serial interface
      Debug.println(" Started!");                                               // Send text to telnet debug interface
			Serial.println();                                                         // Block space to serial interface
		  Debug.println();                                                          // Block space to telnet debug interface
			digitalWrite(ONBOARD_LED, LOW); 																					// Turn on internal LED
    } else {
      Serial.print("failed, rc=");                                              // Send text to serial interface
      Debug.printf("failed, rc=");                                              // Send text to telnet debug interface
      Serial.print(client.state());                                             // Send failure state to serial interface
      //Debug.printf(client.state());                                           // Send failure state to telnet debug interface
      Serial.println(" try again in 5 seconds");                                // Send text to serial interface
      Debug.println(" try again in 5 seconds!");                                // Send text to telnet debug interface
      delay(5000);                                                              // Wait 5 seconds before retrying
    }
  }
}

//************* SETUP ************************************************************************************
//********************************************************************************************************
void setup() {

	static WiFiEventHandler e1, e2;																								// Variable -

	server.on("/", handleRoot);                                                   // Serve root page
  server.on("/reset", handleReset);                                             // Serve root page
  server.onNotFound(handleNotFound);                                            // Serve page not found

	pinMode(ONBOARD_LED, OUTPUT); 																								// Set internal LED as output
	digitalWrite(ONBOARD_LED, HIGH); 																							// Switch off LED

	pinMode(SENSOR_PIN, INPUT_PULLUP);																						// Pullup the sensor pin
  digitalWrite(SENSOR_PIN, LOW);																								// Set sensor pin to low

  pinMode(R_LED_PIN, OUTPUT);																										// Set red LED pin as output
  pinMode(G_LED_PIN, OUTPUT); 																									// Set green LED pin as output
  pinMode(B_LED_PIN, OUTPUT);																										// Set blue LED pin as output

	NTP.onNTPSyncEvent([](NTPSyncEvent_t event) {         												// When NTP syncs...
		ntpEvent = event; 																													// ...mark as triggered
		syncEventTriggered = true; 																									// False if time event has been triggered
	});

	e1 = WiFi.onStationModeGotIP(onSTAGotIP);																			// Start NTP only after IP network is connected
	e2 = WiFi.onStationModeDisconnected(onSTADisconnected);												// Manage network disconnection

	Serial.begin(115200);                                                         // Start serial interface
	Serial.println();                                                             // Send space to serial interface
  Debug.println();                                                              // Send space to telnet debug interface

	Serial.println("- - - - - - - - - - - - - - - - - - - - - - - - - - - - -");  // Block separator to serial interface
  Debug.println("- - - - - - - - - - - - - - - - - - - - - - - - - - - - -");   // Block separator to telnet debug interface
  Serial.println(PROJ_VER);                                                     // Send project name and version to serial interface
  Debug.println(PROJ_VER);                                                      // Send project name and version to telnet debug interface
  Serial.println("- - - - - - - - - - - - - - - - - - - - - - - - - - - - -");  // Block separator to serial interface
  Debug.println("- - - - - - - - - - - - - - - - - - - - - - - - - - - - -");   // Block separator to telnet debug interface
  Serial.println();                                                             // Send space to serial interface
  Debug.println();                                                              // Send space to telnet debug interface

  //Serial.println();                                                           // Connecting to wifi network
  Serial.print("Connecting to "); Serial.println(ssid);                         // Send network name to serial interface
  Debug.printf("Connecting to "); Debug.println(ssid);                          // Send network name to telnet debug interface

	WiFi.mode(WIFI_STA);                                                          // Switch to STA mode
	WiFi.begin(YOUR_WIFI_SSID, YOUR_WIFI_PASSWD);                         				// Start wifi connection
	WiFi.macAddress(mac);                                                         // Get MAC address of the device

	delay(10000);																																	// Give time for wifi to connect

  Serial.println("WiFi connected!");                                            // Send successful connection to serial interface
  Debug.println("WiFi connected!");                                             // Send successful connection to telnet debug interface
	delay(1000);
	digitalWrite(ONBOARD_LED, LOW); delay(250); digitalWrite(ONBOARD_LED, HIGH);// Blink internal LED
	Serial.println();                                                             // Block space to serial interface
  Debug.println();                                                              // Block space to telnet debug interface

  sprintf(myBuffer,"%02X:%02X:%02X:%02X:%02X:%02X",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);    // Get MAC address

  Serial.print("MAC address is "); Serial.println(myBuffer);                    // Send MAC address to serial interface
  Debug.printf("MAC address is "); Debug.println(myBuffer);                     // Send MAC address to telnet debug interface
	Serial.println();                                                             // Block space to serial interface
  Debug.println();                                                              // Block space to telnet debug interface

	Serial.print("Starting HTTP server... ");                                     // Send text to serial interface
  Debug.print("Starting HTTP server... ");                                      // Send text to telnet debug interface
	server.begin();                                                               // Start Web server
	delay(1000);																																	// Wait 1 second
	digitalWrite(ONBOARD_LED, LOW); delay(250); digitalWrite(ONBOARD_LED, HIGH);	// Blink internal LED
  Serial.println(" Started!");                                                  // Send text to serial interface
  Debug.println(" Started!");                                                   // Send text to telnet debug interface

	Serial.print("Starting Telnet server... ");                                   // Send text to serial interface
  Debug.print("Starting telnet server... ");                                    // Send text to telnet debug interface
	Debug.begin(DEVICE_HOSTNAME);                                                 // Start Telnet server
	delay(1000);																																	// Wait 1 second
	digitalWrite(ONBOARD_LED, LOW); delay(250); digitalWrite(ONBOARD_LED, HIGH);	// Blink internal LED
	Serial.println(" Started!");                                                  // Send text to serial interface
  Debug.println(" Started!");                                                   // Send text to telnet debug interface
	Debug.setResetCmdEnabled(true);                                               // Enable/disable (true/false) the reset command (true/false)
  Debug.showTime(false);                                                        // Enable/disable (true/false) timestamps
	Debug.showProfiler(false);                                                    // Enable/disable (true/false) Profiler - time between messages of Debug
  Debug.showDebugLevel(false);                                                  // Enable/disable (true/false) debug levels
  Debug.showColors(true);                                                       // Enable/disable (true/false) colors

	Serial.print("Starting mDNS server... ");                                     // Send text to serial interface
  Debug.print("Starting mDNS server... ");                                      // Send text to telnet debug interface
	MDNS.begin(DEVICE_HOSTNAME);                                                  // Start mDNS service
	MDNS.addService("http", "tcp", 80);                                           // Open por 80 for HTTP and TCP
	delay(1000);																																	// Wait 1 second
	digitalWrite(ONBOARD_LED, LOW); delay(250); digitalWrite(ONBOARD_LED, HIGH);	// Blink internal LED
	Serial.println(" Started!");                                                  // Send text to serial interface
	Debug.println(" Started!");                                                   // Send text to telnet debug interface

	Serial.print("Starting OTA Updater... ");                                     // Block space to serial interface
  Debug.print("Starting OTA Updater... ");                                      // Block space to telnet debug interface
	httpUpdater.setup(&server, UPDATE_PATH, ACCESS_USERNAME, ACCESS_PASSWORD);		// Start HTTP Updater
	delay(1000);																																	// Wait 1 second
	digitalWrite(ONBOARD_LED, LOW); delay(250); digitalWrite(ONBOARD_LED, HIGH);	// Blink internal LED
	Serial.println(" Started!");                                                  // Send text to serial interface
	Debug.println(" Started!");                                                   // Send text to telnet debug interface
	Serial.println();                                                             // Block space to serial interface
  Debug.println();                                                              // Block space to telnet debug interface
	Serial.print("Open http://"); Serial.print(WiFi.localIP());										// Send text to serial interface
	Debug.print("Open http://"); Debug.print(WiFi.localIP());											// Send text to telnet debug interface
	Serial.println(" for web interface");																					// Send text to serial interface
	Debug.println(" for web interface");																					// Send text to telnet debug interface
	Serial.print("Open http://"); Serial.print(WiFi.localIP());										// Send text to serial interface
	Debug.print("Open http://"); Debug.print(WiFi.localIP());											// Send text to telnet debug interface
	Serial.print(UPDATE_PATH); Serial.print(" for OTA. Login with username '");		// Send text to serial interface
	Debug.print(UPDATE_PATH); Debug.print(" for OTA. Login with username '");			// Send text to telnet debug interface
	Serial.print(ACCESS_USERNAME); Serial.print("' and password '");							// Send text to serial interface
	Debug.print(ACCESS_USERNAME); Debug.print("' and password '");								// Send text to telnet debug interface
	Serial.print(ACCESS_PASSWORD); Serial.print("'");															// Send text to serial interface
	Debug.print(ACCESS_PASSWORD); Debug.print("'");																// Send text to telnet debug interface
	Serial.println();                                                             // Block space to serial interface
  Debug.println();                                                              // Block space to telnet debug interface

	ArduinoOTA.begin();																														// Start OTA over wifi
	#include "ota.h"																															// Include OTA file

	client.setServer(MQTT_SERVER, MQTT_PORT);                                     // Start MQTT client

	delay(2000);																																	// Wait 2 seconds
	Serial.print("Daylight Saving period is ");                                   // Send text to serial interface
	Serial.println(NTP.isSummerTime() ? "Summer Time" : "Winter Time");						// Send text to serial interface

}

//************* LOOP *************************************************************************************
//********************************************************************************************************
void loop() {

	server.handleClient();                                                        // Handle http requests

	Breathe.set(B_LED_PIN, HIGH, 1, 5 );                                          // Breathe the external blue LED

	ArduinoOTA.handle();																													// Handle OTA requests via wifi

	if (syncEventTriggered) {																											// If NTP not sync'ed...
		processSyncEvent(ntpEvent);                                                 // ...sync it again
		syncEventTriggered = false;																									// True if time event has been triggered
	}

	if (!client.connected()) {                                                    // If MQTT client disconnects...
    mqttConnect();                                                              // ...connect again
  }

	client.loop();																																// Allow MQTT client to process messages and refresh the connection
	delay(10);                                                                    // Delay - Fixes stability issues with some wifi connections

	if (digitalRead(SENSOR_PIN) == HIGH) {                                        // Sensor Detection - High
    if (lockLow) {
      lockLow = false;                                                          // Wait for a transition to LOW before further output
      client.publish(MQTT_DOOR_STATE_TOPIC, MQTT_DOOR_OPEN, true);              // Publish door state to MQTT topic
      Serial.print("Door state: << Opened >> at ");                             // Send door state to serial interface
      Serial.println(NTP.getTimeDateString());																	// Send state time to serial interface
      Debug.println("Door state: << Opened >> ");                               // Send door state to debug interface
      doorState = "Open";                                                       // Saves door state to variable
			doorStateTimeDate = NTP.getTimeDateString();															// Saves state time to variable
      digitalWrite(R_LED_PIN, HIGH);                                            // Red LED ON
      digitalWrite(G_LED_PIN, LOW);                                             // Green LED OFF
      delay(50);                                                                // Delay
    }
    takeLowTime = true;
  }

  if (digitalRead(SENSOR_PIN) == LOW) {                                         // Sensor Detection - Low
    if (takeLowTime) {
      lowIn = millis();                                                         // Save the time of transition from high to LOW
      takeLowTime = false;                                                      // Make sure this is only done at the start of a LOW phase
    }

    if (!lockLow && millis() - lowIn > pause) {                                 // If sensor is low for more than a given time no more detections will happen
      lockLow = true;                                                           // execute only if new sequence is detected
      client.publish(MQTT_DOOR_STATE_TOPIC, MQTT_DOOR_CLOSED, true);            // Publish door state to MQTT topic
      Serial.print("Door state: >> Closed << at ");                             // Send door state to serial interface
			Serial.println(NTP.getTimeDateString());																	// Send state time to serial interface
      Debug.println("Door state: >> Closed << ");                               // Send door state to debug interface
      doorState = "Closed";                                                     // Saves door state to variable
			doorStateTimeDate = NTP.getTimeDateString();															// Saves state time to variable
      digitalWrite(G_LED_PIN, HIGH);                                            // Red LED OFF
      digitalWrite(R_LED_PIN, LOW);                                             // Green LED ON
      delay(50);
    }
  }

	//if // publish the uptime every hour

	Debug.handle();                                                               // Remote debug over telnet

  yield();                                                                      // Yielding

}

// END
//********************************************************************************************************
