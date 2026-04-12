#include "MitoSoft.h"

LightController::LightController(int pin, int mode) {
	// wir merken uns die Pins für die spätere Verwendung
	this->_pin = pin;
	this->_digitalOutput = new DigitalOutput(pin, mode);

	setState(LOW);
	this->_oldState = _state;
}

int LightController::toggle() {
	if (_state == HIGH) {
		_state = LOW;
	}
	else {
		_state = HIGH;
	}

	setState(_state);
	return _state;
}

bool LightController::isToggled() {
	if (_state != _oldState) {		
		return true;
	}
	else {
		return false;
	}
}

int LightController::setOn() {
	setState(HIGH);
	return HIGH;
}

int LightController::setOff() {
	setState(LOW);
	return LOW;
}

void LightController::setState(int mode) {
	this->_state = mode;
	if (mode == LOW) {
		_digitalOutput->setOff();
	}
	else if (mode == HIGH) {
		_digitalOutput->setOn();
	}
}

int LightController::getState() {
	return _state;
}

void LightController::loop() {
	this->_oldState = _state;
}