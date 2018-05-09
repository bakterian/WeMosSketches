#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "SoftwareSerial.h"

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
#define MQTT_TOPIC          "YOUR_MQTT_TOPIC"
#else
#include "D:\5_GITHUB\0_CONFIG\MqttAwsCredentials.h"
#endif

//LOCAL DEFINES
#define MQTT_PUB_INTERVAL_MS  5000
#define SERIAL_BAUD_RATE      115200
#define USE_DHT22             0

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastSend;
unsigned int gMsgCounter = 0;

void sendMqttMeasData(String counter,String temperature, String humidity)
{
    // Just debug messages
  Serial.print( "Sending temperature and humidity : [" );
  Serial.print( counter ); Serial.print( "," );
  Serial.print( temperature ); Serial.print( "," );
  Serial.print( humidity );
  Serial.print( "]   -> " );

  // Prepare a JSON payload string
  String payload = "{";
  payload += "\"counter\":"; payload += counter; payload += ",";
  payload += "\"temperature\":"; payload += temperature; payload += ",";
  payload += "\"humidity\":"; payload += humidity;
  payload += "}";

  // Send payload
  char attributes[100];
  payload.toCharArray( attributes, 100 );
  client.publish( MQTT_TOPIC, attributes );
  Serial.println( attributes );
}

void sendDummyData(unsigned int msgCounter)
{
  String temperature = String(200);
  String humidity = String(100);

  sendMqttMeasData(String(msgCounter),temperature, humidity);
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
    if ( client.connect(CLIENIT_ID, USER_ID, MQTT_PASS) ) {
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
  InitWiFi();
  client.setServer( thingsboardServer, MQTT_PORT );
  lastSend = 0;
}

void loop() 
{
  
  if (!client.connected()) {
    reconnect();
  }

  if ( millis() - lastSend > MQTT_PUB_INTERVAL_MS ) 
  { 
    gMsgCounter++;    
    sendDummyData(gMsgCounter);  
    lastSend = millis();
  }

  client.loop();
}
