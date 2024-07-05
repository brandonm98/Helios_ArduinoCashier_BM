// hopper sensor pin
#define sensorPin 2
#define sensorPin2 3
#define sensorPin3 7

// motor pin
#define motorPin 4
#define motorPin2 5
#define motorPin3 A4

volatile unsigned int amountDispensed = 0;

uint8_t bufPtr = 0;
byte rb_char;
unsigned int conv_buf = 0;

char buffer[20];
unsigned int receivedChars[3];

void flush()
{
  for (uint8_t i = 0; i < 20; i++)
  {
    buffer[i] = ' ';
  }
}

void rst_receivedChars()
{
  for (uint8_t i = 0; i < 3; i++)
  {
    receivedChars[i] = 0;
  }
}
void reset()
{
  amountDispensed = 0;
  flush();
  rst_receivedChars();
}

void coinPulse()
{
  ++amountDispensed;
}

void convert_serial_frame()
{
  bufPtr = 0;
  while (true)
  {
    rb_char = Serial.read();
    if (rb_char != 255)
    {
      if (rb_char == 44)
      {
        receivedChars[bufPtr] = conv_buf;
        conv_buf = 0;
        bufPtr++;
      }
      else
      {
        if (rb_char != 62)
        {
          if (conv_buf == 0)
          {
            conv_buf = (int(rb_char) - 48);
          }
          else
          {
            conv_buf *= 10;
            conv_buf += (int(rb_char) - 48);
          }
        }
        else
        {
          receivedChars[bufPtr] = conv_buf;
          conv_buf = 0;
          break;
        }
      }
    }
  }
}

int processMotor(unsigned int motorPin_x, unsigned int qnt)
{
  int not_dispensed = 0;
  long to_start = millis();
  while (amountDispensed != qnt)
  {
    digitalWrite(motorPin_x, HIGH);
    if (millis() - to_start > 15000)
    {
      break;
    }
  }
  not_dispensed = qnt - amountDispensed;
  amountDispensed = 0;
  digitalWrite(motorPin_x, LOW);
  return not_dispensed;
}

void setup()
{
  Serial.begin(9600);
  // Define parallel special routine for hopper motor and sensor
  // Start motor if trigger, when sensor go from low to high stop thread
  attachInterrupt(digitalPinToInterrupt(sensorPin), coinPulse, RISING);
  attachInterrupt(digitalPinToInterrupt(sensorPin2), coinPulse, RISING);
  attachInterrupt(digitalPinToInterrupt(sensorPin3), coinPulse, RISING);
  // pull up resistence for sensosr (ensure a value always on digital pin)
  pinMode(sensorPin, INPUT);
  pinMode(sensorPin2, INPUT);
  pinMode(sensorPin3, INPUT);
  // Motor control
  pinMode(motorPin, OUTPUT);
  pinMode(motorPin2, OUTPUT);
  pinMode(motorPin3, OUTPUT);
  // Trun off hoppers
  digitalWrite(motorPin, LOW);
  digitalWrite(motorPin2, LOW);
  digitalWrite(motorPin3, LOW);

  while (!Serial)
  {
    ;
  }
  Serial.println("Startup is complete");
}

void loop()
{
  if (Serial.available())
  {
    byte rx_char = Serial.read();
    if (rx_char == 60)
    {
      convert_serial_frame();

      int rs1 = processMotor(motorPin, receivedChars[0]);
      int rs2 = processMotor(motorPin2, receivedChars[1]);
      int rs3 = processMotor(motorPin3, receivedChars[2]);

      sprintf(buffer, "%d,%d,%d", rs1, rs2, rs3);
      Serial.println(buffer);
      Serial.println("DONE");
    }
  }
  reset();
}
