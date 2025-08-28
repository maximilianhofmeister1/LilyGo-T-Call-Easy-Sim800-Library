//
// Created by Maximilian Hofmeister on 16.07.25.
//

#ifndef SIMMODULE_H
#define SIMMODULE_H

#include <Arduino.h>
#include <HardwareSerial.h>
#include <utilities.h>

class SimModule {
public:
    /**
     * initialize the SIM module with a HardwareSerial object
     * @param serial default HardwareSerial object: Serial2.begin(115200, SERIAL_8N1, 26, 27); // RX=26, TX=27
     */
    explicit SimModule(HardwareSerial* serial);

    /**
     * power the SIM module on LillyGo T-Call Sim800L
     * @param timeout how long to wait for the SIM module to start in milliseconds, default is 10000ms (10sec)
     */
    void start(unsigned int timeout = 10000);

    /**
     * power down the SIM module
     */
    void shutdown();

    /**
     * check if the SIM module is powered on
     * @return true if the SIM module is powered on and answers on AT-Commands, false otherwise
     */
    bool isActive();


    /**
     * send custom AT command to the SIM module
     * @param command the command to send to the SIM module (e.g. 'AT+CLIP=1' to turn on caller ID)
     * @param timeout how long to wait for a response in milliseconds, default is 5000ms
     * @return the response string from the SIM module, or '?' string if no response is received
     */
    String sendATCommand(String command, unsigned long timeout = 5000);


    /**
     * send an SMS message using the SIM module
     * @param number the phone number to send the SMS to, in international format (e.g. "+49123456789")
     * @param text the text to send as SMS
     * @param timeout how long to wait for a response in milliseconds, default is 10000ms (10sec)
     * @param recursion how often to retry sending the SMS if it fails, default is 3 (3 retries)
     * @return true if the SMS was sent successfully, false otherwise
     */
    bool sendSMS(String number, String text, unsigned int timeout = 10000, byte recursion = 0);


    /**
     * get the signal quality of the SIM module
     * @return a string representing the signal quality, e.g. sehr gut, gut, mittel, schlecht, kein Signal or '?' if no response is received
     */
    String getSignalQuality();


    /**
     * set the APN (Access Point Name) for the SIM module
     * @param apn the APN to set
     */
    void setAPN(String apn, String user = "", String pass = "");


    /**
     * send an HTTP request using the SIM module
     * (! make sure to set the APN before using this function !)
     * @param url the URL to send the request to
     * @param method the HTTP method to use (e.g. "GET", "POST"), default is "GET" (currently only GET and POST are supported)
     * @param body the body of the request, default is empty string (content-type is application/json)
     * @param timeout how long to wait for a http-response in milliseconds, default is 10000ms (10sec)
     * @return the response string from the server, or '?' string if no response is received or an error occurred
     */
    String sendHTTPRequest(String url, String method = "GET", String body = "", unsigned long timeout = 10000);


    /**
     * set the HardwareSerial object for debug output
     * @param debug_serial the HardwareSerial object to use for debug output, default is Serial
     */
    void set_debug_serial(HardwareSerial* debug_serial) {
        debugSerial = debug_serial;
    }

    /**
     * enable or disable debug output (default is false)
     * @param debug true to enable debug output, false to disable it
     */
    void set_debug(bool debug) {
        this->debug = debug;
    }

private:
    HardwareSerial* serial;
    HardwareSerial* debugSerial = &Serial;
    bool debug = false;
};


#endif //SIMMODULE_H
