#include "MitoSoft.h"

DigitalOutput::DigitalOutput(int pin, int mode) {
	this->_pin = pin;
	this->_mode = mode;

	pinMode(pin, OUTPUT);
	this->setOff();
}

void DigitalOutput::setOff() {
	if (_mode == INVERTED) {
		digitalWrite(_pin, HIGH);
	}
	else {
		digitalWrite(_pin, LOW);
	}
}

void DigitalOutput::setOn() {
	if (_mode == INVERTED) {
		digitalWrite(_pin, LOW);
	}
	else {
		digitalWrite(_pin, HIGH);
	}
}