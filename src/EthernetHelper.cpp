#include "MitoSoft.h"

//Beispiel: so werden statische Variablen erzeugt
//bool EthernetHelper::_writeLog;

EthernetHelper::EthernetHelper(byte mac[6], IPAddress ip, bool writeLog) {
	this->_mac = mac;
	this->_ip = ip;
	this->_writeLog = writeLog;
}

void EthernetHelper::fixIpSetup() {
	writeSerial("Network connecting via static IP...");

	this->_mode = FIXIP;

	disableSDCard();

	Ethernet.begin(_mac, _ip);

	delay(2000);

	this->check();
}

void EthernetHelper::dhcpSetup() {
	writeSerial("Network connecting via DHCP...");

	this->_mode = DHCP;

	disableSDCard();

	if (Ethernet.begin(_mac) == 0) {
		writeSerial("DHCP error, use static IP...");

		this->fixIpSetup();
	}
	else {
		this->check();
	}
}

void EthernetHelper::check() {
	if (Ethernet.linkStatus() == LinkON) {
		String ip = ipToString(Ethernet.localIP());
		writeSerial("Network connected and IP assigned: IP " + ip);
	}
	else if (Ethernet.linkStatus() == LinkOFF) {
		writeSerial("Link status Off");
	}
	else if (Ethernet.linkStatus() == Unknown) {
		writeSerial("Link status unknown.");
	}
}

String EthernetHelper::ipToString(IPAddress ip) {
	return String(ip[0]) + "." +
		String(ip[1]) + "." +
		String(ip[2]) + "." +
		String(ip[3]);
}

//https://blog.startingelectronics.com/disabling-the-ethernet-chip-and-sd-card-on-an-arduino-ethernet-shield/
void EthernetHelper::disableSDCard() {
	pinMode(4, OUTPUT);
	digitalWrite(4, HIGH);
}

void EthernetHelper::loop() {

	if (DHCP == _mode) {
		if (Ethernet.maintain() % 2 == 1) { //entweder 1 oder 3 (...failed) https://www.arduino.cc/en/Reference/EthernetMaintain
			// Cable disconnected or DHCP server hosed
			writeSerial("Network reconnecting..." + String(millis()));
			this->dhcpSetup();
		}
	}
}

void EthernetHelper::writeSerial(String text) {
	if (true == _writeLog) {
		Serial.println("EthernetHelper: " + text);
	}
};