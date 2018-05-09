#include <SPI.h>
#include <LoRa.h>
#include<Arduino.h>
 
// WIFI_LoRa_32 ports
// GPIO5  -- SX1278's SCK
// GPIO19 -- SX1278's MISO
// GPIO27 -- SX1278's MOSI
// GPIO18 -- SX1278's CS
// GPIO14 -- SX1278's RESET
// GPIO26 -- SX1278's IRQ(Interrupt Request)

#define SCK     14
#define MISO    12
#define MOSI    13
#define CS      15

#define RST     2
#define DI0     26
#define BAND    868300000
#define SF7     7 //Spreading factor 7 is used
#define BW      125E3
#define CR_DEN  5 //Coding rate denominator, the nominator is fixed and set to 4
#define PR_LEN  8 //Preamble lenght set to 8
#define SW      0x34  //The sync word is 0x34 making this a public network
#define PACKET_SEND_DELAY_MS 10000
int counter = 0;
 
void setup() {
   
  Serial.begin(115200);
  while (!Serial); //If just the the basic function, must connect to a computer
  // Initialising the UI will init the display too.

  SPI.pins(SCK,MISO,MOSI,CS);
  Serial.println("LoRa Sender");

  LoRa.setPins(CS,RST,DI0);
  LoRa.setSpreadingFactor(SF7);
  LoRa.setSignalBandwidth(BW);
  LoRa.setCodingRate4(CR_DEN);
  LoRa.setPreambleLength(PR_LEN);
  LoRa.setSyncWord(SW);
  LoRa.enableCrc();
  if (!LoRa.begin(BAND)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }

  
  Serial.println("LoRa Initial OK!");

  delay(2000);
}
void loop() {
  Serial.print("Sending packet: ");
  Serial.println(counter);

  // send packet
  LoRa.beginPacket();
  LoRa.print("Hello..");
  LoRa.print(counter);
  LoRa.endPacket();
  counter++;

  delay(PACKET_SEND_DELAY_MS);
}
