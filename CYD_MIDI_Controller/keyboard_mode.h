#ifndef KEYBOARD_MODE_H
#define KEYBOARD_MODE_H

#include "common_definitions.h"
#include "ui_elements.h"
#include "midi_utils.h"

// Keyboard mode variables
#define NUM_KEYS 8
int keyboardOctave = 4;
int keyboardScale = 0;
int lastKey = -1;

// Function declarations
void initializeKeyboardMode();
void drawKeyboardMode();
void handleKeyboardMode();
void drawKeyboardKey(int keyIndex, bool pressed);
int getKeyFromTouch();
void playKeyboardNote(int keyIndex, bool on);

// Implementations
void initializeKeyboardMode() {
  keyboardOctave = 4;
  keyboardScale = 0;
  lastKey = -1;
}

void drawKeyboardMode() {
  tft.fillScreen(THEME_BG);
  drawHeader("KEYS", scales[keyboardScale].name + " Oct " + String(keyboardOctave));
  
  int keyWidth = 320 / NUM_KEYS;
  int keyHeight = 100;
  int keyY = 65;
  
  for (int i = 0; i < NUM_KEYS; i++) {
    drawKeyboardKey(i, false);
  }
  
  drawRoundButton(10, 175, 40, 25, "OCT-", THEME_SECONDARY);
  drawRoundButton(60, 175, 40, 25, "OCT+", THEME_SECONDARY);
  drawRoundButton(110, 175, 60, 25, "SCALE", THEME_ACCENT);
  
  tft.setTextColor(THEME_TEXT_DIM, THEME_BG);
  tft.drawString("Oct " + String(keyboardOctave), 180, 180, 1);
}

void drawKeyboardKey(int keyIndex, bool pressed) {
  int keyWidth = 320 / NUM_KEYS;
  int keyHeight = 100;
  int keyY = 65;
  int x = keyIndex * keyWidth;
  
  uint16_t bgColor = pressed ? THEME_PRIMARY : THEME_SURFACE;
  uint16_t borderColor = THEME_PRIMARY;
  uint16_t textColor = pressed ? THEME_BG : THEME_TEXT;
  
  tft.fillRect(x + 2, keyY + 2, keyWidth - 4, keyHeight - 4, bgColor);
  tft.drawRect(x, keyY, keyWidth, keyHeight, borderColor);
  
  int note = getNoteInScale(keyboardScale, keyIndex, keyboardOctave);
  String noteName = getNoteNameFromMIDI(note);
  
  tft.setTextColor(textColor, bgColor);
  tft.drawCentreString(noteName, x + keyWidth/2, keyY + keyHeight/2 - 8, 2);
}

void handleKeyboardMode() {
  if (touch.justPressed && isButtonPressed(10, 10, 50, 25)) {
    exitToMenu();
    return;
  }
  
  if (touch.justPressed) {
    if (isButtonPressed(10, 175, 40, 25)) {
      keyboardOctave = max(1, keyboardOctave - 1);
      drawKeyboardMode();
      return;
    }
    if (isButtonPressed(60, 175, 40, 25)) {
      keyboardOctave = min(8, keyboardOctave + 1);
      drawKeyboardMode();
      return;
    }
    if (isButtonPressed(110, 175, 60, 25)) {
      keyboardScale = (keyboardScale + 1) % NUM_SCALES;
      drawKeyboardMode();
      return;
    }
  }
  
  // Key sliding support
  int key = getKeyFromTouch();
  
  if (touch.isPressed && key != -1) {
    if (key != lastKey) {
      if (lastKey != -1) {
        playKeyboardNote(lastKey, false);
        drawKeyboardKey(lastKey, false);
      }
      playKeyboardNote(key, true);
      drawKeyboardKey(key, true);
      lastKey = key;
    }
  } else if (touch.justReleased && lastKey != -1) {
    playKeyboardNote(lastKey, false);
    drawKeyboardKey(lastKey, false);
    lastKey = -1;
  }
}

int getKeyFromTouch() {
  if (touch.y < 65 || touch.y > 165) return -1;
  int keyWidth = 320 / NUM_KEYS;
  int key = touch.x / keyWidth;
  return (key >= 0 && key < NUM_KEYS) ? key : -1;
}

void playKeyboardNote(int keyIndex, bool on) {
  if (!deviceConnected) return;
  
  int note = getNoteInScale(keyboardScale, keyIndex, keyboardOctave);
  sendMIDI(on ? 0x90 : 0x80, note, on ? 100 : 0);
  
  Serial.printf("Key %d: %s %s\n", keyIndex, getNoteNameFromMIDI(note).c_str(), on ? "ON" : "OFF");
}

#endif