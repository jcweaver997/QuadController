
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

union {
byte asBytes[4];
float asFloat;
} Floater;

void setup() {
  Floater.asFloat = 0;
  setAllPower(0);
  Serial.begin(115200);
  Wire.begin();
  WiFi.softAP(ssid, password);

  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  Udp.begin(port);
  
  //Mpu9250Setup();



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

void handlePackets(){
  int packetSize = Udp.parsePacket();

  if (packetSize==5)
  {
    int len = Udp.read(incomingPacket, 5);
    switch(incomingPacket[0]){
      case 0:
        //ledCommand(incomingPacket+1);
      break;
      case 1:
          Floater.asBytes[0] = incomingPacket[1];
          Floater.asBytes[1] = incomingPacket[2];
          Floater.asBytes[2] = incomingPacket[3];
          Floater.asBytes[3] = incomingPacket[4];
      break;
    }
  }
}

void loop() {
  handlePackets();
  setAllPower(Floater.asFloat);
  //MpuTestLoop();


}
