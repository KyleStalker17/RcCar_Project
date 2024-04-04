//include libraries
#include <QMC5883L.h>//compass library
#include <RH_ASK.h> // radiohead library
#include <SPI.h> // Not actually used but needed to compile
#include <Wire.h> // I2C library used to communicate with the compass module

//define pinouts
#define gassing A0
#define braking A1
#define steering A2
#define leftPin 8
#define rightPin 7
#define Light 4
#define Key0 3
#define Key1 5
#define Key2 9
#define Key3 10
#define reversePin 3

#define addr 0x1E

int ID = 1;// the reciever will only execute instructions from the controller with the correct ID. ID is set using a physical key that flips switches.

// Movement variables
int gas = 0;
int steer = 0;
int brake = 0;
bool reverse = false;

//head position variables
float head = 0;
float heading = 0;
float headingDegrees = 0;
int x, y, z;//compass heading variable

//head/taillight control variables
bool light = false;
int leftcount = 0;
bool leftlight = 0;
int rightcount = 0;
bool rightlight = 0;
bool lighttemp = false;
int left = 0;
int right = 0;


//Key switch variables
bool k0 = false;
bool k1 = false;
bool k2 = false;
bool k3 = false;


//library variables
RH_ASK driver;
QMC5883L compass;

void setup()
{
  Serial.begin(9600);// For development only. This will be deleted in the final product
  if (!driver.init())
    Serial.println("init failed");
  //Set input pinmodes
  pinMode (Light, INPUT);
  pinMode (leftPin, INPUT);
  pinMode (rightPin, INPUT);
  pinMode (Key0, INPUT);
  pinMode (Key1, INPUT);
  pinMode (Key2, INPUT);
  pinMode (Key3, INPUT);
  Wire.begin();

  Wire.beginTransmission(addr);
  Wire.write(0x02);
  Wire.write(0x00);
  Wire.endTransmission();
  //compass.init();//start compass
}


void loop()
{
  uint8_t data[9]; //a variable to store all of the values which need to be sent
  //collect data
  data[0] = ID;
  data[1] = gas;
  data[2] = steer;
  data[3] = brake;
  data[4] = right;
  data[5] = left;
  data[6] = light;
  data[7] = headingDegrees;
  data[8] = reverse;

  //send data
  int datalen = sizeof(data);
  driver.send((uint8_t *)data, sizeof(data));
  driver.waitPacketSent();
  delay(10);

  //compass read. This has to be done using the I2C communication protocol, it's a bit more complex and so gets its own section
  int x, y, z;
  Wire.beginTransmission(addr);
  Wire.write(0x03);
  Wire.endTransmission();

  Wire.requestFrom(addr, 6);
  if (6 <= Wire.available()) {
    x = Wire.read() << 8; //MSB  x
    x |= Wire.read(); //LSB  x
    z = Wire.read() << 8; //MSB  z
    z |= Wire.read(); //LSB z
    y = Wire.read() << 8; //MSB y
    y |= Wire.read(); //LSB y
  }
  x = x * -1;
  heading = atan2(y, x);//the heading is the tangent between the x and y values.
  heading += 0.01126;// Account for local magnetic declination
  Serial.print(heading);
  Serial.print(",");
  if (heading < 0) {
    ;//account for cases where the heading is less than 1
    heading += 2 * PI;
  }
  if (heading > 2 * PI) {
    //account for if the heading is greater than 360 degrees
    heading -= 2 * PI;
  }
  headingDegrees = heading * 180 / M_PI;//convert to degrees
  if (headingDegrees < 270) {
    headingDegrees += 90;
  }
  else {
    headingDegrees -= 270;
  }

  Serial.print (heading);
  Serial.print(",");
  Serial.println (headingDegrees);
headingDegrees = (headingDegrees/1.5);

  /*
    head = (headingDegrees / 1.40625); //(1.40625 = 360 degrees /256). convert to a number small enough to be sent over


    //read all of the analog and digital input pins. A quirk with the radiohead library means that the values can't be larger than 256, so the analog values get divided by 10. We lose a little bit of resolution, but it's not a significant amount.
  */

  brake = analogRead(braking);
  brake = (brake / 10);
  gas = analogRead(gassing);
  gas = gas / 10;
  head = analogRead(steering);
  steer = steer / 10;
  left = digitalRead(leftPin);
  right = digitalRead(rightPin);
  light = digitalRead(Light);
  reverse = digitalRead(reversePin);


  //set the ID from the key.
  k0 = digitalRead(Key0);
  k1 = digitalRead(Key1);
  k2 = digitalRead(Key2);
  k3 = digitalRead(Key3);
  ID = ((k0 * 1) + (k1 * 2) + (k2 * 4) + (k3 * 8));//The pins on the key are a number encoded in binary. This will turn the number into decimal.
  ID = 1;
}
