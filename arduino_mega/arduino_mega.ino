// Hopper sensor and motor pins
#define SENSOR_PIN1 20
#define SENSOR_PIN2 19
#define SENSOR_PIN3 21

#define MOTOR_PIN1 42
#define MOTOR_PIN2 48
#define MOTOR_PIN3 43

volatile unsigned int dispensed[4] = {0, 0, 0, 0};  // Array to track dispensed amounts
unsigned int quantities[4] = {0, 0, 0, 0};  // Array to store input quantities

char buffer[50]; // Buffer for formatting output

void setup() {
  Serial.begin(9600);

  // Attach interrupts for sensors
  attachInterrupt(digitalPinToInterrupt(SENSOR_PIN1), [] { ++dispensed[0]; }, RISING);
  attachInterrupt(digitalPinToInterrupt(SENSOR_PIN2), [] { ++dispensed[1]; }, RISING);
  attachInterrupt(digitalPinToInterrupt(SENSOR_PIN3), [] { ++dispensed[2]; }, RISING);

  // Configure sensor and motor pins
  pinMode(SENSOR_PIN1, INPUT);
  pinMode(SENSOR_PIN2, INPUT);
  pinMode(SENSOR_PIN3, INPUT);

  pinMode(MOTOR_PIN1, OUTPUT);
  pinMode(MOTOR_PIN2, OUTPUT);
  pinMode(MOTOR_PIN3, OUTPUT);

  // Ensure motors are initially off
  digitalWrite(MOTOR_PIN1, HIGH);
  digitalWrite(MOTOR_PIN2, HIGH);
  digitalWrite(MOTOR_PIN3, HIGH);

  Serial.println("Startup is complete");
}

void loop() {
  if (Serial.available() && Serial.read() == '<') {
    parseInput();
    if (validInput()) {
      int results[3];
      results[0] = dispenseCoins(MOTOR_PIN1, 0);
      results[1] = dispenseCoins(MOTOR_PIN2, 1);
      results[2] = dispenseCoins(MOTOR_PIN3, 2);
      sprintf(buffer, "%d,%d,%d,%d\nDONE\n", results[0], results[1], results[2], 0);
      Serial.print(buffer);
    } else {
      Serial.println("ERROR: Invalid input format");
    }
    reset();
  }
}

void parseInput() {
  memset(quantities, 0, sizeof(quantities));
  char rb_char;
  unsigned int conv_buf = 0, bufPtr = 0;
  while (bufPtr < 3) {
    rb_char = Serial.read();
    if (rb_char == ',') {
      if (bufPtr < 3) {
        quantities[bufPtr++] = conv_buf;
        conv_buf = 0;
      }
    } else if (rb_char == '>') {
      if (bufPtr < 3) {
        quantities[bufPtr] = conv_buf;
      }
      break;
    } else if (rb_char >= '0' && rb_char <= '9') {
      conv_buf = conv_buf * 10 + (rb_char - '0');
    }
  }
}

bool validInput() {
  return quantities[0] >= 0 || quantities[1] >= 0 || quantities[2] >= 0;
}

int dispenseCoins(unsigned int motorPin, int index) {
  long startTime = millis();
  dispensed[index] = 0;

  while (dispensed[index] < quantities[index]) {
    digitalWrite(motorPin, LOW);
    if (millis() - startTime > 15000) {
      break;
    }
  }

  digitalWrite(motorPin, HIGH);
  int not_dispensed = dispensed[index] > quantities[index] ? 0 : quantities[index] - dispensed[index];
  return not_dispensed;
}

void reset() {
  memset(dispensed, 0, sizeof(dispensed));
}
