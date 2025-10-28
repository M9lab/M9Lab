/*
    Description: 
    Use ATOM LITE + SPK play mp3 files from TF Card
    Before running put the music file to the TF card    
    Please install library before compiling:  
    ESP8266Audio: https://github.com/earlephilhower/ESP8266Audio
    Audio generato da: https://www.readspeaker.com/
*/
// Versione script: 1.1.0 - BT SOLO CONFIGURAZIONE
// Board: Atom Lite + SPK
// Novità: BT per configurazione (vol, time, random) - Audio da USB/Button
// LED BLU quando BT attivo, Audio/Riverloop fermati

#include <M5Atom.h>
#include <WiFi.h>
#include <SD.h>
#include <SPI.h>
#include <TimeLib.h>
#include <BluetoothSerial.h>
#include "AudioFileSourceSD.h"
#include "AudioFileSourceID3.h"
#include "AudioGeneratorMP3.h"
#include "AudioOutputI2S.h"

// === SCRIPT VERSION ===
const char scriptVersion[] = "1.1.0-CONFIG";

// === CONFIG ===
#define AUDIO_FILE "/riverloop.mp3"
#define SCK 23
#define MISO 33
#define MOSI 19
#define I2S_BCLK 22
#define I2S_LRCL 21
#define I2S_DOUT 25

// === AUDIO GLOBALS ===
AudioFileSourceSD *file = nullptr;
AudioFileSourceID3 *id3 = nullptr;
AudioOutputI2S *out = nullptr;
AudioGeneratorMP3 *mp3 = nullptr;

float volume = 0.02;
bool playingPlaylist = false;
unsigned long lastCommandTime = 0;

// === BLUETOOTH ===
BluetoothSerial SerialBT;
bool btEnabled = false;

// === AUDIO STATE ===
bool audioStarting = false;
unsigned long lastAudioAttempt = 0;
int audioRestartAttempts = 0;

// OTTIMIZZAZIONE: buffer fisso invece di String
char inputBuffer[28]; // ridotto a 28 byte
int sm_totalFile = 0;
int sm_playList[30];
bool sm_ready = false;

// === RANDOM PLAY FLAG ===
bool randomPlayFlag = true;
unsigned long lastRandomEvent = 0;
unsigned long randomInterval = 60000; // ogni 60s evento casuale

// === DEBOUNCE ===
unsigned long lastButtonPress = 0;
unsigned long buttonPressStart = 0;

// === DOUBLE CLICK ===
unsigned long lastClickTime = 0;
bool waitingForDoubleClick = false;
#define DOUBLE_CLICK_TIMEOUT 700  // aumentato per maggiore precisione
#define MAX_CLICK_DURATION 350    // massima durata click per doppio click

// === CALLBACKS ===
void StatusCallback(void *cbData, int code, const char *string) {
  (void)cbData; (void)code; (void)string;
}

// === UTILS ===
void playFile(const char* filename) {
  if (mp3 && mp3->isRunning()) mp3->stop();
  
  // OTTIMIZZAZIONE: delete solo se esistenti
  if (file) { delete file; file = nullptr; }
  if (id3) { delete id3; id3 = nullptr; }
  
  // Verifica file esiste
  if (!SD.exists(filename)) {
    Serial.print(F("ERR: File non trovato: "));
    Serial.println(filename);
    return;
  }
  
  file = new AudioFileSourceSD(filename);
  if (!file) {
    Serial.println(F("ERR: AudioFileSourceSD failed"));
    return;
  }
  
  // In BT mode, usa file direttamente senza ID3 wrapper
  if (btEnabled) {
    // Modalità BT: no ID3
    id3 = nullptr;
  } else {
    // Modalità normale: usa ID3
    id3 = new AudioFileSourceID3(file);
    if (!id3) {
      Serial.println(F("ERR: AudioFileSourceID3 failed"));
      delete file;
      file = nullptr;
      return;
    }
  }
  
  if (!mp3) {
    Serial.println(F("ERR: mp3 is null!"));
    return;
  }
  
  // Usa file direttamente se BT mode, altrimenti usa id3
  bool started;
  if (btEnabled) {
    started = mp3->begin(file, out);
  } else {
    started = mp3->begin(id3, out);
  }
  
  if (!started) {
    Serial.println(F("ERR: mp3->begin() failed"));
  }
}

void setVolume(int percent) {
  if (percent < 0) percent = 0;
  if (percent > 100) percent = 100;
  volume = percent / 100.0;
  out->SetGain(volume);
  Serial.print(F("Vol: "));
  Serial.print(percent);
  Serial.println('%');
}

void volumeUp() { setVolume((int)((volume * 100) + 10)); }
void volumeDown() { setVolume((int)((volume * 100) - 10)); }

void printTime(Stream &s = Serial) {
  char buffer[9];
  sprintf(buffer, "%02d:%02d:%02d", hour(), minute(), second());
  s.println(buffer);
}

void clearSerialScreen() {
  Serial.write(27); // ESC
  Serial.print(F("[2J"));
  Serial.write(27);
  Serial.print(F("[H"));
}



void printHelp(bool useBT=false){
  Stream *s = useBT ? (Stream*)&SerialBT : (Stream*)&Serial;
  s->printf("\n📘 === COMANDI (v%s) ===\n", scriptVersion);
  s->print(F("🕐 Orario attuale: "));
  printTime(*s);
  
  if (useBT) {
    // Help per BT - solo comandi configurazione
    s->println(F("\n--- COMANDI BT (solo config) ---"));
    s->println(F("  help           → Questo elenco"));
    s->println(F("  vol+ / vol-    → Volume +/- 10%"));
    s->println(F("  vol=XX         → Imposta volume (0–100)"));
    s->println(F("  settime=H M S  → Imposta orario"));
    s->println(F("  gettime        → Mostra orario"));
    s->println(F("  randomplay=X   → Random on/off (0/1)"));
    s->println(F("  setinterval=X  → Intervallo random (sec)"));
    s->println(F("  ram            → Statistiche RAM"));
    s->println(F("  togglebt       → Spegne BT e riattiva audio"));
    s->println(F("\n  NOTA: Comandi audio NON disponibili via BT"));
    s->println(F("  (alert, playtrain, playaudio)"));
  } else {
    // Help completo per Serial
    s->println(F("  help             → Mostra questo elenco"));
    s->println(F("  playtrain=XYZ    → Annuncio treno"));
    s->println(F("  playaudio=XXX    → Riproduce /0XXX.mp3"));    
    s->println(F("  alert1           → Non indicare i personaggi"));
    s->println(F("  alert2           → Vietato attraversare i binari")) ;
    s->println(F("  alert3           → Vietato aprire le porte"));
    s->println(F("  alert4           → Attenzione, Allontanarsi dalla linea gialla"));
    s->println(F("  alert5           → Mettere like pagina M9Lab"));
    s->println(F("  alert6           → Mettere like pagina M9Lab + link"));
    s->println(F("  alert7           → Allontanarsi dalla linea Gialla"));
    s->println(F("  alert8           → Treno in transito al binario (casuale 1–9)"));
    s->println(F("  alert9           → Si nascondono 5 personaggi"));
    s->println(F("  alert10          → Benvenuti alla maker faire"));
    s->println(F("  randomplay=X     → Random on/off (0/1)"));
    s->println(F("  setinterval=X    → Intervallo random (sec)"));
    s->println(F("  vol+/vol-/vol=XX → Controllo volume"));
    s->println(F("  settime/gettime  → Gestione orario"));
    s->println(F("  ram              → Statistiche RAM"));
    s->println(F("  togglebt         → Toggle Bluetooth"));
  }
  
  s->printf("  Random: %s (%.1fs)\n", randomPlayFlag ? "ON" : "OFF", randomInterval/1000.0);
  s->printf("  BT: %s", btEnabled ? "ATTIVO" : "OFF");
  if (btEnabled) {
    s->println(F(" [LED BLU]"));
    s->println(F("  Audio/Riverloop fermati"));
  } else {
    s->println();
  }
  s->printf("  RAM: %d bytes\n", ESP.getFreeHeap());
  s->println(F("  [Long press 3s = toggle BT]"));
  s->println(F("---\n"));
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

void executeAudioPlayList(const char* command) {
  int sm_train = command[0] - '0';
  int sm_track = command[1] - '0';
  int sm_action = command[2] - '0';

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
  // Previeni chiamate multiple ravvicinate
  if (audioStarting || (millis() - lastAudioAttempt < 2000)) {
    return;
  }
  
  audioStarting = true;
  lastAudioAttempt = millis();
  audioRestartAttempts++;
  
  // Limita tentativi consecutivi
  if (audioRestartAttempts > 5) {
    Serial.println(F("ERR: Troppi tentativi riverloop"));
    Serial.println(F("Prova: spegni BT (long press) per liberare RAM"));
    audioStarting = false;
    delay(10000); // Pausa 10 secondi prima di riprovare
    audioRestartAttempts = 0;
    return;
  }
  
  Serial.print(F("Riverloop ("));
  Serial.print(audioRestartAttempts);
  Serial.print(F(") RAM:"));
  Serial.println(ESP.getFreeHeap());
  
  // Verifica che mp3 esista
  if (!mp3) {
    Serial.println(F("ERR: MP3 null - reinit necessario"));
    audioStarting = false;
    return;
  }
  
  // Verifica SD ancora accessibile
  if (!SD.exists(AUDIO_FILE)) {
    Serial.println(F("ERR: SD o file riverloop non trovato"));
    audioStarting = false;
    return;
  }
  
  // Ferma se in esecuzione
  if (mp3->isRunning()) {
    mp3->stop();
    delay(50);
  }
  
  playFile(AUDIO_FILE);
  
  // Verifica avvio con più dettagli
  delay(150);
  if (mp3->isRunning()) {    
    audioRestartAttempts = 0; // Reset contatore su successo
  } else {
    Serial.println(F("WARN: Audio non avviato"));
    Serial.print(F("mp3 ptr: "));
    Serial.println((int)mp3, HEX);
    Serial.print(F("out ptr: "));
    Serial.println((int)out, HEX);
  }
  
  audioStarting = false;
}

void playSingleFile(int num) {
  // NON riprodurre audio se BT è attivo (usato solo per config)
  if (btEnabled) {
    Serial.println(F("Audio disabilitato in BT mode"));
    Serial.println(F("Usa BT solo per configurazione"));
    return;
  }
  
  char filename[12];
  if (num < 10)
    sprintf(filename, "/000%d.mp3", num);
  else
    sprintf(filename, "/%04d.mp3", num);

  Serial.print(F("> "));
  Serial.println(filename);
  
  if (!mp3) {
    Serial.println(F("ERR: mp3 null, init fallito"));
    return;
  }
  
  playFile(filename);
  
  // Attendi completamento con timeout
  unsigned long startPlay = millis();
  while (mp3 && mp3->isRunning() && (millis() - startPlay < 120000)) {
    if (!mp3->loop()) break;
    delay(1);
    yield();
  }
  
  // Reset contatore
  audioRestartAttempts = 0;
  audioStarting = false;
  lastAudioAttempt = 0;
  
  // Riavvia riverloop
  startRiverLoop();
}

void playPlaylist() {
  // NON riprodurre audio se BT è attivo (usato solo per config)
  if (btEnabled) {
    Serial.println(F("Audio disabilitato in BT mode"));
    Serial.println(F("Usa BT solo per configurazione"));
    return;
  }
  
  if (!mp3) {
    Serial.println(F("ERR: mp3 null, playlist annullata"));
    return;
  }
  
  playingPlaylist = true;
  char filename[12];
  
  for (int i = 0; i < sm_totalFile; i++) {
    if (sm_playList[i] < 10)
      sprintf(filename, "/000%d.mp3", sm_playList[i]);
    else
      sprintf(filename, "/%04d.mp3", sm_playList[i]);
    
    Serial.print(F("> "));
    Serial.println(filename);
    playFile(filename);
    
    unsigned long startPlay = millis();
    while(mp3 && mp3->isRunning() && (millis() - startPlay < 120000)) { 
      if(!mp3->loop()) break; 
      delay(5);
      yield();
    }
  }
  
  playingPlaylist = false;
  
  // Reset contatore dopo playlist completata
  audioRestartAttempts = 0;
  audioStarting = false;
  lastAudioAttempt = 0;
  
  // Riavvia riverloop
  startRiverLoop();
}

// === BLUETOOTH HELPER ===
void reinitAudio() {
  Serial.println(F("Re-init audio..."));
  
  // Pulisci oggetti audio esistenti completamente
  if (mp3) {
    if (mp3->isRunning()) mp3->stop();
    delay(100);
    delete mp3;
    mp3 = nullptr;
  }
  if (id3) {
    delete id3;
    id3 = nullptr;
  }
  if (file) {
    delete file;
    file = nullptr;
  }
  if (out) {
    out->stop();
    delay(100);
    delete out;
    out = nullptr;
  }
  
  delay(200); // Pausa per lasciare tempo al sistema
  
  Serial.print(F("RAM prima init: "));
  Serial.print(ESP.getFreeHeap());
  Serial.print(F(" (largest: "));
  Serial.print(ESP.getMaxAllocHeap());
  Serial.println(F(")"));
  
  // Ricrea oggetti audio con controlli
  out = new AudioOutputI2S();
  if (!out) {
    Serial.println(F("ERR: AudioOutputI2S failed!"));
    return;
  }
  out->SetPinout(I2S_BCLK, I2S_LRCL, I2S_DOUT);
  
  // Buffer MINIMALI con BT attivo per massimizzare RAM disponibile
  int bufferSize = btEnabled ? 128 : 512;
  int bufferCount = btEnabled ? 2 : 2;
  out->SetBuffers(bufferCount, bufferSize);
  out->SetGain(volume);
  Serial.print(F("I2S OK (buf:"));
  Serial.print(bufferSize);
  Serial.println(F(")"));
  
  delay(100);
  
  if (!SD.exists(AUDIO_FILE)) {
    Serial.println(F("ERR: riverloop.mp3 non trovato!"));
    return;
  }
  
  file = new AudioFileSourceSD(AUDIO_FILE);
  if (!file) {
    Serial.println(F("ERR: AudioFileSourceSD failed!"));
    return;
  }
  
  // OTTIMIZZAZIONE BT: Salta ID3 wrapper per risparmiare RAM
  if (btEnabled) {
    // Modalità BT: usa direttamente file senza ID3
    Serial.println(F("BT mode: skip ID3 wrapper"));
    id3 = nullptr;
  } else {
    // Modalità normale: usa ID3
    id3 = new AudioFileSourceID3(file);
    if (!id3) {
      Serial.println(F("ERR: AudioFileSourceID3 failed!"));
      delete file;
      file = nullptr;
      return;
    }
  }
  
  mp3 = new AudioGeneratorMP3();
  if (!mp3) {
    Serial.println(F("ERR: AudioGeneratorMP3 failed!"));
    if (id3) delete id3;
    delete file;
    id3 = nullptr;
    file = nullptr;
    return;
  }
  
  Serial.print(F("RAM prima mp3->begin: "));
  Serial.print(ESP.getFreeHeap());
  Serial.print(F(" (largest: "));
  Serial.print(ESP.getMaxAllocHeap());
  Serial.println(F(")"));
  
  mp3->RegisterStatusCB(StatusCallback, (void*)"mp3");
  
  // Usa file direttamente se BT mode, altrimenti usa id3
  bool beginOK;
  if (btEnabled) {
    beginOK = mp3->begin(file, out);
  } else {
    beginOK = mp3->begin(id3, out);
  }
  
  if (!beginOK) {
    Serial.println(F("ERR: mp3->begin failed!"));
    Serial.print(F("RAM: "));
    Serial.print(ESP.getFreeHeap());
    Serial.print(F(" (largest: "));
    Serial.print(ESP.getMaxAllocHeap());
    Serial.println(F(")"));
    Serial.println(F("RAM insufficiente per decoder MP3!"));
  } else {
    Serial.println(F("MP3 begin OK"));
  }
  
  Serial.println(F("Audio re-init completato"));
  Serial.print(F("RAM dopo init: "));
  Serial.println(ESP.getFreeHeap());
}

void toggleBluetooth() {
  if (btEnabled) {
    // === SPEGNI BT ===
    Serial.println(F("Disattivo Bluetooth..."));
    SerialBT.flush();
    SerialBT.end();
    delay(200);
    btEnabled = false;
    
    // Spegni LED
    M5.dis.drawpix(0, 0x000000);
    
    Serial.println(F("BT SPENTO"));
    Serial.print(F("RAM recuperata: "));
    Serial.println(ESP.getFreeHeap());
    
    // Re-inizializza audio
    reinitAudio();
    audioRestartAttempts = 0;
    audioStarting = false;
    lastAudioAttempt = 0;
    
    // Riavvia riverloop
    delay(300);
    startRiverLoop();
    
    Serial.println(F("Audio/Riverloop riattivati"));
    
  } else {
    // === ATTIVA BT ===
    uint32_t freeHeap = ESP.getFreeHeap();
    Serial.print(F("RAM libera: "));
    Serial.print(freeHeap);
    Serial.println(F(" bytes"));
    
    if (freeHeap < 50000) {
      Serial.println(F("ERRORE: RAM insufficiente per BT!"));
      return;
    }
    
    // Ferma audio completamente
    Serial.println(F("Fermo audio..."));
    if (mp3 && mp3->isRunning()) {
      mp3->stop();
    }
    delay(200);
    
    Serial.println(F("Attivo Bluetooth..."));
    
    // Inizializza BT
    if (!SerialBT.begin("M9Lab-TrainStation")) {
      Serial.println(F("ERRORE: Impossibile avviare BT"));
      return;
    }
    
    btEnabled = true;
    
    // Accendi LED BLU fisso
    M5.dis.drawpix(0, 0x0000FF);
    
    Serial.println(F("BT ATTIVO: M9Lab-TrainStation"));
    Serial.println(F("Solo configurazione (no audio)"));
    Serial.print(F("RAM disponibile: "));
    Serial.println(ESP.getFreeHeap());
  }
}

// === COMANDO HELPER ===
bool startsWith(const char* str, const char* prefix) {
  return strncmp(str, prefix, strlen(prefix)) == 0;
}

bool equalsIgnoreCase(const char* str1, const char* str2) {
  return strcasecmp(str1, str2) == 0;
}

void processCommand(char* cmd, bool fromBT=false) {
  Stream *s = fromBT ? (Stream*)&SerialBT : (Stream*)&Serial;
    
  
  // Trim whitespace
  while (*cmd && isspace(*cmd)) cmd++;
  char* end = cmd + strlen(cmd) - 1;
  while (end > cmd && isspace(*end)) *end-- = '\0';
  
  
  if (equalsIgnoreCase(cmd, "help")) {
    Serial.println(F("Comando help riconosciuto"));
    printHelp(fromBT);
  }
  // Comandi AUDIO - BLOCCATI se BT attivo
  else if (startsWith(cmd, "playtrain=")) {
    if (fromBT) {
      s->println(F("Comando audio non disponibile via BT"));
      s->println(F("Usa BT solo per configurazione"));
    } else {
      executeAudioPlayList(cmd + 10);
      playPlaylist();
    }
  }
  else if (startsWith(cmd, "playaudio=")) {
    if (fromBT) {
      s->println(F("Comando audio non disponibile via BT"));
    } else {
      playSingleFile(atoi(cmd + 10));
    }
  }
  else if (equalsIgnoreCase(cmd, "alert1") || 
           equalsIgnoreCase(cmd, "alert2") ||
           equalsIgnoreCase(cmd, "alert3") ||
           equalsIgnoreCase(cmd, "alert4") ||
           equalsIgnoreCase(cmd, "alert5") ||
           equalsIgnoreCase(cmd, "alert6") ||
           equalsIgnoreCase(cmd, "alert7") ||
           equalsIgnoreCase(cmd, "alert8") ||
           equalsIgnoreCase(cmd, "alert9") ||
           equalsIgnoreCase(cmd, "alert10")) {
    if (fromBT) {
      s->println(F("Comando audio non disponibile via BT"));
      s->println(F("Usa comandi: vol+/vol-/vol=XX, settime, gettime, ram"));
    } else {
      // Esegui comando alert da Serial
      if (equalsIgnoreCase(cmd, "alert1")) playSingleFile(191);
      else if (equalsIgnoreCase(cmd, "alert2")) playSingleFile(171);
      else if (equalsIgnoreCase(cmd, "alert3")) playSingleFile(181);
      else if (equalsIgnoreCase(cmd, "alert4")) playSingleFile(151);
      else if (equalsIgnoreCase(cmd, "alert5")) playSingleFile(176);
      else if (equalsIgnoreCase(cmd, "alert6")) playSingleFile(177);
      else if (equalsIgnoreCase(cmd, "alert7")) playSingleFile(165);
      else if (equalsIgnoreCase(cmd, "alert8")) {
        int binario = random(1,10);
        Serial.print(F("ALERT8 - binario "));
        Serial.println(binario);
        // Usa sistema playlist esistente
        sm_totalFile = 0;
        addToPlayList(161);
        addToPlayList(binario);
        addToPlayList(165);
        playPlaylist();
      }
      else if (equalsIgnoreCase(cmd, "alert9")) playSingleFile(186);
      else if (equalsIgnoreCase(cmd, "alert10")) playSingleFile(196);
    }
  }
  else if (startsWith(cmd, "randomplay=")) {
    int val = atoi(cmd + 11);
    randomPlayFlag = (val != 0);
    s->print(F("Random play "));
    s->println(randomPlayFlag ? F("ON") : F("OFF"));
    lastRandomEvent = millis();
  }
  else if (startsWith(cmd, "setinterval=")) {
    int sec = atoi(cmd + 12);
    if (sec > 0) {
      randomInterval = (unsigned long)sec * 1000;
      s->print(F("Intervallo: "));
      s->print(sec);
      s->println(F(" secondi"));
    } else {
      s->println(F("Valore non valido"));
    }
  }
  else if (equalsIgnoreCase(cmd, "vol+")) volumeUp();
  else if (equalsIgnoreCase(cmd, "vol-")) volumeDown();
  else if (startsWith(cmd, "vol=")) setVolume(atoi(cmd + 4));
  else if (equalsIgnoreCase(cmd, "settime")) {
    setTime(12,0,0,1,1,1970);
    s->print(F("Ora impostata: "));
    printTime(*s);
  }
  else if (startsWith(cmd, "settime=")) {
    int h, m, sec;
    if (sscanf(cmd + 8, "%d %d %d", &h, &m, &sec) == 3) {
      setTime(h,m,sec,1,1,1970);
      s->print(F("Orario aggiornato: "));
      printTime(*s);
    }
  }
  else if (equalsIgnoreCase(cmd, "gettime")) {
    s->print(F("Ora corrente: "));
    printTime(*s);
  }
  else if (equalsIgnoreCase(cmd, "ram")) {
    s->print(F("RAM libera: "));
    s->print(ESP.getFreeHeap());
    s->print(F(" bytes (largest: "));
    s->print(ESP.getMaxAllocHeap());
    s->println(F(")"));
    s->print(F("Heap size: "));
    s->println(ESP.getHeapSize());
  }
  else if (equalsIgnoreCase(cmd, "togglebt")) {
    s->println(F("🔵 Toggle Bluetooth..."));
    toggleBluetooth();
    if (btEnabled) {
      s->println(F("✅ BT ATTIVO - Audio fermato"));
    } else {
      s->println(F("✅ BT SPENTO - Audio riattivato"));
      // Se chiamato da BT, questo sarà l'ultimo messaggio prima che BT si spenga
    }
  }
  else {
    Serial.print(F("DEBUG: Comando non riconosciuto: ["));
    Serial.print(cmd);
    Serial.println(F("]"));
    s->println(F("Comando sconosciuto. Usa 'help'"));
  }
}

// === SETUP ===
void setup() {
  // OTTIMIZZAZIONE CRITICA: Disabilita WiFi per liberare RAM
  WiFi.mode(WIFI_OFF);
  // BT disabilitato all'avvio, si attiva con bottone lungo
  
  M5.begin(true, false, true);
  SPI.begin(SCK, MISO, MOSI, -1);
  Serial.begin(115200);
  
  delay(100);
  Serial.println(F("\n=== ATOM LITE + SPK + BT ==="));
  Serial.print(F("RAM iniziale: "));
  Serial.println(ESP.getFreeHeap());

  if (!SD.begin(-1, SPI, 40000000)) {
    Serial.println(F("ERR: SD!"));
    while (1) {
      delay(1000);
      yield();
    }
  }
  Serial.println(F("SD OK"));

  // OTTIMIZZAZIONE MASSIMA: buffer ridottissimi per ATOM LITE + BT
  out = new AudioOutputI2S();
  out->SetPinout(I2S_BCLK, I2S_LRCL, I2S_DOUT);
  out->SetBuffers(2, 256); // RIDOTTO a 256 per lasciare RAM per BT
  out->SetGain(volume);
  Serial.println(F("I2S OK"));

  file = new AudioFileSourceSD(AUDIO_FILE);
  id3 = new AudioFileSourceID3(file);
  mp3 = new AudioGeneratorMP3();
  mp3->RegisterStatusCB(StatusCallback, (void*)"mp3");
  mp3->begin(id3, out);
  Serial.println(F("MP3 OK"));

  setTime(12, 0, 0, 1, 1, 1970);
  randomSeed(analogRead(0));
  
  Serial.print(F("RAM disponibile: "));
  Serial.println(ESP.getFreeHeap());
  Serial.println(F("Premi 3sec per BT"));
  
  printHelp();
  startRiverLoop();
}

// === LOOP ===
void loop() {
  M5.update();

  // GESTIONE BOTTONE: pressione breve = ALERT1, doppio click = ALERT9, pressione lunga (3s) = Toggle BT
  if (M5.Btn.wasPressed()) {
    buttonPressStart = millis();
  }
  
  if (M5.Btn.wasReleased()) {
    unsigned long pressDuration = millis() - buttonPressStart;
    unsigned long now = millis();
    
    if (pressDuration >= 3000) {
      // Pressione lunga: Toggle Bluetooth
      Serial.println(F("Long press detected"));
      toggleBluetooth();
      // LED gestito in toggleBluetooth()
      waitingForDoubleClick = false; // Reset doppio click
      lastClickTime = 0;
      
    } else if (pressDuration >= 50 && pressDuration <= MAX_CLICK_DURATION && !btEnabled) {
      // Pressione breve: verifica doppio click
      // Solo click rapidi (< MAX_CLICK_DURATION) contano per doppio click
      
      if (waitingForDoubleClick && (now - lastClickTime) < DOUBLE_CLICK_TIMEOUT) {
        // DOPPIO CLICK rilevato -> ALERT9
        Serial.println(F("✓✓ Doppio click -> ALERT9"));
        playSingleFile(186);
        waitingForDoubleClick = false;
        lastClickTime = 0;
        lastButtonPress = now;
      } else {
        // Primo click: aspetta per vedere se arriva il secondo
        Serial.println(F("✓ Click 1/2..."));
        waitingForDoubleClick = true;
        lastClickTime = now;
        lastButtonPress = now;
      }
      
    } else if (pressDuration > MAX_CLICK_DURATION && pressDuration < 3000 && !btEnabled) {
      // Click troppo lungo per doppio click ma non long press
      // Trattalo come click singolo immediato se non stiamo aspettando un secondo click
      if (!waitingForDoubleClick) {
        Serial.println(F("Click singolo (lungo) -> ALERT1"));
        playSingleFile(191);
        lastButtonPress = millis();
      }
      waitingForDoubleClick = false;
      lastClickTime = 0;
      
    } else if (btEnabled) {
      Serial.println(F("Audio disabilitato (BT attivo)"));
      waitingForDoubleClick = false;
      lastClickTime = 0;
    }
  }
  
  // Gestione timeout click singolo (controllo NON in playlist per essere reattivo)
  if (waitingForDoubleClick && !playingPlaylist && (millis() - lastClickTime) > DOUBLE_CLICK_TIMEOUT) {
    // Timeout scaduto, era un click singolo -> ALERT1
    Serial.println(F("✓ Click singolo (timeout) -> ALERT1"));
    playSingleFile(191);
    waitingForDoubleClick = false;
    lastClickTime = 0;
  }

  // LETTURA SERIALE (USB)
  if (Serial.available()) {
    int idx = 0;
    unsigned long timeout = millis() + 100;
    
    while (Serial.available() && idx < 27 && millis() < timeout) {
      char c = Serial.read();
      if (c == '\n' || c == '\r') break;
      inputBuffer[idx++] = c;
      delay(1);
    }
    inputBuffer[idx] = '\0';
    while (Serial.available()) Serial.read();
    
    if (idx > 0) {

      
      lastCommandTime = millis();
      processCommand(inputBuffer, false);
    }
  }

  // LETTURA BLUETOOTH (se attivo)
  if (btEnabled && SerialBT.available()) {
    int idx = 0;
    unsigned long timeout = millis() + 100;
    
    while (SerialBT.available() && idx < 27 && millis() < timeout) {
      char c = SerialBT.read();
      if (c == '\n' || c == '\r') break;
      inputBuffer[idx++] = c;
      delay(1);
    }
    inputBuffer[idx] = '\0';
    while (SerialBT.available()) SerialBT.read();
    
    if (idx > 0) {
      lastCommandTime = millis();
      Serial.print(F("📱 BT> "));
      Serial.println(inputBuffer);
      processCommand(inputBuffer, true);
    }
  }

  // === MODALITÀ RANDOM === (solo se BT spento)
  if (randomPlayFlag && !btEnabled && !playingPlaylist && millis() - lastRandomEvent > randomInterval) {
    lastRandomEvent = millis();
    int tipo = random(2);
    
    if (tipo == 0) {
      // Alert random
      int alertNum = random(1, 11);
      Serial.print(F("RND-AL"));
      Serial.println(alertNum);
      
      if(alertNum == 8) {
        int binario = random(1,10);
        Serial.print(F("bin"));
        Serial.println(binario);
        // Usa sistema playlist esistente
        sm_totalFile = 0;
        addToPlayList(161);
        addToPlayList(binario);
        addToPlayList(165);
        playPlaylist();
      } else {
        int fileNum = alertNum == 1 ? 191 :
                      alertNum == 2 ? 171 :
                      alertNum == 3 ? 181 :
                      alertNum == 4 ? 151 :
                      alertNum == 5 ? 176 :
                      alertNum == 6 ? 177 :
                      alertNum == 7 ? 165 :
                      alertNum == 9 ? 186 : 196;
        playSingleFile(fileNum);
      }
    } else {
      // Annuncio treno random
      int train = random(1, 8);
      int binario = random(1, 10);
      int azione = random(1, 3);
      
      char cmd[4];
      sprintf(cmd, "%d%d%d", train, binario, azione);
      
      Serial.print(F("RND-TRN:"));
      Serial.println(cmd);
      
      executeAudioPlayList(cmd);
      playPlaylist();
    }
  }

  // Audio loop management - SEMPLIFICATO con anti-loop
  if (!playingPlaylist && !audioStarting) {
    if (mp3 && mp3->isRunning()) {
      if (!mp3->loop()) mp3->stop();
    } else if (!btEnabled && mp3 && !mp3->isRunning() && (millis() - lastAudioAttempt > 2000)) {
      // Riavvia SOLO se BT è SPENTO e sono passati almeno 2 secondi
      startRiverLoop();
    }
  }
  
  // CRITICO: yield per evitare watchdog reset
  yield();
}
