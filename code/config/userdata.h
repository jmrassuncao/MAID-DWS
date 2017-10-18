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

// Wifi SSID (the name of your wifi network)
  const char* ssid = "wifi_ssid";
// Wifi password (the password to access your network)
  const char* password = "wifi_password";
// The fixed IP address you want for this node (must be unique inside your network - see above note)
  IPAddress ip(192,168,1,222);
// DNS server IP address (usually the same as your router/gateway but can be another DNS server)
  IPAddress dns(8,8,8,8);
// Gateway IP address (the address you use to access your router/gateway)
  IPAddress gateway(192,168,1,1);
// Subnet mask IP address ()
  IPAddress subnet(255,255,255,0);
// Hostname (the name you want to give to this node, must be unique inside your network)
  #define host_name "your_hostname"

//************ CONFIG ACCESS TO UPDATE PAGE **************************************************************
// Configurations for OTA page - When you access "http://<device-ip>/firmware" the page will ask for a
// username and password. For your security, please change both or at least the password!
//********************************************************************************************************

// Username to access the OTA page
  const char* update_username = "admin";
// Password to access the OTA page
  const char* update_password = "1a2b3c";

//************* CONFIG MQTT ******************************************************************************
// Configurations of your MQTT server -
//********************************************************************************************************

// MQTT server IP ou URL (the ip address or URL of your MQTT broker)
  const char* mqtt_server = "mqtt_server_address";
// MQTT port (usually 1883)
  int mqtt_port = 1883;
// MQTT user (the username of your MQTT broker)
  const char* mqtt_username = "mqtt_user";
// MQTT password (the password of your MQTT broker)
  const char* mqtt_password = "mqtt_password";

//************ MQTT TOPICS *******************************************************************************
// Configurations of your MQTT topics - to match your MQTT broker
//********************************************************************************************************

// MQTT topic to publish values
  const char* outTopic = "home/indoor/sensor/MAID-DWS-01_t";
// MQTT payload for door open
  const char* doorOpen = "Open";
// MQTT payload for door closed
  const char* doorClosed = "Closed";

//************* CONFIG PINS ******************************************************************************
// Configurations of the device pins - This is where you are going to connect the LEDs and sensor. You can
// change them to fit your needs. The pins refer to what is marked on the board
//********************************************************************************************************

// Door sensor pin
  int sensorPin = D1;
// Red LED pin - door opened
  int r_ledPin = D6;
// Green LED pin - door closed
  int g_ledPin = D7;
// Blue LED pin - External running indicator
  int b_ledPin = D8;
// Internal LED in NodeMCU
  int ledPin = D4;

// END
//********************************************************************************************************
