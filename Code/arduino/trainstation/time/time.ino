// Versione aggiornata: tasto AtomLite → alert1, playlist, randomplay, volume, solo ora, alert4
// Versione script: 1.0.1

#include <M5Atom.h>
#include <WiFi.h>
#include <SD.h>
#include <SPI.h>
#include <TimeLib.h>
#include "AudioFileSourceSD.h"
#include "AudioFileSourceID3.h"
#include "AudioGeneratorMP3.h"
#include "AudioOutputI2S.h"

// === SCRIPT VERSION ===
String scriptVersion = "1.0.1";

// === CONFIG ===
#define AUDIO_FILE "/riverloop.mp3"
#define SCK 23
#define MISO 33
#define MOSI 19
#define I2S_BCLK 22
#define I2S_LRCL 21
#define I2S_DOUT 25

// === AUDIO GLOBALS ===
AudioFileSourceSD *file;
AudioFileSourceID3 *id3;
AudioOutputI2S *out;
AudioGeneratorMP3 *mp3;

float volume = 0.05;
bool playingPlaylist = false;
unsigned long lastCommandTime = 0;

String input;
int sm_totalFile = 0;
int sm_playList[30];
bool sm_ready = false;

// === RANDOM PLAY FLAG ===
bool randomPlayFlag = false;

// === DEBOUNCE ===
unsigned long lastButtonPress = 0;

// === CALLBACKS ===
void StatusCallback(void *cbData, int code, const char *string) {
  (void)cbData; (void)code; (void)string;
}

// === UTILS ===
void playFile(String filename) {
  if (mp3 && mp3->isRunning()) mp3->stop();
  delete file; delete id3;
  file = new AudioFileSourceSD(filename.c_str());
  id3 = new AudioFileSourceID3(file);
  mp3->begin(id3, out);
}

void setVolume(int percent) {
  if (percent < 0) percent = 0;
  if (percent > 100) percent = 100;
  volume = percent / 100.0;
  out->SetGain(volume);
  Serial.printf("🔊 Volume: %d%%\n", percent);
}

void volumeUp() { setVolume((int)((volume * 100) + 10)); }
void volumeDown() { setVolume((int)((volume * 100) - 10)); }

void printTime() {
  char buffer[9];
  sprintf(buffer, "%02d:%02d:%02d", hour(), minute(), second());
  Serial.println(buffer);
}

void printHelp() {
  Serial.printf("\n📘 === LISTA COMANDI DISPONIBILI (v%s) ===\n", scriptVersion.c_str());
  Serial.printf("🕐 Ora corrente: %02d:%02d:%02d\n", hour(), minute(), second());
  Serial.println("  help           → Mostra questo elenco");
  Serial.println("  play=XXX       → Esegue la playlist corrispondente (es. play=111)");
  Serial.println("  alert1         → Riproduce / non indicare i personaggi (anche tasto AtomLite)"); 
  Serial.println("  alert2         → Riproduce / vietato attraversare i binari"); 
  Serial.println("  alert3         → Riproduce / vietato aprire le porte"); 
  Serial.println("  alert4         → Riproduce / allontanarsi linea gialla");
  Serial.println("  randomplay=X   → Attiva/disattiva riproduzione random (0=off,1=on)");
  Serial.println("  vol+ / vol-    → Aumenta o diminuisce il volume di 10%");
  Serial.println("  vol=XX         → Imposta volume (0-100)");
  Serial.println("  settime        → Imposta ora default 12:00:00");
  Serial.println("  settime=H M S  → Imposta orario manualmente");
  Serial.println("  gettime        → Mostra ora corrente (solo ora)");
  Serial.printf("  Stato randomplay: %s\n", randomPlayFlag ? "ON" : "OFF");
  Serial.println("-----------------------------------\n");
}

// === PLAYLIST MANAGEMENT ===
void addToPlayList(int mp3num) {
  if (sm_totalFile < 30) sm_playList[sm_totalFile++] = mp3num;
}

int convertIntTo2DigitString(int i) {
  if (i == 0) return 110;   // 00 → 0110
  if (i < 10) return i + 100; // 01..09 → 101..109
  return i;                  // 10..59 rimangono invariati
}

void executeAudioPlayList(String command) {
  int sm_train = command.charAt(0) - '0';
  int sm_track = command.charAt(1) - '0';
  int sm_action = command.charAt(2) - '0';

  sm_totalFile = 0;
  memset(sm_playList, 0, sizeof(sm_playList));

  addToPlayList(111);  // bip iniziale
  addToPlayList(121);  // annuncio partenza

  switch (sm_train) {
    case 1:
      addToPlayList(201);
      addToPlayList(65);
      addToPlayList(100);
      addToPlayList(100);
      addToPlayList(1);
      break;
    case 2:
      addToPlayList(202);
      addToPlayList(45);
      addToPlayList(61);
      break;
    case 3:
      addToPlayList(203);
      addToPlayList(45);
      addToPlayList(59);
      break;
  }

  addToPlayList(140);
  addToPlayList(131);
  addToPlayList(hour());
  addToPlayList(135);
  addToPlayList(convertIntTo2DigitString(minute()));

  if (sm_action == 1) addToPlayList(141);
  if (sm_action == 2) addToPlayList(146);

  addToPlayList(sm_track);
  addToPlayList(165);
  addToPlayList(191); // esempio file finale nella playlist

  sm_ready = true;
}

// === AUDIO LOOP CONTROL ===
void startRiverLoop() {
  Serial.println("🎵 Avvio riverloop...");
  playFile(AUDIO_FILE);
}

void playSingleFile(int num) {
  char filename[12];
  sprintf(filename, "/%04d.mp3", num);
  Serial.printf("▶️  %s\n", filename);
  playFile(filename);
  while (mp3->isRunning()) {
    if (!mp3->loop()) break;
    delay(5);
  }
  startRiverLoop();
}

void playPlaylist() {
  playingPlaylist = true;

  if(randomPlayFlag) {
    for(int i=0; i<sm_totalFile; i++) {
      int idx = random(sm_totalFile);
      char filename[12];
      sprintf(filename, "/%04d.mp3", sm_playList[idx]);
      Serial.printf("▶️ RANDOM %s\n", filename);
      playFile(filename);
      while(mp3->isRunning()) { if(!mp3->loop()) break; delay(5); }
    }
  } else {
    for (int i = 0; i < sm_totalFile; i++) {
      char filename[12];
      sprintf(filename, "/%04d.mp3", sm_playList[i]);
      Serial.printf("▶️  %s\n", filename);
      playFile(filename);
      while(mp3->isRunning()) { if(!mp3->loop()) break; delay(5); }
    }
  }

  playingPlaylist = false;
  startRiverLoop();
}

// === SETUP ===
void setup() {
  M5.begin(true, false, true);
  SPI.begin(SCK, MISO, MOSI, -1);

  Serial.begin(115200);
  while (!Serial);

  if (!SD.begin(-1, SPI, 40000000)) {
    Serial.println("❌ SD card non trovata!");
    while (1);
  }

  out = new AudioOutputI2S();
  out->SetPinout(I2S_BCLK, I2S_LRCL, I2S_DOUT);
  out->SetGain(volume);

  file = new AudioFileSourceSD(AUDIO_FILE);
  id3 = new AudioFileSourceID3(file);
  mp3 = new AudioGeneratorMP3();
  mp3->RegisterStatusCB(StatusCallback, (void*)"mp3");
  mp3->begin(id3, out);

  setTime(12, 0, 0, 1, 1, 1970); // Solo ora iniziale

  printHelp(); // Mostra help all’avvio

  startRiverLoop();

  randomSeed(analogRead(0));
}

// === LOOP ===
void loop() {
  M5.update();

  // --- TASTO PRINCIPALE ATOMLITE ---
  if (M5.Btn.isPressed()) {
    if (millis() - lastButtonPress > 1000) {
      lastButtonPress = millis();
      Serial.println("🔘 Bottone premuto → ALERT1");
      playSingleFile(191);
    }
  }

  // --- COMANDI SERIALI ---
  if (Serial.available()) {
    input = Serial.readStringUntil('\n');
    input.trim();
    lastCommandTime = millis();

    if (input.equalsIgnoreCase("help")) printHelp();
    else if (input.startsWith("play=")) { executeAudioPlayList(input.substring(5)); playPlaylist(); }
    else if (input.equalsIgnoreCase("alert1")) playSingleFile(191);
    else if (input.equalsIgnoreCase("alert2")) playSingleFile(171);
    else if (input.equalsIgnoreCase("alert3")) playSingleFile(181);
    else if (input.equalsIgnoreCase("alert4")) playSingleFile(151);
    else if (input.startsWith("randomplay=")) {
      int val = input.substring(11).toInt();
      randomPlayFlag = (val != 0);
      Serial.printf("🎲 Random play %s\n", randomPlayFlag ? "ON" : "OFF");
    }
    else if (input.equalsIgnoreCase("vol+")) volumeUp();
    else if (input.equalsIgnoreCase("vol-")) volumeDown();
    else if (input.startsWith("vol=")) setVolume(input.substring(4).toInt());
    else if (input.equalsIgnoreCase("settime")) { setTime(12,0,0,1,1,1970); Serial.print("✅ Ora impostata: "); printTime(); }
    else if (input.startsWith("settime=")) {
      int h,m,s;
      if(sscanf(input.substring(8).c_str(),"%d %d %d",&h,&m,&s)==3){ setTime(h,m,s,1,1,1970); Serial.print("✅ Orario aggiornato: "); printTime(); }
    }
    else if (input.equalsIgnoreCase("gettime")) { Serial.print("🕐 Ora corrente: "); printTime(); }
    else { Serial.println("❓ Comando sconosciuto. Digita 'help' per l'elenco."); }
  }

  if (!playingPlaylist && mp3 && mp3->isRunning()) { if (!mp3->loop()) mp3->stop(); }
  else if (!playingPlaylist && !mp3->isRunning()) startRiverLoop();
}
