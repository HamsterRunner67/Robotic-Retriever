///////////RFID and screen
//Written by Nick Koumaris
//edits by @TechWithRita
//Modified to read and display data from RFID tag

/*
 *        ** PINOUT FOR ARDUINO NANO, 128X64 OLED I2C, RFID-RC522  <<>>  SUPER DUPER EASY SETUP!!! **
 *
 *   RFID-RC522: SDA --> ARDUINO NANO D10                           |   OLED: SDA --> ARDUINO NANO A4
 *               SCK --> ARDUINO NANO D13                           |         SCL --> A.N. A5
 *               MOSI --> (A.N, you get the point) D11              |         VCC --> 5V BREADBOARD / ARDUINO NANO SOURCED 5V OUTPUT PIN
 *               MISO --> D12                                       |         GND --> GROUND OR NEGATIVE TERMINAL ON BREADBOARD
 *               IRQ --> NOT USED                                   |
 *               GND --> GROUND OR NEGATIVE TERMINAL ON BREADBOARD  |
 *               RST --> D9
 *               3.3V --> 3V3 PIN ON ARDUINO (NOT THE BREADBOARD'S 5V SOURCE, THAT'S FOR THE OLED DISPLAY ONLY)
*/

#include <MFRC522.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SPI.h>

#define OLED_RESET -1
Adafruit_SSD1306 display(OLED_RESET);
#define SS_PIN 10
#define RST_PIN 9
#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels

MFRC522 rfid(SS_PIN, RST_PIN);  // Instance of the class
MFRC522::MIFARE_Key key;
int code[] = { 48, 13, 137, 34 };  //This is the stored UID
int codeRead = 0;
String uidString;
String tagContent = "";  // Store the content read from tag

void setup() {
  Serial.begin(9600);
  SPI.begin();      // Init SPI bus
  rfid.PCD_Init();  // Init MFRC522

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3D (if 0x3C doesn't work) ... (both 128*64 AND 128*32 can have either code address)
  display.clearDisplay();
  display.display();
  
  // Initialize the RFID key
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;  // Default key FFFFFFFFFFFF
  }
  
  // Display initial message
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Ready to scan...");
  display.display();
}

void loop() {
  if (rfid.PICC_IsNewCardPresent()) {
    readRFID();
  }
  delay(100);
}

void readRFID() {
  if (!rfid.PICC_ReadCardSerial()) {
    return;
  }
  
  Serial.print(F("\nPICC type: "));
  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
  Serial.println(rfid.PICC_GetTypeName(piccType));
  
  // Store UID
  uidString = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    uidString += String(rfid.uid.uidByte[i] < 0x10 ? "0" : "");
    uidString += String(rfid.uid.uidByte[i], HEX);
    if (i < rfid.uid.size - 1) uidString += " ";
  }
  uidString.toUpperCase();
  
  // Read data from the tag
  tagContent = readTagData();
  
  // Print to Serial Monitor
  Serial.println("UID: " + uidString);
  Serial.println("Tag Content: " + tagContent);
  
  // Display on OLED
  displayTagContent();
  
  // Halt PICC and stop crypto
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}

String readTagData() {
  String content = "";
  byte buffer[18];
  byte size = sizeof(buffer);
  
  // Try to read block 4 (common data block for MIFARE Classic)
  // You can change the block number to read from different blocks
  MFRC522::StatusCode status = rfid.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 4, &key, &(rfid.uid));
  
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Authentication failed: "));
    Serial.println(rfid.GetStatusCodeName(status));
    return "Authentication failed";
  }
  
  // Read data from block 4
  status = rfid.MIFARE_Read(4, buffer, &size);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Reading failed: "));
    Serial.println(rfid.GetStatusCodeName(status));
    return "Read failed";
  }
  
  // Convert buffer to string
  for (byte i = 0; i < 16; i++) {
    if (buffer[i] >= 32 && buffer[i] <= 126) {  // Printable ASCII characters
      content += (char)buffer[i];
    }
  }
  
  // Trim null characters
  content.trim();
  
  // If no readable data found, try block 8
  if (content.length() == 0) {
    status = rfid.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 8, &key, &(rfid.uid));
    if (status == MFRC522::STATUS_OK) {
      status = rfid.MIFARE_Read(8, buffer, &size);
      if (status == MFRC522::STATUS_OK) {
        content = "";
        for (byte i = 0; i < 16; i++) {
          if (buffer[i] >= 32 && buffer[i] <= 126) {
            content += (char)buffer[i];
          }
        }
        content.trim();
      }
    }
  }
  
  return content;
}

void displayTagContent() {
  display.clearDisplay();
  
  // Display header
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("RFID TAG SCANNED");
  display.drawLine(0, 10, 128, 10, WHITE);
  
  // Display UID
  display.setCursor(0, 15);
  display.println("UID:");
  display.setCursor(30, 15);
  display.println(uidString);
  
  // Display tag content
  display.setCursor(0, 30);
  display.println("Content:");
  
  // Display the actual tag data
  display.setTextSize(2);
  display.setCursor(0, 40);
  
  if (tagContent.length() > 0) {
    // If content fits on one line
    if (tagContent.length() <= 10) {
      display.println(tagContent);
    } else {
      // Split into two lines if too long
      String line1 = tagContent.substring(0, 10);
      String line2 = tagContent.substring(10);
      display.println(line1);
      display.setCursor(0, 55);
      display.println(line2.substring(0, 10));  // Limit to 10 chars per line
    }
  } else {
    display.println("(No data)");
  }
  
  display.display();
  
  // Keep display on for 5 seconds
  delay(5000);
  
  // Clear and show ready message
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Ready to scan...");
  display.display();
}

void printDec(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], DEC);
  }
}