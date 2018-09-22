The following diagram illustrates flash layout used in Arduino environment:

|--------------|-------|---------------|--|--|--|--|--|
^              ^       ^               ^     ^
Sketch    OTA update   File system   EEPROM  WiFi config (SDK)

WeMos D1 & D1 mini	Spi flash size: 4M, sffs size: 1M

More infos on SPIFFS:
https://github.com/pellepl/spiffs/wiki
https://github.com/esp8266/Arduino/blob/master/cores/esp8266/spiffs/README.md


This is how te output on the COM port monitor looks like:

########## SPI FLASH INFOS ###########
Flash real id:   001625C2
Flash real size: 4194304
Flash ide  size: 4194304
Flash ide speed: 40000000
Flash ide mode:  DIO
######################################

### SPI FLASH FILING SYSYTEM INFOS ###
SPIFFS.totalBytes = 957314
SPIFFS.usedBytes = 0
SPIFFS.blockSize = 8192
SPIFFS.pageSize = 256
SPIFFS.maxOpenFiles = 5
SPIFFS.maxPathLength = 32
######################################