/* TODO
   Expland list of command verbs:
   1. State
   2. Set
   3. Timestamp
   ...

   Add TopicStorage clearing method
*/
#include <SPI.h>
#include <RFM69.h>
#include <avr/pgmspace.h>
#include <PubSubClient.h>
#include <Ethernet.h>
#include "secure.h"
#include "config.h"
#include "defines.h"

#include <radioNode.h>
#include <controls.h>
#include "topicStorage.h"

#define LCOMMAND 11

uint8_t mac[] = {  0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0xED }; // get real mac
uint8_t server[] = { 192, 168, 0, 1 };
IPAddress ip;

const uint8_t HEADER = sizeof(RadioHeader);

const PROGMEM char commands[][LCOMMAND] = {
  { STATE_COMMAND },
  { SET_COMMAND },
  { TIMESTAMP_COMMAND },
};

RFM69 radio;
EthernetClient ethClient;
PubSubClient client(server, 1883, callback, ethClient);
TopicStorage ts;

void callback(char* topic, byte* payload, unsigned int length) {
  // Search for topic and get an id
  // send RF message based on topic, command, and id
  // id is 2 nibbles!
  // how to deal with messages to the base topic...
  uint8_t nCommands = sizeof(commands) / LCOMMAND;
  char command[LCOMMAND] = {0};
  char baseTopic[TOPIC_LENGTH] = {0};
  char *location;
  for (uint8_t i; i < nCommands; i++) {
    strcpy_P(command, commands[i]);
    // This make be not robust enough.  The leading "/" on the command should
    // make this not hit false positives
    location = strstr(topic, command);
    if (location != NULL) {
      // TODO check this pointer math
      strncpy(baseTopic, topic, location - topic);
      break;
    }
  }
  if (location == NULL) {
    // No commands found, must be base topic already?
    // Is there an appropriate action here?
    strcpy(baseTopic, topic);
    command[0] = 0;
  }
  uint8_t id = ts.fromTopicBase(baseTopic);
  // TODO switches with strings...does that work at all?
  if (strcmp(command, STATE_COMMAND) == 0) {
    // Do something?
  } else if (strcmp(command, SET_COMMAND) == 0) {
    // Send change state via RF
  } else if (strcmp(command, TIMESTAMP_COMMAND) == 0) {
    // Possibly add a separate timestamp request channel this one doesn't seem
    // to hae a purpose for now
  }
}

void setup()
{
  Serial.begin(SERIAL_BAUD);
  delay(10);
  RadioNode::setupRadio(FREQUENCY, NODEID, NETWORKID, IS_RFM69HW, ENCRYPTKEY);
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
    char value[RF69_MAX_DATA_LEN - HEADER];

    RadioNode::readRadio(&header, value);
    switch (header.packetType) {
      case REGISTRATION:
        handleRegistration(header, value);
        break;
      case LIGHT_STATUS:
        lightStatusChange(header, *(LightStatus*) value);
        break;
    }
  }
  client.loop();
}

void handleRegistration(RadioHeader header, char *topic) 
{
  Registration reg;
  reg.id = header.id;
  strcpy(reg.topic, topic);
  bool success = ts.writeRegistration(reg);
  if (success) {
    char subscription[TOPIC_LENGTH + 2];
    strcpy(subscription, reg.topic);
    strcat(subscription, "/*");
  } else {
    Serial.println(F("Unable to register, out of space"));
  }
}

void lightStatusChange(RadioHeader header, LightStatus lStatus)
{
  // TODO WIP
  char topic[TOPIC_LENGTH + 6];
  bool result = ts.getTopicBase(topic, header.id);
  if (result) {
    //TODO concat topic and command
    //topic
    client.publish(topic, (char*)lStatus.status);
  } else {
    Serial.println(F("Unable to locate topic!"));
  }
}
