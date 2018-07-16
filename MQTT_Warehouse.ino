/*
 * 
 * ESP8266 (Adafruit HUZZAH) Mosquitto MQTT Publish Example
 * Thomas Varnish (https://github.com/tvarnish), (https://www.instructables.com/member/Tango172)
 * Made as part of my MQTT Instructable - "How to use MQTT with the Raspberry Pi and ESP8266"
 */
#include <ESP8266WiFi.h> // Enables the ESP8266 to connect to the local network (via WiFi)
#include <PubSubClient.h> // Allows us to connect to, and publish to the MQTT broker

#define INT_PIN 12

// WiFi
// Make sure to update this for your own WiFi network!
const char* ssid = "VKE_DEMO";
const char* wifi_password = "IFKADemo";
IPAddress local_ip(10,3,141,5);
IPAddress gateway(10,3,141,1); //dns is
IPAddress subnet(255,255,255,0);

// MQTT
// Make sure to update this for your own MQTT Broker!
const char* mqtt_server = "10.3.141.3";
const char* mqtt_topic;
const char* mqtt_message="A";
const char* mqtt_username = "user";
const char* mqtt_password = "user";
// The client id identifies the ESP8266 device. Think of it a bit like a hostname (Or just a name, like Greg).
const char* clientID = "warehouse";
const int mqtt_port = 1883;
char incomingByte;
char* wake_up = "start_system";

// Initialise the WiFi and MQTT Client objects
WiFiClient wifiClient;
PubSubClient client(mqtt_server, mqtt_port, wifiClient); // 1883 is the listener port for the Broker
void callback(char *topic, byte* payload, unsigned int length);

void setup() {
                                                                                                                                                                                                  
  Serial.begin(115200);
  WiFi.begin(ssid, wifi_password);
  WiFi.config(local_ip,gateway,subnet,gateway,gateway);
  pinMode(INT_PIN,OUTPUT);
  digitalWrite(INT_PIN, 1);
  

  //Serial.println("connecting to wifi");
  // Wait until the connection has been confirmed before continuing
  while (WiFi.status() != WL_CONNECTED) {
    //Serial.print(".");
    delay(500);
  }
  //Serial.println("wifi ok");
  client.setServer(mqtt_server,mqtt_port);
  client.setCallback(callback);

  if (client.connect(clientID, mqtt_username, mqtt_password)) {
    //Serial.println("Client connected");
//    client.subscribe("restart_system");
//    client.subscribe("warehouse_launch_error");
//    client.subscribe("warehouse_no_error_yet");
//    client.subscribe("gateway_error");
//    client.subscribe("gateway_error_reset");
//    client.subscribe("gateway_power_error");
//    client.subscribe("gateway_power_error_reset");
    reSub(client);
  }
  else {
    //Serial.println("Connection to MQTT Broker failed...");
  }
  
  
}

String toSend;
//String sleep = "go_to_sleep";

void loop() {
      //Serial.println(WiFi.macAddress());
      client.loop();
      if(client.connected() == false){
        while(client.connected() == false){
          client.connect(clientID, mqtt_username, mqtt_password);
          delay(50);
        }
          reSub(client);
      }

      while(Serial.available() ) {
        incomingByte = Serial.read();
        toSend += incomingByte;
      }

      if(toSend.length() > 0) {
        if(client.publish(toSend.c_str(), toSend == "go_to_sleep" ? "go_to_sleep":"this is warehouse")){         
          }
          else{
            while(client.publish(toSend.c_str(), toSend == "go_to_sleep" ? "go_to_sleep":"this is warehouse") == false){
                client.connect(clientID, mqtt_username, mqtt_password);
                delay(50);
              }
              client.publish(toSend.c_str(), toSend == "go_to_sleep" ? "go_to_sleep":"this is warehouse");
              reSub(client);                            
            }
        toSend = "";
      }

}
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.flush();
  if(*topic == *wake_up){
      digitalWrite(INT_PIN, 0);
      delay(1000);
      digitalWrite(INT_PIN, 1);
    }
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
}

void reSub(PubSubClient client){
    client.subscribe("start_system");
    client.subscribe("stop_system");
    client.subscribe("restart_system");
    client.subscribe("warehouse_launch_error");
    client.subscribe("warehouse_no_error_yet");
    client.subscribe("gateway_error");
    client.subscribe("gateway_error_reset");
    client.subscribe("gateway_power_error");
    client.subscribe("gateway_power_error_reset");
  }
