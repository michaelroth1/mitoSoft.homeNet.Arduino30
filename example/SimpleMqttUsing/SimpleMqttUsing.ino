/*
 Name:		SimpleMqttUsing.ino
 Created:	1/21/2020 8:49:25 PM
 Author:	Mary
 Remarks: To test, you can use the command line tool mosquitto_pub shipped with mosquitto 
 or the mosquitto-clients package to send MQTT messages. 
 This allows you to operate your cover manually:
 mosquitto_pub -h 127.0.0.1 -t input/homeAssistant/command/set -m "SHOW"
*/

#include <SPI.h>
#include <Ethernet.h>
#include <Arduino.h>
#include <MitoSoft.h>

// network configuration
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress broker(192, 168, 2, 100);

EthernetHelper ethHelper(mac);

EthernetClient ethClient;

PubSubClient mqttClient(broker, 1883, ethClient);
PubSubHelper mqttHelper(mqttClient);

unsigned long lastMillis = 0;

void setup() {
    Serial.begin(9600);
    Serial.println("start SimpleMqttUsing.ino");

    ethHelper.dhcpSetup();

    mqttHelper.init("ClientId");

    Serial.println("start loop");
}

void loop() {

  String t = "";
  String m = "";

  if (mqttHelper.onMessageReceived()){
    t = mqttHelper.getLastTopic();
    m = mqttHelper.getLastMessage();
  }    

  if (m != "") {
    Serial.println("Message received in MAIN LOOP: " + m);
  }

  // publish a message roughly every second.
  if (millis() - lastMillis > 1000) {
    lastMillis = millis();
    mqttHelper.publish("Output2", "High");
  }

  if(mqttHelper.onConnected()){
    mqttHelper.subscribe("input/+/command/#");
  }
  
  ethHelper.loop();
  mqttHelper.loop();
  delay(50);
}