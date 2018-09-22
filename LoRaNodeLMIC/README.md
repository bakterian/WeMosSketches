LMiC 1.5 port to ESP8266
================================================
The sketch NEEDS the lmic-2.5 libraries in the libraries directory. 
Make sure you download these to your system as well.


Hardware Configuration ESP8266
-------------
A minimialistic setup was used for the ESP8266 Wemos D1 due to the limited number of usebale GPIOs.
Both the Reset and DIO3, DIO4, DIO5 pins of the RFMW95 are not connected.
Making space for the I2C interface BMP085 Temperature and Humidity Sensor.
