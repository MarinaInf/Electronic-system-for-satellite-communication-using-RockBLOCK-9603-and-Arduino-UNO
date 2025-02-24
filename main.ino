#include <SoftwareSerial.h>  // Include SoftwareSerial library for communication with GPS and RockBLOCK
#include <TinyGPS++.h>       // Include TinyGPS++ library to work with GPS data
TinyGPSPlus gps;             // Create a TinyGPSPlus object for GPS processing

#define RXpin A3             // GPS RX pin (receives data from GPS)
#define TXpin A2             // GPS TX pin (sends data to GPS)

SoftwareSerial GPSserial(RXpin, TXpin);  // Initialize SoftwareSerial for GPS communication

#include <U8g2lib.h>        // Include U8g2 library for OLED display
#include <Wire.h>            // Include Wire library for I2C communication with OLED

U8G2_SSD1306_128X64_NONAME_1_HW_I2C disp(U8G2_R0, U8X8_PIN_NONE);  // Create an OLED display object (128x64 resolution)

uint32_t startGetFixmS;       // Variable to store the timestamp when GPS fix starts
uint32_t endFixmS;           // Variable to store the timestamp when GPS fix ends

char latBuffer[10];           // Buffer to store the latitude as a string
char lonBuffer[10];           // Buffer to store the longitude as a string
char message[30];             // Final message to be sent (latitude and longitude)

SoftwareSerial rockBlockSerial(10, 11); // RX, TX pins for communication with RockBLOCK (pins 10 and 11)

bool messageSent = false;     // Flag to track if the message has been sent to RockBLOCK

void setup() {
    delay(1000);  // Wait for a second before starting setup
    Serial.begin(115200);   // Start Serial Monitor communication at 115200 baud
    GPSserial.begin(9600);  // Initialize GPS communication at 9600 baud

    disp.begin();  // Initialize OLED display
    disp.clear();  // Clear the display screen
    disp.firstPage();  // Start drawing on the first page of OLED

    do {
        disp.setFont(u8g2_font_helvB10_tr);  // Use a slightly smaller but still readable font
        disp.setCursor(10, 25);  // Adjust cursor position
        disp.print(F("Waiting for"));

        disp.setCursor(10, 45);  // Move to the next line
        disp.print(F("GPS Fix..."));
    } while (disp.nextPage());  // Display content on OLED screen

    startGetFixmS = millis();  // Record the start time of the GPS fix

    delay(2000); // Wait for 2 seconds to allow RockBLOCK to initialize
}



void loop() {
    if (!messageSent && gpsWaitFix(5)) {  // If message is not sent and GPS fix is acquired within 5 seconds
        Serial.println();
        Serial.println();
        Serial.print(F("Fix time: "));
        Serial.print(endFixmS - startGetFixmS);  // Print the time taken to get GPS fix
        Serial.println(F(" ms"));
        GPSserial.end();
        delay(1000);

        rockBlockSerial.begin(19200);
        Serial.println("Initializing RockBLOCK 9603..."); //Print a message on the Serial Monitor for debugging

        checkSignalStrength();

        sendMessage(message);  // Send the message containing latitude and longitude to RockBLOCK
        messageSent = true;  // Set flag to true indicating message has been sent
    } else if (messageSent) {  // If message has already been sent
        Serial.println("Message already sent. Halting transmission.");
        while (true);  // Halt the loop execution indefinitely
    }

  
}

// Function to wait for GPS fix
bool gpsWaitFix(uint16_t waitSecs) {
    uint32_t endwaitmS;
    uint8_t GPSchar;

    Serial.print(F("Waiting for GPS Fix (")); //Print a message on the Serial Monitor for debugging
    Serial.print(waitSecs);
    Serial.println(F(" seconds)"));  

    endwaitmS = millis() + (waitSecs * 1000);  // Set the timeout duration for waiting for GPS fix

    while (millis() < endwaitmS) {  // Loop until the specified time has passed
        if (GPSserial.available() > 0) {  // If there is data available from the GPS
            GPSchar = GPSserial.read();  // Read the incoming GPS data
            gps.encode(GPSchar);  // Encode the GPS data for TinyGPS++
            Serial.write(GPSchar);  // Print the raw GPS data to Serial Monitor for debugging
        }

        if (gps.location.isUpdated()) {  // If new GPS location data is available
            dtostrf(gps.location.lat(), 6, 6, latBuffer);  // Convert latitude to string and store it in latBuffer
            dtostrf(gps.location.lng(), 6, 6, lonBuffer);  // Convert longitude to string and store it in lonBuffer

            snprintf(message, sizeof(message), "Lat:%s Lon:%s", latBuffer, lonBuffer);  // Format the message with latitude and longitude

            endFixmS = millis();  // Record the time when GPS fix was obtained
            return true;  // Return true indicating a GPS fix was acquired
        }
    }
    return false;  // Return false if GPS fix is not acquired within the timeout
}

// Function to check RockBLOCK signal strength
void checkSignalStrength() {
    Serial.println("Checking signal strength..."); //Print a message on the Serial Monitor for debugging
    clearSerialBuffer();  // Clear any data in RockBLOCK serial buffer
    rockBlockSerial.println("AT+CSQ");  // Send command to check signal strength
    delay(2000);  // Wait for 2 seconds
    readResponse();  // Read and print the response from RockBLOCK
}

// Function to update the OLED screen after sending the message
void displayscreenSent() {
    disp.firstPage();  // Start drawing on the first page of OLED
    do {
        disp.setFont(u8g2_font_helvB12_tr);  // Set font for latitude and longitude
        disp.setCursor(10, 15);  // Set cursor position for displaying "Sent."
        disp.print(F("Sent."));

        disp.setCursor(10, 35);  // Set cursor position for latitude display
        disp.print(F("Lat: "));
        disp.print(latBuffer);  // Display the latitude

        disp.setCursor(10, 55);  // Set cursor position for longitude display
        disp.print(F("Lon: "));
        disp.print(lonBuffer);  // Display the longitude
    } while (disp.nextPage());  // Render the content on the OLED screen
}

// Function to send the message to RockBLOCK
void sendMessage(const char* msg) {
    Serial.println("Sending message: " + String(msg)); //Print a message on the Serial Monitor for debugging

    clearSerialBuffer();

    // Send the message to RockBLOCK buffer
    rockBlockSerial.print("AT+SBDWT=");
    rockBlockSerial.println(msg);
    delay(2000);

    if (!waitForResponse("OK")) {  
        Serial.println("Error writing message to buffer.");
        displayscreenNotSent();  // Show "Not Sent" on OLED
        return;
    }

    clearSerialBuffer();

    // Send the command to transmit the message
    Serial.println("Sending AT+SBDIX command..."); //Print a message on the Serial Monitor for debugging
    rockBlockSerial.println("AT+SBDIX");

    Serial.println("Waiting for RockBLOCK response..."); //Print a message on the Serial Monitor for debugging
    delay(10000);

    // Read and store the response
    String response = readResponse();

    // Parse the response to check if the message was sent
    if (response.indexOf("+SBDIX:") != -1) {
        int index = response.indexOf("+SBDIX:") + 8;  // Move index to where the numbers start
        int firstNumber = response.substring(index).toInt();  // Convert first number to an integer

        if (firstNumber == 0) { //If the first integer is 0, the message was sent
            Serial.println("Message successfully sent."); //Print a message on the Serial Monitor for debugging
            displayscreenSent();  // Show "Sent" on OLED
        } else {                //If the first integer is not 0, the message was not sent
            Serial.println("Message failed to send. Error code: " + String(firstNumber));
            displayscreenNotSent();  // Show "Not Sent" on OLED
        }
    } else {
        Serial.println("No valid response received.");
        displayscreenNotSent();  // Show "Not Sent" on OLED
    }
}

// Function to wait for a specific response from RockBLOCK
bool waitForResponse(String expected) {
    unsigned long startTime = millis();
    String response = "";

    while (millis() - startTime < 5000) {  // Wait for up to 5 seconds
        while (rockBlockSerial.available()) {
            char c = rockBlockSerial.read();  // Read incoming data from RockBLOCK
            response += c;  // Append the character to the response string
        }
        if (response.length() > 0) {  // If there is any response
            Serial.println("RockBLOCK response: " + response);  
            if (response.indexOf(expected) != -1) return true;  // Check if the expected response is received
        }
    }
    Serial.println("Error: Expected '" + expected + "' but got: " + response); //Print a message on the Serial Monitor for debugging
    return false;  // Return false if expected response is not received
}

// Function to clear any data in the serial buffer
void clearSerialBuffer() {
    while (rockBlockSerial.available()) {
        rockBlockSerial.read();  // Read and discard any available data
    }
}

// Function to read the response from RockBLOCK and return it as a String
String readResponse() {
    String response = "";
    unsigned long startTime = millis();
    
    // Keep reading for up to 15 seconds 
    while (millis() - startTime < 15000) {
        while (rockBlockSerial.available()) {
            char c = rockBlockSerial.read();  // Read incoming data from RockBLOCK
            response += c;  // Append character to response string
            Serial.write(c);  // Print each character as it's received
        }
        if (response.length() > 0 && response.indexOf("+SBDIX:") != -1) {
            break;  // If we detected the expected response, exit early
        }
    }

    if (response.length() > 0) {
        Serial.println("\nFull RockBLOCK Response: " + response);  // Print the full response in the Serial Monitor for debugging
    } else {
        Serial.println("No response received from RockBLOCK.");
    }

    return response;  // Return the response as a String
}


// Function to update the OLED screen when the message is not sent
void displayscreenNotSent() {
    disp.firstPage();
    do {
        disp.setFont(u8g2_font_helvB12_tr);
        disp.setCursor(10, 30);
        disp.print(F("Not sent"));
    } while (disp.nextPage());
}
