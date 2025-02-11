
#ifndef MitoSoft_h
#define MitoSoft_h

#include "Arduino.h"
#include <SPI.h>
#include <Ethernet.h>
#include <ArduinoMqttClient.h>

const int STANDARD = 1;
const int INVERTED = 2;

class InvertableOutput {

private:

	int _mode = 0;
	int _pin = 0;

public:

	InvertableOutput(int pin, int mode);

	void setOn();

	void setOff();
};

class DebouncingInput {

private:

	byte _pin;
	byte _state;
	byte _lastReading;
	bool _lastNegState;
	bool _lastPosState;
	int _pinMode;
	unsigned long _lastDebounceTime = 0;
	unsigned long _debounceTime = 10;

	byte getByteState();

public:

	DebouncingInput(byte pin, int mode = INPUT_PULLUP, unsigned long debounceTime = 50);

	bool getState();

	bool risingEdge();

	bool fallingEdge();
};

class DigitalOutput {

private:

	int _state = 0;
	int _pin = 0;
	InvertableOutput* _invertableOutput = nullptr;

public:

	DigitalOutput(int pin, int mode = STANDARD);

	int toggle();

	int setOn();

	int setOff();

	void setState(int mode);

	int getState();
};

class DigitalInput {

private:

	bool _lastNegState = false;
	bool _lastPosState = false;
	int _pin = 0;
	int _pinMode = INPUT_PULLUP;

public:

	DigitalInput(int pin, int mode = INPUT_PULLUP);

	bool getState();

	bool risingEdge();

	bool fallingEdge();
};

class ShutterController {

private:

	const int DOWN = 1;
	const int UP = 2;

	int _reverseTime;
	double _upReferencePos;
	double _downReferencePos;
	double _runToPosAfterRefRunSingle = -1.0;
	double _runToPosAfterRefRunDouble = -1.0;
	double _finPosAfterRun = -1.0;
	double _finPosAfterRefRun = -1.0;
 
	int _upPin = 0;
	int _downPin = 0;
	InvertableOutput* _up;
	InvertableOutput* _down;

	bool _isRunning = false;
	bool _isReferenceRun = false;
	bool _openFins = false;
	bool _hasStopped = false; //Event 'stopped' 
	bool _hasStarted = false; //Event 'started' 
	bool _running = false; //Event 'running' 
    bool _writeLog = true;
 
	unsigned long _lastReferenceRun = 0;
	unsigned long _shutterRunTime = 0;
	unsigned long _finOpenTime = 0;
	unsigned long _startWaitTime = 0;
	unsigned long _actualTime = 0;

	double _shutterPos = 0.0;
	double _lastReportedShutterPos = 0.0;
	double _finPos = 0.0;
	double _requestedPos = 0.0;
	int _requestedDir = 0;
	int _lastRunDirection = 0;
	int _runDir = 0;

	double CalculatePositionFromRuntime(double lastPosition, int sutterDirection, unsigned long runTime, unsigned long maxRunTime);

	double CalculateShutterCorrection(double finStartPos, double finEndPos);

	void SetVariablesToInitValue();
    
	void writeSerial(String text);
	
	void stop();
 
public:

	ShutterController(unsigned long maxRunTime, unsigned long finOpenTime = 0, double upReferencePos = -2.0, double downReferencePos = 102.0, int reverseTime = 500, bool writeLog = false);

	void referenceRun();

	void runDown();

	void runUp();

	void setPosition(double newPosition, double newFinPosition);

	void setFinPosition(double newFinPosition);

	void setShutterPosition(double newPosition);

	void setShutterAndFinPosition(double newPosition, double newFinPosition);

	void runStop();

	double getFinPosition();

	double getPosition();

	int getDirection();

	String getDirectionAsText();

	bool stopped();

	bool started();
	
	bool running();

	bool isReferenceRun();

	unsigned long lastReferenceRun();

	bool isRunning();

	void loop();
};

//https://github.com/arduino-libraries/ArduinoMqttClient
class MqttHelper {

private:

	MqttClient* _mqttClient = nullptr;

	bool _writeLog;
    String _topic;
	String _message;
	
	String _clientId;
	IPAddress _brokerIP;	
    
	unsigned long _pollingInterval = 10000;
	unsigned long _actualTime = 0;
	unsigned long _reconnectionTime = 3000;

    bool _onConnected;
	bool _onMessageReceived;
	
	void writeSerial(String text);
	
	void messageReceived(); 
	
	bool connect();
public:

	MqttHelper(MqttClient& client, unsigned long reconnectionTime = 15000, bool writeLog = false);

	void init(IPAddress broker, String clientId);

	String getLastTopic();

	String getLastMessage();

	void publish(String topic, String message, bool retain = false);

    void subscribe(String topic);
	
	void loop();
	
	bool onMessageReceived(); //Event
	
	bool onConnected(); //Event
};

#endif