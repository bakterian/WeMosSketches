/**************************************************************************/
/*! 
    This example will wait for any ISO14443A card or tag, and
    depending on the size of the UID will attempt to read from it.
   
    If the card has a 4-byte UID it is probably a Mifare
    Classic card, and the following steps are taken:
   
    - Authenticate block 4 (the first block of Sector 1) using
      the default KEYA of 0XFF 0XFF 0XFF 0XFF 0XFF 0XFF
    - If authentication succeeds, we can then read any of the
      4 blocks in that sector (though only block 4 is read here)
	 
    If the card has a 7-byte UID it is probably a Mifare
    Ultralight card, and the 4 byte pages can be read directly.
    Page 4 is read by default since this is the first 'general-
    purpose' page on the tags.

    To enable debug message, define DEBUG in PN532/PN532_debug.h
*/
/**************************************************************************/

#define USE_PN532_SPI 1

#define SS_PIN 16u //GPIO16 (WEMOS D0)


#define BLOCK_NO(sectorID, blockIdInSector) ((sectorID*4) + blockIdInSector)
#define BLOCK_2_SEC(blockID)  (blockID/4)

#ifdef USE_PN532_SPI
  #include <SPI.h>
  #include <PN532_SPI.h>
  #include "PN532.h"
  PN532_SPI pn532spi(SPI, SS_PIN);
  PN532 nfc(pn532spi);
#endif

#ifdef USE_PN532_HSU
  #include <PN532_HSU.h>
  #include <PN532.h>
      
  PN532_HSU pn532hsu(Serial1);
  PN532 nfc(pn532hsu);
#endif
 
#ifdef USE_PN532_I2C
  #include <Wire.h>
  #include <PN532_I2C.h>
  #include <PN532.h>
  PN532_I2C pn532i2c(Wire);
  PN532 nfc(pn532i2c);	
#endif

void printCardInfos(PN532& nfc,const uint8_t* uid, const uint8_t uidLength)
{
    Serial.println("Found an ISO14443A card");
    Serial.print("  UID Length: ");Serial.print(uidLength, DEC);Serial.println(" bytes");
    Serial.print("  UID Value: ");
    nfc.PrintHex(uid, uidLength);
    Serial.println("");
}

void writeBlockFromMifareCard(PN532& nfc,const uint8_t blockNo, uint8_t* data)
{
  uint8_t success = nfc.mifareclassic_WriteDataBlock(blockNo, data);
  if(success == 0u) 
  {
    Serial.println("Write failed");
  }
  else 
  {
    Serial.println("Write succeeded");
  }
}

void readPageFromUltralightCard(PN532& nfc, const uint8_t pageID)
{
  Serial.println("Seems to be a Mifare Ultralight tag (7 byte UID)");
  
  // Try to read the first general-purpose user page (#4)
  Serial.println("Reading page ");
  uint8_t data[32];
  uint8_t success = nfc.mifareultralight_ReadPage(pageID, data);
  if (success)
  {
    // Data seems to have been read ... spit it out
    nfc.PrintHexChar(data, 4);
    Serial.println("");
  
    // Wait a bit before reading the card again
    delay(1000);
  }
  else
  {
    Serial.println("Ooops ... unable to read the requested page!?");
  }
}

void readBlockFromMifareCard(PN532& nfc,const uint8_t blockID)
{
  uint8_t data[16];
  uint8_t success = nfc.mifareclassic_ReadDataBlock(blockID, data);
  
  if (success != 0u)
  {
    // Data seems to have been read ... spit it out
    Serial.println("Reading Block :");
    nfc.PrintHexChar(data, 16);
    Serial.println("");
    
    // Wait a bit before reading the card again
    delay(1000);
  }
  else
  {
    Serial.println("Ooops ... unable to read the requested block.  Try another key?");
  }
}

bool authenticateKeyA(PN532& nfc, uint8_t* uid, const uint8_t uidLength, const uint32_t blockNo, uint8_t* keya)
{
  uint8_t result = nfc.mifareclassic_AuthenticateBlock(uid, uidLength, blockNo, MIFARE_CMD_AUTH_A, keya);

  if( result != 0 )
  {
    Serial.print("Sector ");
    Serial.print(BLOCK_2_SEC(blockNo),DEC);
    Serial.println(" has been authenticated.");
  }
  else
  {
    Serial.println("Ooops ... authentication failed: Try another key?");
  }

  return (result != 0);
}


void setup(void) 
{
  Serial.begin(115200);
  Serial.println("Hello!");

  nfc.begin();

  uint32_t versiondata = nfc.getFirmwareVersion();
  if (! versiondata) {
    Serial.print("Didn't find PN53x board");
    while (1); // halt
  }
  // Got ok data, print it out!
  Serial.print("Found chip PN5"); Serial.println((versiondata>>24) & 0xFF, HEX); 
  Serial.print("Firmware ver. "); Serial.print((versiondata>>16) & 0xFF, DEC); 
  Serial.print('.'); Serial.println((versiondata>>8) & 0xFF, DEC);
  
  // configure board to read RFID tags
  nfc.SAMConfig();
  
  Serial.println("Waiting for an ISO14443A Card ...");
}


void loop(void) {
  uint8_t success;
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
  uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
  bool keyLearingActive = false;            // true - write data to fresh/new Cards, false - only reading data

    
  // Wait for an ISO14443A type cards (Mifare, etc.).  When one is found
  // 'uid' will be populated with the UID, and uidLength will indicate
  // if the uid is 4 bytes (Mifare Classic) or 7 bytes (Mifare Ultralight)
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);
  
  if (success) {
    // Display some basic information about the card
    printCardInfos(nfc, uid, uidLength);

    
    if (uidLength == 4)
    {
      // We probably have a Mifare Classic card ... 
      Serial.println("Seems to be a Mifare Classic card (4 byte UID)");
      uint8_t keya[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };  
      bool authSuccesfull = false;
      uint32_t blockNo = 0u;
      
      // DATA ACCESS #1
      // Now we need to try to authenticate it for read/write access
      // Try with the factory default KeyA: 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF
      
      Serial.println("Trying to authenticate block 15 with default KEYA value");
      blockNo = BLOCK_NO(15,0);
      authSuccesfull = authenticateKeyA(nfc, uid, uidLength, blockNo, keya);
      
      if (authSuccesfull)
      {	
       if(keyLearingActive)
       {
        //uint8_t data[16] = { 'u', 'f', 'c', '.', 'm', 'p', '3', 0, 0, 0, 0, 0, 0, 0, 0, 0};
        uint8_t data[16] = { 'n', 'f', 'l', '.', 'm', 'p', '3', 0, 0, 0, 0, 0, 0, 0, 0, 0};
        //uint8_t data[16] = { 'k', 'o', 'p', 'y', 't', 'k', 'o', 'X', 'X', 'L', 0, 0, 0, 0, 0, 0};
        writeBlockFromMifareCard(nfc, blockNo, &data[0]);
       }
            
        // Try to read the contents of block 
        readBlockFromMifareCard(nfc, blockNo);
      }


  /*
      // DATA ACCESS #2
       if(keyLearingActive)
       {
        // Now let's change the access to sector 13
        Serial.println("Trying to authenticate block 13 with default KEYA value");
        blockNo = BLOCK_NO(13,0);
        authSuccesfull = authenticateKeyA(nfc, uid, uidLength, blockNo, keya);

        if(authSuccesfull)
        {
          uint8_t dataSec[16] = { 'u', 'f', 'c', '.', 'm', 'p', '3', 0, 0, 0, 0, 0, 0, 0, 0, 0};
          writeBlockFromMifareCard(nfc, blockNo, &dataSec[0]);
        
          uint8_t sectortrailer[16] = 
          {
          0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5,   // Key A
          0xFF, 0x07, 0x80, 0x69,              // Access Bits (default)
          0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5   // Key B
          };
  
          uint8_t sectorTrailerBlockNo = BLOCK_NO(13,3);
          writeBlockFromMifareCard(nfc, sectorTrailerBlockNo, sectortrailer);

           // Try to read the contents of block 
           readBlockFromMifareCard(nfc, blockNo);
        }       
       }
       else
       {   
        // Now let's authenticate to sector 13 with non-facoty Key
        Serial.println("Trying to authenticate block 13 with secret Key-A value");
        uint8_t keySec[6] = { 0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5 }; //TODO remove this define 
        blockNo = BLOCK_NO(13,0);
        authSuccesfull = authenticateKeyA(nfc, uid, uidLength, blockNo, keySec);   
        if (authSuccesfull)
        {  
          // Try to read the contents of block 
          readBlockFromMifareCard(nfc, blockNo);
        }
      }
*/
    }
    
    if (uidLength == 7)
    {
      // We probably have a Mifare Ultralight card ...
      readPageFromUltralightCard(nfc, 4u);
    }
  }
}

