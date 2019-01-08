
#include <FastPID.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <WiFiUdp.h>
#include <Wire.h> 
#include "mpu9250.h"
#include <Servo.h> 

const char *ssid = "JcQuad";
const char *password = "F!v3five";

WiFiUDP Udp;
unsigned int port = 1296;
char incomingPacket[5];
float pidKp=5, pidKi=0, pidKd=0, pidHz=100;
int output_bits = 16;
bool output_signed = true;
bool enabled = false;

FastPID rollpid(pidKp, pidKi, pidKd, pidHz, output_bits, output_signed);
FastPID pitchpid(pidKp, pidKi, pidKd, pidHz, output_bits, output_signed);
float rollOffset = 12.5f;
float pitchOffset = 6.0f;
float hover = 0;

Servo flMotor;
Servo frMotor;
Servo blMotor;
Servo brMotor;

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
  delay(5000);
  flMotor.attach(D6);
  frMotor.attach(D5);
  blMotor.attach(D7);
  brMotor.attach(D8);
  setPower(flMotor,0);
  setPower(frMotor,0);
  setPower(blMotor,0);
  setPower(brMotor,0);


}

void ledCommand(char* para){
  if(para[0]){
      digitalWrite(LED_BUILTIN, HIGH);
  }else{
      digitalWrite(LED_BUILTIN, LOW);
  }

}

void setPower(Servo servo, float power){
  if(power<0){
    power = 0;
  }
  if(power>1.0f){
    power = 1.0f;
  }
  //printf("Writing %f\n",1000*(1+power));
  servo.writeMicroseconds((int)(1000*(1+power)));
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
      printf("enabled %b\n",enabled);
      break;
      case 1: // Power
      unionHelper inhover;
      fillUH(&inhover, incomingPacket+1);
      hover = inhover.f;
      //printf("hover %f\n",hover);
      break;
      case 2: // kp
      unionHelper newkp;
      fillUH(&newkp, incomingPacket+1);
      pidKp = newkp.f;
      reconfigurePids();
      printf("kp %f\n",pidKp);
      break;
      case 3: // kd
      unionHelper newkd;
      fillUH(&newkd, incomingPacket+1);
      pidKd = newkd.f;
      reconfigurePids();
      printf("kd %f\n",pidKd);
      break;
      case 4: // ki
      unionHelper newki;
      fillUH(&newki, incomingPacket+1);
      pidKi = newki.f;
      reconfigurePids();
      printf("ki %f\n",pidKi);
      case 5: // rollOffset
      unionHelper ro;
      fillUH(&ro, incomingPacket+1);
      rollOffset = ro.f;
      printf("ro %f\n",rollOffset);
      break;
      case 6: // pitchOffset
      unionHelper pith;
      fillUH(&pith, incomingPacket+1);
      pitchOffset = pith.f;
      printf("po %f\n",pitchOffset);
      break;
      
    }
  }
}

void loop() {
  handlePackets();

  if(millis()-lastMessageTime>quadTimeout && enabled){
    printf("timed out\n");
     enabled = false;
  }
  
  if(!enabled){
      setPower(flMotor,0);
      setPower(frMotor,0);
      setPower(blMotor,0);
      setPower(brMotor,0);
      delay(15);
      return;
  }
  MpuTestLoop();
  int16_t output = rollpid.step(0, roll-rollOffset)/(float)PWMRANGE;
  //Serial.printf("roll %f, output: %d\n", roll-rollOffset, output);
  //setPower(D6,output+hover);// right
  //setPower(D8,-output+hover); // left

   float poutput = pitchpid.step(0, pitch-pitchOffset)/(float)PWMRANGE;
   if(-poutput+hover>0){
    Serial.printf(" output: %f\n", -poutput+hover);
   }  
  //setPower(flMotor,poutput+hover);
  setPower(frMotor,-poutput+hover);

  
  //analogWrite(D5,-output);// top
  delay(5);
}


