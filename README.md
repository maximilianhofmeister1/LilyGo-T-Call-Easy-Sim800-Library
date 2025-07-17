# LilyGo-T-Call-Easy-Sim800-Library
This library should be an easy and reliable way for beginners to simply use the Sim800 basic functionalitys of LilyGo's T-Call Sim800 board.

> [!NOTE]
> For more information regarding the LilyGo T-Call Sim800 Module visit the GitHub-Repository [LilyGo-T-Call-SIM800](https://github.com/Xinyuan-LilyGO/LilyGo-T-Call-SIM800)

<br>

## Functions included:
- `start()`
- `shutdown()`
- `isActive()`
- `getSignalQuality()`
- `sendSMS()`
- `sendATCommand()` -> to unlock all functionality of the module. [See AT-Command Manual](https://github.com/Xinyuan-LilyGO/LilyGo-T-Call-SIM800/blob/master/datasheet/sim800_series_at_command_manual_v1.01.pdf)
- more coming soon ...

<br>

## How to use the Library in your projects:
1. add all 3 files to your source-directory of your project
2. include `SimModule.h`
3. Initialize the SimModule with the HardwareSerial: `SimModule module(&Serial2);`
4. begin the Serial Transmission: `Serial2.begin(115200, SERIAL_8N1, 26, 27);`
5. start the module: `module.start();`
6. eventually active debug output: `module.set_debug(true);`
7. You're ready to go! (For further help see demo-code below)

<br>

## Demo Code
```
#include <Arduino.h>
#include <SimModule.h> //include SimModule

SimModule module(&Serial2); // Initialize the SimModule with Serial2

void setup() {
    Serial.begin(9600); // Initialize Serial for debugging
    Serial2.begin(115200, SERIAL_8N1, 26, 27); // Initialize Serial2 for the SIM module

    module.start(); // Power-On the SIM module

    module.set_debug(true); //optional: Enable debug output
    module.set_debug_serial(&Serial); //optional: Set the debug serial to Serial for output
}

void loop() {
    Serial.println(module.isActive()); // Check if the SIM module is active and ready to use

    Serial.println(module.getSignalQuality()); // Get the signal quality of the SIM module in a human-readable format

    Serial.println(module.sendATCommand("AT+CBC")); // Send a custom AT command to the SIM module, that returns the response (e.g. battery status)
    
    module.sendSMS("+49123456789", "Hello from ESP32!"); // Send an SMS message using the SIM module

    module.shutdown(); //optional: Power down the SIM module (to save power when not in use)

    delay(10000);
}
```
