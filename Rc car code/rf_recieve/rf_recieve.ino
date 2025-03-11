#include <ServoTimer2.h>
#include <RH_ASK.h>
#include <SPI.h>  // Not actualy used but needed to compile

ServoTimer2 Wheel;
ServoTimer2 Cam;
ServoTimer2 Drive;

#define Fright 13
#define Fleft 3
#define Bright 4
#define Bleft 2
#define Drive 5

//Determine pedal sensitivity
int acc = 5;
int bpress = 10;

//Determine which controller it listens to
int ID = 1;

//movement variables
int gas_temp = 0;
int gas = 0;
float brake = 0;
int steer = 0;
int head = 1200;
bool reverse = false;

// headlight/taillight/brake light variables
int leftcounter = 0;
int rightcounter = 0;
bool light = false;
bool Brakelights = false;
bool left = false;
bool right = false;
bool FleftOn = false;
bool FrightOn = false;
bool BleftOn = false;
bool BrightOn = false;


//library variables
RH_ASK driver;


void setup() {
  Serial.begin(9600);  // Development only
  if (!driver.init())
    Serial.println("init failed");
  //set LED pinmodes
  pinMode(Fright, OUTPUT);
  pinMode(Fleft, OUTPUT);
  pinMode(Bright, OUTPUT);
  pinMode(Bleft, OUTPUT);

  //attach the servos to the correct pins
  Wheel.attach(7);
  Cam.attach(8);
}

void loop() {
  uint8_t buf[9];                 //variable to store the incoming data
  uint8_t buflen = sizeof(buf);   //variable to store incoming data length
  if (driver.recv(buf, &buflen))  //If we get a message the is the right length,
  {
    int i;
    if (buf[0] = ID)  //If this came from the correct controller,
    {
      //Set all variables from recieved data
      gas_temp = buf[1];
      steer = buf[2];
      brake = buf[3];
      right = buf[4];
      left = buf[5];
      light = buf[6];
      head = buf[7];
      reverse = buf[8];

      // Steering
      steer = ((steer * 15) + 700);  //Make sure it's the right value for the servo. This servo library requires values between 700 and 2200, with 1500 being the middle.
      Wheel.write(steer);

      // forward/backwards Motion
      Serial.print("Gas_temp now");
      Serial.print(gas_temp);
      gas_temp = (gas_temp * 5);                     //Make sure it's the right value for the motor.
      if ((gas < gas_temp) && (reverse == false)) {  //if the current speed is less than the desired speed and the car is in forward gear, speed up a little. This is to make sure the car can't accelerate instantly and has a sense of mass.
        gas += acc;
      }
      if ((gas < gas_temp) && (reverse == true)) {  //If the car is in reverse gear, speed up backwards a little more each loop until we reach the desired speed.
        gas -= acc;
      }

      if (gas > 250) {  //Make sure the throttle number can't keep going up once the motor has reached it's maximum speed
        gas = 250;
      }
      brake = (brake / bpress);  //determine brake sensitivity
      if (brake < 2) {           //make sure there's always a little bit of braking so that the car slows down like a real one would
        brake = 2;
      }
      if ((10 < gas) && (150 > gas) && (gas_temp < 15)) {
        brake = 30;
      }
      if ((10 < gas) && (150 > gas)) {
        acc = 30;

      } else {
        acc = 5;
      }
      if (gas > 0) {  //If the car is moving forward, move the throttle value down by however much the brake is applied.
        gas = (gas - brake);
      }
      //else {
      //  gas = (gas + brake);  //If the car is in reverse gear, move the throttle value up by however much the brake is applied.
      //}
      if ((gas < 0) && (reverse == false)) {
        gas = 0;
      }

      analogWrite(Drive, gas);  //send instructions to the drive motor

      Serial.print("Gas_Temp : ");
      Serial.print(gas_temp);
      Serial.print(", ");
      Serial.print("Gas : ");
      Serial.print(gas);
      Serial.print(" , ");
      Serial.print("Brake = ");
      Serial.print(brake);
      Serial.print(",");
      Serial.print("Light = ");
      Serial.println(BrightOn);


      head = (head * 6.491);  //math for servo value
      head = (head + 700);    //make sure the minimum value is 700, or all the way left.
      Cam.write(head);        //send to camera servo
      if (light == false) {   //If the headlights need to be on, tell the headlights to turn on. This action won't actually be taken until all the light data is collected because we still need to check if the turn signal is on or off.
        FrightOn = true;
        FleftOn = true;
      } else {
        FrightOn = false;  //If the headlights have been turned off, tell the LEDs to shut off
        FleftOn = false;
      }

      if (buf[3] > 5) {  //If the brake has been applied, turn on the brake lights
        BrightOn = true;
        BleftOn = true;
      } else {
        BrightOn = false;  //If the brakes have been let off, turn off the brake lights.
        BleftOn = false;
      }

      if (left == true) {  //If the left turn signal is on, flip the left side lights on and off. If the headlights or brake lights are on, ignore them on this side and do this instead.
        leftcounter += 1;  // Add one to the counter. If it's less than five, turn the lights on.
        if (leftcounter < 5) {
          FleftOn = true;
          BleftOn = true;
        } else {  //If the counter is more than five, turn the lights off.
          FleftOn = false;
          BleftOn = false;
        }
        if (leftcounter >= 10) {  //If the counter is ten or more, set it back to zero.
          leftcounter = 0;
        }
      } else {
        leftcounter = 0;  //If the turn signal goes off, reset the counter at zero for next time. This way it will always flash first, and won't pick up where it left off.
      }


      if (right == true) {  //This is the same as the left side, except all of the variables correspond to the right side.
        rightcounter += 1;
        if (rightcounter < 5) {
          FrightOn = true;
          BrightOn = true;
        } else {
          FrightOn = false;
          BrightOn = false;
        }
        if (rightcounter >= 10) {
          rightcounter = 0;
        }
      } else {
        rightcounter = 0;
      }

      //Finally, send the instructions to the lights.
      digitalWrite(Fright, FrightOn);
      digitalWrite(Bright, BrightOn);
      digitalWrite(Fleft, FleftOn);
      digitalWrite(Bleft, BleftOn);
    }
  }
}
