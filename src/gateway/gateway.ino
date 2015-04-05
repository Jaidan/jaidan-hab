/* TODO
   Expland list of command verbs:
   1. State
   2. Set
   3. Timestamp
   ...

   Consider sacrificing another byte of topic space for sensor type definition
   that can carry applicaple comman details
*/
#include "../common/secure.h"
#include "config.h"
#include "../common/defines.h"
#include <EEPROM.h>
#include <avr/pgmspace.h>

#include <RFM69.h>
#include <SPI.h>
#include <PubSubClient.h>
#include <Ethernet.h>
#include "radioNode.h"
#include "topicStorage.h"
#include "controls.h"

#define LCOMMAND 10

uint8_t mac[] = {  0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0xED }; // get real mac
uint8_t server[] = { 192, 168, 0, 1 };
uint8_t ip[4];

const uint8_t HEADER = sizeof(RadioHeader);

const PROGMEM char commands[][LCOMMAND] = {
  { "state" },
  { "set" },
  { "timestamp" },
}

RFM69 radio;
EthernetClient ethClient;
PubSubClient client(server, 1883, callback, ethClient);
TopicStorage ts;

void callback(char* topic, byte* payload, unsigned int length) {
  // split on `/` 
  // join all but last
  // the last is the command
  // Search for topic and get an id
  // send RF message based on topic, command, and id
  // id is 2 nibbles!
  // how to deal with messages to the base topic...
}

void setup()
{
  Serial.begin(SERIAL_BAUD);
  delay(10);
  RadioNode::setupRadio();
  radio = RadioNode::getRadio();
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    while(true);
  }
  ip = Ethernet.localIP();
  ts = TopicStorage();
}

void loop()
{
  if (!client.connected()) {
    if (!client.connect("arduinoClient")) {
      Serial.println("Failed to connect to MQTT server...");
      delay(5000);
    }
  }
  char input = Serial.read();
  if (Serial.available() > 0)
    RadioNode::executeCommand(input);
  if (radio.receiveDone()) {
    RadioHeader header;
    char[RF69_MAX_DATA_LEN - HEADER] value;

    RadioNode::readRadio(&header, value);
    switch header.packetType {
      case REGISTRATION:
        handleRegistration(header, value);
        break;
      case LIGHT_STATUS:
        lightStatusChange(header, (LightStatus*) value);
        break;
    }
  }
  client.loop();
}

void handleRegistration(RadioHeader header, char* topic) 
{
  Registration reg = { header.id, topic };
  ts.writeRegistration(header, reg);
  char subscription[TOPIC_LENGTH + 2];
  strcpy(subscription, registration.topic);
  strcat(subscription, "/*");
  client.subscribe(subscription);
  for (int i = 0; i < ; i++) {
    char[TOPIC_LENGTH + LCOMMAND] topic;
    char command[LCOMMAND] = {0};
    for (i = 0; i < LCOMMAND; i++) {
      strcpy_P(command, commands[i]);
    }
    strcpy(topic, registration.topic);
    strcat(topic, "/");
    strcat(topic, command);
    client.subscribe(topic);
  } else {
    Serial.println(F("Unable to register, out of space"));
  }
}

void lightStatusChange(RadioHeader header, LightStatus lStatus)
{
  // TODO WIP
  char[TOPIC_LENGTH + 6] topic;
  bool result = ts.getTopicBase(topic, header.id);
  if result
    //TODO concat topic and command
    //topic
    client.publish(topic, (char*)lstatus.status);
  else
    Serial.println(F("Unable to locate topic!"));
}
