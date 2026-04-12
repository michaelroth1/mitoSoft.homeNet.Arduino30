/*
  Name: MqttAndGpio.ino
  Short example: one cover (shutter) and one light with MQTT+GPIO
*/

#include <SPI.h>
#include <Ethernet.h>
#include <MitoSoft.h>

// network configuration (adjust to your network)
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192, 168, 2, 200);
IPAddress broker(192, 168, 2, 125);

EthernetClient ethClient;
EthernetHelper ethHelper(mac, ip, false);
MqttClient mqttClient(ethClient);
MqttHelper mqttHelper(mqttClient, 15000, false);

// cover
DebouncingInput coverBtnDown(2);
DebouncingInput coverBtnUp(3);
ShutterController cover(20000, 0);
DigitalOutput coverUp(5, STANDARD);
DigitalOutput coverDown(6, STANDARD);

// light
DebouncingInput lightBtn(7);
DigitalOutput lightOut(8, INVERTED);

void setup() {
  ethHelper.fixIpSetup();
  mqttHelper.init(broker, "MqttAndGpio", "user", "passwd");
  cover.referenceRun();
}

void loop() {
  String topic = "";
  String message = "";
  if (mqttHelper.onMessageReceived()) {
    topic = mqttHelper.getLastTopic();
    message = mqttHelper.getLastMessage();
  }

  // cover control (buttons or MQTT)
  if (coverBtnDown.risingEdge() || (topic == "MqttAndGpio/cover/command/mode" && message == "down")) {
    cover.runDown();
  } else if (coverBtnUp.risingEdge() || (topic == "MqttAndGpio/cover/command/mode" && message == "up")) {
    cover.runUp();
  } else if (topic == "MqttAndGpio/cover/command/pos") {
    cover.setShutterPosition(message.toDouble());
  }

  // light control (button or MQTT)
  if (lightBtn.risingEdge() || (topic == "MqttAndGpio/light/command/mode" && message == "toggle")) {
    mqttHelper.publish("MqttAndGpio/light/state/mode", String(lightOut.toggle()), true);
  }

  // publish cover state
  if (cover.started()) {
    if (1 == cover.getDirection()) { // DOWN
      coverUp.setOff(); coverDown.setOn();
      mqttHelper.publish("MqttAndGpio/cover/state/mode", "closing", false);
    } else if (2 == cover.getDirection()) { // UP
      coverUp.setOn(); coverDown.setOff();
      mqttHelper.publish("MqttAndGpio/cover/state/mode", "opening", false);
    }
  }
  if (cover.stopped()) {
    coverUp.setOff(); coverDown.setOff();
    mqttHelper.publish("MqttAndGpio/cover/state/mode", "stopped", false);
    mqttHelper.publish("MqttAndGpio/cover/state/pos", String(cover.getPosition()), true);
  }

  cover.loop();

  if (mqttHelper.onConnected()) mqttHelper.subscribe("MqttAndGpio/+/command/#");
  ethHelper.loop();
  mqttHelper.loop();
  delay(10);
}
