//
// Created by Maximilian Hofmeister on 16.07.25.
//

#include "SimModule.h"

::SimModule::SimModule(HardwareSerial* serial) {
    this->serial = serial;
    this->debug = false;
}

void ::SimModule::start(unsigned int timeout) {
    setupModem();
    delay(timeout);

    sendATCommand("AT+CLIP=1");
    sendATCommand("AT+CNMI=2,2,0,0,0");
    delay(1000);
}

void ::SimModule::shutdown() {
    sendATCommand("AT+CPOWD=1");
    digitalWrite(MODEM_POWER_ON, LOW);
}

bool ::SimModule::isActive() {
    String response = sendATCommand("AT");
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
    if(number.equals("") || text.equals("")) {
        if (debug) {
            debugSerial->println("ERROR sendSMS: number or text is empty"); //DEBUG
        }
        return false;
    }
    if(!number.startsWith("+")) {
        if (debug) {
            debugSerial->println("ERROR sendSMS: number is not in international format (e.g. +49123456789)"); //DEBUG
        }
        return false;
    }

    //max. 3 Versuche
    if (recursion >= 3) {
        return false;
    }
    if (!isActive()) {
        start();
        sendATCommand("AT+CLIP=1");
        sendATCommand("AT+CNMI=2,2,0,0,0");
    }

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
    if (response.indexOf("OK") != -1) {
        if (debug) {
            debugSerial->println("SMS erfolgreich gesendet: " + text); //DEBUG
        }
        delay(100);
        return true;
    }
    else {
        //Fehlerbehandlung
        if (debug) {
            debugSerial->println("Fehler beim Senden der SMS"); //DEBUG
        }
        if (!isActive()) {
            start();
            delay(1000);
            return sendSMS(number, text, timeout, recursion + 1);
        }
        else {
            delay(10000);
            return sendSMS(number, text, timeout, recursion + 1);
        }
    }
}

String SimModule::getSignalQuality() {
    if (!isActive()) {
        debugSerial->println("DEBUG getSignalQuality: SIM module is not active (restarting)");
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

void SimModule::setAPN(String apn, String user, String pass) {
    if (!isActive()) {
        debugSerial->println("DEBUG setAPN: SIM module is not active (restarting)");
        start();
    }

    sendATCommand("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"");
    String response = sendATCommand("AT+SAPBR=3,1,\"APN\",\"" + apn + "\"");
    if (!user.isEmpty() || !pass.isEmpty()) {
        sendATCommand("AT+SAPBR=3,1,\"USER\",\"" + user + "\"");
        sendATCommand("AT+SAPBR=3,1,\"PWD\",\"" + pass + "\"");
    }

    if (response.indexOf("OK") != -1) {
        if (debug) {
            debugSerial->println("DEBUG APN erfolgreich gesetzt: " + apn); //DEBUG
        }
    }
}

String SimModule::sendHTTPRequest(String url, String method, String body, unsigned long timeout) {
    if (!isActive()) {
        debugSerial->println("DEBUG sendHTTPRequest: SIM module is not active");
        return "?";
    }

    String response = sendATCommand("AT+SAPBR=1,1", timeout);

    sendATCommand("AT+HTTPINIT");
    sendATCommand("AT+HTTPPARA=\"CID\",1"); //Bearer Profile ID
    sendATCommand("AT+HTTPPARA=\"URL\",\"" + url + "\""); //Set URL
    sendATCommand("AT+HTTPPARA=\"CONTENT\",\"application/json\""); //Set Content-Type

    if (!body.isEmpty()) {
        response = sendATCommand("AT+HTTPDATA=" + String(body.length()) + ",10000", 10000);
        if (response.indexOf("DOWNLOAD") == -1) {
            return "?";
        }
        sendATCommand(body);
        delay(100);
    }

    String actionType = (method == "POST") ? "1" : "0";
    response = sendATCommand("AT+HTTPACTION=" + actionType, timeout);
    if (response.indexOf("OK") == -1) {
        return "?";
    }

    response = "?";
    unsigned long startTime = millis(); // Startzeit für das Timeout
    while (millis() - startTime < timeout) {
        if (serial->available()) {
            response = serial->readString();
            break;
        }
    }
    if (response.indexOf("+HTTPACTION:") != -1) {
        response = sendATCommand("AT+HTTPREAD", timeout);
    }

    sendATCommand("AT+HTTPTERM", timeout); // Terminate HTTP service
    sendATCommand("AT+SAPBR=0,1"); // Deactivate Bearer
    return response;
}
