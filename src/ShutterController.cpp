#include "MitoSoftOpta.h"

ShutterController::ShutterController(unsigned long maxRunTime, unsigned long finOpenTime, double upReferencePos, double downReferencePos, int reverseTime, bool writeLog) {
	_shutterRunTime = maxRunTime;
	_reverseTime = reverseTime;
	_finOpenTime = finOpenTime;
	_upReferencePos = upReferencePos;
	_downReferencePos = downReferencePos;
	_isReferenceRun = false;
	_isRunning = false;
	_running = false;
	_hasStopped = true;
	_runDir = 0;
	_lastReportedShutterPos = 0;
	this->SetVariablesToInitValue();
	this->_writeLog = writeLog;	
}

void ShutterController::runDown() {
	if (this->isReferenceRun()) {
		return;
	}
	else if (this->isRunning()) {
		this->SetVariablesToInitValue();
		this->stop();
	}
	else if (_shutterPos >= 100.0) {
		this->SetVariablesToInitValue();
		return;
	}
	else if (_requestedDir > 0 && this->isRunning() == false) {
		this->SetVariablesToInitValue();
		_requestedDir = 0;
	}
	else {
		this->SetVariablesToInitValue();
		_requestedDir = DOWN;
		_requestedPos = _downReferencePos;
	}
}

void ShutterController::runUp() {
	if (this->isReferenceRun()) {
		return;
	}
	else if (this->isRunning()) {
		this->SetVariablesToInitValue();
		this->stop();
	}
	else if (_shutterPos <= 0.0) {
		this->SetVariablesToInitValue();
		return;
	}
	else if (_requestedDir > 0 && this->isRunning() == false) {
		this->SetVariablesToInitValue();
		_requestedDir = 0;
	}
	else {
		this->SetVariablesToInitValue();
		_requestedDir = UP;
		_requestedPos = _upReferencePos;
	}
}

void ShutterController::runStop() {
    if (this->isReferenceRun()) {
		return;
	}
	else if (this->isRunning() == false){
	    return;
	}
	this->stop();
}

void ShutterController::setPosition(double newPosition, double newFinPosition)
{
	if (newPosition == -1 && newFinPosition >= 0) {
		this->setFinPosition(newPosition);
	}
	else if (newPosition >= 0 && newFinPosition == -1) {
		this->setShutterPosition(newFinPosition);
	}
	else if (newPosition >= 0 && newFinPosition >= 0) {
		this->setShutterAndFinPosition(newPosition, newFinPosition);
	}
}

void ShutterController::setFinPosition(double newFinPosition) {
	if (newFinPosition < 0 || newFinPosition > 100) {
		return;
	}
	else if (this->isReferenceRun() || this->isRunning() || _requestedDir > 0) {
		_finPosAfterRun = newFinPosition;
		return;
	}
	else if (abs(this->getFinPosition() - newFinPosition) < 5.0) { //Abweichung muss mindestens 5% sein
		return;
	}	
	else if (newFinPosition < _finPos) { //um Lamellen aufzumachen, muss die Jalousie aufgefahern werden
		double absCorrection = this->CalculateShutterCorrection(_finPos, newFinPosition);
		this->setShutterPosition(_shutterPos - absCorrection);
	}
	else if (newFinPosition > _finPos) { //Jalousie abfahren
		double absCorrection = this->CalculateShutterCorrection(_finPos, newFinPosition);
		this->setShutterPosition(_shutterPos + absCorrection);
	}
}

void ShutterController::setShutterAndFinPosition(double newPosition, double newFinPosition) {
	if (newPosition > _downReferencePos || newPosition < _upReferencePos || newFinPosition < 0 || newFinPosition > 100) {
		return;
	}
	else if (this->isReferenceRun()) {
		_runToPosAfterRefRunDouble = newPosition;
		_finPosAfterRefRun = newFinPosition;
		return;
	}
	else if (abs(_shutterPos - newPosition) < 1.0 && abs(this->getFinPosition() - newFinPosition) < 5.0) { //Abweichung muss mindestens 1% bzw. 5% sein
		return;
	}	
	else if (_shutterPos < newPosition) { //Rolladen muss nach unten
		double absCorrection = this->CalculateShutterCorrection(100.0, newFinPosition);

		this->setShutterPosition(newPosition + absCorrection);
		this->setFinPosition(newFinPosition);
	}
	else if (_shutterPos > newPosition) { //Rolladen muss hoch
		double absCorrection = this->CalculateShutterCorrection(0.0, newFinPosition);

		this->setShutterPosition(newPosition - absCorrection);
		this->setFinPosition(newFinPosition);
	}
}

void ShutterController::setShutterPosition(double newPosition) {
	if (newPosition > _downReferencePos || newPosition < _upReferencePos) {
		return;
	}
	else if (this->isReferenceRun()) {
		_runToPosAfterRefRunSingle = newPosition;
		return;
	}
	else if (abs(_shutterPos - newPosition) < 1.0) { //Abweichung muss mindestens 1% sein
		return;
	}	
	else if (newPosition > _shutterPos) {
		_requestedDir = DOWN;
		_requestedPos = newPosition;
	}
	else if (newPosition < _shutterPos) {
		_requestedDir = UP;
		_requestedPos = newPosition;
	}
}

void ShutterController::referenceRun() {
	if (this->isReferenceRun()) {
		return;
	}
	else {
		writeSerial("Shutter started reference run.");
		_actualTime = millis();
		_shutterPos = 100.0;
		_finPos = 100.0;
		this->runUp();
		_isReferenceRun = true;
	}
}

void ShutterController::stop() {	
	_isReferenceRun = false;
	_isRunning = false;
	_hasStopped = true;
	_runDir = 0;
	
	this->writeSerial("Shutter stopped.");
	    
	if (_shutterPos < 0.0) {
		_shutterPos = 0;
		_finPos = 0;
	}
	else if (_shutterPos > 100.0) {
		_shutterPos = 100.0;
		_finPos = 100.0;
	}

	//Gespeicherte Position (Rolladen und Lamellen) nach Referenzfahrt anfahren
	if (_runToPosAfterRefRunDouble >= 0) {
		double shutterPos = _runToPosAfterRefRunDouble;
		double finPos = _finPosAfterRefRun;
		_runToPosAfterRefRunDouble = -1.0;
		_finPosAfterRefRun = -1.0;
		this->setShutterAndFinPosition(shutterPos, finPos);
	}
	//Gespeicherte Position nach Referenzfahrt anfahren
	else if (_runToPosAfterRefRunSingle >= 0) {
		double pos = _runToPosAfterRefRunSingle;
		_runToPosAfterRefRunSingle = -1.0;
		this->setShutterPosition(pos);
	}
	//Gespeicherte Lamellen-Position nach Fahrt anfahren
	else if (_finPosAfterRun >= 0) {
		double pos = _finPosAfterRun;
		_finPosAfterRun = -1.0;
		this->setFinPosition(pos);
	}
}

void ShutterController::loop() {
	//Umschaltzeit berechnen
	if (this->isRunning()) {
		_startWaitTime = millis();
	}
	bool waitTimeOver = false;
	if (millis() - _startWaitTime > _reverseTime) {
		waitTimeOver = true;
	}

	//STARTEN -> Auf- und Abwärtsfahrt 
	if (_shutterPos < 100.0 && _requestedDir == DOWN && _lastRunDirection == UP && waitTimeOver ||
		_shutterPos < 100.0 && _requestedDir == DOWN && _lastRunDirection == DOWN ||
		_shutterPos < 100.0 && _requestedDir == DOWN && _lastRunDirection == 0 && waitTimeOver) {

        this->writeSerial("Shutter started with direction 'DOWN'.");
		_runDir = DOWN;
		_hasStarted = true;
		_requestedDir = 0;
		_isRunning = true;
		_lastRunDirection = DOWN;		
	}
	else if (_shutterPos > 0.0 && _requestedDir == UP && _lastRunDirection == DOWN && waitTimeOver ||
		_shutterPos > 0.0 && _requestedDir == UP && _lastRunDirection == UP ||
		_shutterPos > 0.0 && _requestedDir == UP && _lastRunDirection == 0 && waitTimeOver) {

		this->writeSerial("Shutter started with direction 'UP'.");
		_runDir = UP;
		_hasStarted = true;
		_requestedDir = 0;
		_isRunning = true;
		_lastRunDirection = UP;
	}

	//POSITION berechnen
	unsigned long time = millis();
	unsigned long cyclicTime = time - _actualTime;
	_shutterPos = CalculatePositionFromRuntime(_shutterPos, _runDir, cyclicTime, _shutterRunTime);
	_actualTime = time;

	//POSITION regelmäßig melden -> über 'running' Event 
    if (abs(_shutterPos - _lastReportedShutterPos) > 5){
		_lastReportedShutterPos = _shutterPos;
		_running = true;
    }
	
	//LamellenPosition berechnen
	_finPos = CalculatePositionFromRuntime(_finPos, _runDir, cyclicTime, _finOpenTime);
	if (_finPos < 0.0) {
		_finPos = 0.0;
	}
	else if (_finPos > 100.0) {
		_finPos = 100.0;
	}

	//Fahrt als Referenzfahrt speichern wenn oben oder unten über den Standardpos hinaus gefahren wurde
	if (_shutterPos <= _upReferencePos || _shutterPos >= _downReferencePos) {
		_lastReferenceRun = millis();
	}

	//STOPPEN
	if (_runDir == DOWN && _shutterPos >= _requestedPos) {
		this->stop();
	}
	else if (_runDir == UP && _shutterPos <= _requestedPos) {
		this->stop();
	}
}

//*******************************************************************************************************************************
//EVENTS
//
bool ShutterController::stopped() {	
	bool hasStopped = _hasStopped;
	_hasStopped = false;
	return  hasStopped;
}

bool ShutterController::started() {
	bool hasStarted = _hasStarted;
	_hasStarted = false;
	return  hasStarted;
}

bool ShutterController::running() {
	bool running = _running;
	_running = false;
	return running;
}

//*******************************************************************************************************************************
//PUBLIC PROPERTIES
//
bool ShutterController::isReferenceRun() {
	return _isReferenceRun;
}

unsigned long  ShutterController::lastReferenceRun() {
	return _lastReferenceRun;
}

bool ShutterController::isRunning() {
	return _isRunning;
}

double ShutterController::getPosition() {
	if (_shutterPos < 0.0) {
		return 0.0;
	}
	else if (_shutterPos > 100.0) {
		return 100.0;
	}
	else {
		return _shutterPos;
	}
}

double ShutterController::getFinPosition() {
	return _finPos;
}

int ShutterController::getDirection() {
	return _runDir;
}

String ShutterController::getDirectionAsText()
{
	int dir = this->getDirection();
	if (DOWN == dir) {
		return "DOWN";
	}
	else if (UP == dir) {
		return "UP";
	}
	else {
		return "STOP";
	}
}

//*******************************************************************************************************************************
//PRIVATE
//
double ShutterController::CalculatePositionFromRuntime(double lastPosition, int sutterDirection, unsigned long runTime, unsigned long maxRunTime)
{
	if (0 == sutterDirection) {
		return lastPosition;
	}

	double newPosition = 0.0;
	double runnedWay = (double)runTime / (double)maxRunTime * 100.0;

	if (DOWN == sutterDirection)
	{
		newPosition = lastPosition + runnedWay;
	}
	else if (UP == sutterDirection)
	{
		newPosition = lastPosition - runnedWay;
	}

	return newPosition;
}

double ShutterController::CalculateShutterCorrection(double finStartPos, double finEndPos)
{
	//verhältnis bei 20Sek Laufzeit zu 2 Sek Öffnungszeit wäre z.B. 10.0 (%)
	double verhaeltnis = (double)_finOpenTime / (double)_shutterRunTime * 100.0;
	//Wenn der Behang bei 100% steht und 50% gewünscht ist ist der Fahrweg 50%
	double finCorrection = abs(finStartPos - finEndPos); //z.B. 50
	double shutterCorrectionValue = finCorrection / 100.0 * verhaeltnis; //50 / 100 * 10

	return shutterCorrectionValue;
}

void ShutterController::SetVariablesToInitValue()
{
	_runToPosAfterRefRunSingle = -1.0;
	_runToPosAfterRefRunDouble = -1.0;
	_finPosAfterRefRun = -1.0;
	_finPosAfterRun = -1.0;
};
	
void ShutterController::writeSerial(String text) {
	if (true == _writeLog) {
		Serial.println("ShutterController: " + text);
	}
};