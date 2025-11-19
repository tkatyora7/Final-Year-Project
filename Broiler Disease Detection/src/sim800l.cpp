// #include "sim800l.h"



// bool SIM800L_Manager::begin()
// {
//     serial.begin(9600, SERIAL_8N1, 16, 17);

//     if (powerPin != -1) {
//         pinMode(powerPin, OUTPUT);
//         powerCycle();
//     }

//     Serial.println("Initializing modem...");

//     if (!modem.restart()) {
//         Serial.println("Failed to restart modem");
//         return false;
//     }

//     Serial.println("Waiting for network...");
//     if (!modem.waitForNetwork()) {
//         Serial.println("Network failed");
//         return false;
//     }

//     if (!modem.isNetworkConnected()) {
//         Serial.println("Network not connected");
//         return false;
//     }

//     Serial.println("Network connected");
//     Serial.print("Signal quality:");
//     Serial.println(modem.getSignalQuality());
//     return true;
// }

// bool SIM800L_Manager::sendSMS(const char* number, const char* message) {
//     Serial.println("Starting SMS send procedure...");
//     Serial.println("Checking modem basics:");
  
//   // 1. Power verification
//   Serial.print("Power status: ");
//   Serial.println(modem.isNetworkConnected() ? "OK" : "FAIL");
  
//   // 2. Signal quality (0-31, 99=unknown)
//   Serial.print("Signal quality: ");
//   Serial.println(modem.getSignalQuality());
  
//   // 3. SIM card detection
//   Serial.print("SIM status: ");
//   Serial.println(modem.getSimStatus() == 1 ? "READY" : "FAIL");
  
//   // 4. Network registration
//   Serial.print("Network: ");
//   Serial.println(modem.isNetworkConnected() ? "REGISTERED" : "NO SERVICE");
//     if(!modem.isNetworkConnected()) {
//         Serial.println("⚠️ No network connection!");
//         return false;
//     }
    
//     // 1. Verify network
//     if(!modem.isNetworkConnected()) {
//         Serial.println("⚠️ No network connection!");
//         return false;
//     }

//     // 2. Set text mode (retry 3 times)
//     for(int i=0; i<3; i++) {
//         modem.sendAT("+CMGF=1"); // Set text mode
//         if(modem.waitResponse(1000) == 1) { // 1 = OK
//             break;
//         }
//         delay(1000);
//     }

//     // 3. Send SMS with proper error handling
//     Serial.println("Attempting to send SMS...");
//     bool success = modem.sendSMS(number, message);
    
//     if (!success) {
//         Serial.println("❌ SMS send failed");
//         // Get last error
//         modem.sendAT("+CMEE=2"); // Enable verbose errors
//         modem.waitResponse();
//         modem.sendAT("+CEER"); // Get extended error
//         String response;
//         if(modem.waitResponse(1000, response)) {
//             Serial.print("Error details: ");
//             Serial.println(response);
//         }
//     } else {
//         Serial.println("✅ SMS sent successfully");
//     }
    
//     return success;
// }

// void SIM800L_Manager::debugSMSError() {
//     // 1. Check if module responds
//     modem.sendAT("AT");
//     if(modem.waitResponse(1000) != 1) {
//         Serial.println("‼️ Modem not responding - check power/connections");
//         return;
//     }

//     // 2. Check signal quality
//     modem.sendAT("+CSQ");
//     String response;
//     if(modem.waitResponse(1000, response)) {
//         Serial.print("Signal quality: ");
//         Serial.println(response);
//     }

//     // 3. Check message storage
//     modem.sendAT("+CPMS?");
//     modem.waitResponse(1000, response);
//     Serial.print("Storage info: ");
//     Serial.println(response);
// }


// void SIM800L_Manager::powerCycle() {
//     if (powerPin == -1) return;

//     digitalWrite(powerPin, LOW);
//     delay(1000);
//     digitalWrite(powerPin, HIGH);
//     delay(2000);
// }





