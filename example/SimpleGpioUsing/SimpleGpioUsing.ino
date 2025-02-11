/*
 Name:		SimpleGPIOUsing.ino
 Created:	1/22/2020 11:47:24 PM
 Author:	Bob
*/

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