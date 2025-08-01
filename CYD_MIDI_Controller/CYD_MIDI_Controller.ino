/*******************************************************************
 MIDI Controller Main Launcher for ESP32 Cheap Yellow Display
 Main file - handles setup, menu, and mode switching
 *******************************************************************/

#include <SPI.h>
#include <XPT2046_Touchscreen.h>
#include <TFT_eSPI.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>

// Include mode files
#include "keyboard_mode.h"
#include "sequencer_mode.h"
#include "bouncing_ball_mode.h"
#include "random_generator_mode.h"
#include "xy_pad_mode.h"
#include "ui_elements.h"
#include "midi_utils.h"

// Hardware setup
#define XPT2046_IRQ 36
#define XPT2046_MOSI 32
#define XPT2046_MISO 39
#define XPT2046_CLK 25
#define XPT2046_CS 33

// Global objects
SPIClass mySpi = SPIClass(VSPI);
XPT2046_Touchscreen ts(XPT2046_CS, XPT2046_IRQ);
TFT_eSPI tft = TFT_eSPI();

// BLE MIDI globals
BLECharacteristic *pCharacteristic;
bool deviceConnected = false;
uint8_t midiPacket[] = {0x80, 0x80, 0x00, 0x60, 0x7F};

// Touch state
TouchState touch;

// App state
AppMode currentMode = MENU;

// Menu system
#define MENU_ITEMS 5
String menuItems[MENU_ITEMS] = {"KEYS", "BEATS", "ZEN", "RNG", "XY PAD"};
String menuIcons[MENU_ITEMS] = {"♪", "♫", "●", "※", "◈"};

class MIDICallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      updateStatus();
    }
    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      updateStatus();
      // Stop all notes
      for (int i = 0; i < 128; i++) {
        sendMIDI(0x80, i, 0);
      }
    }
};

void setup() {
  Serial.begin(115200);
  
  // Touch setup
  mySpi.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  ts.begin(mySpi);
  ts.setRotation(1);
  
  // Display setup
  tft.init();
  tft.setRotation(1);
  pinMode(21, OUTPUT);
  digitalWrite(21, HIGH);
  
  // BLE MIDI Setup
  Serial.println("Initializing BLE MIDI...");
  BLEDevice::init("CYD MIDI");
  Serial.println("BLE Device initialized");
  
  BLEServer *server = BLEDevice::createServer();
  server->setCallbacks(new MIDICallbacks());
  Serial.println("BLE Server created");
  
  BLEService *service = server->createService(BLEUUID(SERVICE_UUID));
  Serial.println("BLE Service created");
  
  pCharacteristic = service->createCharacteristic(
    BLEUUID(CHARACTERISTIC_UUID),
    BLECharacteristic::PROPERTY_READ | 
    BLECharacteristic::PROPERTY_WRITE | 
    BLECharacteristic::PROPERTY_NOTIFY
  );
  
  pCharacteristic->addDescriptor(new BLE2902());
  service->start();
  Serial.println("BLE Service started");
  
  BLEAdvertising *advertising = server->getAdvertising();
  advertising->addServiceUUID(service->getUUID());
  BLEAdvertisementData adData;
  adData.setName("CYD MIDI");
  adData.setCompleteServices(BLEUUID(SERVICE_UUID));
  advertising->setAdvertisementData(adData);
  advertising->setScanResponse(true);
  advertising->setMinPreferred(0x06);
  advertising->setMaxPreferred(0x12);
  advertising->start();
  Serial.println("BLE Advertising started - Device discoverable as 'CYD MIDI'");
  
  // Initialize mode systems
  initializeKeyboardMode();
  initializeSequencerMode();
  initializeBouncingBallMode();
  initializeRandomGeneratorMode();
  initializeXYPadMode();
  
  drawMenu();
  updateStatus();
  Serial.println("MIDI Controller ready!");
}

void loop() {
  updateTouch();
  
  switch (currentMode) {
    case MENU:
      if (touch.justPressed) handleMenuTouch();
      break;
    case KEYBOARD:
      handleKeyboardMode();
      break;
    case SEQUENCER:
      handleSequencerMode();
      break;
    case BOUNCING_BALL:
      handleBouncingBallMode();
      break;
    case RANDOM_GENERATOR:
      handleRandomGeneratorMode();
      break;
    case XY_PAD:
      handleXYPadMode();
      break;
  }
  
  delay(20);
}

void drawMenu() {
  tft.fillScreen(THEME_BG);
  
  // Header
  tft.fillRect(0, 0, 320, 50, THEME_SURFACE);
  tft.drawFastHLine(0, 50, 320, THEME_PRIMARY);
  tft.setTextColor(THEME_PRIMARY, THEME_SURFACE);
  tft.drawCentreString("MIDI CONTROLLER", 160, 12, 4);
  tft.setTextColor(THEME_TEXT_DIM, THEME_SURFACE);
  tft.drawCentreString("Touch Music Interface", 160, 32, 2);
  
  // App-style icon grid (3x2 for 5 items + space for more)
  int iconSize = 50;
  int spacing = 15;
  int cols = 3;
  int startX = (320 - (cols * iconSize + (cols-1) * spacing)) / 2;
  int startY = 65;
  
  // Connection status
  if (!deviceConnected) {
    tft.setTextColor(THEME_ERROR, THEME_BG);
    tft.drawCentreString("BLE WAITING...", 160, 210, 2);
  }
  tft.setTextColor(deviceConnected ? THEME_SUCCESS : THEME_ERROR, THEME_BG);
  tft.drawString(deviceConnected ? "●" : "○", 290, 55, 2);
  
  for (int i = 0; i < MENU_ITEMS; i++) {
    int col = i % cols;
    int row = i / cols;
    int x = startX + col * (iconSize + spacing);
    int y = startY + row * (iconSize + spacing + 15);
    
    // App icon background
    uint16_t iconColor = THEME_PRIMARY;
    if (i == 1) iconColor = THEME_SECONDARY;      // BEATS
    else if (i == 2) iconColor = THEME_ACCENT;    // BOUNCY
    else if (i == 3) iconColor = THEME_WARNING;   // AI JAM
    else if (i == 4) iconColor = THEME_SUCCESS;   // XY PAD
    
    tft.fillRoundRect(x, y, iconSize, iconSize, 12, iconColor);
    tft.drawRoundRect(x, y, iconSize, iconSize, 12, THEME_TEXT);
    
    // Icon symbol
    tft.setTextColor(THEME_BG, iconColor);
    tft.drawCentreString(menuIcons[i], x + iconSize/2, y + iconSize/2 - 12, 6);
    
    // App name
    tft.setTextColor(THEME_TEXT, THEME_BG);
    tft.drawCentreString(menuItems[i], x + iconSize/2, y + iconSize + 5, 1);
  }
}

void handleMenuTouch() {
  int iconSize = 50;
  int spacing = 15;
  int cols = 3;
  int startX = (320 - (cols * iconSize + (cols-1) * spacing)) / 2;
  int startY = 65;
  
  for (int i = 0; i < MENU_ITEMS; i++) {
    int col = i % cols;
    int row = i / cols;
    int x = startX + col * (iconSize + spacing);
    int y = startY + row * (iconSize + spacing + 15);
    
    if (isButtonPressed(x, y, iconSize, iconSize)) {
      enterMode((AppMode)(i + 1));
      return;
    }
  }
}

void enterMode(AppMode mode) {
  currentMode = mode;
  switch (mode) {
    case KEYBOARD:
      drawKeyboardMode();
      break;
    case SEQUENCER:
      drawSequencerMode();
      break;
    case BOUNCING_BALL:
      drawBouncingBallMode();
      break;
    case RANDOM_GENERATOR:
      drawRandomGeneratorMode();
      break;
    case XY_PAD:
      drawXYPadMode();
      break;
  }
  updateStatus();
}

void exitToMenu() {
  currentMode = MENU;
  stopAllModes();
  drawMenu();
  updateStatus();
}