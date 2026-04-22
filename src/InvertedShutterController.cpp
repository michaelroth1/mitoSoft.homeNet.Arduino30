#include "MitoSoft.h"

InvertedShutterController::InvertedShutterController(unsigned long maxRunTime, unsigned long finOpenTime, double upReferencePos, double downReferencePos, int reverseTime, bool writeLog) {
	_controller = new ShutterController(maxRunTime, finOpenTime, upReferencePos, downReferencePos, reverseTime, writeLog);
}

InvertedShutterController::~InvertedShutterController() {
	delete _controller;
}

double InvertedShutterController::ConvertPosition(double position) {
	if (position >= 0.0 && position <= 100.0) {
		return 100.0 - position;
	}
	return position;
}

void InvertedShutterController::referenceRun() {
	_controller->referenceRun();
}

void InvertedShutterController::runDown() {
	_controller->runDown();
}

void InvertedShutterController::runUp() {
	_controller->runUp();
}

void InvertedShutterController::setPosition(double newPosition, double newFinPosition) {
	newPosition = ConvertPosition(newPosition);
	newFinPosition = ConvertPosition(newFinPosition);
	_controller->setPosition(newPosition, newFinPosition);
}

void InvertedShutterController::setFinPosition(double newFinPosition) {
	newFinPosition = ConvertPosition(newFinPosition);
	_controller->setFinPosition(newFinPosition);
}

void InvertedShutterController::setShutterPosition(double newPosition) {
	newPosition = ConvertPosition(newPosition);
	_controller->setShutterPosition(newPosition);
}

void InvertedShutterController::setShutterAndFinPosition(double newPosition, double newFinPosition) {
	newPosition = ConvertPosition(newPosition);
	newFinPosition = ConvertPosition(newFinPosition);
	_controller->setShutterAndFinPosition(newPosition, newFinPosition);
}

void InvertedShutterController::runStop() {
	_controller->runStop();
}

double InvertedShutterController::getFinPosition() {
	return ConvertPosition(_controller->getFinPosition());
}

double InvertedShutterController::getPosition() {
	return ConvertPosition(_controller->getPosition());
}

int InvertedShutterController::getDirection() {
	return _controller->getDirection();
}

String InvertedShutterController::getDirectionAsText() {
	return _controller->getDirectionAsText();
}

bool InvertedShutterController::stopped() {
	return _controller->stopped();
}

bool InvertedShutterController::started() {
	return _controller->started();
}

bool InvertedShutterController::running() {
	return _controller->running();
}

bool InvertedShutterController::isReferenceRun() {
	return _controller->isReferenceRun();
}

unsigned long InvertedShutterController::lastReferenceRun() {
	return _controller->lastReferenceRun();
}

bool InvertedShutterController::isRunning() {
	return _controller->isRunning();
}

void InvertedShutterController::loop() {
	_controller->loop();
}
