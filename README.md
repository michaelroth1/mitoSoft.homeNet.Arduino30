# MitoSoft.HomeNet.Arduino

Lightweight Arduino library to use Arduino and Opto devices in a IoT environment for smart-homes.

## includes functionality:

- debounced GPIO input (usage as pullup as well as pulldown)
- inverted GPIO output (to use with standarsd optocoupler)
- blind and jalousie controller (refennrence run, actual position, run to a specific position, syncron run, blind positioning, ...)
- MQTT helpers (included stanard interfaces for different polpular Arduino MQTT-broker)
- Ethernet helper (standar interface with reconnetion pattern for Arduino EthernetShilds)

In smart homes its easy to implement:

- blind controller,
- light controller,
- alarm controller,
- control the main door, as well as
- controll the garage doors.

## Dependencies

The library has dependencies to the following packages:

- SPI.h
- Ethernet.h
- ArduinoMqttClient.h

## Licence

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

## Examples

Blind controlling sketch:

```c++

#include <MitoSoft.h>

DebouncingInput shutter1Pos(23, INPUT_PULLUP, 50);
DebouncingInput shutter1DownButton(24, INPUT_PULLUP, 50);
DebouncingInput shutter1UpButton(25, INPUT_PULLUP, 50);
ShutterController shutter1(20000, 2000);
InvertableOutput shutter1Up(44, STANDARD); //INVERTED
InvertableOutput shutter1Down(45, STANDARD); //INVERTED

// the setup function runs once when you press reset or power the board
void setup() {
    Serial.begin(9600);

    Serial.println("start SimpleGPIOUsing.ino");

    shutter1.referenceRun();
    shutter1.setShutterAndFinPosition(50.0, 0.0);
}

// the loop function runs over and over again until power down or reset
void loop() {

    if (shutter1DownButton.risingEdge()) {
	shutter1.runDown();
    }
    else if (shutter1UpButton.risingEdge()) {
	shutter1.runUp();
    }
    else if (shutter1Pos.risingEdge()) {
 	String message = "60;50";
	double absPos = StringHelper().split(message, ';', 0).toDouble();
	double finPos = StringHelper().split(message, ';', 1).toDouble(); 
	shutter1.setPosition(absPos, finPos);
    }

    if (shutter1.started()) {
	Serial.println("Started Direction: " + shutter1.getDirectionAsText());
	if (1 == shutter1.getDirection()) { //DOWN
	    shutter1Up.setOff();
  	    shutter1Down.setOn();
	}
	else if (2 == shutter1.getDirection()) { //UP
	    shutter1Up.setOn();
	    shutter1Down.setOff();
	}
    }
    else if (shutter1.stopped()) {
	Serial.println("Stopped Pos: " + String(shutter1.getPosition()) + "; Fin-Pos: " + String(shutter1.getFinPosition()));
	shutter1Up.setOn();
	shutter1Down.setOn();
    }
	
    //looping
    shutter1.loop();
    delay(10);
}
```

Beispielsketch für die Verwendung mit MQTT (mit Paket <ArduinoMqttClient.h>):

```c++
/*
 To test, you can use the command line tool mosquitto_pub shipped with mosquitto 
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
IPAddress broker(127, 0, 0, 1);

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
```
