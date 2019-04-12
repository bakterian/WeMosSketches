#include <Arduino.h>
#ifdef ESP32
  #include <WiFi.h>
  #include "SPIFFS.h"
#else
  #include <ESP8266WiFi.h>
#endif
#include <SPI.h>
#include "AudioFileSourceSD.h"
//#include "AudioFileSourceSPIFFS.h"
#include "AudioFileSourceID3.h"
#include "AudioGeneratorMP3.h"
#include "AudioOutputI2SNoDAC.h"

#include <Wire.h>
#include "SSD1306Wire.h"
#include "xbmImages.h"

#define SDA_PIN 4u // D2 -> GPIO4
#define SCL_PIN 5u // D1 -> GPIO5

SSD1306Wire  display(0x3c, 4, 5);
AudioGeneratorMP3 *mp3;
AudioFileSourceSD *sdAudioSource;
AudioOutputI2S *out;
AudioFileSourceID3 *id3;
bool trackNo2played = false;
File root;
char** pMp3List;
const int mp3ListSize = 50;
const int maxNameSize = 12;
int totalMp3Count = 0;
int playedBackCount = 0;

// Called when a metadata event occurs (i.e. an ID3 tag, an ICY block, etc.
void MDCallback(void *cbData, const char *type, bool isUnicode, const char *string)
{
  (void)cbData;
  Serial.printf("ID3 callback for: %s = '", type);

  if (isUnicode) {
    string += 2;
  }
  
  while (*string) {
    char a = *(string++);
    if (isUnicode) {
      string++;
    }
    Serial.printf("%c", a);
  }
  Serial.printf("'\n");
  Serial.flush();
}

bool registerMetadatHandler(AudioFileSource *src)
{
   if(id3) delete id3;
   id3 = new AudioFileSourceID3(src);
   return id3->RegisterMetadataCB(MDCallback, (void*)"ID3TAG");
}

bool initSerial()
{
  Serial.begin(115200);
  delay(1000);
  return Serial;
}

bool initDisplay()
{
  bool res = display.init();
  if(res)
  {
    display.flipScreenVertically();
    display.setFont(ArialMT_Plain_10);
  }
  return res;
}

void printLogo()
{
  display.clear();
  display.drawXbm(68, 0, logo_width, logo_height, doubleNote_bits);
  display.setTextAlignment(TEXT_ALIGN_RIGHT);
  display.display();
}

void printMusicFiles(File dir) 
{
  dir.rewindDirectory();
  while (true) 
  {
    File entry =  dir.openNextFile();
    if (!entry) 
    {
      // no more files
      break;
    }
    
    if (entry.isDirectory()) 
    {
      continue; //we will keep all files in one folder for now, thus skip
    }
    else if(strstr(entry.name(),"MP3"))
    {
      Serial.print(entry.name());
      Serial.print("\t\t");
      Serial.println(entry.size(), DEC);
      if(totalMp3Count < mp3ListSize)
      {
        strncpy(pMp3List[totalMp3Count], entry.name(), maxNameSize);
        ++totalMp3Count;
      }
      else
      {
        Serial.print("No room in playlist array for this mp3");
      }   
    }
    entry.close();
  }
}


bool initSDcardOverSPI(uint8_t chipSelectPin, const char *filename = NULL)
{
  bool res = false;
  do
  {
    res = SD.begin(chipSelectPin);
    if(!res)
    {
      Serial.print("[ERROR] SD card library init failed)");
      continue;
    }

     root = SD.open("/");
     printMusicFiles(root);  

    if(filename)
    { 
      res = SD.exists(filename);
      if(res)
      {
         Serial.println("Def audio file was found on the SD card"); 
         sdAudioSource = new AudioFileSourceSD(filename);
      }
      else
      {
        Serial.println("ERROR %s was found not found the SD card");
      }
    }  
    else
    {
      sdAudioSource = new AudioFileSourceSD();
    }
    
  }while(false);

  return res;
}

bool conifgureAudioOutput(float outputGain)
{
  out = new AudioOutputI2S();
  return out->SetGain(outputGain);
}

bool startAudioProcessing()
{
  if(mp3) delete mp3;
  mp3 = new AudioGeneratorMP3();
  return mp3->begin(id3, out);
}

bool pickNextTrack()
{
  bool res = true;
  if(totalMp3Count != playedBackCount)
  {
    do
    {
      res = sdAudioSource->open(pMp3List[playedBackCount]);
      if(!res)
      {
       Serial.println("new track opening was not succesfull.");
       continue;
      }
    
      res = registerMetadatHandler(sdAudioSource);
      if(!res)
      {
        Serial.println("new track metadata registration was not succesfull.");
        continue;
      }
    
       res = startAudioProcessing();
       if(!res)
       {
          Serial.println("new track playback start was not succesfull.");
       }
      }while(false);
  
   if(res)
    {
      Serial.printf("Starting mp3 playback");
    }
    else
    {
      Serial.printf("Could not start next mp3.");
    }
  }
}

void initMp3List()
{
    pMp3List = new char*[mp3ListSize];
    for(int i = 0; i < mp3ListSize; ++i)
    {
      pMp3List[i] = new char[maxNameSize];
    }
}

void setup()
{
  WiFi.mode(WIFI_OFF);
  initMp3List();
  
  do
  {
    bool res = initSerial();
    if(!res)continue;

    res = initDisplay();
    if(!res)
    {
      Serial.println("[ERROR] SSD1306 I2C Display initilization went bad");
      continue;
    }

    printLogo();
        
    res = initSDcardOverSPI(16); //using DO (GPIO16) as we don't want to use the Deep Sleep functionality
    if(!res)
    {
      Serial.println("[ERROR] SD Card Init or File search went bad");
      continue;
    }
    
    res = registerMetadatHandler(sdAudioSource);
    if(!res)
    {
      Serial.println("[ERROR] Metadata handler registration failed.");
      continue;
    }
    
    conifgureAudioOutput(0.1f); //using low gain as the MAX98357A can output some seroius power 
    if(!res)
    {
      Serial.println("[ERROR] Audio output congfiguration failed.");
      continue;
    }
       
    Serial.printf("Sample MP3 playback begins...\n");  
  }
  while(false);
}

void loop()
{
  if(totalMp3Count != playedBackCount)
  {
    if ((mp3) && mp3->isRunning()) 
    {
      if (!mp3->loop())
      {
        mp3->stop();
        ++playedBackCount;
        pickNextTrack();
      }
    }
    else
    {
      pickNextTrack();
    }
  } 
  else 
  {
    Serial.printf("MP3 Playback done\n");
    delay(5000);
  }
}

