/*
  Name:    MqttAndGpio.ino
  Created: 2/5/2020 10:32:36 PM
  Author:  Jeff
*/

#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>
#include <MitoSoft.h>
#include <StringHelper.h>
#include <ArduinoMqttClient.h>

// Network-Configuration
byte mac[] = <your mac adress> //{ 0xA8, 0x61, 0x0A, 0xAE, 0x16, 0x3D };
IPAddress ip<your ip adress> //(192, 168, 1, 200);
IPAddress broker<your broker adress> //(192, 168, 1, 100);

EthernetHelper ethHelper(mac);

EthernetClient ethClient;
PubSubClient client(broker, 1883, ethClient);
PubSubHelper mqttHelper(client, 15000, true);

//DebouncingInput emergencyUp(22); //e.g. windmeter
DebouncingInput shutterDownTaster(23);
DebouncingInput shutterUpTaster(24);
ShutterController shutter(25000, 0, -5.0, 105.0, 500, true);
InvertableOutput shutterUp(44, STANDARD); //INVERTED
InvertableOutput shutterDown(45, STANDARD); //INVERTED

DebouncingInput lightButton(25);
DigitalOutput light(46, INVERTED);

void setup() {
  Serial.begin(9600);
  Serial.println("start");
   
  ethHelper.fixIpSetup(ip);

  mqttHelper.init("TestController");

  shutter.referenceRun();
  
  mqttHelper.publish("TestController/light/state/mode", "0", true);
}

void loop() {
  String topic = "";
  String message = "";
  
  if (mqttHelper.onMessageReceived()){
    topic = mqttHelper.getLastTopic();
    message = mqttHelper.getLastMessage();
  }
  
  //***********************************************************************************************************
  //Shutter
  //
  if (shutterDownTaster.risingEdge()) {
    shutter.runDown();
  }
  else if (topic == "TestController/shutter/command/mode" && message == "down") {
    shutter.runDown();
  }
  else if (shutterUpTaster.risingEdge()) {
    shutter.runUp();
  }
  else if (topic == "TestController/shutter/command/mode" && message == "up") {
    shutter.runUp();
  }
  else if (topic == "TestController/shutter/command/mode" && message == "stop") {
    shutter.runStop();
  }
  else if (topic == "TestController/shutter/command/pos") {
    //double shutterPos = StringHelper().split(message, ';', 0).toDouble();
    //double finPos = StringHelper().split(message, ';', 1).toDouble();
    //shutter.setShutterAndFinPosition(shutterPos, finPos);
    shutter.setShutterPosition(message.toDouble());
  }

  if (shutter.started()) {
    if (1 == shutter.getDirection()) { //DOWN
      shutterUp.setOff();
      shutterDown.setOn();
      mqttHelper.publish("TestController/shutter/state/mode", "closing", false); //state_closing:
    }
    else if (2 == shutter.getDirection()) { //UP
      shutterUp.setOn();
      shutterDown.setOff();
      mqttHelper.publish("TestController/shutter/state/mode", "opening", false); //state_opening:
    }
  }

  if(shutter.running()) {
      mqttHelper.publish("TestController/shutter/state/pos", String(shutter.getPosition()), true);
  }

  if (shutter.stopped()) {
    shutterUp.setOff();
    shutterDown.setOff();
    mqttHelper.publish("TestController/shutter/state/mode", "stopped", false);
    mqttHelper.publish("TestController/shutter/state/pos", String(shutter.getPosition()), true);
    if (0.0 >= shutter.getPosition()) { 
      mqttHelper.publish("TestController/shutter/state/mode", "opened", false);
    }
    else if (100.0 <= shutter.getPosition()) {
      mqttHelper.publish("TestController/shutter/state/mode", "closed", false);
    }
  }

  //***********************************************************************************************************
  //Light
  //
  if (lightButton.risingEdge()) {
    mqttHelper.publish("TestController/light/state/mode", String(light.toggle()), true);
  }
  else if (topic == "TestController/light/command/mode" && message == "toggle") {
    mqttHelper.publish("TestController/light/state/mode", String(light.toggle()), true);
  }
  else if (topic == "TestController/light/command/mode" && message == "on") {
    mqttHelper.publish("TestController/light/state/mode", String(light.setOn()), true);
  }
  else if (topic == "TestController/light/command/mode" && message == "off") {
    mqttHelper.publish("TestController/light/state/mode", String(light.setOff()), true);
  }

  if(mqttHelper.onConnected()){
    mqttHelper.subscribe("TestController/+/command/#");
  }

  //looping
  ethHelper.loop();
  mqttHelper.loop();
  shutter.loop();
  delay(10);
}