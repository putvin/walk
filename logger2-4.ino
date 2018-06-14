/*Pin connections for SD card to Uno
  VCC=3.3V
  D0=D12
  CMD=D11
  CLK=D13
  D3=D10
  D2 (not connected)
  D1 (not connected)
  CD= this is the Card Detect pin. It shorts to ground when a card is
inserted. You should connect a pull up resistor (10K or so) and wire this to another pin if you
want to detect when a card is inserted.

Pin connections for GY-521 to Uno
  VCC=5V
  SCL=A5 (SCL on R3)
  SDA=A4 (SDA on R3)
  XDA
  XCL
  AD0=different digital out (4-8) for each IMU
  INT
*/

/*
I2Cdev and MPU6050 libraries are maintained by Jeff Rowberg
https://github.com/jrowberg/i2cdevlib/tree/master/Arduino

SDFat library is maintained by Bill Greiman
https://github.com/greiman/SdFat

TimerOne library is maintained by Paul Stoffregen
https://github.com/PaulStoffregen/TimerOne

After the IMU data is collected, it can be converted 
to position data and visualized using xIMU
https://github.com/xioTechnologies/Gait-Tracking-With-x-IMU
*/

#include <Wire.h>
#include <I2Cdev.h>
#include <MPU6050.h>
#include <SdFat.h>
#include <SdFatConfig.h>
#include <TimerOne.h>

const int sensors=5; // Number of IMUs to collect data from
/*microseconds between measurements
40hz=25000
30hz=33333*/
const unsigned long howlong=25000; 
const int imu[7]={4,5,7,6,8}; //Pins for controlling the address of each IMU
const char* filename="log.csv";
const char* header="D4gx, D4gy, D4gz, D4ax, D4ay, D4az, D5gx, D5gy, D5gz, D5ax, D5ay, D5az, D7gx, D7gy, D7gz, D7ax, D7ay, D7az, D6gx, D6gy, D6gz, D6ax, D6ay, D6az, D8gx, D8gy, D8gz, D8ax, D8ay, D8az, clock";
const uint8_t chipSelect = 10; // SD chip select pin
char buffer[230];
char _int2str[10];
volatile boolean run=LOW;

int16_t ax, ay, az;
int16_t gx, gy, gz;

// file system object
SdFat sd;

// create Serial stream
  ArduinoOutStream cout(Serial);

// store error strings in flash to save RAM
#define error(s) sd.errorHalt(PSTR(s))

// Default I2C address for the MPU-6050 is 0x68.
// But only if the AD0 pin is low.
// Some sensor boards have AD0 high, and the
// I2C address thus becomes 0x69.
MPU6050 mpu(0x69);

void setup()
{      
  int error;
  uint8_t c;
  int i;

  Serial.begin(19600);

  // Set address pins as outputs.
  for (i=0; i<sensors; i++) {
    pinMode(imu[i], OUTPUT);
    digitalWrite(imu[i], LOW);
  }
    
  // Initialize the 'Wire' class for the I2C-bus.
  Wire.begin();

  // initialize the SD card at SPI_HALF_SPEED to avoid bus errors with
  // breadboards.  use SPI_FULL_SPEED for better performance.
  if (!sd.begin(chipSelect, SPI_FULL_SPEED)) sd.initErrorHalt();

  //    MPU6050 default at power-up:
  //    Gyro at 250 degrees second
  //    Acceleration at 2g
  //    Clock source at internal 8MHz
  //    The device is in sleep mode
  for (i=0; i<sensors; i++) {
    digitalWrite(imu[i], HIGH);
    mpu.setSleepEnabled(false);
    mpu.setClockSource(MPU6050_CLOCK_PLL_XGYRO);
    mpu.setFullScaleGyroRange(MPU6050_GYRO_FS_1000);
    mpu.setFullScaleAccelRange(MPU6050_ACCEL_FS_8);
    digitalWrite(imu[i], LOW);
  }

// Write file header to log file to label columns.
ofstream sdout(filename, ios::out | ios::app);
    if (!sdout) error("open failed");
    sdout << header << endl;
    // close the stream
    sdout.close();
    
  //Initialize timer.
  Timer1.initialize(howlong);
  Timer1.attachInterrupt(measure, howlong);
}

void measure() {
  run=HIGH;
}

void loop()
{
  //Prevents measuring before the interrupt occurs
  while(run==LOW) {
 } 
 
 run=LOW;
    int n;
    //Saves the values from eash sensor to the buffer
    for (n=0; n<sensors; n++) {
      readimu(n);
    }
    //Adds the clock value to the buffer and writes it all to the SD card
    strcat(buffer, int2str((int)millis()));
    SDtransfer();
}

void readimu(int i)
{
  int error;
  digitalWrite(imu[i], HIGH);
  //Read the raw values from the MPU6050
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  //Add the values to the buffer
  strcat(buffer, int2str(gx));
  strcat(buffer, ",");
  strcat(buffer, int2str(gy));
  strcat(buffer, ",");
  strcat(buffer, int2str(gz));
  strcat(buffer, ",");
  strcat(buffer, int2str(ax));
  strcat(buffer, ",");
  strcat(buffer, int2str(ay));
  strcat(buffer, ",");
  strcat(buffer, int2str(az));
  strcat(buffer, ",");

  digitalWrite(imu[i], LOW);
}

void SDtransfer() {
    // open stream for append
    ofstream sdout(filename, ios::out | ios::app);
    if (!sdout) error("open failed");
    sdout << buffer << endl;
    // close the stream
    sdout.close();
    buffer[0] = '\0'; 
}  

char* int2str( register int i ) {
  //Optimized integer to string conversion
  //From http://stackoverflow.com/questions/7910339/how-to-convert-int-to-string-on-arduino
  register unsigned char L = 1;
  register char c;
  register boolean m = false;
  register char b;  // lower-byte of i
  // negative
  if ( i < 0 ) {
    _int2str[ 0 ] = '-';
    i = -i;
  }
  else L = 0;
  // ten-thousands
  if( i > 9999 ) {
    c = i < 20000 ? 1
      : i < 30000 ? 2
      : 3;
    _int2str[ L++ ] = c + 48;
    i -= c * 10000;
    m = true;
  }
  // thousands
  if( i > 999 ) {
    c = i < 5000
      ? ( i < 3000
          ? ( i < 2000 ? 1 : 2 )
          :   i < 4000 ? 3 : 4
        )
      : i < 8000
        ? ( i < 6000
            ? 5
            : i < 7000 ? 6 : 7
          )
        : i < 9000 ? 8 : 9;
    _int2str[ L++ ] = c + 48;
    i -= c * 1000;
    m = true;
  }
  else if( m ) _int2str[ L++ ] = '0';
  // hundreds
  if( i > 99 ) {
    c = i < 500
      ? ( i < 300
          ? ( i < 200 ? 1 : 2 )
          :   i < 400 ? 3 : 4
        )
      : i < 800
        ? ( i < 600
            ? 5
            : i < 700 ? 6 : 7
          )
        : i < 900 ? 8 : 9;
    _int2str[ L++ ] = c + 48;
    i -= c * 100;
    m = true;
  }
  else if( m ) _int2str[ L++ ] = '0';
  // decades (check on lower byte to optimize code)
  b = char( i );
  if( b > 9 ) {
    c = b < 50
      ? ( b < 30
          ? ( b < 20 ? 1 : 2 )
          :   b < 40 ? 3 : 4
        )
      : b < 80
        ? ( i < 60
            ? 5
            : i < 70 ? 6 : 7
          )
        : i < 90 ? 8 : 9;
    _int2str[ L++ ] = c + 48;
    b -= c * 10;
    m = true;
  }
  else if( m ) _int2str[ L++ ] = '0';
  // last digit
  _int2str[ L++ ] = b + 48;
  // null terminator
  _int2str[ L ] = 0;  
  return _int2str;
}
