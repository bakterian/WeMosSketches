# AudioPlayer
This is a ardruino projectof a portable speaker.
It is planned to use a WiFi Server for music control
As for the source uploaded mp3s or a internet radio can be used.

### Hardware List:
  - Esp 8266 Wemos D1 Mini Board
  - LiPo battery holder with a battery charing cicruit
  - Adafruit I2S 3W Class D Amplifier Breakout MAX98357A
  - 3W 4Ohm Audio Speaker
  - SD Card Holder with SD-card a 4Gb model was used
  - LED's for battery charing status indication
  - USB connectors
  - Plastic Box
  - On/Off Switch
  - Cables and a Uniwersal PCB Board

### Prerequisties
For building the application either the Arduino IDE or a Makefile approach can be used.
When chosing the latter please download or clone this arduino esp makefile builder which is referenced from the project.
    [Esp8266-Arduino-Makefile](https://github.com/bakterian/ESP8266Audio/releases/tag/RL_001.000.000)
After the download is done please refer to a dedicated readme and trigger the esp-sdk core source and libraries.

### Installation
It is recommended to use released version or just download/clone the master.
For installing the AudioPlayer just execute the installLibraries bash script.
This can be done either from linux or cygwin like so:
```bassh
$ bash installLibraries.bash
```
Please note that without downloading and installing Esp8266-Arduino-Makefile (with default repo-folder naming) the installation scripts will fail.

### Building
When both the AudioPlayer and Arduino-Makefile are configured just run make to build the application.
```bassh
$ make -j8
```

Software updates can also performed using this make file,
```bassh
$ make upload
```

### Development
When doing development and adding new librarry depencies please bear in mind to modify the libraries.json files.