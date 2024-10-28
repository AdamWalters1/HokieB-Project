#include <SPI.h>
#include <MFRC522.h>
#include <Keypad.h>

#define RST_PIN 9           // Reset pin for RFID
#define SS_PIN 10           // Slave select pin for RFID
#define GREEN_LED 3         // Green LED pin
#define RED_LED 4           // Red LED pin
#define TIMEOUT 10000       // 10 seconds timeout for PIN entry

MFRC522 rfid(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;

const byte ROWS = 4;        // Keypad rows
const byte COLS = 3;        // Keypad columns
char keys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};
byte rowPins[ROWS] = {5, 6, 7, 8};      // Row pins
byte colPins[COLS] = {A1, A2, A3};      // Column pins

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

unsigned long startTime;
bool accessGranted = false;

// User data (RFID UID and corresponding PINs)
// Replace '0x00, 0x00, 0x00, 0x00' with actual RFID UIDs when known
struct User {
  byte uid[4];               // Placeholder for UID of RFID tag
  String pin;                // Unique PIN for each user
};

User users[3] = {
  {{0x00, 0x00, 0x00, 0x00}, "1111"},  // User 1
  {{0x00, 0x00, 0x00, 0x00}, "2222"},  // User 2
  {{0x00, 0x00, 0x00, 0x00}, "3333"}   // User 3
};

void setup() {
  Serial.begin(9600);
  SPI.begin();
  rfid.PCD_Init();
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(RED_LED, LOW);
  Serial.println("Scan your RFID tag...");
}

void loop() {
  // Check for an RFID card presence
  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
    int userID = getUserID();  // Find user ID based on UID
    if (userID != -1) {
      Serial.println("RFID recognized. Enter your PIN:");
      startTime = millis();    // Start timer for PIN entry
      if (verifyPIN(users[userID].pin)) {
        grantAccess();         // Correct PIN entered
      } else {
        denyAccess();          // Incorrect PIN
      }
    } else {
      Serial.println("Access Denied: Unknown RFID tag.");
      denyAccess();
    }
    rfid.PICC_HaltA();         // Stop reading
    rfid.PCD_StopCrypto1();    // Stop encryption
  }
}

// Function to get the user ID based on RFID UID
int getUserID() {
  for (int i = 0; i < 3; i++) {
    if (memcmp(rfid.uid.uidByte, users[i].uid, 4) == 0) {
      return i;   // Return index if UID matches
    }
  }
  return -1;      // Return -1 if no match
}

// Function to verify PIN input with timeout
bool verifyPIN(String correctPin) {
  String inputPin = "";
  while ((millis() - startTime) < TIMEOUT) {  // Check for timeout
    char key = keypad.getKey();
    if (key) {
      if (key == '#') {                       // '#' to submit PIN
        return inputPin == correctPin;        // Check if PIN matches
      } else if (key == '*') {                // '*' to clear entry
        inputPin = "";
        Serial.println("PIN cleared. Re-enter:");
      } else {
        inputPin += key;
        Serial.print("*");                    // Print * for each digit
      }
    }
  }
  Serial.println("\nTimeout. Please scan again.");
  return false;                               // Timeout reached
}

// Grant access function (green LED on)
void grantAccess() {
  Serial.println("\nAccess Granted!");
  digitalWrite(GREEN_LED, HIGH);
  delay(2000);                                // Hold green LED for 2 seconds
  digitalWrite(GREEN_LED, LOW);
}

// Deny access function (red LED on)
void denyAccess() {
  Serial.println("Access Denied.");
  digitalWrite(RED_LED, HIGH);
  delay(2000);                                // Hold red LED for 2 seconds
  digitalWrite(RED_LED, LOW);
}
