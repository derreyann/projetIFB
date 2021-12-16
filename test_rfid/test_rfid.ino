#include <SPI.h>
#include <MFRC522.h>
#include <WiFi.h>
#include <PubSubClient.h>


// Update these with values suitable for your network.
const char* ssid = "hi bud how's it going";
const char* password = "heybuddy";
const char* mqtt_server = "mqtt.ci-ciad.utbm.fr";
#define mqtt_port 1883
#define MQTT_USER ""
#define MQTT_PASSWORD ""
#define MQTT_SERIAL_PUBLISH_CH "IF3B/Projet_Acces/serialdata/idcarte"
#define MQTT_SERIAL_RECEIVER_CH "IF3B/Projet_Acces/serialdata/feedback"
#define SS_PIN 23
#define RST_PIN 16
WiFiClient wifiClient;
PubSubClient client(wifiClient);
MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.
 

int i = 0;
bool test = true;

void setup() 
{
  delay(10);
    // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.println("waiting pairing...");
    }
  randomSeed(micros());
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.begin(115200);   // Initiate a serial communication
  SPI.begin();      // Initiate  SPI bus
  mfrc522.PCD_Init();   // Initiate MFRC522
  Serial.println("Approximate your card to the reader...");
  Serial.println();

}


void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(),MQTT_USER,MQTT_PASSWORD)) {
      Serial.println("connected");
      //Once connected, publish an announcement...
      client.publish("IF3B/test/serialdata/tx", "reconnected");
      // ... and resubscribe
      client.subscribe(MQTT_SERIAL_RECEIVER_CH);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void callback(char* topic, byte *payload, unsigned int length) {
      Serial.print("Message arrived on topic: ");
      Serial.print(topic);
      Serial.print(". Message: ");
      String messageTemp;
      
      for (int i = 0; i < length; i++) {
        Serial.print((char)payload[i]);
        messageTemp += (char)payload[i];
      }
      Serial.println();
    if (String(topic) == "IF3B/Projet_Acces/serialdata/feedback") {
    Serial.print("feedback reçu est");
    if(messageTemp == "1"){
      Serial.println("Badge valide");
          digitalWrite(12,LOW);
          digitalWrite(0,HIGH);
          digitalWrite(A0, HIGH);
          delay(400);
          digitalWrite(A0, LOW);
          delay(3000);
          digitalWrite(12,HIGH);
          digitalWrite(0,LOW);
          delay(3000);
    }
    else if(messageTemp == "0"){
      Serial.println("Badge inconnu");
            for (int j=0; i<10; i++){
            digitalWrite(0,HIGH);
            delay(100);
            digitalWrite(0,LOW);
          }
        }
        delay(3000);
    }
}
 

void loop() 
{
  // Look for new cards
  if ( ! mfrc522.PICC_IsNewCardPresent()) 
  {
    return;
  }
  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) 
  {
    return;
  }
  //Show UID on serial monitor
  Serial.print("UID tag :");
  String content= "";
  byte letter;
  byte publishing=1;
  for (byte i = 0; i < mfrc522.uid.size; i++) 
  {
     Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
     Serial.print(mfrc522.uid.uidByte[i], HEX);
     content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
     content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  Serial.println();
  Serial.print("Message : ");
  content.toUpperCase();

  //Creating publishing functionalities
 if (!client.connected()) {
    reconnect();
  }else{
    client.loop();

    char tempString[8];
    Serial.print("Idread");
    Serial.println(tempString);
    client.publish("IF3B/projetporte/idcarte", tempString);

    //Deuxième publication pour laser    
    char lasString[8];
    Serial.print("Laserstatus: ");
    Serial.println(lasString);
    client.publish("IF3B/projetporte/laser", lasString);
  }
}
