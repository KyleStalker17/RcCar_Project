
//include libraries
#include <ServoTimer2.h>
#include <RH_ASK.h>
#include <SPI.h> // Not actualy used but needed to compile


//define LED output pins
#define LIGHTS_FL 3
#define LIGHTS_FR 2
#define LIGHTS_BL 7
#define LIGHTS_BR 4



//define variables for the Radiohead library
RH_ASK driver;



//define variables for the servo. The timer 2 library must be used because timer 1 is being used by the RF reciever to recieve data.
ServoTimer2 steering;
ServoTimer2 Motor;
ServoTimer2 hangle;




//Define motion control sensitivites
int accel = 15;
int bpressure = 6;




//define head and tailight variables
int Fleft = 0; // This controls the Front Left light, Fright is Front Right, etc...
int Fright = 0;
int Bleft = 0;
int Bright = 0;
bool lights = 0;
bool left = false;
bool right = false;
int leftcount = 0;
int rightcount = 0;



//this ID determines which controller is used to control the car
int ID = 1;

//Driving control variables
int angle = 1600;
int throttle = 1500;
int head = 0;
int Brake = 0;
int Throttle_temp = 0;
bool reverse = false;





void setup()
{
  Serial.begin(9600);
  //define the pins to control the servos
  steering.attach(6);
  Motor.attach(5);
  hangle.attach(8);
  //set light pin mode
  pinMode(LIGHTS_FR, OUTPUT);
  pinMode(LIGHTS_FL, OUTPUT);
  pinMode(LIGHTS_BR, OUTPUT);
  pinMode(LIGHTS_BL, OUTPUT);
}

void loop()
{


  
  uint8_t data[7];//create a variable to put the data in
  uint8_t datalen = sizeof(data);//create a varibale to get the size of the data
  if (driver.recv(data, &datalen)) // If we receive a transmission of the correct length,
  {
    if (data[0] == ID) {//check ID. If the ID is wrong, the message came from a different controller and is ignored.
      
      
      
      
      
      //Set variables based on the recieved data
      angle = (data[2]);
      Throttle_temp = (data[1]);//The throttle needs to look at the throttle from the previous iteration and the new data, so this will just be a variable in the throttle formula
      left = data[5];
      right = data [4];
      Brake = data[3];
      lights = data[6];
      head = data[7];
      reverse = data[8];


      
      
      //brake
      Brake = (Brake / bpressure);//this number determines brake sensitivity
      if (Brake < 4) { //make sure the brake is always at least a little bit on so the car will slowly come to a stop when the throttle is released
        Brake = 5;
      }




      //steering
      angle = ((angle * 16) + 700);//math necessary to make the steering angle behave properly. The values for the servos need to be between 700 and 2200, with 1500 being the center position. This is a quirk of the Timer2 library.
      steering.write(angle);//send the angle data to the steering servo




      //throttle
      Throttle_temp = ((Throttle_temp * 8) + 1500); //math to set throttle properly
      if (reverse == 0){//if the car is in forward
      if (throttle < Throttle_temp) { //if the car is speeding up,
          throttle += accel;//this determines throttle sensitivity
        }
      }
      else { //if the car is in reverse
        Throttle_temp = (Throttle_temp * -1);
        if (throttle > Throttle_temp) {
          throttle -=5;
      }
    }
    throttle = throttle - (Brake); //The brake will always be a little bit applied, so the car will slow to a stop without throttle
      if ((throttle < 1500)&&(reverse == 0)) { //make sure we're not going backwards
        throttle = 1500;
      }
      Motor.write(throttle);//Send the throttle data to the driving servo





      //headlights, tailights, and turn signals
      if (Brake > 6) { //turn on the brake lights if the brake is pressed enough
        Bleft = 1;
        Bright = 1;
      }
      else {
        Bleft = 0;
        Bright = 0;
      }
      if (lights == 1) { //turn on the headlights
        Fleft = 1;
        Fright = 1;
      }
      else {
        (Fleft = 0);
        (Fright = 0);
      }

      if (left == 1) { //If the left turn signal is on, start flipping the left lights on and off. If the brake and/or headlights are on, they will be ignored and this will happen instead.
        if (leftcount < 5) {
          Fleft = 1;
          Bleft = 1;
          leftcount += 1;
        }
        else {
          Bleft = 0;
          Fleft = 0;
          leftcount = leftcount + 1;
          if (leftcount >= 10) {
            leftcount = 0;
          }
        }
      }
      else {
        leftcount = 0;
      }
      if (right == 1) { //if the right signal is on, start flipping the right lights on and off.
        if (rightcount < 5) {
          Fright = 1;
          Bright = 1;
          rightcount = rightcount + 1;
        }
        else {
          Fright = 0;
          Bright = 0;
          rightcount = rightcount + 1;
          if (rightcount >= 10) {
            rightcount = 0;
          }
        }
      }
      else {
        rightcount = 0;
      }

      //Send light data to the light pins
      digitalWrite(LIGHTS_BL, Bleft);
      digitalWrite(LIGHTS_BR, Bright);
      digitalWrite(LIGHTS_FL, Fleft);
      digitalWrite(LIGHTS_FR, Fright);



      
      //camera rotation servo control
      head = (((head * 2) * (1500 / 360)) + 700); //math for correct servo value
      hangle.write(head);//send the data to the camera servo





      
      //all the rest is just for development purposes and will be deleted/commented out in the final version
      Serial.print (data[0]);
      Serial.print(",");
      Serial.print(data[1]);
      Serial.print(",");
      Serial.print( data[2]);
      Serial.print(",");
      Serial.print(data[3]);
      Serial.print(",");
      Serial.print(data[4]);
      Serial.print(",");
      Serial.print(data[5]);
      Serial.print(",");
      Serial.print(data[6]);
      Serial.print("|     |");
      Serial.print (throttle);
      Serial.print(",");
      Serial.print(angle);
      Serial.print(",");
      Serial.print(Brake);
      Serial.print(",");
      Serial.print(Fright);
      Serial.print(",");
      Serial.print(Fleft);
      Serial.print(",");
      Serial.print(Bright);
      Serial.print(","),
      Serial.print(Bleft);
      Serial.print(",");
      Serial.print(Throttle_temp);
      Serial.print(",");
      Serial.print(head);
      Serial.print(",");
      Serial.println(reverse);
    };
  }
}
