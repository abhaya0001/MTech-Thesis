#include <Wire.h>
#include <Servo.h>

#define DBG 1

#define RX_PIN 3
#define TX_PIN 2

#define M_SERVO 5

#define I2C_ADDRESS 3

int M_ANG[3] = {90, 50, 130}; //Neutral Ang, Right Ang, Left Ang

//Data Format -- INT -> MSB <Speed 0-63 (6 bit)> <Position (2 bit)> LSB
//Motor -- Relaxed -> 0, Right Contract -> 1, Left Contract -> 2

Servo mServo;
int curr_ang = 0, curr_pos = 0;
char c = 'f';

void setup(){
  Wire.begin(I2C_ADDRESS);
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);
  Serial.begin(9600);

  mServo.attach(M_SERVO);
}

void loop(){
}

void receiveEvent(int n)
{
  if(Wire.available()){
    byte data = Wire.read();
    while(Wire.available() > 0 ) {
      Wire.read();
    }
    Serial.print("Data: ");
    Serial.println(data);
    int pos = 0, spd = 0;
    processData(data, &pos, &spd);
#ifdef DBG
    Serial.print("POS: ");
    Serial.println(pos);
    Serial.print("SPD: ");
    Serial.println(spd);
#endif
    curr_pos = pos;
    writePosition(pos, spd);
  }
}

void requestEvent(int hw){
  bool ak = 0;
  for (uint8_t i=0; i<3; i++) {
    if (M_ANG[i] == curr_ang) {
      Serial.println(i);
      Wire.write(i);
      ak = 1;
      break;
    }
  }
  if(!ak){
    Wire.write(5); 
  }
  //  Wire.write(curr_pos);
}

void processData(int data, int *pos, int *spd){
  for(int i = 0; i<8; i++){
    if(i<2){
      if(bitRead(data, i)){
        bitSet(*pos,i);
      }
    }
    else{
      if(bitRead(data,i)){
        bitSet(*spd,i-2);
      }
    }
  }
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
