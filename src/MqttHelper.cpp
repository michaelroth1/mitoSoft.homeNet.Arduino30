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

void MqttHelper::messageReceived() {
	if (_mqttClient->connected()) {
		int messageSize = _mqttClient->parseMessage();
		if (messageSize) {
			_topic = _mqttClient->messageTopic();
			_message = "";
			while (_mqttClient->available()) {
				_message += (char)_mqttClient->read();
			}
			writeSerial("MQTT message received - Topic " + _topic + "; Message " + _message);
			_onMessageReceived = true;
		}
		else {
			_onMessageReceived = false;
		}
	}
}

void MqttHelper::init(IPAddress broker, String clientId, String user, String password) {
	this->_clientId = clientId;
	this->_brokerIP = broker;
	if (user != "" && password != "") {
		_mqttClient->setUsernamePassword(user, password);
	}

	// KeepAlive auf 15 Sekunden setzen (statt default 60s)
	// um schneller zu erkennen wenn die Verbindung abbricht
	_mqttClient->setKeepAliveInterval(15 * 1000);

	// Connection Timeout auf 5 Sekunden setzen
	// um nicht ewig auf eine Verbindung zu warten
	_mqttClient->setConnectionTimeout(5 * 1000);
}

String MqttHelper::getLastTopic() {
	return _topic;
}

String MqttHelper::getLastMessage() {
	return _message;
}

void MqttHelper::subscribe(String topic) {
	// QoS 0 verwenden! QoS 1/2 erzeugen ACK-Nachrichten vom Broker,
	// die den internen Buffer der ArduinoMqttClient Library verstopfen
	// und nach dem Senden das Empfangen von Nachrichten blockieren.
	_mqttClient->subscribe(topic, 0); 

	writeSerial("MQTT subscription - Topic " + topic);
}

void MqttHelper::publish(String topic, String message, bool retain) {
	if (_mqttClient->connected()) {
		// QoS 0 verwenden! QoS 1/2 erzeugen ACK-Nachrichten vom Broker,
	    // die den internen Buffer der ArduinoMqttClient Library verstopfen
	    // und nach dem Senden das Empfangen von Nachrichten blockieren.
		_mqttClient->beginMessage(topic, retain, 0); 
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