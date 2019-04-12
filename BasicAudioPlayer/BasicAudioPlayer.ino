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

// To run, set your ESP8266 build to 160MHz, and include a SPIFFS of 512KB or greater.
// Use the "Tools->ESP8266/ESP32 Sketch Data Upload" menu to write the MP3 to SPIFFS
// Then upload the sketch normally.  

struct DeleterAudioGenerator {
    void operator() (AudioGeneratorMP3* p) {
        Serial.println("Calling delete for AudioGeneratorMP3 object... \n");
        delete p;
    }
};

struct DeleterAudioSourceID3 {
    void operator() (AudioFileSourceID3* p) {
        Serial.println("Calling delete for AudioGeneratorMP3 object... \n");
        delete p;
    }
};

AudioGeneratorMP3 *mp3;
//AudioFileSourceSPIFFS *spiffsAudioSource;
AudioFileSourceSD *sdAudioSource;
AudioOutputI2S *out;
AudioFileSourceID3 *id3;
bool trackNo2played = false;



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

bool initSPIFFS(const char *filename)
{
  //bool res = SPIFFS.begin();
  //spiffsAudioSource = new AudioFileSourceSPIFFS(filename);
  //return res;
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

    if(filename)
    { 
      res = SD.exists(filename);
      if(res)
      {
         Serial.println("%s was found on the SD card"); 
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

void setup()
{
  WiFi.mode(WIFI_OFF);
  do
  {
    bool res = initSerial();
    if(!res)continue;
    
    /*initSPIFFS("/nfl.mp3");
    if(!res)
    {
      Serial.println("Spiffs init failed");
      continue;
    }*/
    
    res = initSDcardOverSPI(16,"ufc.mp3"); //using DO (GPIO16) as we don't want to use the Deep Sleep functionality
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
    
    conifgureAudioOutput(0.3f); //using low gain as the MAX98357A can output some seroius power 
    if(!res)
    {
      Serial.println("[ERROR] Audio output congfiguration failed.");
      continue;
    }
    
    startAudioProcessing();
    if(!res)
    {
      Serial.println("[ERROR] Audio generator initialization failed.");
      continue;
    } 
       
    Serial.printf("Sample MP3 playback begins...\n");
    
  }
  while(false);
}

void loop()
{
  if (mp3->isRunning()) {
    if (!mp3->loop())
    {
      mp3->stop();
      if(!trackNo2played)
      {
        bool openSucc = sdAudioSource->open("yb.mp3");
        if(!openSucc)
        {
         Serial.println("Track 2 opening was not succesfull.");
        }
        else
        {
          bool regSucceded = registerMetadatHandler(sdAudioSource);
          if(!regSucceded)
          {
            Serial.println("Track 2 metadata registration was not succesfull.");
          }
          else
          {
            if(startAudioProcessing())
            {
              Serial.println("Track 2 playbakc start was not succesfull.");
            }
          }
        }
        trackNo2played = true;  
      }
    }
  } else {
    Serial.printf("MP3 done\n");
    delay(3000);
  }
}

