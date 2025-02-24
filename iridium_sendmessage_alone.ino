#include <SoftwareSerial.h>

SoftwareSerial rockBlockSerial(10, 11); // RX, TX (Arduino <-> RockBLOCK)

void setup() {
    Serial.begin(9600);  // Monitor output
    rockBlockSerial.begin(19200); // RockBLOCK communication

    Serial.println("Initializing RockBLOCK 9603...");

    delay(2000); // Allow RockBLOCK to initialize

    checkSignalStrength();  // Ensure satellite signal is available
    sendMessage("Hello from Arduino Uno!"); // Send test message

}

void loop() {
    // Listen for responses from RockBLOCK
    while (rockBlockSerial.available()) {
        Serial.write(rockBlockSerial.read()); // Print response to Serial Monitor
    }
}

void checkSignalStrength() {
    Serial.println("Checking signal strength...");
    clearSerialBuffer(); //clears any leftover data from the serial buffer to avoid interference
    rockBlockSerial.println("AT+CSQ"); //sends the AT command to the RockBLOCK
    delay(2000);
    readResponse();
}

void sendMessage(String msg) {
    Serial.println("Sending message: " + msg);

    // Send the command to store the message
    clearSerialBuffer();
    rockBlockSerial.print("AT+SBDWT="); 
    rockBlockSerial.println(msg);
    delay(2000);  // Wait for RockBLOCK response

    // Wait for confirmation "OK"
    if (!waitForResponse("OK")) {
        Serial.println("Error writing message to buffer.");
        return;
    }


    // Initiate message transmission
    clearSerialBuffer(); //clears buffer
    rockBlockSerial.println("AT+SBDIX");
    delay(8000);  // Give it time to transmit

    // Read and print the response
    readResponse();

    Serial.println("Message sent successfully!");
}




bool waitForResponse(String expected) { //this function waits for a specific response, called expected
    unsigned long startTime = millis();
    String response = ""; // Store full response

    while (millis() - startTime < 5000) { // 5-second timeout
        while (rockBlockSerial.available()) {
            char c = rockBlockSerial.read();
            response += c; // Collect response fron rockBlockserial.read
        }
        if (response.length() > 0) {
            Serial.println("RockBLOCK response: " + response); // Debug print
            if (response.indexOf(expected) != -1) return true; // Check for expected response
        }
    }
    Serial.println("Error: '" + expected + "' not received. Got: " + response);
    return false;
}

void clearSerialBuffer() {
    while (rockBlockSerial.available()) {
        rockBlockSerial.read();
    }
}


void readResponse() {
    while (rockBlockSerial.available()) {
        char c = rockBlockSerial.read();
        Serial.write(c);
        delay(5);  // Small delay to prevent buffer overflow
    }
    Serial.println();
}

