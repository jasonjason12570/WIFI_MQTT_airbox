//WIFI版本
#include <Wire.h>
#include "Adafruit_SGP30.h"
#include <Arduino.h>
#include "Adafruit_SHT31.h"
#include "PMS.h"
#include <SoftwareSerial.h>


SoftwareSerial pms5003(14, 12); // RX, TX ；接到pms模組上的TX,RX（相反）
PMS pms(pms5003);
PMS::DATA data;
int FAN_PIN = 7;

Adafruit_SGP30 sgp;
bool enableHeater = false;
uint8_t loopCnt = 0;
Adafruit_SHT31 sht31 = Adafruit_SHT31();

////=== Wi-Fi === start =================
#include <ESP8266WiFi.h>

const char* ssid     = "Owen";
const char* password = "27368857";
String LocalIP="";
////=== Wi-Fi === end ===================

///=== MQTT Pub.sub message === start ======================
#include <PubSubClient.h>

const char* mqttServer = "140.128.99.71";
const int   mqttPort     = 1883;
const char* mqttUser     = "course";
const char* mqttPassword = "iot999";
const char* mqttPublishTopic = "ST019/Team99/Sensors_Owen";
const char* mqttSubscribeTopic = "ST019/Team99/Relay";

WiFiClient espClient;
PubSubClient client(espClient);
///=== MQTT Pub.Sub. message === end =======================




void setup_wifi() {      // We start by connecting to a WiFi network   
  delay(10);
  Serial.println();
  Serial.printf("Connecting to %s ", ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  randomSeed(micros());
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  LocalIP = WiFi.localIP().toString();
  Serial.println("IP address: " + LocalIP);
}


void callback(char* topic, byte* payload, unsigned int length) {
  Serial.printf("Message arrived [%s] ", topic);
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(LED_BUILTIN_AUX, LOW);   // Turn the LED on 
    
  } else {
    digitalWrite(LED_BUILTIN_AUX, HIGH);  // Turn the LED off

  }
}


void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(), mqttUser, mqttPassword)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
//      client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe(mqttSubscribeTopic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}



void setup() {
  
  pms5003.begin(9600);
  Serial.begin(9600);
  Serial.println("================");
  setup_wifi();
  
  while (!pms5003) { delay(10); } // Wait for serial console to open!
  Serial.println("pms5003 OK");
  while (!Serial) { delay(10); } // Wait for serial console to open!
  Serial.println("Serial OK");

  
  Serial.println("SGP30 test");
  if (! sht31.begin(0x44)) {   // Set to 0x45 for alternate i2c addr
    Serial.println("Couldn't find SHT31");
    
    while (! sht31.begin(0x44)) {
      Serial.print(".");
      delay(1000);
      }
  }
  if (! sgp.begin()){
    Serial.println("Sensor not found :(");
    while (1);
  }

  
  Serial.print("Found SGP30 serial #");
  Serial.print(sgp.serialnumber[0], HEX);
  Serial.print(sgp.serialnumber[1], HEX);
  Serial.println(sgp.serialnumber[2], HEX);
//////////==============//////////////////
  Serial.print("Heater Enabled State: ");
  if (sht31.isHeaterEnabled())
    Serial.println("ENABLED");
  else
    Serial.println("DISABLED");
  // If you have a baseline measurement from before you can assign it to start, to 'self-calibrate'
  //sgp.setIAQBaseline(0x8E68, 0x8F41);  // Will vary for each sensor!
  client.setServer( mqttServer, mqttPort);
  client.setCallback(callback);

  Serial.println("================");

  
}

int counter = 0;
void loop() {
  //Serial.println("inloop");
  if (pms.read(data))
  {
    Serial.println(millis());
    
    digitalWrite(LED_BUILTIN_AUX, HIGH);  // Turn the LED off
    Serial.print("PM 1.0 (ug/m3): ");
    Serial.println(data.PM_AE_UG_1_0);

    Serial.print("PM 2.5 (ug/m3): ");
    Serial.println(data.PM_AE_UG_2_5);

    Serial.print("PM 10.0 (ug/m3): ");
    Serial.println(data.PM_AE_UG_10_0);

    Serial.println();
    /*
    if(data.PM_AE_UG_2_5>5){
      digitalWrite(FAN_PIN,LOW);
      Serial.println("digitalWrite(FAN_PIN,LOW);");
    }else{
      
    digitalWrite(FAN_PIN,HIGH);
      Serial.println("digitalWrite(FAN_PIN,HIGH);");
      }
   */
    if(data.PM_AE_UG_2_5>1){
      digitalWrite(FAN_PIN,LOW);
      Serial.println("digitalWrite(FAN_PIN,LOW);");
    }else{
      
    digitalWrite(FAN_PIN,HIGH);
      Serial.println("digitalWrite(FAN_PIN,HIGH);");
      }

  // If you have a temperature / humidity sensor, you can set the absolute humidity to enable the humditiy compensation for the air quality signals
  //float temperature = 22.1; // [°C]
  //float humidity = 45.2; // [%RH]
  //sgp.setHumidity(getAbsoluteHumidity(temperature, humidity));
  float t = sht31.readTemperature();
  float h = sht31.readHumidity();
  
  if (! isnan(t)) {  // check if 'is not a number'
    Serial.print("Temp *C = "); Serial.print(t); Serial.print("\t\t");
  } else { 
    Serial.println("Failed to read temperature");
  }
  
  if (! isnan(h)) {  // check if 'is not a number'
    Serial.print("Hum. % = "); Serial.println(h);
  } else { 
    Serial.println("Failed to read humidity");
  }
  
  if (! sgp.IAQmeasure()) {
    Serial.println("Measurement failed");
  }
  Serial.print("TVOC "); Serial.print(sgp.TVOC); Serial.print(" ppb\t");
  Serial.print("eCO2 "); Serial.print(sgp.eCO2); Serial.println(" ppm");

  if (! sgp.IAQmeasureRaw()) {
    Serial.println("Raw Measurement failed");
  }
  Serial.print("Raw H2 "); Serial.print(sgp.rawH2); Serial.print(" \t");
  Serial.print("Raw Ethanol "); Serial.print(sgp.rawEthanol); Serial.println("");
  
  

  counter++;
  if (counter == 30) {
    counter = 0;

    uint16_t TVOC_base, eCO2_base;
    if (! sgp.getIAQBaseline(&eCO2_base, &TVOC_base)) {
      Serial.println("Failed to get baseline readings");
    }
    Serial.print("****Baseline values: eCO2: 0x"); Serial.print(eCO2_base, HEX);
    Serial.print(" & TVOC: 0x"); Serial.println(TVOC_base, HEX);
  }
  /*
   * data.PM_AE_UG_1_0
   * data.PM_AE_UG_2_5
   * data.PM_AE_UG_10_0
   * t
   * h
   * sgp.TVOC
   * sgp.eCO2
   * sgp.rawH2
   * sgp.rawEthanol
   */




  
  //=== MQTT Publish message - start =============================//
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
 
  char msg[500];
  //sprintf (msg, "{\"Temperature\":%4.2f,\"Humidity\":%4.2f}", t, h);
  sprintf (msg, "{\"Temperature\":%4.2f,\"Humidity\":%4.2f,\"PM1.0\":%d,\"PM2.5\":%d,\"PM10.0\":%d,\"TVOC\":%d,\"eCO2\":%d,\"rawH2\":%d,\"rawEthanol\":%d}", t, h,data.PM_AE_UG_1_0,data.PM_AE_UG_2_5,data.PM_AE_UG_10_0,sgp.TVOC,sgp.eCO2,sgp.rawH2,sgp.rawEthanol);
  Serial.print("Publish message: ");
  Serial.println(msg);
  if (client.publish(mqttPublishTopic, msg) == true) {
    Serial.println("Success sending message");
  } else {
    Serial.println("Error sending message");
  }
  //=== MQTT Publish message - end =============================//
  digitalWrite(LED_BUILTIN_AUX, LOW);  // Turn the LED off
  Serial.println("================================");
  delay(1000);

  }
  
}
