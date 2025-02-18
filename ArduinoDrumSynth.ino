// ======= Button Pins ======= //
const int kickButton   = 2;
const int snareButton  = 3;
const int pad1Button   = 4;
const int pad2Button   = 5;
const int pad3Button   = 6;
const int presetButton = 7;

// ======= Piezo Speaker Pin ======= //
const int speakerPin = 11;

// ======= Button States ======= //
bool kickPressed   = false;
bool snarePressed  = false;
bool pad1Pressed   = false;
bool pad2Pressed   = false;
bool pad3Pressed   = false;
bool presetPressed = false;

unsigned long lastSoundTime = 0;
int preset = 0; // Current drum preset (0-9)

// ======= Sound Buffer ======= //
int soundQueue[50];
int soundIndex = 0;

void setup() {
  pinMode(kickButton, INPUT_PULLUP);
  pinMode(snareButton, INPUT_PULLUP);
  pinMode(pad1Button, INPUT_PULLUP);
  pinMode(pad2Button, INPUT_PULLUP);
  pinMode(pad3Button, INPUT_PULLUP);
  pinMode(presetButton, INPUT_PULLUP);
}

void loop() {
  checkButtonState(kickButton, kickPressed, playKick);
  checkButtonState(snareButton, snarePressed, playSnare);
  checkButtonState(pad1Button, pad1Pressed, playPad1);
  checkButtonState(pad2Button, pad2Pressed, playPad2);
  checkButtonState(pad3Button, pad3Pressed, playPad3);
  checkButtonState(presetButton, presetPressed, changePreset);

  if (millis() - lastSoundTime > 10) {
    updateSounds();
    lastSoundTime = millis();
  }
}

void checkButtonState(int buttonPin, bool &buttonState, void (*action)()) {
  if (digitalRead(buttonPin) == LOW) {
    if (!buttonState) {
      delay(20);
      if (digitalRead(buttonPin) == LOW) {
        buttonState = true;
        action();
      }
    }
  } else {
    buttonState = false;
  }
}

void changePreset() {
  preset = (preset + 1) % 10;
}

void queueSound(int frequency) {
  if (soundIndex < 50) {
    soundQueue[soundIndex++] = frequency;
  }
}

void clearSounds() {
  soundIndex = 0;
  noTone(speakerPin);
}

void updateSounds() {
  if (soundIndex > 0) {
    tone(speakerPin, soundQueue[0]);
    for (int i = 1; i < soundIndex; i++) {
      soundQueue[i - 1] = soundQueue[i];
    }
    soundIndex--;
  } else {
    noTone(speakerPin);
  }
}

// ======= SOUND GENERATORS ======= //
void playKick() {
  clearSounds();
  switch (preset) {
    case 0: queuePitchDrop(150, 40, 5); break;
    case 1: queuePitchDrop(200, 60, 8); break;
    case 2: queuePitchDrop(120, 30, 5); break;
    case 3: queuePitchDrop(100, 40, 6); break;
    case 4: queuePitchDrop(80, 20, 4); break;
    case 5: queueSound(2000); queuePitchDrop(180, 50, 15); break;
    case 6: queuePitchDrop(100, 40, 2); break;
    case 7: queueSound(2500); queuePitchDrop(200, 60, 20); break;
    case 8: queueSound(3000); queuePitchDrop(150, 30, 10); break;
    case 9: queuePitchDrop(80, 30, 5); break;
  }
}

void playSnare() {
  clearSounds();
  switch (preset) {
    case 0: queuePitchDrop(250, 120, 10); break;
    case 1: queuePitchDrop(300, 150, 12); break;
    case 2: queuePitchDrop(200, 100, 8); break;
    case 3: queuePitchDrop(280, 130, 10); break;
    case 4: queuePitchDrop(350, 180, 15); break;
    case 5: queuePitchDrop(500, 180, 30); break;
    case 6: queuePitchDrop(800, 200, 50); break;
    case 7: queueSound(1200); queuePitchDrop(400, 150, 40); break;
    case 8: queuePitchDrop(600, 200, 30); queueSound(1500); break;
    case 9: for (int i = 0; i < 5; i++) queueSound(2000); break;
  }
}

void playPad1() {
  clearSounds();
  switch (preset) {
    case 0: queueArpeggio(440, 554, 659); break;
    case 1: for (int i = 3000; i >= 200; i -= 200) queueSound(i); break;
    case 2: queueArpeggio(220, 330, 440); break;
    case 3: queueArpeggio(400, 500, 620); break;
    case 4: queueArpeggio(300, 554, 784); break;
    case 5: queueSound(523); queueSound(659); queueSound(784); break;
    case 6: queueArpeggio(523, 622, 784); break;
    case 7: queueArpeggio4(523, 659, 784, 932); break;
    case 8: queueArpeggio(523, 698, 784); break;
    case 9: for (int i = 0; i < 10; i++) queueSound(200 + (i % 3) * 300); break;
  }
}

void playPad2() {
  clearSounds();
  switch (preset) {
    case 0: queueArpeggio(660, 880, 1320); break;
    case 1: for (int i = 500; i <= 800; i += 20) queueSound(i); for (int i = 800; i >= 500; i -= 20) queueSound(i); break;
    case 2: for (int i = 400; i >= 200; i -= 10) queueSound(i); break;
    case 3: queueArpeggio(1000, 500, 300); break;
    case 4: queueArpeggio(880, 440, 220); break;
    case 5: queueSound(1318); queueSound(1567); queueSound(3136); break;
    case 6: for (int i = 0; i < 10; i++) queueSound((random(100,800)/50)*50); break;
    case 7: queuePitchDrop(1500, 50, 100); break;
    case 8: queueSound(200); queueSound(5000); queueSound(100); queueSound(3000); break;
    case 9: queueArpeggio(261, 523, 1046); break;
  }
}

void playPad3() {
  clearSounds();
  switch (preset) {
    case 0: for (int i = 0; i < 6; i++) queueSound(random(100, 2000)); break;// Glitch
    case 1: for (int i = 500; i < 2000; i += 150) queueSound(i); break;// Jump
    case 2: queuePitchDrop(1500, 500, 50); break;
    case 3: queuePitchDrop(2000, 200, 100); break;
    case 4: for (int i = 300; i < 1500; i += 100) queueSound(i); break;
    case 5: for (int i = 0; i < 8; i++) queueSound(random(100, 2000)); break;
    case 6: queuePitchDrop(880, 220, 50); break;
    case 7: for (int i = 0; i < 3; i++) { queueSound(440); queueSound(880); } break;
    case 8: queuePitchDrop(800, 200, 50); queuePitchDrop(1200, 400, 50); break;
    case 9: queueArpeggio(784, 1046, 1318); break;
  }
}

// ======= HELPER FUNCTIONS ======= //
void queuePitchDrop(int startFreq, int endFreq, int step) {
  for (int freq = startFreq; freq >= endFreq; freq -= step) {
    queueSound(freq);
  }
}

void queueArpeggio(int f1, int f2, int f3) {
  for (int i = 0; i < 3; i++) {
    queueSound(f1);
    queueSound(f2);
    queueSound(f3);
  }
}

void queueArpeggio4(int f1, int f2, int f3, int f4) {
  for (int i = 0; i < 3; i++) {
    queueSound(f1);
    queueSound(f2);
    queueSound(f3);
    queueSound(f4);
  }
}

void addSnareRattle() {
  for (int i = 0; i < 3; i++) queueSound(2000);
}

void queueNoiseBurst() {
  for (int i = 0; i < 8; i++) queueSound(random(1500, 3000));
}