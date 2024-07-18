//New code with modified admin functionality

#include <WiFi.h>
#include <HTTPClient.h>
#include <Keypad.h>
#include <LiquidCrystal_I2C.h>
#include <MFRC522.h>
#include <SPI.h>
#include <Adafruit_NeoPixel.h>
#include <ArduinoJson.h>
#include <Ticker.h>

// RFID pins
#define SS_PIN 5
#define RST_PIN 4

// Keypad setup
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  { '1', '2', '3', 'A' },
  { '4', '5', '6', 'B' },
  { '7', '8', '9', 'C' },
  { '*', '0', '#', 'D' }
};
byte rowPins[ROWS] = { 13, 12, 14, 27 };
byte colPins[COLS] = { 26, 25, 33, 32 };
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// LCD setup
LiquidCrystal_I2C lcd(0x27, 16, 2);

// RFID setup
MFRC522 rfid(SS_PIN, RST_PIN);

// NeoPixel setup
#define NEOPIXEL_PIN 15  // Pin connected to the NeoPixels
#define NUM_PIXELS 16    // Number of NeoPixels
#define DELAY_MS 100     // Animation delay in milliseconds

Adafruit_NeoPixel pixels(NUM_PIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

// WiFi credentials and server URL
const char* ssid = "Mahima";
const char* password = "mahima123";
const char* serverURL = "http://172.20.10.5:4000/getBoardData/";

bool adminMode = false;

String adminKey = "1234";

int uniqueId = 40005;

int deviceLocationId = 1;  //Main Gate
//192.168.25.10

Ticker timer;
bool timeElapsed = false;
bool timerStarted = false;


void onTimer() {
  timeElapsed = true;
}

void startTimer() {
  timerStarted = true;
  timeElapsed = false;
  timer.attach(5, onTimer);  // Start the timer with a 5-second interval
  Serial.println("Timer started. You have 5 seconds to enter '*'.");
}

void setup() {
  Serial.begin(115200);

  // Initialize LCD
  lcd.init();
  lcd.backlight();

  // Initialize RFID
  SPI.begin();
  rfid.PCD_Init();

  // Initialize NeoPixel
  pixels.begin();

  // WiFi connection
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    // Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
}

void loop() {
  // adminLogin();
  // scanRFID();
  Serial.println("Executing scanCard");
  scanCard();
}

// void adminLogin() {
//   adminMode = false;
//   lcd.clear();
//   lcd.setCursor(0, 0);
//   lcd.print("Enter Admin Key:");
//   String enteredKey = "";

//   while (true) {
//     char key = keypad.getKey();
//     if (key) {
//       if (key == '#') {
//         if (enteredKey == "1234") {  // Change "1234" to your admin password
//           lcd.clear();
//           lcd.print("Login Successful");
//           delay(2000);
//           adminMode = true;
//           lcd.clear();
//           return;
//         } else {
//           lcd.clear();
//           lcd.print("Invalid Key");
//           delay(2000);
//           adminMode = false;
//           enteredKey = "";
//           lcd.clear();
//           lcd.print("Enter Admin Key:");
//         }
//       } else {
//         enteredKey += key;
//         lcd.setCursor(0, 1);
//         lcd.print(enteredKey);
//       }
//     }
//   }
// }

// void scanRFID() {
//   lcd.clear();
//   lcd.print("Scan RFID Card");
//   while (true) {
//     if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
//       String uid = "";
//       for (byte i = 0; i < rfid.uid.size; i++) {
//         uid += String(rfid.uid.uidByte[i] < 0x10 ? "0" : "");
//         uid += String(rfid.uid.uidByte[i], HEX);
//       }
//       uid.toUpperCase();
//       lcd.clear();
//       lcd.print("RFID: " + uid);
//       delay(2000);
//       lcd.clear();
//       lcd.print("Data Sending...");
//       delay(1500);
//       lcd.clear();


//       sendToServer(uid);
//       rfid.PICC_HaltA();
//       rfid.PCD_StopCrypto1();

//       lcd.print("Sent Success");
//       delay(1000);
//       lcd.clear();

//       lcd.print("Scan RFID Card:");
//     }
//   }
// }

void scanCard() {
  bool cardTapped = false;
  bool adminMode = false;

  lcd.clear();
  lcd.print("Scan RFID Card");

  while (!cardTapped) {
    loadingAnimation(0x00, 0x00, 0xFF);
    bool isCardPresent = rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial();
    if (isCardPresent) {
      String uid = "";
      for (byte i = 0; i < rfid.uid.size; i++) {
        uid += String(rfid.uid.uidByte[i] < 0x10 ? "0" : "");
        uid += String(rfid.uid.uidByte[i], HEX);
      }

      uid.toUpperCase();
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("RFID: " + uid);

      cardTapped = true;
      delay(2000);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Enter * to register");

      String userInput = "";
      if (!timerStarted) {
        startTimer();
      }

      unsigned long startMillis = millis();

      while (!adminMode && !timeElapsed) {
        char key = keypad.getKey();
        if (key) {
          if (key == '*') {
            lcd.clear();
            timer.detach();
            lcd.print("Enter Admin Key");
            int enterCount = 0;
            while (!adminMode) {
              key = keypad.getKey();
              if (key) {
                if (key == '#') {
                  if (userInput == adminKey) {
                    adminMode = true;
                    lcd.clear();
                    lcd.print("Login Successful");
                    delay(2000);
                    lcd.clear();
                    lcd.print("Sending Data");
                    sendDataToServer(uid, true);
                  } 
                  else if(enterCount == 2){
                    lcd.clear();
                    lcd.print("Too many attempts");
                    delay(2000);
                    return;
                  }
                  else {
                    lcd.clear();
                    lcd.print("Invalid Key");
                    delay(2000);
                    adminMode = false;
                    userInput = "";
                    lcd.clear();
                    enterCount++;
                    lcd.print("Enter Admin Key");
                  }
                } else {
                  userInput += key;
                  lcd.setCursor(0, 1);
                  lcd.print(userInput);
                }
              }
            }
            timerStarted = false;
            return;
          }
        }

        if (timeElapsed || millis() - startMillis >= 5000) {
          lcd.clear();
          lcd.print("5 seconds expired");
          delay(2000);
          lcd.clear();
          lcd.print("Sending Data");
          delay(2000);
          lcd.clear();
          sendDataToServer(uid, false);
          timeElapsed = false;
          timerStarted = false;
          timer.detach();
          return; // Exit the function to allow loop() to run again
        }
      }
      rfid.PICC_HaltA();
      rfid.PCD_StopCrypto1();
      delay(1000);
      lcd.clear();
      return; // Exit the function to allow loop() to run again
    }
  }
}

// void sendToServer(String uid) {
//   if (WiFi.status() == WL_CONNECTED) {
//     // Blue color loading animation before sending data
//     loadingAnimation(0x00, 0x00, 0xFF);

//     HTTPClient http;
//     http.begin(serverURL);  // Use the new endpoint URL
//     http.addHeader("Content-Type", "application/json");

//     // Prepare JSON payload
//     StaticJsonDocument<200> doc;
//     doc["RFIDKey"] = uid;
//     doc["adminMode"] = adminMode;
//     doc["deviceId"] = uniqueId;
//     doc["deviceLocationId"] = deviceLocationId;

//     String jsonData;
//     serializeJson(doc, jsonData);

//     int httpResponseCode = http.POST(jsonData);

//     if (httpResponseCode > 0) {
//       String response = http.getString();
//       Serial.println(httpResponseCode);
//       Serial.println(response);
//       // Green color animation for successful data transmission
//       loadingAnimation(0x00, 0xFF, 0x00);
//     } else {
//       Serial.print("Error on sending POST: ");
//       Serial.println(httpResponseCode);
//       // Red color animation for failed data transmission
//       loadingAnimation(0xFF, 0x00, 0x00);
//     }
//     http.end();
//   } else {
//     Serial.println("Error in WiFi connection");
//   }
// }

void sendDataToServer(String uid, bool registerMode) {
  if (WiFi.status() == WL_CONNECTED) {
    // Blue color loading animation before sending data
    loadingAnimation(0x00, 0x00, 0xFF);

    HTTPClient http;
    http.begin(serverURL);  // Use the new endpoint URL
    http.addHeader("Content-Type", "application/json");

    // Prepare JSON payload
    StaticJsonDocument<200> doc;
    doc["RFIDKey"] = uid;
    doc["registerMode"] = registerMode;
    doc["deviceId"] = uniqueId;
    doc["deviceLocationId"] = deviceLocationId;

    String jsonData;
    serializeJson(doc, jsonData);

    int httpStatusCode = http.POST(jsonData);

    if(httpStatusCode >= 200 && httpStatusCode < 300)
    {
      loadingAnimation(0x00, 0xFF, 0x00);
      lcd.clear();
      lcd.print("Data sent succeccfully");
      delay(3000);
      lcd.clear();
      loadingAnimation(0x00, 0x00, 0xFF);
      return;
    }
    else{
      loadingAnimation(0xFF, 0x00, 0x00);
      lcd.clear();
      lcd.print("Unable to send");
      delay(3000);
      lcd.clear();
      loadingAnimation(0x00, 0x00, 0xFF);
      return;
    }
    http.end();
  } else {
    Serial.println("Error in WiFi connection");
  }
}

//---------------------------------------------------------------------------------
// Loading animation function
void loadingAnimation(uint8_t red, uint8_t green, uint8_t blue) {
  for (int i = 0; i < NUM_PIXELS; i++) {
    // Clear all pixels
    pixels.clear();

    // Set the pixels from 0 to i (inclusive) to the specified color
    for (int j = 0; j <= i; j++) {
      pixels.setPixelColor(j, pixels.Color(red, green, blue));
    }

    // Show the updated pixels
    pixels.show();

    // Delay between each pixel update
    delay(DELAY_MS);
  }
}
