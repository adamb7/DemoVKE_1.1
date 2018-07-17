/*
 * ESP8266 (Adafruit HUZZAH) Mosquitto MQTT Publish Example
 * Thomas Varnish (https://github.com/tvarnish), (https://www.instructables.com/member/Tango172)
 * Made as part of my MQTT Instructable - "How to use MQTT with the Raspberry Pi and ESP8266"
 */
#include <ESP8266WiFi.h> // Enables the ESP8266 to connect to the local network (via WiFi)
#include <PubSubClient.h> // Allows us to connect to, and publish to the MQTT broker

#define INT_PIN 12
#define RECONNECT_SPEED 50
#define WIFI_RECONNECT_SPEED 500

// WiFi
// Make sure to update this for your own WiFi network!
const char* ssid = "VKE_DEMO";
const char* wifi_password = "IFKADemo";
IPAddress local_ip(10,3,141,7);
IPAddress gateway(10,3,141,1); //dns ugyanez
IPAddress subnet(255,255,255,0);

// MQTT
// Make sure to update this for your own MQTT Broker!
const char* mqtt_server = "10.3.141.3";
const char* mqtt_topic;
const char* mqtt_message="mega";
const char* mqtt_username = "user";
const char* mqtt_password = "user";
// The client id identifies the ESP8266 device. Think of it a bit like a hostname (Or just a name, like Greg).
const char* clientID = "mega";
const int mqtt_port = 1883;
char incomingByte;
String toSend;
char* toSend_char;
char* pch;
char* pch_num;
char* wake_up = "start_system";
uint32_t recon_millis;
unsigned long wl_millis;
uint8_t wifi_ready;
uint32_t millis_value;

// Initialise the WiFi and MQTT Client objects
WiFiClient wifiClient;
PubSubClient client(mqtt_server, mqtt_port, wifiClient); // 1883 is the listener port for the Broker
void callback(char *topic, byte* payload, unsigned int length);

void setup() {
                                                                                                                                                                                                  
  Serial.begin(115200);
  recon_millis = millis();
  wl_millis = millis();
  wifi_ready = 0;
  //delay(2000);
  //Serial.print("Connecting to ");
  //Serial.println(ssid);
  WiFi.begin(ssid, wifi_password);
  WiFi.config(local_ip,gateway,subnet,gateway,gateway);
  pinMode(INT_PIN,OUTPUT);
  digitalWrite(INT_PIN, 1);

  // Wait until the connection has been confirmed before continuing
//  while(wifi_ready == 0){
//     if(int(millis() - wl_millis) > 500){
//        wl_millis = millis();
//        if(WiFi.status() == WL_CONNECTED){
//            wifi_ready = 1;
//          }
//        }
//    }
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    //Serial.print("millis: ");
    //Serial.println(millis());
//    if(int(millis() - wl_millis) > 500)
//    {
//      Serial.print("millis - wl_millis: ");
//      Serial.println(millis() - wl_millis);
//      wl_millis = millis();
//    }
    
    Serial.print(".");
  }
  // Debugging - Output the IP Address of the ESP8266
  //Serial.println("WiFi connected");
  //Serial.print("IP address: ");
  //Serial.println(WiFi.localIP());

  client.setServer(mqtt_server,mqtt_port);
  client.setCallback(callback);

  if (client.connect(clientID, mqtt_username, mqtt_password)) {
    reSub(client);
    Serial.println("connect_ok");
    delay(100);
  }
  else {
    //Serial.println("Connection to MQTT Broker failed...");
  }
  
  
}



void loop() {
      //Serial.println(WiFi.macAddress());
      client.loop();
      if(client.connected() == false){
        while(client.connected() == false){
          //if(millis() - recon_millis > 1000){
              //recon_millis = millis();
              client.connect(clientID, mqtt_username, mqtt_password);
              Serial.println("no_connect");
            //}
          
          delay(50);
          //Serial.println("no_connect");
        }
          reSub(client);
          //Serial.println("reconnected");
      }
      
      while(Serial.available() ) {
        incomingByte = Serial.read();
        toSend += incomingByte;
        //Serial.println(toSend);
      }
      if(toSend.length() > 0) {
        toSend_char = (char *)calloc(toSend.length(),sizeof(char));
        toSend.toCharArray(toSend_char,toSend.length());
        pch = strtok(toSend_char,".");
        pch_num = strtok(NULL," ");
        if(client.publish((const char*)pch,(const char*)pch_num)){
        }
        else{
             while(client.publish((const char*)pch,(const char*)pch_num) == false){
                //if(millis() - recon_millis > 1000){
                    //recon_millis = millis();
                    client.connect(clientID, mqtt_username, mqtt_password);
                  //}
                delay(50);
              }
            //client.connect(clientID, mqtt_username, mqtt_password);
            //delay(50);
            client.publish((const char*)pch,(const char*)pch_num); 
            reSub(client);
        }
        free(toSend_char);
        toSend = "";
      }
      
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.flush();
//  if(*topic == *wake_up){
//    digitalWrite(INT_PIN, 0);
//    delay(1000);
//    digitalWrite(INT_PIN, 1);
//  }
  for (int i = 0; i < length; i++) {
    //Serial.print("LENGTH: ");
    //Serial.println(length);
    Serial.print((char)payload[i]);
  }
}  
void reSub(PubSubClient client){
    client.subscribe("go_to_sleep");
    client.subscribe("start_system");
    client.subscribe("restart_system");
    client.subscribe("gateway_error");
    client.subscribe("gateway_error_reset");
    client.subscribe("gateway_power_error");
    client.subscribe("gateway_power_error_reset");
    client.subscribe("belt_plc_error");
    client.subscribe("belt_plc_error_reset");
  }

