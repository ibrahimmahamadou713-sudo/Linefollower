#include <Preferences.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// UUID's voor de Nordic UART Service (gebruikt door Bluefruit Connect)
#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

#define Baudrate 115200

// --- PIN DEFINITIES ---
#define MotorLeftA D3
#define MotorLeftB D2
#define MotorRightA D5
#define MotorRightB D4

const int sensor[] = {A0, A1, A2, A3, A4, A5, A6, A7};
const int StatusLED = LED_BUILTIN;
const int Drukknop = D1;
const int RunStatusLED = D11;   // Groene LED (RUN)
const int StopStatusLED = D8;   // Rode LED (STOP)

// --- GLOBALE VARIABELEN ---
Preferences preferences;

BLEServer* pServer = NULL;
BLECharacteristic* pTxCharacteristic = NULL;
bool deviceConnected = false;
String bleInputBuffer = "";

bool run = false;
unsigned long previous, calculationTime;
float iTerm = 0;
float lastErr;

unsigned long ledPreviousMillis = 0;
const long ledInterval = 250;
bool ledState = LOW;

struct param_t
{
  unsigned long cycleTime;
  int black[8];
  int white[8];
  int power;
  float diff;
  float kp;
  float ki;
  float kd;
} params;

const char *PREFS_NAME = "robot_params";
const char *PARAM_KEY = "config_data";
const size_t PARAM_SIZE = sizeof(params);

int normalised[8];
float debugPosition;
float output;

// --- PROTOTYPES ---
void processCommand(String commandLine);
void onUnknownCommand(String command);
void onSet(String param, String value);
void onDebug();
void onCalibrate(String param);
void onRun();
void onStop();
void readParams();
void writeParams();
void bleSerialPrint(String data);
void bleSerialPrintln(String data);

// --- BLE CALLBACKS ---
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      Serial.println("BLE Client verbonden.");
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      Serial.println("BLE Client verbroken. Start opnieuw adverteren.");
      BLEDevice::startAdvertising();
    }
};

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      String receivedString = pCharacteristic->getValue();

      if (receivedString.length() > 0) {
        for (int i = 0; i < receivedString.length(); i++) {
          char c = receivedString[i];
          bleInputBuffer += c;

          if (c == '\n' || c == '\r') {

            bleInputBuffer.trim();

            if (bleInputBuffer.length() > 0) {
                processCommand(bleInputBuffer);
            }

            bleInputBuffer = "";
          }
        }
      }
    }
};

// --- COMMANDO LOGICA ---
void processCommand(String commandLine) {
    commandLine.toLowerCase();

    int firstSpace = commandLine.indexOf(' ');
    String cmd = commandLine;
    String params = "";

    if (firstSpace != -1) {
        cmd = commandLine.substring(0, firstSpace);
        params = commandLine.substring(firstSpace + 1);
    }

    if (cmd == "run") {
        onRun();
    } else if (cmd == "stop") {
        onStop();
    } else if (cmd == "debug") {
        onDebug();
    }
    else if (cmd == "set") {
        int secondSpace = params.indexOf(' ');
        if (secondSpace != -1) {
            String param = params.substring(0, secondSpace);
            String value = params.substring(secondSpace + 1);
            onSet(param, value);
        } else {
            onUnknownCommand(commandLine);
        }
    } else if (cmd == "calibrate") {
        onCalibrate(params);
    }
    else {
        onUnknownCommand(commandLine);
    }
}

// --- HELPER FUNCTIES ---
void bleSerialPrint(String data) {
    if (deviceConnected && pTxCharacteristic != NULL) {
        pTxCharacteristic->setValue(data.c_str());
        pTxCharacteristic->notify();
    }
    Serial.print(data);
}

void bleSerialPrintln(String data) {
    bleSerialPrint(data + "\n");
}

void readParams() {
  preferences.begin(PREFS_NAME, true);
  if (preferences.getBytesLength(PARAM_KEY) == PARAM_SIZE) {
    preferences.getBytes(PARAM_KEY, &params, PARAM_SIZE);
  } else {
    params.cycleTime = 10000;
    params.power = 150;
    params.diff = 0.5;
    params.kp = 1.0;
    params.ki = 0.0;
    params.kd = 0.0;
    for (int i = 0; i < 8; i++) {
      params.black[i] = 4095;
      params.white[i] = 0;
    }
    writeParams();
    Serial.println("Initial parameters set and saved.");
  }
  preferences.end();
}

void writeParams() {
  preferences.begin(PREFS_NAME, false);
  preferences.putBytes(PARAM_KEY, &params, PARAM_SIZE);
  preferences.end();
}

// --- SETUP ---
void setup()
{
  Serial.begin(Baudrate);

  BLEDevice::init("NanoESP32_Robot");
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);

  pTxCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID_TX,
                      BLECharacteristic::PROPERTY_NOTIFY
                    );
  pTxCharacteristic->addDescriptor(new BLE2902());

  BLECharacteristic *pRxCharacteristic = pService->createCharacteristic(
                                           CHARACTERISTIC_UUID_RX,
                                           BLECharacteristic::PROPERTY_WRITE
                                        );
  pRxCharacteristic->setCallbacks(new MyCallbacks());

  pService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);
  pAdvertising->setMaxPreferred(0x08);

  BLEDevice::startAdvertising();

  Serial.println("BLE Initialisatie voltooid. Adverteert.");

  readParams();

  pinMode(StatusLED, OUTPUT);
  digitalWrite(StatusLED, HIGH);

  pinMode(RunStatusLED, OUTPUT);
  digitalWrite(RunStatusLED, LOW);

  pinMode(StopStatusLED, OUTPUT);
  digitalWrite(StopStatusLED, LOW);

  pinMode(MotorLeftA, OUTPUT);
  pinMode(MotorLeftB, OUTPUT);
  pinMode(MotorRightA, OUTPUT);
  pinMode(MotorRightB, OUTPUT);

  pinMode(Drukknop, INPUT_PULLUP);

  bleSerialPrintln("ready");
}

// --- LOOP ---
void loop()
{
  if (!deviceConnected && run) {
    onStop();
  }

  // --- LED LOGICA ---
  if (run) {
    unsigned long currentMillis = millis();

    if (currentMillis - ledPreviousMillis >= ledInterval) {
      ledPreviousMillis = currentMillis;

      ledState = !ledState;
      digitalWrite(RunStatusLED, ledState);
    }

    digitalWrite(StopStatusLED, LOW);

  } else {
    unsigned long currentMillis = millis();

    if (currentMillis - ledPreviousMillis >= ledInterval) {
      ledPreviousMillis = currentMillis;

      ledState = !ledState;
      digitalWrite(StopStatusLED, ledState);
    }

    digitalWrite(RunStatusLED, LOW);
  }

  // --- PID CYCLUS ---
  unsigned long current = micros();

  if (current - previous >= params.cycleTime)
  {
    previous = current;

    for (int i = 0; i < 8; i++)
    {
      normalised[i] = constrain(map(analogRead(sensor[i]), params.white[i], params.black[i], 1000, 0), 0, 1000);
    }

    float position = 0;
    int index = 0;
    for(int i = 1; i < 8; i++) if (normalised[i] < normalised[index]) index = i;

    if (index == 0) position = -30;
    else if (index == 7) position = 30;
    else
    {
      int sensor_nul = normalised[index];
      int sensor_min_een = normalised[index-1];
      int sensor_plus_een = normalised[index+1];

      float b = sensor_plus_een - sensor_min_een;
      b = b / 2;

      float a = sensor_plus_een - b - sensor_nul;

      position = -b / (2 * a);
      position += index;
      position -= 3.5;

      position *= 9.525;
    }
    debugPosition = position;

    float error = -position;

    output = error * params.kp;
    iTerm += params.ki * error;
    iTerm = constrain(iTerm, -510, 510);
    output += iTerm;
    output += params.kd * (error - lastErr);
    lastErr = error;
    output = constrain(output, -510, 510);

    if (run && deviceConnected)
    {
      int leftSpeed = constrain(params.power + (int)output, -255, 255);
      int rightSpeed = constrain(params.power - (int)output, -255, 255);

      analogWrite(MotorLeftA, leftSpeed > 0 ? leftSpeed : 0);
      analogWrite(MotorLeftB, leftSpeed < 0 ? -leftSpeed : 0);

      analogWrite(MotorRightA, rightSpeed > 0 ? rightSpeed : 0);
      analogWrite(MotorRightB, rightSpeed < 0 ? -rightSpeed : 0);
    }
  }

  unsigned long difference = micros() - current;
  if (difference > calculationTime) calculationTime = difference;
}

// --- COMMANDOS ---
void onUnknownCommand(String command)
{
  bleSerialPrint("unknown command: \"");
  bleSerialPrint(command);
  bleSerialPrintln("\"");
}

void onRun()
{
  if (!deviceConnected) {
    bleSerialPrintln("Kan niet starten: geen BLE verbinding.");
    return;
  }
  run = true;
  digitalWrite(StatusLED, HIGH);
  bleSerialPrintln("Robot gestart.");
}

void onStop()
{
  run = false;
  digitalWrite(StatusLED, LOW);

  analogWrite(MotorLeftA, 0);
  analogWrite(MotorLeftB, 0);
  analogWrite(MotorRightA, 0);
  analogWrite(MotorRightB, 0);

  bleSerialPrintln("Robot gestopt.");
}

void onSet(String param, String value)
{
  if (param.length() == 0 || value.length() == 0) {
    bleSerialPrintln("Error: Missing parameter or value.");
    return;
  }

  if (param == "cycle")
  {
    long newCycleTime = value.toInt();
    if (newCycleTime > 0) {
      float ratio = ((float)newCycleTime) / ((float)params.cycleTime);
      params.ki *= ratio;
      params.kd /= ratio;
      params.cycleTime = newCycleTime;
    }
  }
  else if (param == "ki")
  {
    float cycleTimeInSec = ((float)params.cycleTime) / 1000000;
    params.ki = value.toFloat() * cycleTimeInSec;
  }
  else if (param == "kd")
  {
    float cycleTimeInSec = ((float)params.cycleTime) / 1000000;
    params.kd = value.toFloat() / cycleTimeInSec;
  }
  else if (param == "power") params.power = value.toInt();
  else if (param == "diff") params.diff = value.toFloat();
  else if (param == "kp") params.kp = value.toFloat();

  writeParams();
  bleSerialPrintln("Parameter saved.");
}

void onDebug()
{
  bleSerialPrintln("--- DEBUG ---");
  bleSerialPrint("cycle time (us): ");
  bleSerialPrintln(String(params.cycleTime));

  bleSerialPrint("black: ");
  for (int i = 0; i < 8; i++) bleSerialPrint(String(params.black[i]) + " ");
  bleSerialPrintln("");

  bleSerialPrint("white: ");
  for (int i = 0; i < 8; i++) bleSerialPrint(String(params.white[i]) + " ");
  bleSerialPrintln("");

  bleSerialPrint("normalised: ");
  for (int i = 0; i < 8; i++) bleSerialPrint(String(normalised[i]) + " ");
  bleSerialPrintln("");

  bleSerialPrint("output: "); bleSerialPrintln(String(output, 2));
  bleSerialPrint("power: "); bleSerialPrintln(String(params.power));
  bleSerialPrint("kp: "); bleSerialPrintln(String(params.kp, 3));

  float cycleTimeInSec = ((float)params.cycleTime) / 1000000;
  float ki_display = params.ki / cycleTimeInSec;
  bleSerialPrint("ki (sec): "); bleSerialPrintln(String(ki_display, 5));

  float kd_display = params.kd * cycleTimeInSec;
  bleSerialPrint("kd (sec): "); bleSerialPrintln(String(kd_display, 5));

  bleSerialPrint("iTerm: "); bleSerialPrintln(String(iTerm, 2));

  bleSerialPrint("calculation time (us): ");
  bleSerialPrintln(String(calculationTime));
  calculationTime = 0;
}

void onCalibrate(String param)
{
  if (param.length() == 0) {
    bleSerialPrintln("Error: Missing calibration type (black/white).");
    return;
  }

  int rawReadings[8];
  for (int i = 0; i < 8; i++) {
    rawReadings[i] = analogRead(sensor[i]);
  }

  if (param == "black")
  {
    bleSerialPrint("start calibrating black... ");
    for (int i = 0; i < 8; i++) params.black[i] = rawReadings[i];
    bleSerialPrintln("done");
  }
  else if (param == "white")
  {
    bleSerialPrint("start calibrating white... ");
    for (int i = 0; i < 8; i++) params.white[i] = rawReadings[i];
    bleSerialPrintln("done");
  } else {
    bleSerialPrintln("Error: Unknown calibration type.");
    return;
  }

  writeParams();
}
