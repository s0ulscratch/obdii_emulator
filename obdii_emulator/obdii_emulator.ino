#include <ArduinoBLE.h>  // BLE library for Bluetooth communication

// BLE service and characteristic for OBD2
BLEService elm327Service("000018F0-0000-1000-8000-00805F9B34FB");
BLECharacteristic elm327Characteristic("00002AF1-0000-1000-8000-00805F9B34FB", BLERead | BLEWrite | BLENotify, 20);

// Function prototypes
void processATCommand(String command);
void handleOBD2Request(String request);
void sendOBD2Response(int pid, int value);
int getWheelSpeed();
int getRPM();
int getEngineTemp();

volatile int pulseCount = 0;
unsigned long lastTime = 0;
bool isConnected = false;  // Connection flag to prevent repetitive messages
const float wheelCircumference = 2.1;  // in meters

void setup() {
  Serial.begin(115200);

  // Initialize BLE
  if (!BLE.begin()) {
    Serial.println("BLE initialization failed!");
    while (1);
  }

  // Set the BLE local name and service
  BLE.setLocalName("ELM327_Emulator");  // BLE name that OBD apps will see
  BLE.setAdvertisedService(elm327Service);
  elm327Service.addCharacteristic(elm327Characteristic);
  BLE.addService(elm327Service);

  // Set advertising interval (160 corresponds to 100ms)
  BLE.setAdvertisingInterval(160);  // Adjust the interval if needed

  // Start BLE advertising
  BLE.advertise();

  Serial.println("BLE OBD2 emulator started, waiting for connections...");
}

void loop() {
  BLEDevice central = BLE.central();  // Check for a connected central device (phone, etc.)

  if (central) {
    if (central.connected() && !isConnected) {
      // First time connection detected
      Serial.print("Connected to central: ");
      Serial.println(central.address());
      isConnected = true;  // Set the flag to avoid printing again
    }

    // When the central sends data
    if (elm327Characteristic.written()) {
      String command = String((char*)elm327Characteristic.value());
      Serial.print("Received command: ");
      Serial.println(command);

      // Process the command
      if (command.startsWith("AT")) {
        processATCommand(command);  // Handle AT commands
      } else {
        handleOBD2Request(command);  // Handle OBD2 requests
      }
    }
  } else if (isConnected) {
    // If the central device disconnects
    Serial.println("Disconnected from central.");
    isConnected = false;  // Reset the flag when disconnected
  }

  // Simulate vehicle speed, RPM, and engine temperature every 5 seconds (5000 ms)
  unsigned long currentTime = millis();
  if (currentTime - lastTime >= 5000) {  // Change from 1000 to 5000 for a 5-second interval
    lastTime = currentTime;

    int speed = getWheelSpeed();
    int rpm = getRPM();
    int temp = getEngineTemp();

    Serial.print("Speed: ");
    Serial.print(speed);
    Serial.println(" km/h");

    Serial.print("RPM: ");
    Serial.println(rpm);

    Serial.print("Engine Temp: ");
    Serial.print(temp);
    Serial.println(" °C");
  }
}

// Handle AT commands
void processATCommand(String command) {
  command.trim();  // Remove leading and trailing whitespace
  command.toUpperCase();  // Normalize to uppercase for case-insensitive comparison

  // Limit the command to the first 3 characters to avoid extra data being appended
  String cleanCommand = command.substring(0, 3);

  // Debugging: Print raw command and cleaned command for inspection
  Serial.print("Raw received command: ");
  Serial.println(command);
  Serial.print("Cleaned command: ");
  Serial.println(cleanCommand);

  if (cleanCommand.equals("ATZ")) {  // Reset command
    elm327Characteristic.writeValue("ELM327 v1.5");
    Serial.println("Reply: ELM327 v1.5");  // Output to Serial Monitor
  } 
  else if (cleanCommand.equals("ATI")) {  // Identify command
    elm327Characteristic.writeValue("ELM327 v1.5");
    Serial.println("Reply: ELM327 v1.5");  // Output to Serial Monitor
  } 
  else if (cleanCommand.equals("ATE")) {  // Echo off (assuming ATE0)
    elm327Characteristic.writeValue("OK");
    Serial.println("Reply: OK (Echo off)");  // Output to Serial Monitor
  } 
  else if (cleanCommand.equals("ATH")) {  // Headers off (assuming ATH0)
    elm327Characteristic.writeValue("OK");
    Serial.println("Reply: OK (Headers off)");  // Output to Serial Monitor
  } 
  else if (cleanCommand.equals("ATS")) {  // Set protocol to auto (assuming ATSP0)
    elm327Characteristic.writeValue("OK");
    Serial.println("Reply: OK (Protocol auto)");  // Output to Serial Monitor
  } 
  else {
    elm327Characteristic.writeValue("ERROR");  // Unrecognized command
    Serial.println("Reply: ERROR");  // Output to Serial Monitor
  }
}

// Handle OBD2 PID requests
void handleOBD2Request(String request) {
  if (request == "010C") {  // RPM PID
    int rpm = getRPM();
    sendOBD2Response(0x0C, rpm);
    Serial.print("Reply (RPM): ");
    Serial.println(rpm);
  } else if (request == "010D") {  // Speed PID
    int speed = getWheelSpeed();
    sendOBD2Response(0x0D, speed);
    Serial.print("Reply (Speed): ");
    Serial.println(speed);
  } else if (request == "0105") {  // Engine coolant temperature PID
    int temp = getEngineTemp();
    sendOBD2Response(0x05, temp);
    Serial.print("Reply (Temp): ");
    Serial.println(temp);
  } else {
    elm327Characteristic.writeValue("NO DATA");  // Unsupported PID
    Serial.println("Reply: NO DATA");
  }
}

// Send OBD2 response for a given PID
void sendOBD2Response(int pid, int value) {
  String response = "41" + String(pid, HEX) + " " + String(value, HEX);
  elm327Characteristic.writeValue(response.c_str());  // Respond to OBD2 apps
}

// Simulate vehicle speed based on reed switch
int getWheelSpeed() {
  float distancePerPulse = wheelCircumference / 1000.0;  // Convert to kilometers
  float speed = (pulseCount * distancePerPulse) * 3600;  // km/h
  pulseCount = 0;  // Reset for the next calculation
  return (int)speed;
}

// Simulate RPM and engine temperature
int getRPM() {
  return random(800, 3000);  // Simulate RPM between 800 and 3000
}

int getEngineTemp() {
  return random(70, 100);  // Simulate temperature between 70°C and 100°C
}

void countPulse() {
  pulseCount++;
}
