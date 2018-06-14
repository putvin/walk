# walk
Arduino code to log data from five IMUs (number of IMUs can be altered). Used to monitor gait. This code was originally written for an Arduino Uno but could be modified to work on other devices. 

LIBRARIES NEEDED:

I2Cdev and MPU6050 libraries are maintained by Jeff Rowberg
https://github.com/jrowberg/i2cdevlib/tree/master/Arduino

SDFat library is maintained by Bill Greiman
https://github.com/greiman/SdFat

TimerOne library is maintained by Paul Stoffregen
https://github.com/PaulStoffregen/TimerOne

POST-PROCESSING:

Python code posted in this repository can be used to import, process, and classify the data.

After the IMU data is collected, it can be converted 
to position data and visualized using xIMU
https://github.com/xioTechnologies/Gait-Tracking-With-x-IMU
