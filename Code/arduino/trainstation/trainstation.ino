/*
    Description: 
    Use ATOM SPK play mp3 files from TF Card
    Before running put the music file to the TF card    
    Please install library before compiling:  
    ESP8266Audio: https://github.com/earlephilhower/ESP8266Audio
    Audio generato da: https://www.readspeaker.com/
*/
// Versione script: 1.0.8
// Board: Atom5
// Novità: comando setinterval=X per impostare intervallo random in secondi (default 60s)

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
String scriptVersion = "1.0.8";

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

float volume = 0.02;
bool playingPlaylist = false;
unsigned long lastCommandTime = 0;

String input;
int sm_totalFile = 0;
int sm_playList[30];
bool sm_ready = false;

// === RANDOM PLAY FLAG ===
bool randomPlayFlag = true;
unsigned long lastRandomEvent = 0;
unsigned long randomInterval = 60000; // ogni 60s evento casuale

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

void clearSerialScreen() {
  Serial.write(27); // ESC
  Serial.print("[2J"); // Clear screen
  Serial.write(27);
  Serial.print("[H"); // Move cursor to home
}


void printHelp() {
  clearSerialScreen();
  Serial.printf("\n📘 === LISTA COMANDI DISPONIBILI (v%s) ===\n", scriptVersion.c_str());
  Serial.printf("🕐 Ora corrente: %02d:%02d:%02d\n", hour(), minute(), second());
  Serial.println("  help           → Mostra questo elenco");
  Serial.println("  playtrain=XYZ  → Annuncio treno (X=1–7 tipo treno, Y=1–9 binario, Z=1 partenza / 2 arrivo)");
  Serial.println("  playaudio=XXX  → Riproduce direttamente il file /0XXX.mp3 (test singolo)");
  Serial.println("  alert1         → Non indicare i personaggi");
  Serial.println("  alert2         → Vietato attraversare i binari");
  Serial.println("  alert3         → Vietato aprire le porte");
  Serial.println("  alert4         → Attenzione, Allontanarsi dalla linea gialla");
  Serial.println("  alert5         → Mettere like pagina M9Lab");
  Serial.println("  alert6         → Mettere like pagina M9Lab + link");
  Serial.println("  alert7         → Allontanarsi dalla linea Gialla");
  Serial.println("  alert8         → Treno in transito al binario (casuale 1–9)");
  Serial.println("  alert9         → Si nascondono 5 personaggi");
  Serial.println("  alert10        → Benvenuti alla maker faire");
  Serial.println("  randomplay=X   → Attiva/disattiva modalità casuale (0=off,1=on)");
  Serial.println("  setinterval=X  → Imposta intervallo random in secondi");
  Serial.println("  vol+ / vol-    → Aumenta o diminuisce il volume di 10%");
  Serial.println("  vol=XX         → Imposta volume (0–100)");
  Serial.println("  settime / settime=H M S / gettime → Gestione orario");
  Serial.printf("  Stato randomplay: %s (intervallo = %.1f s)\n", randomPlayFlag ? "ON" : "OFF", randomInterval / 1000.0);
  Serial.println("-----------------------------------\n");
}


// === PLAYLIST MANAGEMENT ===
void addToPlayList(int mp3num) {
  if (sm_totalFile < 30) sm_playList[sm_totalFile++] = mp3num;
}

int convertIntTo2DigitString(int i) {
  if (i == 0) return 110;
  if (i < 10) return i + 100;
  return i;
}

void executeAudioPlayList(String command) {
  int sm_train = command.charAt(0) - '0';
  int sm_track = command.charAt(1) - '0';
  int sm_action = command.charAt(2) - '0';

  sm_totalFile = 0;
  memset(sm_playList, 0, sizeof(sm_playList));

  addToPlayList(111);
  addToPlayList(121);

  switch (sm_train) {
    case 1: addToPlayList(201); addToPlayList(65); addToPlayList(100); addToPlayList(100); addToPlayList(1); break;
    case 2: addToPlayList(202); addToPlayList(45); addToPlayList(61); break;
    case 3: addToPlayList(203); addToPlayList(45); addToPlayList(59); break;
    case 4: addToPlayList(204); addToPlayList(35); addToPlayList(100); addToPlayList(100); addToPlayList(4); break;
    case 5: addToPlayList(205); addToPlayList(23); addToPlayList(55); break;
    case 6: addToPlayList(206); addToPlayList(60); addToPlayList(44); break;
    case 7: addToPlayList(207); addToPlayList(55); addToPlayList(21); break;
  }

  addToPlayList(140);
  addToPlayList(131);
  addToPlayList(hour());
  addToPlayList(135);
  addToPlayList(convertIntTo2DigitString(minute()));

  if (sm_action == 1) addToPlayList(141);
  if (sm_action == 2) addToPlayList(146);

  addToPlayList(sm_track);
  if (sm_action == 1) addToPlayList(165);
  addToPlayList(191);

  sm_ready = true;
}

// === AUDIO LOOP CONTROL ===
void startRiverLoop() {
  Serial.println("🎵 Avvio riverloop...");
  playFile(AUDIO_FILE);
}

void playSingleFile(int num) {
  char filename[12];
  if (num < 10)
    sprintf(filename, "/000%d.mp3", num);
  else
    sprintf(filename, "/%04d.mp3", num);

  Serial.printf("▶️  %s\n", filename);
  playFile(filename);
  while (mp3->isRunning()) {
    if (!mp3->loop()) break;
    delay(1);
  }
  startRiverLoop();
}

void playPlaylist() {
  playingPlaylist = true;
  for (int i = 0; i < sm_totalFile; i++) {
    char filename[12];
    if (sm_playList[i] < 10)
      sprintf(filename, "/000%d.mp3", sm_playList[i]);
    else
      sprintf(filename, "/%04d.mp3", sm_playList[i]);
    Serial.printf("▶️  %s\n", filename);
    playFile(filename);
    while(mp3->isRunning()) { if(!mp3->loop()) break; delay(5); }
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
  out->SetBuffers(2, 2048);
  out->SetGain(volume);

  file = new AudioFileSourceSD(AUDIO_FILE);
  id3 = new AudioFileSourceID3(file);
  mp3 = new AudioGeneratorMP3();
  mp3->RegisterStatusCB(StatusCallback, (void*)"mp3");
  mp3->begin(id3, out);

  setTime(12, 0, 0, 1, 1, 1970);
  printHelp();
  startRiverLoop();
  randomSeed(analogRead(0));
}

// === LOOP ===
void loop() {
  M5.update();

  if (M5.Btn.isPressed() && millis() - lastButtonPress > 1000) {
    lastButtonPress = millis();
    Serial.println("🔘 Bottone premuto → ALERT1");
    playSingleFile(191);
  }

  if (Serial.available()) {
    input = Serial.readStringUntil('\n');
    input.trim();
    lastCommandTime = millis();

    if (input.equalsIgnoreCase("help")) printHelp();
    else if (input.startsWith("playtrain=")) { executeAudioPlayList(input.substring(5)); playPlaylist(); }
    else if (input.startsWith("playaudio=")) playSingleFile(input.substring(10).toInt());
    else if (input.equalsIgnoreCase("alert1")) playSingleFile(191);
    else if (input.equalsIgnoreCase("alert2")) playSingleFile(171);
    else if (input.equalsIgnoreCase("alert3")) playSingleFile(181);
    else if (input.equalsIgnoreCase("alert4")) playSingleFile(151);
    else if (input.equalsIgnoreCase("alert5")) playSingleFile(176);
    else if (input.equalsIgnoreCase("alert6")) playSingleFile(177);
    else if (input.equalsIgnoreCase("alert7")) playSingleFile(165);
    else if (input.equalsIgnoreCase("alert8")) {
      int binario = random(1,10);
      Serial.printf("🚉 ALERT8 - treno in transito al binario %d\n", binario);
      playSingleFile(161);
      playSingleFile(binario);
      playSingleFile(165); // aggiunto come richiesto
    }
    else if (input.equalsIgnoreCase("alert9")) playSingleFile(186);
    else if (input.equalsIgnoreCase("alert10")) playSingleFile(196);
    else if (input.startsWith("randomplay=")) {
      int val = input.substring(11).toInt();
      randomPlayFlag = (val != 0);
      Serial.printf("🎲 Random play %s\n", randomPlayFlag ? "ON" : "OFF");
      lastRandomEvent = millis();
    }
    else if (input.startsWith("setinterval=")) {
      int sec = input.substring(12).toInt();
      if (sec > 0) {
        randomInterval = (unsigned long)sec * 1000;
        Serial.printf("⏱️ Intervallo random impostato a %d secondi\n", sec);
      } else {
        Serial.println("⚠️ Valore non valido. Usa: setinterval=XX (in secondi)");
      }
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

  // === MODALITÀ RANDOM ===
  if (randomPlayFlag && millis() - lastRandomEvent > randomInterval) {
    lastRandomEvent = millis();
    int tipo = random(2);
    if (tipo == 0) {
      int alertNum = random(1, 11);
      Serial.printf("🎲 Random ALERT%d\n", alertNum);
      if(alertNum == 8) {
        int binario = random(1,10);
        Serial.printf("▶️ RANDOM ALERT8 - treno in transito al binario %d\n", binario);
        playSingleFile(161);
        playSingleFile(binario);
        playSingleFile(165);       // suono aggiuntivo
      } else {
        int fileNum = alertNum == 1 ? 191 :
                      alertNum == 2 ? 171 :
                      alertNum == 3 ? 181 :
                      alertNum == 4 ? 151 :
                      alertNum == 5 ? 176 :
                      alertNum == 6 ? 177 :
                      alertNum == 7 ? 196 :
                      alertNum == 9 ? 186 : 196;
        playSingleFile(fileNum);
      }
    } else {
      int train = random(1, 8);
      int binario = random(1, 10);
      int azione = random(1, 3);
      String cmd = String(train) + String(binario) + String(azione);
      Serial.printf("🚆 Random annuncio: play=%s\n", cmd.c_str());
      executeAudioPlayList(cmd);
      playPlaylist();
    }
  }

  if (!playingPlaylist && mp3 && mp3->isRunning()) { if (!mp3->loop()) mp3->stop(); }
  else if (!playingPlaylist && !mp3->isRunning()) startRiverLoop();
}
