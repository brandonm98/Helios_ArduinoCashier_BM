#include <HX711_ADC.h>
#include "DHT.h"
#include <EEPROM.h>
// IR sensor pin
#define PIRPin 40
#define IRPin 42
// Alarm pin
#define alarmPin 36
// hopper sensor pin
#define sensorPin 21
#define sensorPin2 20
#define sensorPin3 19

// motor pin
#define motorPin 52
#define motorPin2 50
#define motorPin3 48
// TMP sensor type and pin
#define DHTPIN 38
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

HX711_ADC LoadCell(2, 3); // DT, SCK
long t;

int ind1;
int ind2;
int ind3;
String incomingString;
byte tara;
//////////////////////////////////////////////
const int calVal_eepromAdress = 0;
float newCalibrationValue;
//////////////////////////////////////////////
void calibrate()
{
  Serial.println("***");
  Serial.println("Start calibration:");
  Serial.println("Place the load cell an a level stable surface.");
  Serial.println("Remove any load applied to the load cell.");
  LoadCell.update();
  LoadCell.tare();
  Serial.println("Tare complete");
  boolean _resume = false;
  Serial.println("Now, place your known mass on the loadcell.");
  Serial.println("Then send the weight of this mass (i.e. 100.0) from serial monitor.");
  float known_mass = 0;
  _resume = false;
  while (_resume == false)
  {
    LoadCell.update();
    if (Serial.available() > 0)
    {
      known_mass = Serial.parseFloat();
      if (known_mass != 0)
      {
        Serial.print("Known mass is: ");
        Serial.println(known_mass);
        _resume = true;
      }
    }
  }
  LoadCell.refreshDataSet();                                    // refresh the dataset to be sure that the known mass is measured correct
  newCalibrationValue = LoadCell.getNewCalibration(known_mass); // get the new calibration value
  EEPROM.put(calVal_eepromAdress, newCalibrationValue);
  EEPROM.get(calVal_eepromAdress, newCalibrationValue);
  LoadCell.setCalFactor(newCalibrationValue);
  Serial.print("Value ");
  Serial.print(newCalibrationValue);
  Serial.print(" saved to EEPROM address: ");
  Serial.println(calVal_eepromAdress);
  Serial.println("End calibration");
}
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
  delay(250);
}
/////////////////////////////////////////////
void alarm()
{
  while (flag3 < 3)
  {
    digitalWrite(alarmPin, HIGH);
    delay(350);
    digitalWrite(alarmPin, LOW);
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
    LoadCell.update();
    long i = LoadCell.getData();
    float h = dht.readHumidity();
    int temp = dht.readTemperature();
    float f = dht.readTemperature(true);
    if (isnan(t) || isnan(f))
    {
      Serial.println("Failed to read from DHT sensor!");
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
    Serial.println(i);
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
  pinMode(sensorPin, INPUT_PULLUP);
  pinMode(sensorPin2, INPUT_PULLUP);
  pinMode(sensorPin3, INPUT_PULLUP);
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
  EEPROM.get(calVal_eepromAdress, newCalibrationValue);
  LoadCell.begin();
  // time to calibrate load cell
  long stabilisingtime = 5000;
  LoadCell.start(stabilisingtime);
  Serial.println("Cal val:");
  Serial.println(newCalibrationValue);
  LoadCell.setCalFactor(newCalibrationValue); // user set calibration factor (float)
  Serial.println("Startup + calibration is complete");
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

    // tara == 't' in ascii code
    if (tara == 116)
    {
      int i = 0;
      LoadCell.tare();
      Serial.println("Tare complete");
    }
    else
    {
      if (tara >= 48 and tara <= 57)
      {
        flag4 = 1;
        while (Serial.available() == 0)
        {
        }
        incomingString = char(tara) + Serial.readString();
        incomingString.trim();
        ind1 = incomingString.indexOf(',');
        quantity = incomingString.substring(0, ind1).toInt();
        ind2 = incomingString.indexOf(',', ind1 + 1);
        quantity2 = incomingString.substring(ind1 + 1, ind2).toInt();
        ind3 = incomingString.indexOf(',', ind2 + 1);
        quantity3 = incomingString.substring(ind2 + 1, ind3).toInt();

        // Give coins for each hopper
        while (amountDispensed != quantity)
        {
          digitalWrite(motorPin, HIGH);
          delay(90);
          flag = 1;
        }
        digitalWrite(motorPin, LOW);

        while (amountDispensed2 != quantity2)
        {
          digitalWrite(motorPin2, HIGH);
          delay(90);
          flag2 = 1;
        }
        digitalWrite(motorPin2, LOW);

        while (amountDispensed3 != quantity3)
        {
          digitalWrite(motorPin3, HIGH);
          delay(90);
          flag5 = 1;
        }
        digitalWrite(motorPin3, LOW);
        Serial.println("DONE");
      }
      else
      {
        if (tara == 99)
        {
          calibrate();
        }
      }
    }
    reset();
  }
}