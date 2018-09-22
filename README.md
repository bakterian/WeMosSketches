# WeMosSketches
Contains arduino sketches which are to be run on a ESP8266 Wemos D1 Mini.

# Pin Mapping
PIN WEMOS
TX	TXD
RX	RXD
A0	Analog Input (max 3.3V)
D0	IO: Sleep Wakeup GPIO16
D1	IO, SCL	GPIO5
D2	IO, SDA	GPIO4
D3	IO, 10k Pull-up	GPIO0
D4	IO, 10k Pull-up, BUILTIN_LED	GPIO2
D5	IO, SCK	GPIO14
D6	IO, MISO	GPIO12
D7	IO, MOSI	GPIO13
D8	IO, 10k Pull-down, SS	GPIO15
G	Ground	GND
5V	5V	–
3V3	3.3V	3.3V
RST	Reset	RST

# Implementation Notes
Don't touch the D3 (GPIO0), D4 (GPIO2), D8 (GPIO15)
Those are essenital at start-up and during download

MODE							GPIO 0 (D3, PULL-UP)	GPIO 2 (D4, PULL-UP)	GPIO 15 (D8, PULL-DOWN)
UART Download (Programming)		0						1						0
Flash Startup (Normal)			1						1						0
SD-Card Boot					0						0						1

It is recommended to use only the following GPIOs:
D0 GPIO16
D1 GPIO5
D2 GPIO4
D4 GPIO2 (Acceptable, in case of flashing issues diconnect/unsoldier stuff connected here)
D5 GPIO14
D6 GPIO12
D7 GPIO13
D8 GPIO15 (Acceptable, in case of flashing issues diconnect/unsoldier stuff connected here)