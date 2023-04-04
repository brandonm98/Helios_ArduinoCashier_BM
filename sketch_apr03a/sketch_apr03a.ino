#include <HX711_ADC.h>
#include "DHT.h"
#define PIRPin 40
#define IRPin 42
#define sensorPin 21
#define sensorPin2 20
#define sensorPin3 19
#define alarmPin 36
#define motorPin 52
#define motorPin2 50
#define motorPin3 48
#define DHTPIN 38
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

volatile int i; 
volatile int temp;
volatile int motion;
volatile int pos;
volatile int amountDispensed = 0;
volatile int amountDispensed2 = 0;
volatile int amountDispensed3 = 0;

volatile int flag = 0; //CONT1
volatile int flag2 = 0; //CONT2
volatile int flag3 = 0; //ALARM
volatile int flag4 = 0; //SERIAL COM
volatile int flag5 = 0; //CONT3 

int quantity;
int quantity2;
int quantity3;
HX711_ADC LoadCell(2, 3);  //DT, SCK
long t;
//////////////////////////////////////////////
//void(* resetFunc) (void) = 0;

void reset(){
volatile int flag = 0; //CONT1
volatile int flag2 = 0; //CONT2
volatile int flag3 = 0; //ALARM
volatile int flag4 = 0; //SERIAL COM
volatile int flag5 = 0; //CONT3 
volatile int amountDispensed = 0;
volatile int amountDispensed2 = 0;
volatile int amountDispensed3 = 0;
volatile byte tara;
String serialString="0";
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
    volatile int i = LoadCell.getData();
    //Serial.print("Load_cell output val: ");
// Serial.println(i);
 float h = dht.readHumidity();
  volatile  int temp = dht.readTemperature();
    float f = dht.readTemperature(true);
    if (isnan(t) || isnan(f)) {
   Serial.println("Failed to read from DHT sensor!");
    return;
    }
    float hif = dht.computeHeatIndex(f, h);
    float hic = dht.computeHeatIndex(temp, h, false);
    if (temp>30){
      Serial.println("Overheating!");
     alarm();} 
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
    t = millis(); }
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
 Serial.begin(2000000);
 attachInterrupt(digitalPinToInterrupt(sensorPin), coinPulse, RISING);
 attachInterrupt(digitalPinToInterrupt(sensorPin2), coinPulse2, RISING);
 attachInterrupt(digitalPinToInterrupt(sensorPin3), coinPulse3, RISING);
 pinMode(sensorPin, INPUT_PULLUP);
 pinMode(sensorPin2, INPUT_PULLUP);
 pinMode(sensorPin3, INPUT_PULLUP);
 pinMode(motorPin, OUTPUT);
 pinMode(motorPin2, OUTPUT);
 pinMode(motorPin3, OUTPUT);
 pinMode(53, OUTPUT);
 pinMode(51, OUTPUT);
 pinMode(49, OUTPUT);
 pinMode(47, OUTPUT);
 pinMode(45, OUTPUT);
 pinMode(43, OUTPUT);
 pinMode(41, OUTPUT);
 pinMode(39, OUTPUT);
 digitalWrite(motorPin, LOW);
 digitalWrite(motorPin2, LOW);
 digitalWrite(motorPin3, LOW);
 digitalWrite(53, HIGH);
 digitalWrite(51, HIGH);
 digitalWrite(49, HIGH);
 digitalWrite(47, HIGH);
 digitalWrite(45, HIGH);
 digitalWrite(43, HIGH);
 digitalWrite(41, HIGH);
 digitalWrite(39, HIGH);
 dht.begin();
 pinMode(PIRPin, INPUT);
 LoadCell.begin();
 long stabilisingtime = 2000;
 LoadCell.start(stabilisingtime);
 LoadCell.setCalFactor(-20.15); // user set calibration factor (float)
 Serial.println("Startup + tare is complete");}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void loop() {
 if(flag4==0){
 sensores();
 }
 
///////////////TARA
 if (Serial.available()){
 volatile byte tara = Serial.read();
 //Serial.print(tara);
  if(tara==116){
    volatile int i=0;            
    LoadCell.tareNoDelay();
    Serial.println("Tare complete");
    delay(1000);
  }
 
  if(tara!=116){
  if(tara!=13){
  flag4=1; 
  String serialString = Serial.readString();
  Serial.println(serialString);
  char charArray[serialString.length()];
  serialString.toCharArray(charArray, serialString.length());
  char * response = strtok(charArray, ";");
  response = strtok(NULL, ";");
  int quantity= atoi(response);
  response = strtok(NULL, ";");
  response = strtok(NULL, ";");
  int quantity2 = atoi(response);  
  response = strtok(NULL, ";");
  response = strtok(NULL, ";");
  int quantity3 = atoi(response); 
  response = strtok(NULL, ";");

 
  while(amountDispensed != quantity){
  digitalWrite(motorPin, HIGH);
  delay(90);
  flag=1;}
  digitalWrite(motorPin, LOW);

  while(amountDispensed2 != quantity2){    
  digitalWrite(motorPin2, HIGH);
  delay(120);
  flag2=1;}
  digitalWrite(motorPin2, LOW);
 
  while(amountDispensed3 != quantity3){  
  digitalWrite(motorPin3, HIGH);
  delay(120);
  flag5=1;}
  digitalWrite(motorPin3, LOW);
 
 Serial.println("DONE");
  delay(100);
  
 reset();
 
  }
  }
 } 
  
}
//}
