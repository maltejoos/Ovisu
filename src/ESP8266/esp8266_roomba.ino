#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>
#include <PubSubClient.h>
#include <ROI.h>

#define SERIAL_RX     14  // D5 pin for SoftwareSerial RX
#define SERIAL_TX     12  // D6 pin for SoftwareSerial TX
SoftwareSerial roombaSerial(SERIAL_RX, SERIAL_TX);

WiFiClient espClient;
PubSubClient mqttClient(espClient);

ROI roi(roombaSerial);

bool read_odometrie_data = false;

void mqtt_msg_callback(char* c_topic, byte* payload, unsigned int length)
{
	String topic(c_topic);
	
	if(topic == "Roomba/Clean")
	{		
		if((char)payload[0] == '0')
		{
			roi.seekDock();
			Serial.println("Seeking Dock...");
		}
		else if((char)payload[0] == '1')
		{
			roi.startClean();
			Serial.println("Cleaning...");
		}
	}
	else if(topic == "Roomba/OdometryBurst")
	{
		if((char)payload[0] == '0')
		{
			read_odometrie_data = false;
			Serial.println("Odometrie Burst off...");
		}
		else if((char)payload[0] == '1')
		{
			read_odometrie_data = true;
			Serial.println("Odometrie Burst on...");
		}
	}
}

void mqtt_reconnect() 
{
  // Loop until we're reconnected
  while (!mqttClient.connected()) {
    //Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (mqttClient.connect(clientId.c_str())) {
      //Serial.println("connected");
      //subscribe
      mqttClient.subscribe("Roomba/Clean");
      mqttClient.subscribe("Roomba/OdometryBurst");
    } else {
      //Serial.print("failed, rc=");
      //Serial.print(mqttClient.state());
      //Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  
  Serial.begin(115200);
  
//  bring down Roombas baud rate to 19200
//  delay(2000);
//  pinMode(5, OUTPUT);
//  for(int i=0; i<3; ++i)
//  {
//	  digitalWrite(5, LOW);
//	  delay(200);
//	  digitalWrite(5, HIGH);
//	  delay(200);
//  }

  roombaSerial.begin(115200);
  
//  change baud rate via OI
//  roombaSerial.write(129);
//  int b = 11;
//  roombaSerial.write(b);

/* Explicitly set the ESP8266 to be a WiFi-client, otherwise, it by default,
     would try to act as both a client and an access-point and could cause
     network-issues with your other WiFi-devices on your WiFi-network. */
  WiFi.mode(WIFI_STA);
  WiFi.begin("SSID", "PW");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    //Serial.print(".");
  }

  //Serial.println("");
  //Serial.println("WiFi connected");  
  //Serial.println("IP address: ");
  //Serial.println(WiFi.localIP());

  randomSeed(micros());
  mqttClient.setServer("192.168.2.107", 1883);
  mqttClient.setCallback(mqtt_msg_callback);
  
  roi.begin();
}

void loop() 
{  
  delay(20);//15ms is update rate of Roomba

  if (!mqttClient.connected()) {
    mqtt_reconnect();
  }
  mqttClient.loop();
  
  if(read_odometrie_data)
  {
	  int dtrans = roi.getDeltaTranslationMm();
	  Serial.println(dtrans);
	  int drot = roi.getDeltaRotationDeg();
	  Serial.println(drot);
	  
	  //only send if any data is not zero
	  if((dtrans != 0) || (drot != 0))
	  {
		  String payload = String(dtrans) + ";" + String(drot);
		  
		  //Serial.println(payload);
		  
		  mqttClient.publish("Roomba/OdometryData", payload.c_str());
	  }
  }  

  //int battery = roi.getBatteryPct();
  //Serial.println(battery);
  //mqttClient.publish("Roomba/Battery", String(battery).c_str(), true);
  //delay(20);
  int charging = roi.isCharging();
  Serial.println(charging);
  mqttClient.publish("Roomba/IsCharging", String(charging).c_str(), true);	
}
