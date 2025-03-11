//include libraries
#include <RH_ASK.h>  // radiohead library
#include <SPI.h>     // Not actually used but needed to compile
#include <Wire.h>    // I2C library used to communicate with the compass module

//define pinouts
#define gassing A3
#define braking A1
#define steering A2
#define leftPin 8
#define rightPin 7
#define Light 4
#define reversePin 3

#define addr 0x1E

int ID = 1;  // the reciever will only execute instructions from the controller with the correct ID.

// Movement variables
int gas = 0;
int steer = 0;
int brake = 0;
bool reverse = false;

//head position variables
float head = 0;
float heading = 0;
float headingDegrees = 0;
int x, y, z;  //compass heading variable

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

uint8_t data[9];  //a variable to store all of the values which need to be sent
int datalen = sizeof(data);


void setup() {
  Serial.begin(9600);  // For development only. This will be deleted in the final product
  if (!driver.init())
    Serial.println("init failed");
  //Set input pinmodes
  Serial.println("Start");
  pinMode(Light, INPUT);
  pinMode(leftPin, INPUT);
  pinMode(rightPin, INPUT);

  Wire.begin();
  Wire.beginTransmission(addr);
  Wire.write(0x02);
  Wire.write(0x00);
  Wire.endTransmission();
  Serial.println("Starting");
}


void loop() {
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
  datalen = sizeof(data);
  driver.send((uint8_t *)data, sizeof(data));
  driver.waitPacketSent();
  delay(1);
  //compass read. This has to be done using the I2C communication protocol, it's a bit more complex and so gets its own section
  int x, y, z;
  Wire.beginTransmission(addr);
  Wire.write(0x03);
  Wire.endTransmission();

  Wire.requestFrom(addr, 6);
  if (6 <= Wire.available()) {
    x = Wire.read() << 8;  //MSB  x
    x |= Wire.read();      //LSB  x
    z = Wire.read() << 8;  //MSB  z
    z |= Wire.read();      //LSB z
    y = Wire.read() << 8;  //MSB y
    y |= Wire.read();      //LSB y
  } else {
    Serial.println("Compass failed");
  }
  x = x * -1;
  heading = atan2(y, x);  //the heading is the tangent between the x and y values.
  heading += 0.01126;     // Account for local magnetic declination
  if (heading < 0) {
    ;  //account for cases where the heading is less than 1
    heading += 2 * PI;
  }
  if (heading > 2 * PI) {
    //account for if the heading is greater than 360 degrees
    heading -= 2 * PI;
  }
  headingDegrees = heading * 180 / M_PI;  //convert to degrees
  if (headingDegrees < 270) {
    headingDegrees += 90;
  } else {
    headingDegrees -= 270;
  }
  headingDegrees = (headingDegrees / 1.5);


  head = (headingDegrees / 1.40625);  //(1.40625 = 360 degrees /256). convert to a number small enough to be sent over


  //read all of the analog and digital input pins. A quirk with the radiohead library means that the values can't be larger than 256, so the analog values get divided by 10, then multiplied by 10 on the other end once they've been transmitted. We lose a little bit of resolution, but it's not a significant amount.
  brake = analogRead(braking);
  brake = (brake / 10);
  brake = (brake * 3.3);
  gas = analogRead(gassing);
  gas = gas / 10;
  gas = gas - 15;
  gas = gas * 2.6;
  
  steer = analogRead(steering);
  steer = steer / 10;
  left = digitalRead(leftPin);
  right = digitalRead(rightPin);
  light = digitalRead(Light);
  reverse = digitalRead(reversePin);


  ID = 1;
  Serial.print("gas = ");
  Serial.print(gas);
  Serial.print(",");
  Serial.print("left = ");
  Serial.print(left);
  Serial.print(",");
  Serial.print("right = ");
  Serial.println(right);
}
