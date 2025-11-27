/*
  Sketch: Simulazione luce saldatore RGB
  Hardware: Modulo LED RGB collegato a ESP32
  Pin:
    - Rosso: pin 25
    - Verde: pin 26
    - Blu: pin 27
  Funzionamento:
    - Bagliore di fondo leggero e casuale
    - Raffiche di scintille con rosso dominante e verde/blu casuali
    - Durata delle scintille e pause variabili per effetto naturale
    - Pausa più lunga tra le raffiche per simulare intervallo tra “scariche”
*/

const int pinR = 25; // Rosso
const int pinG = 26; // Verde
const int pinB = 27; // Blu

void setup() {
  pinMode(pinR, OUTPUT);
  pinMode(pinG, OUTPUT);
  pinMode(pinB, OUTPUT);
  randomSeed(analogRead(0)); // Migliora la casualità
}

void loop() {
  // Bagliore di fondo
  digitalWrite(pinR, random(0,2));
  digitalWrite(pinG, random(0,2));
  digitalWrite(pinB, random(0,2));
  delay(random(50, 150));

  // Raffica di scintille
  int scintille = random(2, 6); // numero di lampeggi veloci
  for (int i = 0; i < scintille; i++) {
    digitalWrite(pinR, HIGH);                    // rosso dominante
    digitalWrite(pinG, random(0,2));             // verde casuale
    digitalWrite(pinB, random(0,2));             // blu casuale
    delay(random(30, 70));

    // Spegnimento parziale per effetto pulsante
    digitalWrite(pinR, LOW);
    digitalWrite(pinG, random(0,2));
    digitalWrite(pinB, random(0,2));
    delay(random(20, 50));
  }

  // Pausa lunga tra raffiche (intervallo tra scariche)
  digitalWrite(pinR, LOW);
  digitalWrite(pinG, LOW);
  digitalWrite(pinB, LOW);
  delay(random(2000, 4000)); // 2-4 secondi
}
