# Sensoren proof of concept

minimale hard- en software die aantoont dat minimaal 6 sensoren onafhankelijk van elkaar kunnen uitgelezen worden (geen calibratie, normalisatie of interpolatie). Hierbij moet een zo groot mogelijk bereik van de AD converter benut worden (indien van toepassing)


code:

const int NUM_SENSORS = 8;

//aangesloten pinnen
int sensorPins[NUM_SENSORS] = {A0, A1, A2, A3, A4, A5, A6, A7};

// Var om waarden op te slaan
int sensorValues[NUM_SENSORS];

void setup() {
  Serial.begin(9600);
  delay(1000);
  Serial.println("IR-sensor test gestart...");
}

void loop() {
  bool allValid = true;

  for (int i = 0; i < NUM_SENSORS; i++) {
    sensorValues[i] = analogRead(sensorPins[i]);
    if (sensorValues[i] <= 0 || sensorValues[i] >= 1023) {
      allValid = false;
    }
  }

  // Waarden weergeven
  Serial.print("Sensorwaarden: ");
  for (int i = 0; i < NUM_SENSORS; i++) {
    Serial.print(sensorValues[i]);
    if (i < NUM_SENSORS - 1) Serial.print(", ");
  }

  if (allValid) {
    Serial.println("  ✅ (binnen bereik)");
  } else {
    Serial.println("  ⚠️  (0 of max gedetecteerd)");
  }

  delay(500);
}
