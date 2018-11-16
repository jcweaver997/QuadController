#include <FastPID.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <WiFiUdp.h>
#include <Wire.h> 
#include "mpu9250.h"

const char *ssid = "JcQuad";
const char *password = "F!v3five";

WiFiUDP Udp;
unsigned int port = 1296;
char incomingPacket[5];
float pidKp=35, pidKi=0, pidKd=0, pidHz=100;
int output_bits = 16;
bool output_signed = true;
bool enabled = false;

FastPID rollpid(pidKp, pidKi, pidKd, pidHz, output_bits, output_signed);
FastPID pitchpid(pidKp, pidKi, pidKd, pidHz, output_bits, output_signed);
float rollOffset = 12.5f;
float pitchOffset = 6.0f;
float hover = 0;

unsigned long quadTimeout = 2000;

unsigned long lastMessageTime = 0;

union unionHelper
{
  float f;
  uint32_t ui;
  int32_t i;
  byte b[4];
};

void setup() {
  setAllPower(0);

  //delay(5000);

  Serial.begin(115200);
  Wire.begin(4,0);
  Mpu9250Setup();
  WiFi.softAP(ssid, password);
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  Udp.begin(port);
  rollpid.setOutputRange(-PWMRANGE, PWMRANGE);
  pitchpid.setOutputRange(-PWMRANGE, PWMRANGE);
  if (rollpid.err() || pitchpid.err()) {
    Serial.println("There is a configuration error!");
    for (;;) {}
  }


}




void ledCommand(char* para){
  if(para[0]){
      digitalWrite(LED_BUILTIN, HIGH);
  }else{
      digitalWrite(LED_BUILTIN, LOW);
  }

}
void setAllPower(float power){
  int powers = power*PWMRANGE;
  if(powers<1){
    powers = 1;
  }
  analogWrite(D5,powers);
  analogWrite(D6,powers);
  analogWrite(D7,powers);
  analogWrite(D8,powers);
  //Serial.print("Motors set to: ");
  //Serial.print(powers);
  //Serial.print("\n");
}

void setPower(int pin, int power){
  if(power<0){
    power = 0;
  }
  analogWrite(pin, power);
}

void fillUH(unionHelper* uh, char* incomingPacket){
      uh->b[0] = incomingPacket[0];
      uh->b[1] = incomingPacket[1];
      uh->b[2] = incomingPacket[2];
      uh->b[3] = incomingPacket[3];
}

void reconfigurePids(){
      rollpid.clear();
      rollpid.configure(pidKp, pidKi, pidKd, pidHz, output_bits, output_signed);
      pitchpid.clear();
      pitchpid.configure(pidKp, pidKi, pidKd, pidHz, output_bits, output_signed);
      
}

void handlePackets(){
  int packetSize = Udp.parsePacket();

  if (packetSize==5)
  {
    lastMessageTime = millis();
    int len = Udp.read(incomingPacket, 5);
    
    switch(incomingPacket[0]){
      case 0: // Enable == 1296
      unionHelper enabledu;
      fillUH(&enabledu, incomingPacket+1);
      enabled = enabledu.ui==port?true:false;
      break;
      case 1: // Power
      unionHelper inhover;
      fillUH(&inhover, incomingPacket+1);
      hover = inhover.f;
      break;
      case 2: // kp
      unionHelper newkp;
      fillUH(&newkp, incomingPacket+1);
      pidKp = newkp.f;
      reconfigurePids();
      break;
      case 3: // kd
      unionHelper newkd;
      fillUH(&newkd, incomingPacket+1);
      pidKd = newkp.f;
      reconfigurePids();
      break;
      case 4: // ki
      unionHelper newki;
      fillUH(&newki, incomingPacket+1);
      pidKi = newki.f;
      reconfigurePids();
      case 5: // rollOffset
      unionHelper ro;
      fillUH(&ro, incomingPacket+1);
      rollOffset = ro.f;
      break;
      case 6: // pitchOffset
      unionHelper pith;
      fillUH(&pith, incomingPacket+1);
      pitchOffset = pith.f;
      break;
      
    }
  }
}

void loop() {
  handlePackets();

  if(millis()-lastMessageTime>quadTimeout){
     enabled = false;
  }
  
  if(!enabled){
      delay(5);
      return;
  }
  MpuTestLoop();
  int16_t output = rollpid.step(0, roll-rollOffset);
  //Serial.printf("roll %f, output: %d\n", roll-rollOffset, output);
  setPower(D6,output+hover);// right
  setPower(D8,-output+hover); // left

   int16_t poutput = pitchpid.step(0, pitch-pitchOffset);
  //Serial.printf("pitch %f, output: %d\n", pitch, poutput);
  setPower(D5,poutput+hover);// top
  setPower(D7,-poutput+hover); // bottom
  
  //analogWrite(D5,-output);// top
  delay(5);
}


