/*
 * Atlas One onboard software version 0.6.1 (compatible with terminal 0.5.1) 
 * Created by Marcin Łojek on 21.01.2021
 * 
 * This version can detect complex gests listed in the spreadsheet or in
 * the comments in this file.
 * 
 * The X axis goes from the front to the back, while the Y axis goes from the left to the right.
 * The PITCH angle is the angle between the flat surface and the X axis, and the ROLL is the angle
 * of Y axis roll around the X axis.
 * 
 * Serial parameters: baudrate 115200, 8 data bits, 1 stop bit, no parity.
 */

#define redLed 3
#define greenLed 5
#define blueLed 6
#define statusLed 13

#include <Wire.h>

//Device variables:
bool      deviceStatus = true;                                      //False if device is in sleep mode
uint64_t  loopStart, loopStop;                                      //Variables to measure loop time

struct MPU  {
  //This struct handles MPU-6050 and stores its data.
  
  const PROGMEM uint8_t   gyroScale = 3;                            //[0;3] Scale = +-(2^GYRO_SCALE)*250 DEG/SEC
  const PROGMEM uint8_t   accRange = 3;                             //[0;3] Range = +-(2^ACC_RANGE)*2 G
  const PROGMEM uint8_t   gyroFactor = 131 / pow(2, gyroScale);     //Gyro sensitivity factor, LSB/DEG
  const PROGMEM uint16_t  accFactor = 16384 / pow(2, accRange);     //Accelerometer sensitivity factor, LSB/G
  const PROGMEM uint8_t   sampleRateDiv = 19;                       //[0;255]Freqency = 1KHz/(1+SAMPLE_RATE)
  const PROGMEM uint8_t   DLPF_CFG = 1;                             //[0;8] Digital low pass filter, if =0 or =8 Frequency=8KHz/(1+SAMPLE_RATE)
  
  uint8_t   address;                                                //I2C address (0x68 or 0x69)
  float     gyroX, gyroY, gyroZ;                                    //Gyro variables (angular velocities)
  float     accX, accY, accZ;                                       //Accelerometer variables (forces in Gs)
  int16_t   gXOffset, gYOffset, gZOffset;                           //Raw gyro offset
  int16_t   aXOffset, aYOffset, aZOffset;                           //Raw acc offset
  float     stability;                                              //Sqrt of the sum of squared accelerometer forces
  float     pitch, roll;                                            //Pitch and roll position in degrees

  void start(uint8_t addr){
    //This function sets up an MPU.
    address = addr;
    Wire.beginTransmission(address);
    Wire.write(0x6B);                 //Access Power Management Register
    Wire.write(0x00);                 //Disable Sleep Mode
    Wire.endTransmission();
    Wire.beginTransmission(address);
    Wire.write(0x1B);                 //Access Gyroscope config register
    Wire.write(gyroScale << 3);       //Set Gyroscope scale
    Wire.endTransmission();
    Wire.beginTransmission(address);
    Wire.write(0x1C);                 //Access Accelerometer config register
    Wire.write(accRange << 3);        //Set Accelerometer range
    Wire.endTransmission();
    Wire.beginTransmission(address);
    Wire.write(0x19);                 //Access Sample Rate register
    Wire.write(sampleRateDiv);        //Set Sample rate divider
    Wire.endTransmission();
    Wire.beginTransmission(address);
    Wire.write(0x1A);                 //Access Digital Low Pass Filter config register
    Wire.write(DLPF_CFG);             //Set DLPF config value
    Wire.endTransmission();
  }
  void offset() {
    /*  This function measures the factory offset of the gyro and accelerometer. 
     *  It's very important to initialize the device in perfectly flat and still position.
     */
    int16_t offset[6];
    for (uint8_t i = 0; i < 6; i++)  {
      offset[i] = 0;
    }
    
    for (uint8_t i = 0; i < 10; i++) {
      Wire.beginTransmission(address);
      Wire.write(0x3B);
      Wire.endTransmission();
      Wire.requestFrom(address, 6);
      offset[0] += Wire.read() << 8 | Wire.read();
      offset[1] += Wire.read() << 8 | Wire.read();
      offset[2] += Wire.read() << 8 | Wire.read();
      offset[2] -= accFactor * 0.98;              //Acc raw sensitivity for 1G (0.98m/s^2)
      Wire.beginTransmission(address);
      Wire.write(0x43);
      Wire.endTransmission();
      Wire.requestFrom(address, 6);
      offset[3] += Wire.read() << 8 | Wire.read();
      offset[4] += Wire.read() << 8 | Wire.read();
      offset[5] += Wire.read() << 8 | Wire.read();
    }
    
    for (int i = 0; i < 6; i++) {
      offset[i] /= 10;
    }
  
    aXOffset = offset[0];
    aYOffset = offset[1];
    aZOffset = offset[2];
    gXOffset = offset[3];
    gYOffset = offset[4];
    gZOffset = offset[5];
  }
  void fetchGyro()  {
    /*  This function fetches raw gyro data from MPU registers
     *  and puts it in gyroXYZ variables. Also subtracts the offsets.
     */
    Wire.beginTransmission(address);
    Wire.write(0x43);
    Wire.endTransmission();
    Wire.requestFrom(address, 6);
    gyroX = Wire.read() << 8 | Wire.read();
    gyroY = Wire.read() << 8 | Wire.read();
    gyroZ = Wire.read() << 8 | Wire.read();
    gyroX -= gXOffset;
    gyroY -= gYOffset;
    gyroZ -= gZOffset;
  }
  void fetchAcc()   {
    /*  This function fetches raw accelerometer data from registers
     *  and puts it in accXYZ variables. Also subtracts the offsets.
     */
    Wire.beginTransmission(address);
    Wire.write(0x3B);
    Wire.endTransmission();
    Wire.requestFrom(address, 6);
    accX = Wire.read() << 8 | Wire.read();
    accY = Wire.read() << 8 | Wire.read();
    accZ = Wire.read() << 8 | Wire.read();
    accX -= aXOffset;
    accY -= aYOffset;
    accZ -= aZOffset;
  }
  void processGyroData()  {
    /*  This function calculates gyro positions when the device is not in
     *  stable position. It also calculates angular velocities.
     */
    pitch += gyroY / (gyroFactor * 50);
    roll -= gyroX / (gyroFactor * 50);
    
    //angular velocities:
    gyroX /= gyroFactor;
    gyroY /= gyroFactor;
    gyroZ /= gyroFactor;
  }
  void processAccData()   {
    //This function calculates accelerometer forces in Gs and stability.
    accX /= accFactor;
    accY /= accFactor;
    accZ /= accFactor;
  
    stability = sqrt(sq(accX) + sq(accY) + sq(accZ));
  }
  void autocalibrate()  {
    /*  If the device is in a stable position, the gyro position is measured using
     *  accelerometer data. Some basic trygonomery is done here.
     */
    if (abs(stability - 0.98) <= 0.05)  {
      float alpha, beta;                                                //Sines of pitch and roll angles respectively
  
      alpha = -accX / stability;                                        //Calculating the sine of pitch angle
      beta = -accY / sqrt(sq(accY) + sq(accZ));                         //Calculating the sine of roll angle
      pitch = asin(alpha) * RAD_TO_DEG;                                 //Calculating the pitch angle
      roll = asin(beta) * RAD_TO_DEG;                                   //Calculating the roll angle
  
      /*  The sine is very "flat" near 90 and -90 degrees, so the precision is very low
       *  near those values. To increase the precision we use cosine function, which is
       *  way steeper.
       */
  
      if (abs(pitch) > 45) {
        alpha = sqrt(1 - sq(alpha));
        pitch = acos(alpha) * RAD_TO_DEG;
      }
      if (abs(roll) > 45) {
        beta = sqrt(1 - sq(beta));
        roll = acos(beta) * RAD_TO_DEG;
      }
  
      /*  The cosine has the same values for (-90,-45) and (45,90), so we have to correct
       *  the sign of the angle manually.
       */
  
      if (accX > 0 && pitch > 0) {
        pitch *= -1;
      }
      if (accY > 0 && roll > 0) {
        roll *= -1;
      }
  
      //We have to recalculate the ROLL angle when the device is flipped over.
      
      if (accZ < 0)  {
        if (accY < 0) roll = 180 - roll;
        else  roll = -180 - roll;
      }
    }
  }
  void printPosition()  {
    //Prints the PITCH and ROLL angles.
    Serial.print("PITCH:");
    Serial.print(pitch);
    Serial.print("\t");
    Serial.print("ROLL:");
    Serial.println(roll);
  }
};
struct Log  {
  //This struct handles gest log and any operations done on it
  
  uint8_t list[50];               //Last 50 (1 sec.) simple gests, 0 is the freshest

  Log() {
    for (int i=0;i<50;i++)  {
      list[i] = 0;
    }
  }
  bool inLog(uint8_t what,uint8_t when) {
    /*  This function checks if given gest ID "what" is in last "when" cycles.
     */
    for (int i=0;i<when;i++)
    {
      if(list[i]==what)  return true;
    }
    return false;
  }
  void add(uint8_t id)  {
    /*  This function puts given sample in the array and shifts other samples by one place
     */

    for (int i=49;i>0;i--)  {
      list[i]=list[i-1];
    }
    list[0]=id;
  }
  bool awakeEnough()    {
    /*  This function returns true if the device is awake for 1 second or longer.
     */
     
    for(int i=0;i<50;i++) {
      if(list[i]==0)  return false;
    }
    return true;
  }
};
struct GestData {
  //A container for gests data

  String  stamp;                                          //6 char Gest stamp
  uint8_t simpleGest;                                     //Current Simple Gest ID
  char    complexGest;                                    //Current Complex Gest ID
  char    lastComplex;                                    //Previous Complex Gest ID
  char    mode;                                           //Current Gest Mode

  const PROGMEM char  maxMode = 'C';                            //Maximum Gest Mode ID, so modes can loop

  GestData()  {
    stamp = "FXXXOX";
    simpleGest = 0;
    complexGest = '2';
    lastComplex = '2';
    mode = 'A';
  }
  void printComplex()  {
    Serial.print(mode);
    Serial.println(complexGest);
  }
};

MPU mpu[2];
Log gestLog;
GestData gests;

void setup() {
  Serial.begin(115200);
  mpu[0].start(0x68);
  mpu[1].start(0x69);
  mpu[0].offset();
  mpu[1].offset();

  //Compatibility check:
  Serial.print("AW051");
  while (Serial.available() == 0) {}
  char key = Serial.read();
  if (key != 'K') {
    while (true)  {
      digitalWrite(statusLed, HIGH);
      delay(500);
      digitalWrite(statusLed, LOW);
      delay(500);
    }
  }
  digitalWrite(statusLed, HIGH);
  RGB(0, 255, 0);
}
void loop() {
  loopStart = micros();

  mpu[0].fetchGyro();
  mpu[1].fetchGyro();
  mpu[0].fetchAcc();
  mpu[1].fetchAcc();
  
  mpu[0].processGyroData();
  mpu[1].processGyroData();
  mpu[0].processAccData();
  mpu[1].processAccData();
  mpu[0].autocalibrate();
  mpu[1].autocalibrate();

  detectPrimary();
  detectSimple();
  detectComplex();
  
  gests.printComplex();

  modeLed(gests.mode);
  
  loopStop = micros();
  delayMicroseconds(20000 - loopStop + loopStart);
}

//Gests detection:
void detectPrimary()  {  
  /*  This function processes the data from MPUS, detects primary
   *  gests and puts them in the gests.stamp string.
   */

  int16_t scroll = mpu[1].gyroY - mpu[0].gyroY;
  int16_t swipe = mpu[1].gyroZ - mpu[0].gyroZ;
   
  //Arm position:
  if (mpu[0].pitch < -45) gests.stamp[0] = 'D';
  else if (mpu[0].pitch > 30) gests.stamp[0] = 'U';
  else  {
    if (mpu[0].roll <= 45 && mpu[0].roll >= -45) gests.stamp[0] = 'F';
    else if (mpu[0].roll > -135 && mpu[0].roll < -45) gests.stamp[0] = 'L';
    else if (mpu[0].roll > 45 && mpu[0].roll < 135) gests.stamp[0] = 'R';
    else gests.stamp[0] = 'O';
  }
  //Arm roll:
  if (abs(mpu[0].gyroX) < 600) gests.stamp[1] = 'X';
  else if (mpu[0].gyroX <= -600) gests.stamp[1] = 'R';
  else if (mpu[0].gyroX >= 600) gests.stamp[1] = 'L';
  //Hand scroll:
  if (abs(scroll) < 800) gests.stamp[2] = 'X';
  else if (scroll >= 800) gests.stamp[2] = 'U';
  else if (scroll <= -800) gests.stamp[2] = 'D';
  //Hand swipe:
  if (abs(swipe) < 400) gests.stamp[3] = 'X';
  else if (swipe <= -400) gests.stamp[3] = 'R';
  else if (swipe >= 400) gests.stamp[3] = 'L';
  //Hand position:
  if (mpu[1].accZ < -0.5) gests.stamp[4] = 'C';
  else gests.stamp[4] = 'O';
  //Pull:
  if (mpu[0].accX + sin(mpu[0].pitch * DEG_TO_RAD) <= -1) gests.stamp[5] = 'P';
  else gests.stamp[5] = 'X';
}
void detectSimple()   {  
  //This function detects simple gests listed in the spreadsheet.

  byte xCount = 0;                                 //No. of X-s in roll, scroll and swipe
  if (gests.stamp[1] == 'X')  xCount++;
  if (gests.stamp[2] == 'X')  xCount++;
  if (gests.stamp[3] == 'X')  xCount++;

  //if device inactive simplegest = 0
  if (gests.stamp[5] == 'P')  gests.simpleGest = 1;                   //Pull
  else if (gests.stamp[0] == 'D')   gests.simpleGest = 0;             //Inactive
  else if (xCount < 2)  gests.simpleGest = 255;                       //Ambiguous
  else if (xCount == 3) gests.simpleGest = 2;                         //Static
  else if (gests.stamp[0] == 'F') {
    if (gests.stamp[1] == 'L')  gests.simpleGest = 3;                 //Roll left
    else if (gests.stamp[1] == 'R') gests.simpleGest = 4;             //Roll right
    else if (gests.stamp[2] == 'D') gests.simpleGest = 5;             //Scroll down
    else if (gests.stamp[2] == 'U') gests.simpleGest = 6;             //Scroll up
    else if (gests.stamp[3] == 'L') {
      if (gests.stamp[4] == 'O') gests.simpleGest = 7;                //Swipe left
      else if (gests.stamp[4] == 'C')  gests.simpleGest = 12;         //Fist swipe right
    }
    else if (gests.stamp[3] == 'R') {
      if (gests.stamp[4] == 'O') gests.simpleGest = 8;                //Swipe right
      else if (gests.stamp[4] == 'C')  gests.simpleGest = 11;         //Fist swipe left
    }
  }
  else if (gests.stamp[0] == 'L') {
    if (gests.stamp[1] == 'L')  gests.simpleGest = 3;                 //Roll left
    else if (gests.stamp[1] == 'R') gests.simpleGest = 4;             //Roll right
    else if (gests.stamp[2] == 'D') gests.simpleGest = 8;             //Swipe right
    else if (gests.stamp[2] == 'U') gests.simpleGest = 7;             //Swipe left
    else if (gests.stamp[3] == 'L') gests.simpleGest = 2;             //Unused
    else if (gests.stamp[3] == 'R') gests.simpleGest = 2;             //Unused
  }
  else if (gests.stamp[0] == 'R') {
    if (gests.stamp[1] == 'L')  gests.simpleGest = 3;                 //Roll left
    else if (gests.stamp[1] == 'R') gests.simpleGest = 4;             //Roll right
    else if (gests.stamp[2] == 'D') gests.simpleGest = 7;             //Swipe left
    else if (gests.stamp[2] == 'U') gests.simpleGest = 8;             //Swipe right
    else if (gests.stamp[3] == 'L') gests.simpleGest = 2;             //Unused
    else if (gests.stamp[3] == 'R') gests.simpleGest = 2;             //Unused
  }
  else if (gests.stamp[0] == 'U') {
    if (gests.stamp[1] == 'L')  gests.simpleGest = 9;                 //Raised roll left
    else if (gests.stamp[1] == 'R') gests.simpleGest = 10;            //Raised roll right
    else if (gests.stamp[2] == 'D') gests.simpleGest = 2;             //Unused
    else if (gests.stamp[2] == 'U') gests.simpleGest = 2;             //Unused
    else if (gests.stamp[3] == 'L') gests.simpleGest = 2;             //Unused
    else if (gests.stamp[3] == 'R') gests.simpleGest = 2;             //Unused
  }
  else if (gests.stamp[0] == 'O') {
    if (gests.stamp[1] == 'L')  gests.simpleGest = 3;                 //Roll left
    else if (gests.stamp[1] == 'R') gests.simpleGest = 4;             //Roll right
    else if (gests.stamp[2] == 'D') gests.simpleGest = 2;             //Unused
    else if (gests.stamp[2] == 'U') gests.simpleGest = 2;             //Unused
    else if (gests.stamp[3] == 'L') gests.simpleGest = 2;             //Unused
    else if (gests.stamp[3] == 'R') gests.simpleGest = 2;             //Unused
  }

  if(deviceStatus == false && gests.simpleGest !=9 && gests.simpleGest != 10) gests.simpleGest = 0; 
  //Device not active nor being woken up

  gestLog.add(gests.simpleGest);
}
void detectComplex()  {  
  //This function detects complex gests listed in the spreadsheet.

  gests.lastComplex = gests.complexGest;
  
  if (deviceStatus == true) {
    if (gests.simpleGest == 0 || gests.simpleGest == 1)  {
      gests.complexGest = '0';                                    //Sleep
      deviceStatus = false;
      digitalWrite(statusLed, LOW);
    }
    else if (gests.simpleGest == 2) {
      gests.complexGest = '2';                                    //Active, no movement
    }
    else if (gests.simpleGest == 3 && gestLog.inLog(4, 25) == false) {
      gests.complexGest = '3';                                    //Roll left
    }
    else if (gests.simpleGest == 4 && gestLog.inLog(3, 25) == false) {
      gests.complexGest = '4';                                    //Roll right
    }
    else if (gests.simpleGest == 5 && gestLog.inLog(6, 25) == false) {
      gests.complexGest = '5';                                    //Scroll down
    }
    else if (gests.simpleGest == 6 && gestLog.inLog(5, 25) == false) {
      gests.complexGest = '6';                                    //Scroll up
    }
    else if (gests.simpleGest == 7 && gestLog.inLog(8, 25) == false) {
      gests.complexGest = '7';                                    //Swipe left
    }
    else if (gests.simpleGest == 8 && gestLog.inLog(7, 25) == false) {
      gests.complexGest = '8';                                    //Swipe right
    }
    else if (gests.simpleGest == 9 && gestLog.inLog(10, 25) == false && gestLog.awakeEnough() == true) {
      gests.complexGest = '9';                                    //Mode-
      if (gests.complexGest != gests.lastComplex) {
        if (gests.mode == 'A')  gests.mode = gests.maxMode;
        else gests.mode--;
      }
    }
    else if (gests.simpleGest == 10 && gestLog.inLog(9, 25) == false && gestLog.awakeEnough() == true) {
      gests.complexGest = 'A';                                    //Mode+
      if (gests.complexGest != gests.lastComplex) {
        if (gests.mode == gests.maxMode)  gests.mode = 'A';
        else gests.mode++;
      }
    }
    else if (gests.simpleGest == 11 && gestLog.inLog(12, 25) == false) {
      gests.complexGest = 'B';                                    //Fist swipe left
    }
    else if (gests.simpleGest == 12 && gestLog.inLog(11, 25) == false) {
      gests.complexGest = 'C';                                    //Fist swipe right
    }
  }
  else if (deviceStatus == false) {
    if (gests.simpleGest == 9 && gestLog.inLog(10, 50) == true) {
      gests.complexGest = '1';                                    //Wake up
      deviceStatus = true;
      digitalWrite(statusLed, HIGH);
    }
    else if (gests.simpleGest == 10 && gestLog.inLog(9, 50) == true) {
      gests.complexGest = '1';                                    //Wake up
      deviceStatus = true;
      digitalWrite(statusLed, HIGH);
    }
  }
}
void RGB(uint8_t red,uint8_t green, uint8_t blue) {
  //This function lights the onboard RGB LED

  analogWrite(redLed, 255 - red);
  analogWrite(greenLed, 255 - green);
  analogWrite(blueLed, 255 - blue);
}
void modeLed(char mode) {
  //This function sets RGB color to current mode

  if(deviceStatus)  {
    if (mode == 'A')  RGB(255, 255, 255);
    else if (mode == 'B') RGB(0, 255, 0);
    else if (mode == 'C') RGB(255, 0, 0);
  }
  else RGB(0, 0, 0);
}
