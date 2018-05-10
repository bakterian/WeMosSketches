#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include "BH1750.h"
#include <DHT.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#if 0 //Set your wifi credentials or even better keep those in a un-tracked file
#define WIFI_AP "YOUR_WIFI_AP"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"
#else
#include "D:\5_GITHUB\0_CONFIG\WiFICredentials.h"
#endif

#if 0  //Set your mqtt credentials or even better keep those in a un-tracked file
#define MQTT_PORT           1883
#define CLIENIT_ID          "YOUR_MQTT_CLIENT_ID"
#define USER_ID             "YOUR_MQTT_USER_ID"
#define MQTT_PASS           "YOUR_MQTT_PASS"
char thingsboardServer[] =  "YOUR_MQTT_SERVER_ADDRESS"
#define MQTT_TOPIC_IN       "YOUR_MQTT_IN_TOPIC"
#define MQTT_TOPIC_OUT      "YOUR_MQTT_OUT_TOPIC"
#else
#include "D:\5_GITHUB\0_CONFIG\MqttAwsCredentials.h"
#endif

//LOCAL DEFINES
#define MQTT_PUB_INTERVAL_MS  60000
#define SERIAL_BAUD_RATE      115200

//SENOSR RELATED DEFINES I2C Pins
#define SCL                   5                 // GPIO5 / D1
#define SDA                   4                 // GPIO4 / D2
#define LIGHT_SENSOR_ADDR     0x23
#define LED_PIN               16                //GPIO16 / D0
#define DHT_TYPE              DHT22             
#define DHT_PIN               12                //GPIO12 / MISO /  D6

//GLOBALS
WiFiClient espClient;
PubSubClient client(espClient);
BH1750 lightMeter(LIGHT_SENSOR_ADDR);
DHT dht(DHT_PIN, DHT_TYPE);

WiFiUDP ntpUDP;
// timeClient(ntpUdp,ntpServer,offset,updateInterval);
NTPClient timeClient(ntpUDP,"2.pl.pool.ntp.org",0,MQTT_PUB_INTERVAL_MS/2);

unsigned long lastSend;
unsigned int gMsgCounter = 0;
bool volatile gLedOn = false;

struct DhtMeasData
{
  String Temperature;
  String Humidity; 
};

String Float2String(const float value) {
  // Convert a float to String with two decimals.
  char temp[15];
  String s;

  dtostrf(value, 13, 2, temp);
  s = String(temp);
  s.trim();
  return s;
}

void UpdateLED()
{
    digitalWrite(LED_PIN, gLedOn ? HIGH : LOW);
}

DhtMeasData GetDhtSensorData() 
{
  String s = "";
  int i = 0;
  float h;
  float t;
  DhtMeasData dhtData;
  Serial.println("Start reading DHT22");
  
  while (i++ < 5) 
  {
    h = dht.readHumidity(); //Read Humidity
    t = dht.readTemperature(); //Read Temperature
    
    if (isnan(t) || isnan(h)) 
    {
      delay(100);
      h = dht.readHumidity(true); //Read Humidity
      t = dht.readTemperature(false, true); //Read Temperature
    }
    
    if (isnan(t) || isnan(h)) 
    {
      Serial.println("DHT22 couldn't be read");
    } 
    else 
    {
      dhtData.Temperature = Float2String(t);
      dhtData.Humidity = Float2String(h);
      break;
    }
  }

  Serial.println("End reading DHT11/22");

  return dhtData;
}


void onMqttMsgReceive(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  gLedOn = ((char)payload[0] == '1');
  UpdateLED();
}

void sendMqttMeasData(String counter,String timeStamp, String light,DhtMeasData dhtData, bool ledOn)
{
    // Just debug messages
  Serial.print( "Sending measurments: [" );
  Serial.print( counter ); Serial.print( "," );
  Serial.print( timeStamp ); Serial.print( "," );
  Serial.print( dhtData.Temperature ); Serial.print( "," );
  Serial.print( dhtData.Humidity ); Serial.print( "," );
  Serial.print( light ); Serial.print( "," );
  Serial.print( ledOn );
  Serial.print( "]   -> " );

  // Prepare a JSON payload string
  String payload = "{";
  payload += "\"counter\":"; payload += counter; payload += ",";
  payload += "\"time\":"; payload += timeStamp; payload += ",";
  payload += "\"temp\":"; payload += dhtData.Temperature; payload += ",";
  payload += "\"humid\":"; payload += dhtData.Humidity; payload += ",";
  payload += "\"light\":"; payload += light; payload += ",";
  payload += "\"LED\":"; payload += ledOn;
  payload += "}";

  // Send payload
  char attributes[110];
  payload.toCharArray( attributes, 110 );
  client.publish( MQTT_TOPIC_OUT, attributes );
  Serial.println( attributes );
}


void readMeasData(unsigned int msgCounter)
{
  uint16_t lux = lightMeter.readLightLevel();

  auto dhtData = GetDhtSensorData();
  String light = String(lux);
  auto timeStamp = timeClient.getFormattedTime();
  
  sendMqttMeasData(String(msgCounter),timeStamp, light,dhtData, gLedOn);
}


void InitWiFi()
{
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_AP);

  WiFi.begin(WIFI_AP, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Connecting to ThingsBoard node ...");
    // Attempt to connect (clientId, username, password)
    if ( client.connect(CLIENIT_ID, USER_ID, MQTT_PASS) ) 
    {
      client.subscribe(MQTT_TOPIC_IN);
      Serial.println( "[DONE]" );
    } else {
      Serial.print( "[FAILED] [ rc = " );
      Serial.print( client.state() );
      Serial.println( " : retrying in 5 seconds]" );
      // Wait 5 seconds before retrying
      delay( 5000 );
    }
  }
}

void setup() 
{
  //init LED GPIO
  pinMode(LED_PIN,OUTPUT);
  UpdateLED();
   
  // initialize serial for debugging
  Serial.begin(SERIAL_BAUD_RATE);

  Wire.begin(SDA, SCL);
  // On esp8266 you can select SCL and SDA pins using Wire.begin(D4, D3);
  lightMeter.begin(BH1750::ONE_TIME_HIGH_RES_MODE);
  
  InitWiFi();
  client.setServer(thingsboardServer, MQTT_PORT );
  client.setCallback(onMqttMsgReceive);
  lastSend = 0;

  timeClient.begin(123);
}


void loop() 
{
  
  if (!client.connected()) {
    reconnect();
  }

  if ( millis() - lastSend > MQTT_PUB_INTERVAL_MS ) 
  { 
    gMsgCounter++;
    timeClient.update();  
    readMeasData(gMsgCounter);   
    lastSend = millis();
  }

  client.loop();
}
