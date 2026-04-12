/*
  Name: SimpleMqttUsing.ino
  Short MQTT-only example: one cover and one light
*/

#include <SPI.h>
#include <Ethernet.h>
#include <MitoSoft.h>

// network configuration
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192, 168, 2, 200);
IPAddress broker(192, 168, 2, 125);

EthernetClient ethClient;
EthernetHelper ethHelper(mac, ip, false);
MqttClient mqttClient(ethClient);
MqttHelper mqttHelper(mqttClient, 15000, false);

// single cover and light
ShutterController cover(20000, 0);
DigitalOutput coverUp(5, STANDARD);
DigitalOutput coverDown(6, STANDARD);
LightController lightOut(8, INVERTED);

void setup() {
  ethHelper.dhcpSetup();
  mqttHelper.init(broker, "SimpleMqttUsing", "user", "passwd");
  cover.referenceRun();
}

void loop() {
  String topic = "";
  String message = "";

  if (mqttHelper.onMessageReceived()) {
    topic = mqttHelper.getLastTopic();
    message = mqttHelper.getLastMessage();
  }

  // cover via MQTT
  if (topic == "SimpleMqttUsing/cover/command/mode") {
    if (message == "down") cover.runDown();
    else if (message == "up") cover.runUp();
    else if (message == "stop") cover.runStop();
  }
  else if (topic == "SimpleMqttUsing/cover/command/pos") {
    cover.setShutterPosition(message.toDouble());
  }

  // light via MQTT
  if (topic == "SimpleMqttUsing/light/command/mode") {
    if (message == "toggle") {
      mqttHelper.publish("SimpleMqttUsing/light/state/mode", String(lightOut.toggle()), true);
    }
    else if (message == "on") {
      lightOut.setOn();
      mqttHelper.publish("SimpleMqttUsing/light/state/mode", "1", true);
    }
    else if (message == "off") {
      lightOut.setOff();
      mqttHelper.publish("SimpleMqttUsing/light/state/mode", "0", true);
    }
  }

  // publish cover state when stopped
  if (cover.stopped()) {
    mqttHelper.publish("SimpleMqttUsing/cover/state/pos", String(cover.getPosition()), true);
    mqttHelper.publish("SimpleMqttUsing/cover/state/mode", "stopped", false);
  }

  cover.loop();
  lightOut.loop();

  if (mqttHelper.onConnected()) mqttHelper.subscribe("SimpleMqttUsing/+/command/#");
  ethHelper.loop();
  mqttHelper.loop();
  delay(50);
}