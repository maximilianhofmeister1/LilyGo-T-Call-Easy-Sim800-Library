//
// Created by Maximilian Hofmeister on 16.07.25.
//

#include "SimModule.h"

::SimModule::SimModule(HardwareSerial *serial) {
    this->serial = serial;
    this->debug = false;
}

void ::SimModule::start(unsigned int timeout) {
    setupModem();
    delay(timeout);
}

void ::SimModule::shutdown() {
    sendATCommand("AT+CPOWD=1");
    digitalWrite(MODEM_POWER_ON, LOW);
}

bool ::SimModule::isActive() {
    String response = sendATCommand("AT");
    debugSerial->println("DEBUG isActiveResponse: " + response + " index:" + response.indexOf("OK")); //DEBUG
    if (response.indexOf("OK") != -1) {
        return true;
    }
    return false;
}

String SimModule::sendATCommand(String command, unsigned long timeout) {
    String response = "?";
    serial->print(command + "\r"); // send the AT-Command to the module
    unsigned long startTime = millis(); // Start time for the timeout
    while (millis() - startTime < timeout) {
        if (serial->available()) {
            response = serial->readString();
            break;
        }
    }

    if (debug) {
        debugSerial->println("DEBUG AT Command: " + command);
        debugSerial->println("DEBUG Response: " + response);
    }

    return response;
}

bool ::SimModule::sendSMS(String number, String text, unsigned int timeout, byte recursion) {
    //max. 3 Versuche
    if (recursion >= 3) {
        return false;
    }
    if (!isActive()) {
        start();
        sendATCommand("AT+CLIP=1");
        sendATCommand("AT+CNMI=2,2,0,0,0");
    }

    digitalWrite(LED_GPIO, HIGH); // LED an
    delay(100);
    sendATCommand("AT+CMGF=1", timeout);
    delay(1000);
    sendATCommand("AT+CMGS=\"" + number + "\"", timeout);
    delay(1000);
    sendATCommand(text, timeout);
    delay(100);
    serial->print(char(26));
    delay(2000);

    //Response auswerten
    String response = "SMS_Response_?";
    unsigned long startTime = millis(); // Startzeit für das Timeout
    while (millis() - startTime < timeout) {
        if (serial->available()) {
            response = serial->readString();
            break;
        }
    }
    if (response.indexOf("OK") != -1 && debug) {
        debugSerial->println("SMS erfolgreich gesendet: " + text); //DEBUG
    }
    else {
        //Fehlerbehandlung
        if (debug) {
            debugSerial->println("Fehler beim Senden der SMS"); //DEBUG
        }
        if (!isActive()) {
            start();
            sendSMS(number, text, timeout, recursion + 1);
        } else {
            delay(10000);
            sendSMS(number, text, timeout, recursion + 1);
        }
    }

    delay(100);
    digitalWrite(LED_GPIO, LOW); // LED aus
}

String SimModule::getSignalQuality() {
    if (!isActive()) {
        debugSerial->println("DEBUG getSignalQuality: SIM module is not active");
        start();
    }

    String response = sendATCommand("AT+CSQ");
    if (response.equals("?")) {
        return "?";
    }
    if (response.length() > 0) {
        int startIndex = response.indexOf("+CSQ: ");
        if (startIndex != -1) {
            startIndex += 6; // Position nach "+CSQ: "
            int endIndex = response.indexOf(",", startIndex);
            if (endIndex != -1) {
                String rssiStr = response.substring(startIndex, endIndex);
                int rssi = rssiStr.toInt();

                // Signalqualität bewerten
                if (rssi >= 20) return "sehr gut";
                if (rssi >= 15) return "gut";
                if (rssi >= 10) return "mittel";
                if (rssi >= 1) return "schlecht";
                return "kein Signal";
            }
        }
    }
    return "?";
}
