// hopper sensor pin
#define sensorPin 21
#define sensorPin2 20
#define sensorPin3 19

// motor pin
#define motorPin 52
#define motorPin2 50
#define motorPin3 48

volatile int amountDispensed = 0;
volatile int amountDispensed2 = 0;
volatile int amountDispensed3 = 0;

int flag = 0;  // CONT1
int flag2 = 0; // CONT2
int flag4 = 0; // SERIAL COM
int flag5 = 0; // CONT3

int quantity;
int quantity2;
int quantity3;

long to_start;

int ind1;
int ind2;
int ind3;

byte tara;
uint8_t bufPtr = 0;
//////////////////////////////////////////////
char buffer[20];
char receivedChars[20];
//////////////////////////////////////////////
void flush() {
  bufPtr = 0;
  for (uint8_t i = 0; i < 8; i++) {
    receivedChars[i] = ' ';
    buffer[i] = ' ';
  }
}
void reset()
{
  flag = 0;  // CONT1
  flag2 = 0; // CONT2
  flag4 = 0; // SERIAL COM
  flag5 = 0; // CONT3
  amountDispensed = 0;
  amountDispensed2 = 0;
  amountDispensed3 = 0;
  tara = 13;
  flush();
}
////////////////////////////////////////////////////////////////////////////////
void coinPulse()
{
  if (flag == 1)
  {
    ++amountDispensed;
  }
  else
  {
    amountDispensed = 0;
  }
}

void coinPulse2()
{
  if (flag2 == 1)
  {
    ++amountDispensed2;
  }
  else
  {
    amountDispensed2 = 0;
  }
}

void coinPulse3()
{
  if (flag5 == 1)
  {
    ++amountDispensed3;
  }
  else
  {
    amountDispensed3 = 0;
  }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////
void setup()
{
  Serial.begin(9600);
  // Define parallel special routine for hopper motor and sensor
  // Start motor if trigger, when sensor go from low to high stop thread
  attachInterrupt(digitalPinToInterrupt(sensorPin), coinPulse, RISING);
  attachInterrupt(digitalPinToInterrupt(sensorPin2), coinPulse2, RISING);
  attachInterrupt(digitalPinToInterrupt(sensorPin3), coinPulse3, RISING);
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

  while(!Serial){
    ;
  }
  Serial.println("Startup is complete");
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void loop()
{
  ///////////////TARA
  if (Serial.available())
  {
    byte tara = Serial.read();
    if (tara >= 48 and tara <= 57)
    {
      flag4 = 1;
      bufPtr = 1;
      byte rb_char;
      receivedChars[0] = char(tara);
      while(true){
        rb_char = Serial.read();
        if(rb_char != 255){
          receivedChars[bufPtr] = char(rb_char);
          bufPtr++;
          if (bufPtr == 5){
            break;
          }
        }
      }
      quantity = int(receivedChars[0])-48;
      quantity2 = int(receivedChars[2])-48;
      quantity3 = int(receivedChars[4])-48;
      // Give coins for each hopper
      to_start = millis();
      while (amountDispensed != quantity)
      {
        digitalWrite(motorPin, HIGH);
        flag = 1;
        if(millis()-to_start > 15000){break;}
      }
      digitalWrite(motorPin, LOW);

      to_start = millis();
      while (amountDispensed2 != quantity2)
      {
        digitalWrite(motorPin2, HIGH);
        flag2 = 1;
        if(millis()-to_start > 15000){break;}
      }
      digitalWrite(motorPin2, LOW);
      to_start = millis();
      while (amountDispensed3 != quantity3)
      {
        digitalWrite(motorPin3, HIGH);
        flag5 = 1;
        if(millis()-to_start > 15000){break;}
      }
      digitalWrite(motorPin3, LOW);
      sprintf(buffer, "%d,%d,%d", quantity-amountDispensed, quantity2-amountDispensed2, quantity3-amountDispensed3);
      Serial.println(buffer);
      Serial.println("");
      Serial.println("DONE");
    }
  }
  reset();
}