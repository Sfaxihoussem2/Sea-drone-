#include <ArduinoJson.h>
#include <Servo.h>

Servo ESC1;
Servo ESC2;

// Constants and variables
String inputString = "";       // A string to hold incoming data
boolean stringComplete = false;  // Whether the string is complete
String signalLL = "$GPGLL";    // GPS Latitude and Longitude sentence
String signalVTG = "$GPVTG";   // GPS Velocity made good sentence

void setup() {
    Serial.begin(9600);   // Start primary serial for debugging
    Serial2.begin(115200); // Start secondary serial for ESP32 communication
    inputString.reserve(200); // Reserve 200 bytes for incoming string to avoid frequent reallocation
    Serial.println("Setup complete, awaiting data...");
    ESC1.attach(9); // Attach ESC1 to pin 9
    ESC2.attach(10); // Attach ESC2 to pin 10

    // Initialize the ESCs to neutral position
    ESC1.write(90);
    ESC2.write(90);
}

void loop() {
    // Handle incoming data from ESP32
    if (Serial2.available() > 0) {
        String espData = Serial2.readStringUntil('\n');
        Serial.println("Received from ESP32: " + espData);
       
        // Check if the received message contains "Value["
        int valueIndex = espData.indexOf("Value[");
        if (valueIndex != -1) {
            // Extract the speed value from the message
            int startIndex = valueIndex + 6; // Length of "Value[" is 6
            int endIndex = espData.indexOf(']', startIndex);
            String speedStr = espData.substring(startIndex, endIndex);
            int speed = speedStr.toInt();
            
            // Constrain the speed value to the appropriate range for the servo
            speed = constrain(speed, -180, 180); // Adjust range if necessary

            // Command the motors based on the speed value
            if (speed > 0) {
                ESC1.write(map(speed, 0, 180, 90, 180)); // Map positive speed to ESC1
            } else if (speed < 0) {
                ESC2.write(map(-speed, 0, 180, 90, 180)); // Map negative speed to ESC2
            }
            Serial.print("Motor command set to: ");
            Serial.println(speed);
        }
    }

    // Process complete strings received from GPS or other sources
    if (stringComplete) {
        Serial.println("Processing received string: " + inputString);
        if (inputString.startsWith(signalLL)) {
            sendLatLonAsJson();
        } else if (inputString.startsWith(signalVTG)) {
            sendSpeedAsJson();
        }

        inputString = "";    // Clear the string for new data
        stringComplete = false;
    }
}

void sendLatLonAsJson() {
    // Extract latitude
    String LAT = inputString.substring(7, 17);
    LAT = LAT.substring(0, 2) + "." + LAT.substring(2) + " N";

    // Extract longitude
    String LON = inputString.substring(20, 31);
    LON = LON.substring(0, 3) + "." + LON.substring(3) + " E";

    // Prepare JSON object
    StaticJsonDocument<200> jsonDoc;
    jsonDoc["type"] = "LatLon";
    jsonDoc["latitude"] = LAT;
    jsonDoc["longitude"] = LON;

    // Serialize JSON to string
    String jsonString;
    serializeJson(jsonDoc, jsonString);
    Serial.println("Sending JSON to ESP32: " + jsonString);
    Serial2.println(jsonString);  // Send JSON string via Serial2 to ESP32
}

void sendSpeedAsJson() {
    int startIndex = inputString.indexOf(",", 30); // Index after the speed in knots
    int endIndex = inputString.indexOf("N", startIndex); // Index of 'N' for speed in knots
    String speedKnots = inputString.substring(startIndex + 1, endIndex);

    // Prepare JSON object
    StaticJsonDocument<200> jsonDoc;
    jsonDoc["type"] = "Speed";
    jsonDoc["speed_knots"] = speedKnots + " Knots";

    // Serialize JSON to string
    String jsonString;
    serializeJson(jsonDoc, jsonString);
    Serial.println("Sending JSON to ESP32: " + jsonString);
    Serial2.println(jsonString);  // Send JSON string via Serial2 to ESP32
}

void serialEvent() {
    while (Serial.available()) {
        char inChar = (char)Serial.read();
        inputString += inChar;
        if (inChar == '\n') {
            stringComplete = true;
        }
    }
}
