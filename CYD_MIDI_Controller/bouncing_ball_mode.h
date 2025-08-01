#ifndef BOUNCING_BALL_MODE_H
#define BOUNCING_BALL_MODE_H

#include "common_definitions.h"
#include "ui_elements.h"
#include "midi_utils.h"

// Bouncing Ball mode variables
struct Ball {
  float x, y;
  float vx, vy;
  uint16_t color;
  float size;
  float prevX, prevY; // Previous position for erasing
};

#define MAX_BALLS 5
Ball balls[MAX_BALLS];
int numBalls = 2;

struct EdgeNote {
  int x, y, w, h;
  int note;
  String noteName;
  uint16_t color;
  bool active;
  unsigned long activeTime;
  int side; // 0=top, 1=right, 2=bottom, 3=left
};

#define MAX_EDGE_NOTES 12
EdgeNote edgeNotes[MAX_EDGE_NOTES];
int numEdgeNotes = 8;
int ballOctave = 5;

// Function declarations
void initializeBouncingBallMode();
void drawBouncingBallMode();
void handleBouncingBallMode();
void initializeBalls();
void initializeEdgeNotes();
void updateBouncingBall();
void updateBalls();
void drawBalls();
void drawEdgeNotes();
void checkBallCollisions();

// Implementations
void initializeBouncingBallMode() {
  ballOctave = 5;
  initializeBalls();
  initializeEdgeNotes();
}

void drawBouncingBallMode() {
  tft.fillScreen(THEME_BG);
  drawHeader("ZEN BALLS", "Peaceful Music");
  
  drawRoundButton(260, 10, 50, 25, "ADD", THEME_SUCCESS);
  drawRoundButton(10, 200, 50, 25, "RESET", THEME_WARNING);
  drawRoundButton(70, 200, 40, 25, "OCT-", THEME_SECONDARY);
  drawRoundButton(120, 200, 40, 25, "OCT+", THEME_SECONDARY);
  
  tft.setTextColor(THEME_TEXT_DIM, THEME_BG);
  tft.drawCentreString("Oct " + String(ballOctave), 195, 207, 1);
  
  drawEdgeNotes();
  drawBalls();
}

void initializeBalls() {
  for (int i = 0; i < MAX_BALLS; i++) {
    balls[i].x = random(80, 240);
    balls[i].y = random(80, 150);
    balls[i].prevX = balls[i].x;
    balls[i].prevY = balls[i].y;
    // Slower, more zen-like movement
    balls[i].vx = random(-15, 15) / 10.0; // -1.5 to 1.5
    balls[i].vy = random(-15, 15) / 10.0;
    if (abs(balls[i].vx) < 0.5) balls[i].vx = (balls[i].vx >= 0) ? 0.8 : -0.8;
    if (abs(balls[i].vy) < 0.5) balls[i].vy = (balls[i].vy >= 0) ? 0.8 : -0.8;
    // Softer, more zen colors
    balls[i].color = random(0x2000, 0x8FFF);
    balls[i].size = random(4, 7);
  }
  numBalls = 2;
}

void initializeEdgeNotes() {
  // Create notes around the full perimeter in C major scale
  int scaleNotes[] = {0, 2, 4, 5, 7, 9, 11}; // C major intervals
  int noteSize = 20;
  
  numEdgeNotes = 12; // Fill the perimeter with more notes
  int noteIndex = 0;
  
  // Top edge - 4 notes
  for (int i = 0; i < 4; i++) {
    edgeNotes[noteIndex].x = 40 + i * 60;
    edgeNotes[noteIndex].y = 50;
    edgeNotes[noteIndex].w = noteSize;
    edgeNotes[noteIndex].h = noteSize;
    edgeNotes[noteIndex].note = 60 + scaleNotes[i % 7] + (ballOctave - 4) * 12;
    edgeNotes[noteIndex].noteName = getNoteNameFromMIDI(edgeNotes[noteIndex].note);
    edgeNotes[noteIndex].color = THEME_PRIMARY;
    edgeNotes[noteIndex].side = 0;
    edgeNotes[noteIndex].active = false;
    noteIndex++;
  }
  
  // Right edge - 3 notes
  for (int i = 0; i < 3; i++) {
    edgeNotes[noteIndex].x = 280;
    edgeNotes[noteIndex].y = 80 + i * 40;
    edgeNotes[noteIndex].w = noteSize;
    edgeNotes[noteIndex].h = noteSize;
    edgeNotes[noteIndex].note = 60 + scaleNotes[(4 + i) % 7] + (ballOctave - 4) * 12;
    edgeNotes[noteIndex].noteName = getNoteNameFromMIDI(edgeNotes[noteIndex].note);
    edgeNotes[noteIndex].color = THEME_SECONDARY;
    edgeNotes[noteIndex].side = 1;
    edgeNotes[noteIndex].active = false;
    noteIndex++;
  }
  
  // Bottom edge - 3 notes
  for (int i = 0; i < 3; i++) {
    edgeNotes[noteIndex].x = 220 - i * 60;
    edgeNotes[noteIndex].y = 175;
    edgeNotes[noteIndex].w = noteSize;
    edgeNotes[noteIndex].h = noteSize;
    edgeNotes[noteIndex].note = 60 + scaleNotes[(6 - i) % 7] + (ballOctave - 4) * 12;
    edgeNotes[noteIndex].noteName = getNoteNameFromMIDI(edgeNotes[noteIndex].note);
    edgeNotes[noteIndex].color = THEME_ACCENT;
    edgeNotes[noteIndex].side = 2;
    edgeNotes[noteIndex].active = false;
    noteIndex++;
  }
  
  // Left edge - 2 notes
  for (int i = 0; i < 2; i++) {
    edgeNotes[noteIndex].x = 20;
    edgeNotes[noteIndex].y = 140 - i * 40;
    edgeNotes[noteIndex].w = noteSize;
    edgeNotes[noteIndex].h = noteSize;
    edgeNotes[noteIndex].note = 60 + scaleNotes[i % 7] + (ballOctave - 4) * 12 + 12; // Octave up
    edgeNotes[noteIndex].noteName = getNoteNameFromMIDI(edgeNotes[noteIndex].note);
    edgeNotes[noteIndex].color = THEME_WARNING;
    edgeNotes[noteIndex].side = 3;
    edgeNotes[noteIndex].active = false;
    noteIndex++;
  }
}

void handleBouncingBallMode() {
  // Back button
  if (touch.justPressed && isButtonPressed(10, 10, 50, 25)) {
    exitToMenu();
    return;
  }
  
  if (touch.justPressed) {
    // Add ball button
    if (isButtonPressed(260, 10, 50, 25)) {
      if (numBalls < MAX_BALLS) {
        balls[numBalls].x = random(60, 260);
        balls[numBalls].y = random(70, 170);
        balls[numBalls].prevX = balls[numBalls].x;
        balls[numBalls].prevY = balls[numBalls].y;
        balls[numBalls].vx = random(-15, 15) / 10.0;
        balls[numBalls].vy = random(-15, 15) / 10.0;
        if (abs(balls[numBalls].vx) < 0.5) balls[numBalls].vx = (balls[numBalls].vx >= 0) ? 0.8 : -0.8;
        if (abs(balls[numBalls].vy) < 0.5) balls[numBalls].vy = (balls[numBalls].vy >= 0) ? 0.8 : -0.8;
        balls[numBalls].color = random(0x2000, 0x8FFF);
        balls[numBalls].size = random(4, 7);
        numBalls++;
      }
      return;
    }
    
    // Reset button
    if (isButtonPressed(10, 200, 50, 25)) {
      initializeBalls();
      drawBouncingBallMode();
      return;
    }
    
    // Octave controls
    if (isButtonPressed(70, 200, 40, 25)) {
      ballOctave = max(2, ballOctave - 1);
      initializeEdgeNotes();
      drawBouncingBallMode();
      return;
    }
    
    if (isButtonPressed(120, 200, 40, 25)) {
      ballOctave = min(7, ballOctave + 1);
      initializeEdgeNotes();
      drawBouncingBallMode();
      return;
    }
  }
  
  // Update physics and display
  updateBouncingBall();
}

void updateBouncingBall() {
  // Zen-like smooth animation - slower update (30 FPS)
  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate > 33) {
    // Store previous positions for erasing
    for (int i = 0; i < numBalls; i++) {
      balls[i].prevX = balls[i].x;
      balls[i].prevY = balls[i].y;
    }
    
    updateBalls();
    checkBallCollisions();
    
    // Erase previous ball positions
    for (int i = 0; i < numBalls; i++) {
      tft.fillCircle(balls[i].prevX, balls[i].prevY, balls[i].size + 1, THEME_BG);
    }
    
    // Redraw edge notes that might have been erased
    drawEdgeNotes();
    
    // Draw balls at new positions
    drawBalls();
    
    lastUpdate = millis();
  }
}

void updateBalls() {
  for (int i = 0; i < numBalls; i++) {
    // Update position
    balls[i].x += balls[i].vx;
    balls[i].y += balls[i].vy;
    
    // Bounce off walls (inside the note perimeter)
    if (balls[i].x <= 50 || balls[i].x >= 270) {
      balls[i].vx = -balls[i].vx;
      balls[i].x = constrain(balls[i].x, 50, 270);
    }
    if (balls[i].y <= 80 || balls[i].y >= 165) {
      balls[i].vy = -balls[i].vy;
      balls[i].y = constrain(balls[i].y, 80, 165);
    }
  }
}

void drawBalls() {
  for (int i = 0; i < numBalls; i++) {
    tft.fillCircle(balls[i].x, balls[i].y, balls[i].size, balls[i].color);
    tft.drawCircle(balls[i].x, balls[i].y, balls[i].size, THEME_TEXT);
  }
}

void drawEdgeNotes() {
  static bool lastActiveStates[MAX_EDGE_NOTES] = {false};
  
  for (int i = 0; i < numEdgeNotes; i++) {
    uint16_t color = edgeNotes[i].color;
    bool needsRedraw = false;
    
    // Check if active state changed
    if (edgeNotes[i].active != lastActiveStates[i]) {
      needsRedraw = true;
      lastActiveStates[i] = edgeNotes[i].active;
    }
    
    // Smooth glow effect when active
    if (edgeNotes[i].active) {
      unsigned long elapsed = millis() - edgeNotes[i].activeTime;
      if (elapsed < 200) {
        // Fade from bright to normal
        float intensity = 1.0 - (float)elapsed / 200.0;
        uint16_t brightColor = color;
        if (intensity > 0.5) {
          brightColor = THEME_TEXT;
        }
        color = brightColor;
        needsRedraw = true;
      } else {
        edgeNotes[i].active = false;
        needsRedraw = true;
      }
    }
    
    // Only redraw if state changed
    if (needsRedraw) {
      tft.fillRoundRect(edgeNotes[i].x, edgeNotes[i].y, 
                       edgeNotes[i].w, edgeNotes[i].h, 6, color);
      tft.drawRoundRect(edgeNotes[i].x, edgeNotes[i].y, 
                       edgeNotes[i].w, edgeNotes[i].h, 6, THEME_TEXT);
      
      // Note name
      tft.setTextColor(THEME_BG, color);
      tft.drawCentreString(edgeNotes[i].noteName, 
                          edgeNotes[i].x + edgeNotes[i].w/2, 
                          edgeNotes[i].y + edgeNotes[i].h/2 - 4, 1);
    }
  }
}

void checkBallCollisions() {
  for (int b = 0; b < numBalls; b++) {
    for (int n = 0; n < numEdgeNotes; n++) {
      // Check collision with note
      if (balls[b].x + balls[b].size >= edgeNotes[n].x && 
          balls[b].x - balls[b].size <= edgeNotes[n].x + edgeNotes[n].w &&
          balls[b].y + balls[b].size >= edgeNotes[n].y && 
          balls[b].y - balls[b].size <= edgeNotes[n].y + edgeNotes[n].h) {
        
        if (!edgeNotes[n].active) {
          // Play note
          if (deviceConnected) {
            sendMIDI(0x90, edgeNotes[n].note, 100);
            delay(5);
            sendMIDI(0x80, edgeNotes[n].note, 0);
          }
          
          edgeNotes[n].active = true;
          edgeNotes[n].activeTime = millis();
          
          // Bounce ball away from note
          float centerX = edgeNotes[n].x + edgeNotes[n].w/2;
          float centerY = edgeNotes[n].y + edgeNotes[n].h/2;
          float dx = balls[b].x - centerX;
          float dy = balls[b].y - centerY;
          
          if (abs(dx) > abs(dy)) {
            balls[b].vx = (dx > 0) ? abs(balls[b].vx) : -abs(balls[b].vx);
          } else {
            balls[b].vy = (dy > 0) ? abs(balls[b].vy) : -abs(balls[b].vy);
          }
          
          Serial.printf("Ball hit note %s\n", edgeNotes[n].noteName.c_str());
        }
      }
    }
  }
}

#endif