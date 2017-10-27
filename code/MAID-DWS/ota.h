/**
 * File contents of "ota.h" for MAID - Door and Window Sensor
 *
 * This file is used to create the folowing webpages:
 * Root page ("/"), reset page ("/reset") and "page not found"
 *
 * For more information visit https://github.com/jorgeassuncao/MAID-DWS
 */

  ArduinoOTA.setPort(8266);    // Port defaults to 8266

  ArduinoOTA.setHostname(DEVICE_HOSTNAME);   // Hostname defaults to esp8266-[ChipID]

  ArduinoOTA.setPassword(ACCESS_PASSWORD);  // No authentication by default

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
      type = "sketch";
    else // U_SPIFFS
      type = "filesystem";

// If updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()

    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });

// END
//********************************************************************************************************
