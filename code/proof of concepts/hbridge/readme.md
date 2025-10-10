# H-Bridge proof of concept

minimale hard- & software + stappenplan dat aantoont dat 2 motoren onafhankelijk van elkaar kunnen draaien, en (traploos) regelbaar zijn in snelheid en draairichting.

// Motor links
const int IN1_L = 5;
const int IN2_L = 6;
const int PWM_L = 9;

// Motor rechts
const int IN1_R = 7;
const int IN2_R = 8;
const int PWM_R = 10;

void setup() {
  pinMode(IN1_L, OUTPUT);
  pinMode(IN2_L, OUTPUT);
  pinMode(PWM_L, OUTPUT);

  pinMode(IN1_R, OUTPUT);
  pinMode(IN2_R, OUTPUT);
  pinMode(PWM_R, OUTPUT);
}

// Functie: stuur 1 motor aan
// snelheid: -255 = volle snelheid achteruit, 0 = uit, +255 = volle snelheid vooruit
void setMotor(int in1, int in2, int pwmPin, int snelheid) {
  if (snelheid > 0) {
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
    analogWrite(pwmPin, snelheid);
  } else if (snelheid < 0) {
    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
    analogWrite(pwmPin, -snelheid);
  } else {
    digitalWrite(in1, LOW);
    digitalWrite(in2, LOW);
    analogWrite(pwmPin, 0);
  }
}

void loop() {
  // --- Individueel testen ---
  // Motor links vooruit traploos
  for (int s = 0; s <= 255; s += 5) {
    setMotor(IN1_L, IN2_L, PWM_L, s);
    delay(40);
  }
  for (int s = 255; s >= 0; s -= 5) {
    setMotor(IN1_L, IN2_L, PWM_L, s);
    delay(40);
  }

  delay(500);

  // Motor rechts achteruit traploos
  for (int s = 0; s >= -255; s -= 5) {
    setMotor(IN1_R, IN2_R, PWM_R, s);
    delay(40);
  }
  for (int s = -255; s <= 0; s += 5) {
    setMotor(IN1_R, IN2_R, PWM_R, s);
    delay(40);
  }

  delay(500);

  // --- Beide motoren tegelijk, zelfde richting ---
  // Traploos vooruit
  for (int s = 0; s <= 200; s += 5) {
    setMotor(IN1_L, IN2_L, PWM_L, s);
    setMotor(IN1_R, IN2_R, PWM_R, s);
    delay(40);
  }
  for (int s = 200; s >= 0; s -= 5) {
    setMotor(IN1_L, IN2_L, PWM_L, s);
    setMotor(IN1_R, IN2_R, PWM_R, s);
    delay(40);
  }

  delay(500);

  // Traploos achteruit
  for (int s = 0; s >= -200; s -= 5) {
    setMotor(IN1_L, IN2_L, PWM_L, s);
    setMotor(IN1_R, IN2_R, PWM_R, s);
    delay(40);
  }
  for (int s = -200; s <= 0; s += 5) {
    setMotor(IN1_L, IN2_L, PWM_L, s);
    setMotor(IN1_R, IN2_R, PWM_R, s);
    delay(40);
  }

  delay(1000);
}
