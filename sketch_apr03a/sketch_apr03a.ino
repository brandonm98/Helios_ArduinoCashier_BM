#include <HX711_ADC.h>
#include "DHT.h"
//IR sensor pin
#define PIRPin 40
#define IRPin 42
//Alarm pin
#define alarmPin 36
//hopper sensor pin
#define sensorPin 21
#define sensorPin2 20
#define sensorPin3 19

//motor pin
#define motorPin 52
#define motorPin2 50
#define motorPin3 48
//TMP sensor type and pin
#define DHTPIN 38
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

int i; 
int temp;
int motion;
int pos;
int amountDispensed = 0;
int amountDispensed2 = 0;
int amountDispensed3 = 0;

int flag = 0; //CONT1
int flag2 = 0; //CONT2
int flag3 = 0; //ALARM
int flag4 = 0; //SERIAL COM
int flag5 = 0; //CONT3 

int quantity;
int quantity2;
int quantity3;

HX711_ADC LoadCell(2, 3);  //DT, SCK
long t;

int ind1;
int ind2;
int ind3;
String incomingString;
bool started = false;
bool ended = false;
//////////////////////////////////////////////


void reset(){
  flag = 0; //CONT1
  flag2 = 0; //CONT2
  flag3 = 0; //ALARM
  flag4 = 0; //SERIAL COM
  flag5 = 0; //CONT3 
  amountDispensed = 0;
  amountDispensed2 = 0;
  amountDispensed3 = 0;
}
/////////////////////////////////////////////
void alarm(){
  while(flag3<3){
    digitalWrite(alarmPin, HIGH);
    delay(350);
    digitalWrite(alarmPin, LOW);
    delay(500);
    flag3++;
    }
    flag3=0;
}
////////////////////////////////////////////
void sensores(){ 
  if (millis() > t + 100) {
    LoadCell.update();
    int i = LoadCell.getData();
    float h = dht.readHumidity();
    int temp = dht.readTemperature();
    float f = dht.readTemperature(true);
    if (isnan(t) || isnan(f)) {
      Serial.println("Failed to read from DHT sensor!");
      return;
    }
    float hif = dht.computeHeatIndex(f, h);
    float hic = dht.computeHeatIndex(temp, h, false);
    if (temp>30){
      Serial.println("Overheating!");
      alarm();
    } 
    if(digitalRead(PIRPin)==HIGH){
      motion=1;
      alarm();
    }else{
      motion=0;   
    }
    if(digitalRead(IRPin)==HIGH){
      pos=1;
      alarm();
    }else{
      pos=0;  
    }
    char buf[100];
    snprintf(buf, 100,"Temperature: %d Weight: %d Motion: %d Position: %d",temp,i,motion,pos);
    Serial.println(buf);
    t = millis(); 
  }
}

////////////////////////////////////////////////////////////////////////////////
void coinPulse(){
  if(flag==1){
    ++amountDispensed;
  }else{
    amountDispensed=0;  
  } 
}

void coinPulse2(){
  if(flag2==1){
    ++amountDispensed2;
  }else{
    amountDispensed2=0;  
  } 
}

void coinPulse3(){
  if(flag5==1){
    ++amountDispensed3;
  }else{
    amountDispensed3=0;  
  } 
}
///////////////////////////////////////////////////////////////////////////////////////////////////////
void setup() {
  Serial.begin(9600);
  //Define parallel special routine for hopper motor and sensor
  //Start motor if trigger, when sensor go from low to high stop thread
  attachInterrupt(digitalPinToInterrupt(sensorPin), coinPulse, RISING);
  attachInterrupt(digitalPinToInterrupt(sensorPin2), coinPulse2, RISING);
  attachInterrupt(digitalPinToInterrupt(sensorPin3), coinPulse3, RISING);
  //pull up resistence for sensosr (ensure a value always on digital pin)
  pinMode(sensorPin, INPUT_PULLUP);
  pinMode(sensorPin2, INPUT_PULLUP);
  pinMode(sensorPin3, INPUT_PULLUP);
  //Motor control
  pinMode(motorPin, OUTPUT);
  pinMode(motorPin2, OUTPUT);
  pinMode(motorPin3, OUTPUT);
  //alarm set up pin
  pinMode(alarmPin, OUTPUT);
  //IR sensor set up
  pinMode(PIRPin, INPUT_PULLUP);
  pinMode(IRPin, INPUT_PULLUP);

  
  //Trun off hoppers
  digitalWrite(motorPin, LOW);
  digitalWrite(motorPin2, LOW);
  digitalWrite(motorPin3, LOW);

  pinMode(53, OUTPUT);
  pinMode(51, OUTPUT);
  pinMode(49, OUTPUT);
  pinMode(47, OUTPUT);
  pinMode(45, OUTPUT);
  pinMode(43, OUTPUT);
  pinMode(41, OUTPUT);
  pinMode(39, OUTPUT);

  digitalWrite(53, HIGH);
  digitalWrite(51, HIGH);
  digitalWrite(49, HIGH);
  digitalWrite(47, HIGH);
  digitalWrite(45, HIGH);
  digitalWrite(43, HIGH);
  digitalWrite(41, HIGH);
  digitalWrite(39, HIGH);

  //init temp sens
  dht.begin();
  // pinMode(PIRPin, INPUT);
  LoadCell.begin();
  //time to calibrate load cell
  long stabilisingtime = 2000;
  LoadCell.start(stabilisingtime);

  LoadCell.setCalFactor(1); // user set calibration factor (float)
  Serial.println("Startup + calibration is complete");}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void loop() {
  if(flag4==0){
    sensores();
  }

///////////////TARA

  if (Serial.available()){
    byte tara = Serial.read();
    
    // tara == 't' in ascii code
    if(tara==116){
      int i=0;            
      LoadCell.tareNoDelay();
      while(LoadCell.getTareStatus()){}
      delay(3000);
      Serial.println("Tare complete");
    }
    if(tara!=116&&tara!=13){
      flag4=1;
      while (Serial.available() == 0) {}  
      incomingString = char(tara) + Serial.readString();
      incomingString.trim();
      ind1 = incomingString.indexOf(',');
      quantity = incomingString.substring(0, ind1).toInt();
      ind2 = incomingString.indexOf(',', ind1+1);
      quantity2 = incomingString.substring(ind1+1, ind2).toInt();
      ind3 = incomingString.indexOf(',', ind2+1);
      quantity3 = incomingString.substring(ind2+1, ind3).toInt();

      //Give coins for each hopper
      while(amountDispensed != quantity){
        digitalWrite(motorPin, HIGH);
        delay(90);
        flag=1;
      }
      digitalWrite(motorPin, LOW);

      while(amountDispensed2 != quantity2){    
        digitalWrite(motorPin2, HIGH);
        delay(120);
        flag2=1;
      }
      digitalWrite(motorPin2, LOW);

      while(amountDispensed3 != quantity3){  
        digitalWrite(motorPin3, HIGH);
        delay(120);
        flag5=1;
      }
      digitalWrite(motorPin3, LOW);
      Serial.println("DONE");
      delay(100);
      reset();
      incomingString = "";
      tara = 13;
    }
  } 
}