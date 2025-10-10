// Pin config DRV8833
const int AIN1 = 10;
const int AIN2 = 11;
const int BIN1 = 5;
const int BIN2 = 6;

void setup() {
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);
}

void loop() {
  //  Vooruit: traploos versnellen 
  for (int s = 0; s <= 255; s += 5) {
    analogWrite(AIN1, s); analogWrite(AIN2, 0); // Motor A vooruit
    analogWrite(BIN1, s); analogWrite(BIN2, 0); // Motor B vooruit
    delay(50);
  }

  //  Rechts bocht vooruit 
  for (int i = 0; i < 50; i++) {
    analogWrite(AIN1, 200); analogWrite(AIN2, 0); // linker motor trager
    analogWrite(BIN1, 255); analogWrite(BIN2, 0); // rechter motor sneller
    delay(50);
  }

  //  Links bocht vooruit 
  for (int i = 0; i < 50; i++) {
    analogWrite(AIN1, 255); analogWrite(AIN2, 0); // linker motor sneller
    analogWrite(BIN1, 200); analogWrite(BIN2, 0); // rechter motor trager
    delay(50);
  }

  //  Vooruit  vertragen 
  for (int s = 255; s >= 0; s -= 5) {
    analogWrite(AIN1, s); analogWrite(AIN2, 0);
    analogWrite(BIN1, s); analogWrite(BIN2, 0);
    delay(50);
  }

  //  Achteruit versnellen 
  for (int s = 0; s <= 255; s += 5) {
    analogWrite(AIN1, 0); analogWrite(AIN2, s); // Motor A achteruit
    analogWrite(BIN1, 0); analogWrite(BIN2, s); // Motor B achteruit
    delay(50);
  }

  //  Rechts bocht achteruit 
  for (int i = 0; i < 50; i++) {
    analogWrite(AIN1, 0); analogWrite(AIN2, 200); // linker motor trager
    analogWrite(BIN1, 0); analogWrite(BIN2, 255); // rechter motor sneller
    delay(50);
  }

  // Links bocht achteruit 
  for (int i = 0; i < 50; i++) {
    analogWrite(AIN1, 0); analogWrite(AIN2, 255); // linker motor sneller
    analogWrite(BIN1, 0); analogWrite(BIN2, 200); // rechter motor trager
    delay(50);
  }

  //  Achteruit vertragen 
  for (int s = 255; s >= 0; s -= 5) {
    analogWrite(AIN1, 0); analogWrite(AIN2, s);
    analogWrite(BIN1, 0); analogWrite(BIN2, s);
    delay(50);
  }

  //  Stoppen 
  analogWrite(AIN1, 0); analogWrite(AIN2, 0);
  analogWrite(BIN1, 0); analogWrite(BIN2, 0);
  delay(2000); // korte pauze voordat loop opnieuw start
}
