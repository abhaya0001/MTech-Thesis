#include <Wire.h>
#include <SoftwareSerial.h>
#include <Servo.h>

#define DBG 1
#define SLAVE_MID_ADDRESS 2
#define SLAVE_TAIL_ADDRESS 3
#define RX_PIN 3
#define TX_PIN 2
#define M_SERVO 5

int M_ANG[3] = {90, 45, 135}; //Neutral Ang, Right Ang, Left Ang

//Data Format -- INT -> MSB <Speed 0-63 (6 bit)> <Position (2 bit)> LSB
//Motor -- Relaxed -> 0, Right Contract -> 1, Left Contract -> 2

SoftwareSerial btSerial(RX_PIN, TX_PIN); //mSerial(A4, A5);
Servo mServo;

int curr_ang = 0;
int prev_time = 0;

#define THRSH 600
int sped = 0;

char c = 'f';

void setup(){
    Wire.begin();
    btSerial.begin(9600);
    Serial.begin(9600);
    mServo.attach(M_SERVO);
    Serial.println("Initializing...");
    initi();
    Serial.println("Initialized..!");
}

int a=0;
int IR[3] = {3,6,2};  // Left, Mid, Right      

void loop() {
//  if(Serial.available()){
//    char ch = Serial.read();    
//    
//    if(ch=='a'){
//      a += 5;
//    }
//    else if(ch=='b'){
//      a -= 5;
//    }
//    Serial.println(a);
//  }
//  mServo.write(a);
//  mServo.write(a);
  moveForward();
  
  if (btSerial.available()){        
    String s = btSerial.readString();
    char ch = s.charAt(0);
    sped = s.substring(1).toInt();
    if (ch > 0 && ch < 255){
        c = ch;
    }
  }
  a = 0;
  for (int i = 0; i < 3; i++) {        
    Serial.print(analogRead(IR[i]));Serial.print(" ");
    a += ((analogRead(IR[i]) > THRSH) ? 1 : 0) * (100/pow(10,i));
  }
  Serial.print("A: ");Serial.print(a);Serial.print(" >>");    
  // LMR
  moveForward();
/*  if(c=='f' || c=='F'){
    if(a==111 || a==101 || a==10 || a==1 || a==11){
      Serial.println("Left");
      moveLeft();
    }
    else if(a==0){
      Serial.println("Forward");
      moveForward();
    }
    else if(a==110 || a==100){
      Serial.println("Right");
      moveRight();
    }
  }
  else if(c=='s' || c=='S'){
    Serial.println("Stopped");      
    c = '.';
    initi();
  }
  else if(c=='l' || c=='L'){
    Serial.println("Left");      
    moveLeft();
  }
  else if(c=='l' || c=='L'){
    Serial.println("Left");      
    moveRight();
  }*/
}

void moveLeft() { // H2 T1 H0 T0    
    writePosition(2, sped);
    writeSlave(SLAVE_TAIL_ADDRESS, 1, sped);
    writePosition(0, sped);
    writeSlave(SLAVE_TAIL_ADDRESS, 0, sped);
}

void moveRight() {  // H1 T2 H0 T0    
    writePosition(1, sped);
    writeSlave(SLAVE_TAIL_ADDRESS, 2, sped);
    writePosition(0, sped);
    writeSlave(SLAVE_TAIL_ADDRESS, 0, sped);
}

void moveForward(){ // H2 T1 M2 H1 T2 M1
    writePosition(2, sped);
    writeSlave(SLAVE_TAIL_ADDRESS, 1, sped);
    writeSlave(SLAVE_MID_ADDRESS, 2, sped);
    writePosition(1, sped);
    writeSlave(SLAVE_TAIL_ADDRESS, 2, sped);
    writeSlave(SLAVE_MID_ADDRESS, 1, sped);
}

void writeSlave(int addr, int pos, int spd) {
  int data = processData(pos, spd);
#ifdef DBG
  Serial.print("Writing Slave : ");
  Serial.println(addr);
  Serial.print("Data");
  Serial.println(data);
#endif
  Wire.beginTransmission(addr);
  Wire.write(data);
  Wire.endTransmission();
  Serial.println("*********Slave Wrote***********");

  while(1){
    if(checkSlave(addr,pos,data)){
      Serial.println("Success");
      break;
    }
    else{
      Serial.println("NOT DONE");
      delay(20);
    }
  }
  
  /*
  prev_time = millis();
  while (1) {
    if (millis() - prev_time > 10) {
      if (checkSlave(addr, pos, data)) {
        Serial.println("Success");
        break;
      }
      else {
        prev_time = millis();
      }
    }
  }*/
}

bool checkSlave(int addr, int pos, int data) {
  Wire.requestFrom(addr, 1);
 /* int resp;
  while(1){
    if(Wire.available()){
      resp = Wire.read();
      break;
    }
  }*/
  int resp = Wire.read();  
  while (Wire.available() > 0) {
    Wire.read();
  }
#ifdef DBG
  Serial.print("Response : ");
  Serial.println(resp);
#endif
  if (resp == pos){
    return true;
  }
  return false;
}

void initi(){
  mServo.write(M_ANG[0]);
  writeSlave(SLAVE_TAIL_ADDRESS, 0, 0);
  writeSlave(SLAVE_MID_ADDRESS, 0, 0);
  delay(1000);
}

void writePosition(int pos, int spd) {
  Serial.println("Writing Position...");
  while (curr_ang != M_ANG[pos]){
#ifdef DBG
    Serial.println(curr_ang);
#endif
    if (((M_ANG[pos] - curr_ang) < 5) && ((M_ANG[pos] - curr_ang) > -5)) {
      curr_ang = M_ANG[pos];
    }
    else {
      if ((M_ANG[pos] - curr_ang) > 0) {
        curr_ang += 5;
      }
      else {
        curr_ang -= 5;
      }
    }
    mServo.write(curr_ang);
    mServo.write(curr_ang);
    mServo.write(curr_ang);
    delay(spd);
  }
  Serial.println("**********Pos Wrote************");
}

int processData(int pos, int spd){
  int data = 0;
  for(int i = 0; i<8; i++){
    if(i<2){
      if(bitRead(pos, i)){
        bitSet(data,i);
      }
    }
    else{
      if(bitRead(spd,i-2)){
        bitSet(data,i);
      }
    }
  }
  return data;
}
