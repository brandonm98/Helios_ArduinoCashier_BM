#include "DHT.h"
// IR sensor pin
#define PIRPin A5
#define IRPin A0
// Alarm pin
#define alarmPin 12
// hopper sensor pin
#define sensorPin 2
#define sensorPin2 3
#define sensorPin3 7

// motor pin
#define motorPin 4
#define motorPin2 5
#define motorPin3 A4
// TMP sensor type and pin
#define DHTPIN A3
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

int motion;
int pos;
volatile int amountDispensed = 0;
volatile int amountDispensed2 = 0;
volatile int amountDispensed3 = 0;

int flag = 0;  // CONT1
int flag2 = 0; // CONT2
int flag3 = 0; // ALARM
int flag4 = 0; // SERIAL COM
int flag5 = 0; // CONT3

int quantity;
int quantity2;
int quantity3;

long t;
long to_start;

int ind1;
int ind2;
int ind3;
String incomingString;
byte tara;
//////////////////////////////////////////////
char buffer[100];
//////////////////////////////////////////////
void reset()
{
  flag = 0;  // CONT1
  flag2 = 0; // CONT2
  flag3 = 0; // ALARM
  flag4 = 0; // SERIAL COM
  flag5 = 0; // CONT3
  amountDispensed = 0;
  amountDispensed2 = 0;
  amountDispensed3 = 0;
  tara = 13;
  incomingString = "";
  delay(120);
}
/////////////////////////////////////////////
void alarm()
{
  while (flag3 < 3)
  {
    tone(alarmPin, 988, 500);
    delay(350);
    analogWrite(alarmPin, 0);
    delay(500);
    flag3++;
  }
  flag3 = 0;
}
////////////////////////////////////////////
void sensores()
{
  if (millis() > t + 100)
  {
    float h = dht.readHumidity();
    int temp = dht.readTemperature();
    float f = dht.readTemperature(true);
    if (isnan(t) || isnan(f))
    {
      //Serial.println("Failed to read from DHT sensor!");
      return;
    }
    float hif = dht.computeHeatIndex(f, h);
    float hic = dht.computeHeatIndex(temp, h, false);
    if (temp > 30)
    {
      Serial.println("Overheating!");
      alarm();
    }
    if (digitalRead(PIRPin) == HIGH)
    {
      motion = 1;
      alarm();
    }
    else
    {
      motion = 0;
    }
    if (digitalRead(IRPin) == HIGH)
    {
      pos = 1;
      alarm();
    }
    else
    {
      pos = 0;
    }
    t = millis();
  }
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
  // alarm set up pin
  pinMode(alarmPin, OUTPUT);
  // IR sensor set up
  pinMode(PIRPin, INPUT_PULLUP);
  pinMode(IRPin, INPUT_PULLUP);

  // Trun off hoppers
  digitalWrite(motorPin, LOW);
  digitalWrite(motorPin2, LOW);
  digitalWrite(motorPin3, LOW);

  // init temp sens
  dht.begin();
  Serial.println("Startup is complete");
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void loop()
{
  if (flag4 == 0)
  {
    sensores();
  }

  ///////////////TARA

  if (Serial.available())
  {
    byte tara = Serial.read();
    if (tara >= 48 and tara <= 57)
    {
      flag4 = 1;
      while (Serial.available() == 0){}
      incomingString = char(tara) + Serial.readString();
      incomingString.trim();
      ind1 = incomingString.indexOf(',');
      quantity = incomingString.substring(0, ind1).toInt();
      ind2 = incomingString.indexOf(',', ind1 + 1);
      quantity2 = incomingString.substring(ind1 + 1, ind2).toInt();
      ind3 = incomingString.indexOf(',', ind2 + 1);
      quantity3 = incomingString.substring(ind2 + 1, ind3).toInt();

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
