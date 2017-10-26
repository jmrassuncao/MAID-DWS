/**
 * File contents of "userdata.h" for MAID - Door and Window Sensor
 *
 * This file is used to change your personal user data to meet your needs
 * For more information visit https://github.com/jorgeassuncao/MAID-DWS
 */

//************ CONFIG WIFI *******************************************************************************
// Configurations of your wifi network - Fixed IP is used because it's quicker than DHCP. IP address and
// and Hostname must be unique inside your network
//********************************************************************************************************

  #ifndef WIFI_CONFIG_H
// Wifi SSID (the name of your wifi network)
  #define YOUR_WIFI_SSID "wifi_ssid";
// Wifi password (the password to access your network)
  #define YOUR_WIFI_PASSWD "wifi_password";
  #endif // !WIFI_CONFIG_H

// If you want to use a fixed IP address, open "wifiConfig.h" and edit lines 123, 124 and 125

// Hostname (the name you want to give to this node, must be unique inside your network)
  #define DEVICE_HOSTNAME "your_hostname"

//************ CONFIG NTP ********************************************************************************
// Configurations for the NTP server - Use the address of the server closer to your location and configure
// your timezone relative to UTC
//********************************************************************************************************
  static const char ntpServerName[] = "ntp02.oal.ul.pt";

  const int timeZone = 0;  // 0 = UTC

//************ CONFIG ACCESS TO UPDATE PAGE **************************************************************
// Configurations for OTA page - When you access "http://<device-ip>/firmware" the page will ask for a
// username and password. For your security, please change both or at least the password!
//********************************************************************************************************

// Username to access the OTA page
  const char* ACCESS_USERNAME = "admin";
// Password to access the OTA page
  const char* ACCESS_PASSWORD = "1a2b3c";
// Path to update page
  const char* UPDATE_PATH = "/firmware";

//************* CONFIG MQTT ******************************************************************************
// Configurations of your MQTT server -
//********************************************************************************************************

// MQTT server IP ou URL (the ip address or URL of your MQTT broker)
  const char* MQTT_SERVER = "mqtt_server_address";
// MQTT user (the username of your MQTT broker)
  const char* MQTT_USERNAME = "mqtt_user";
// MQTT password (the password of your MQTT broker)
  const char* MQTT_PASSWORD = "mqtt_password";
// MQTT port (usually 1883)
  int MQTT_PORT = 1883;

//************ MQTT LWT **********************************************************************************
// Configurations of your MQTT LWT - Define here the Last Will and Testment of your device. the MQTT broker and the
// payload for door open and closed
//********************************************************************************************************

// MQTT last will topic
  const char* MQTT_WILL_TOPIC = "home/indoor/sensor/MAID-DWS-01_t/status";
// MQTT last will message
  const char* MQTT_WILL_MESSAGE = "online";
// MQTT last will QoS (0,1 or 2)
  int MQTT_WILL_QOS = 1;
// MQTT last will retain (0 or 1)
  int MQTT_WILL_RETAIN = 1;

//************ MQTT TOPICS *******************************************************************************
// Configurations of your MQTT topics - to match your MQTT broker
//********************************************************************************************************

// MQTT topic to publish values
  const char* MQTT_DOOR_STATE_TOPIC = "home/indoor/sensor/MAID-DWS-01_t";
// MQTT payload for door open
  const char* MQTT_DOOR_OPEN = "Open";
// MQTT payload for door closed
  const char* MQTT_DOOR_CLOSED = "Closed";

//************* CONFIG PINS ******************************************************************************
// Configurations of the device pins - This is where you are going to connect the LEDs and sensor. You can
// change them to fit your needs. The pins refer to what is marked on the board
//********************************************************************************************************

// Internal LED in NodeMCU
  #define ONBOARD_LED 2
// Door sensor pin
  #define SENSOR_PIN 5
// Red LED pin - door opened
  #define R_LED_PIN 12
// Green LED pin - door closed
  #define G_LED_PIN 13
// Blue LED pin - External running indicator
  #define B_LED_PIN 15

// END
//********************************************************************************************************
