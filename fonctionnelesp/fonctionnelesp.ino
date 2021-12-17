#include <WiFi.h>
#include <PubSubClient.h>
#include <SPI.h>
#include <MFRC522.h>
#define SS_PIN 23
#define RST_PIN 16
// Credentials
const char* ssid = "hey bud how's it going";
const char* password = "heybuddy";
const char* mqtt_server = "mqtt.ci-ciad.utbm.fr";

WiFiClient espClient;
MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.
PubSubClient client(espClient);

// LED Pin
const int ledPin = 4;

void setup() {
  Serial.begin(115200);
  setup_wifi();
  SPI.begin();      // Initiate  SPI bus
  mfrc522.PCD_Init();   // Initiate MFRC522
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  Serial.println("Approximate your card to the reader...");
  Serial.println();
}

void setup_wifi() {
  delay(10);
  // Connecting to wifi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  // If a message is received on the feedback channel, validated by server
  // Changes the output state according to the message
  if (String(topic) == "IF3B/Projet_Acces/serialdata/feedback") {
    Serial.print("Changing output to ");
    if(messageTemp == "1"){
      Serial.println("1");
      digitalWrite(ledPin, HIGH); //OUVRIR PORTE
      // ALLUMER LED CONSTANTE PENDANT 5 SEC
      client.publish("IF3B/Projet_Acces/serialdata/test", "c'est un test badge valide");

    }
    else if(messageTemp == "0"){
      Serial.println("0");
      digitalWrite(ledPin, LOW); // INVALIDE PORTE
      // CLIGNOTER LED ROUGE
      client.publish("IF3B/Projet_Acces/serialdata/test", "badge invalide");
    }
    else if(messageTemp == "2"){
      Serial.println("2");
      digitalWrite(ledPin, LOW); // APPAIRAGE DETECTÉ server-size
      // CLIGNOTER LED ROUGE et VERTE
      client.publish("IF3B/Projet_Acces/serialdata/test", "appairage carte en cours, placer sur le lecteur");
    }
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESPCLIENTYANN")) {
      Serial.println("connected");
      // Re-Subscribe to the topic
      client.subscribe("IF3B/Projet_Acces/serialdata/feedback");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
void loop() {
// Look for new cards
  if ( ! mfrc522.PICC_IsNewCardPresent()) 
  {
    delay(500);
    return;
  }
// Select one of the cards
  while( ! mfrc522.PICC_ReadCardSerial()) 
  {
    delay(500);
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
  Serial.print("Carte lue:");
  content.toUpperCase();
  Serial.println(content.substring(1));
  
  if (!client.connected()) {
    reconnect();  //Check for disconnection
  }
  client.loop();
    
  //After reading the card, publishing id to server for SQL comparison analysis 
    char buf[32];
    Serial.println(content.substring(1));
    String s = String(content.substring(1));
    s.toCharArray(buf,32);
    client.publish("IF3B/Projet_Acces/serialdata/idcarte", buf);
    delay(3000);  
}
