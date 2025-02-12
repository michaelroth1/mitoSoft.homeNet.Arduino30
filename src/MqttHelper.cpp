#include "MitoSoft.h"

//https://github.com/arduino-libraries/ArduinoMqttClient
MqttHelper::MqttHelper(MqttClient& client, unsigned long reconnectionTime, bool writeLog) {
	this->_mqttClient = &client;
	this->_writeLog = writeLog;
	this->_pollingInterval = 1000;
	this->_reconnectionTime = reconnectionTime;
	this->_actualTime = millis();
	
	_onConnected = false;
	_onMessageReceived = false;
	
	_topic = "";
	_message = "";
}

void MqttHelper::messageReceived(){	
	if (_mqttClient->connected()) {
		int messageSize = _mqttClient->parseMessage();
		if (messageSize) {
			_topic = _mqttClient->messageTopic();
			if (_mqttClient->available()) {
				_message = _mqttClient->readString();
			}
			else {
				_message = "";
			}
			writeSerial("MQTT message received - Topic " + _topic + "; Message " + _message);
			_onMessageReceived = true;
		}
		else{
			_onMessageReceived = false;
		}
	}
}

void MqttHelper::init(IPAddress broker, String clientId) {
	this->_clientId = clientId;
	this->_brokerIP = broker;
}

String MqttHelper::getLastTopic() {
	return _topic;
}

String MqttHelper::getLastMessage() {
	return _message;
}

void MqttHelper::subscribe(String topic) {
	
	_mqttClient->subscribe(topic, 2); //TODO war ohne qos Level
	
	writeSerial("MQTT subscription - Topic " + topic);
}

void MqttHelper::publish(String topic, String message, bool retain) {
	if (_mqttClient->connected()) {
		_mqttClient->beginMessage(topic, retain, 2); //TODO war ma qos Level 2
		_mqttClient->print(message);
		_mqttClient->endMessage();
		
		writeSerial("MQTT message published - Topic " + topic + "; Message " + message);
	}
}

void MqttHelper::loop() {
	if (!_mqttClient->connected()) {
	    if (millis() - _actualTime > _reconnectionTime) {
			_actualTime = millis();
			writeSerial("MQTT reconnecting... " + String(millis()) + " millis");
			this->connect();
		}
	}
	else {
		this->messageReceived();
	}
}

bool MqttHelper::connect() {
	this->_mqttClient->setId(_clientId);
	
	if (!_mqttClient->connect(_brokerIP)) {
		writeSerial("MQTT connection failed: Error code " + String(_mqttClient->connectError()));

		return false;
	}
	else {
		writeSerial("MQTT connected to broker: " + _clientId);

		_onConnected = true;
		
		return true;
	}
}

void MqttHelper::writeSerial(String text) {
	if (true == _writeLog) {
		Serial.println("MqttHelper: " + text);
	}
};

//*******************************************************************************************************************************
//EVENTS
//
bool MqttHelper::onConnected() {	
	bool connected = _onConnected;
	_onConnected = false;
	return connected;
}

bool MqttHelper::onMessageReceived() {	
	bool messageReceived = _onMessageReceived;
	_onMessageReceived = false;
	return messageReceived;
}