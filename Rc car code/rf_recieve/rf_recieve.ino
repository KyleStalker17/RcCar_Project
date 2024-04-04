#include <ServoTimer2.h>
#include <RH_ASK.h>
#include <SPI.h> // Not actualy used but needed to compile

ServoTimer2 Motor;
ServoTimer2 Wheel;
ServoTimer2 Cam;

#define Fright 2
#define Fleft 3
#define Bright 4
#define Bleft 5

int acc = 20;
int bpress = 2;

int ID = 1;
int gas_temp = 0;
int gas = 1500;
float brake = 0;
int steer = 0;
int head = 0;
bool reverse = false;

int leftcounter = 0;
int rightcounter = 0;
bool light = false;
bool Brakelights = false;
bool left = false;
bool right = false;

RH_ASK driver;

void setup()
{
  Serial.begin(9600);  // Debugging only
  if (!driver.init())
    Serial.println("init failed");

  pinMode (Fright, OUTPUT);
  pinMode (Fleft, OUTPUT);
  pinMode (Bright, OUTPUT);
  pinMode (Bleft, OUTPUT);
  Motor.attach (6);
  Wheel.attach (7);
  Cam.attach (8);

}

void loop()
{
  uint8_t buf[9];
  uint8_t buflen = sizeof(buf);
  if (driver.recv(buf, &buflen))
  {
    int i;
    // Message with a good checksum received, dump it.

    if (buf[0] = ID)
    {
      gas_temp = buf[1];
      steer = buf[2];
      brake = buf[3];
      right = buf[4];
      left = buf[5];
      light = buf[6];
      head = buf[7];
      reverse = buf[9];
      /*      Serial.print(buf[0]);
            Serial.print(",");
            Serial.print(buf[1]);
            Serial.print(",");
            Serial.print(buf[2]);
            Serial.print(",");
            Serial.print(buf[3]);
            Serial.print(",");
            Serial.print(buf[4]);
            Serial.print(",");
            Serial.print(buf[5]);
            Serial.print(",");
            Serial.print(buf[6]);
            Serial.print(",");
            Serial.print(buf[7]);
            Serial.print(",");
            Serial.print(buf[8]);
            Serial.print(",");
            Serial.println(buf[9]);
      */

      steer = ((steer * 15) + 700);
      Wheel.write(steer);


      gas_temp = ((gas_temp * 15) + 700);
      if (gas < gas_temp) {
        gas += acc;
      }
      if (gas > 2200) {
        gas = 2200;
      }

      brake = (brake / bpress);
      if (brake < 5) {
        brake = 5;
      }
      gas = (gas - brake);
      if (gas < 1500) {
        gas = 1500;
      }
      Motor.write(gas);
      
      head = (head*6.491);
      head = (head + 700);
      Serial.print(head);
      Serial.print(",");
      Serial.println(buf[7]);
      Cam.write(head);

      if (light == true) {
        digitalWrite(Fright, HIGH);
        digitalWrite(Fleft, HIGH);
      }
      else {
        digitalWrite(Fright, LOW);
        digitalWrite(Fleft, LOW);
      }

      if (buf[3] > 5) {
        digitalWrite(Bright, HIGH);
        digitalWrite(Bleft, HIGH);
      }
      else {
        digitalWrite(Bright, LOW);
        digitalWrite(Bleft, LOW);
      }

      if (left == true) {
        leftcounter += 1;
        if (leftcounter < 5) {
          digitalWrite(Fleft, HIGH);
          digitalWrite(Bleft, HIGH);
        }
        else {
          digitalWrite(Fleft, LOW);
          digitalWrite(Bleft, LOW);
        }
        if (leftcounter >= 10) {
          leftcounter = 0;
        }
      }
      else {
        leftcounter = 0;
      }
      if (right == true) {
        rightcounter += 1;
        if (rightcounter < 5) {
          digitalWrite(Fright, HIGH);
          digitalWrite(Bright, HIGH);
        }
        else {
          digitalWrite(Fright, LOW);
          digitalWrite(Bright, LOW);
        }
        if (rightcounter >= 10) {
          rightcounter = 0;
        }
      }
      else {
        rightcounter = 0;
      }
    }
  }
}
