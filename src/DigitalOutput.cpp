#include "MitoSoft.h"

DigitalOutput::DigitalOutput(int pin, int mode) {
	// wir merken uns die Pins für die spätere Verwendung
	this->_pin = pin;
	this->_invertableOutput = new InvertableOutput(pin, mode);

	setState(LOW);
}

int DigitalOutput::toggle() {
	if (_state == HIGH) {
		_state = LOW;
	}
	else {
		_state = HIGH;
	}

	setState(_state);
	return _state;
}

bool DigitalOutput::isToggled() {
	if (_state != _oldState) {
		this->_oldState = _state;
		return true;
	}

	return false;
}

int DigitalOutput::setOn() {
	setState(HIGH);
	return HIGH;
}

int DigitalOutput::setOff() {
	setState(LOW);
	return LOW;
}

void DigitalOutput::setState(int mode) {
	this->_state = mode;
	this->_oldState = mode;
	if (mode == LOW) {
		_invertableOutput->setOff();
	}
	else if (mode == HIGH) {
		_invertableOutput->setOn();
	}
}

int DigitalOutput::getState() {
	return _state;
}