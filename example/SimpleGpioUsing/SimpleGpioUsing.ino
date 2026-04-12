/*
 Name: SimpleGPIOUsing.ino
 Short GPIO-only example: one shutter controlled by two buttons
*/

#include <MitoSoft.h>

// inputs (buttons)
DebouncingInput btnDown(24, INPUT_PULLUP, 50);
DebouncingInput btnUp(25, INPUT_PULLUP, 50);

// single cover (short example)
ShutterController shutter(20000, 2000);
DigitalOutput outUp(44, STANDARD);
DigitalOutput outDown(45, STANDARD);

void setup() {
  Serial.begin(9600);
  Serial.println("start SimpleGPIOUsing.ino (short)");

  // make a reference run and set a test position
  shutter.referenceRun();
  shutter.setShutterAndFinPosition(50.0, 0.0);
}

void loop() {
  // button triggered commands
  if (btnDown.risingEdge()) {
	shutter.runDown();
  } else if (btnUp.risingEdge()) {
	shutter.runUp();
  }

  // update outputs according to shutter state
  if (shutter.started()) {
	Serial.println("Started: " + shutter.getDirectionAsText());
	if (shutter.getDirection() == 1) { // DOWN
	  outUp.setOff(); outDown.setOn();
	} else { // UP
	  outUp.setOn(); outDown.setOff();
	}
  } else if (shutter.stopped()) {
	Serial.println("Stopped Pos: " + String(shutter.getPosition()));
	// deactivate outputs when stopped
	outUp.setOff(); outDown.setOff();
  }

  shutter.loop();
  delay(10);
}
