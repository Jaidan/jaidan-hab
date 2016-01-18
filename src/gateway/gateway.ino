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

const PROGMEM char STATE_COMMAND[] = "/state";
const PROGMEM char SET_COMMAND[] = "/set";
const PROGMEM char TIMESTAMP_COMMAND[] = "/timestamp";

const char * const PROGMEM commands[] = { STATE_COMMAND, SET_COMMAND, TIMESTAMP_COMMAND };

RFM69 radio;
EthernetClient ethClient;
PubSubClient client(server, 1883, callback, ethClient);
TopicStorage ts;

void callback(char *topic, byte *payload, unsigned int length) {
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
  Registration reg;
  bool found = ts.fromTopicBase(baseTopic, &reg);
  if (found) {
    if (strcmp_P(command, STATE_COMMAND) == 0) {
      // Do something?
    } else if (strcmp_P(command, SET_COMMAND) == 0) {
      RadioHeader header = { reg.id, reg.controlType };
      switch(header.controlType) {
        case SWITCHEDTOGGLE_STATUS:
          setStateSwitchedToggle(&header, payload, length);
          break;
      }
      setStateSwitchedToggle(&header, payload, length);
    } else if (strcmp_P(command, TIMESTAMP_COMMAND) == 0) {
      // Possibly add a separate timestamp request channel this one doesn't seem
      // to have a purpose for now
    }
  }
}

void setStateSwitchedToggle(RadioHeader *header, byte *payload, unsigned int length)
{
  // Skip memcpy since it's only one byte
  SwitchedToggle body = (SwitchedToggle) { (bool)*payload };
  RadioNode::sendData((const RadioHeader *)header, (const void *)&body, sizeof(body));
}

void setup()
{
  Serial.begin(SERIAL_BAUD);
  delay(10);
  RadioNode::setupRadio(FREQUENCY, NODEID, NETWORKID, IS_RFM69HW, ENCRYPTKEY);
  radio = RadioNode::getRadio();
  if (Ethernet.begin(mac) == 0) {
    Serial.println(F("Failed to configure Ethernet using DHCP"));
    while(true);
  }
  ip = Ethernet.localIP();
  ts = TopicStorage();
}

void loop()
{
  if (!client.connected()) {
    if (!client.connect("arduinoClient")) {
      Serial.println(F("Failed to connect to MQTT server..."));
      delay(5000);
    }
  }
  char input = Serial.read();
  if (Serial.available() > 0)
    RadioNode::executeCommand(input);
  if (radio.receiveDone()) {
    RadioHeader header;
    char value[RF69_MAX_DATA_LEN - LHEADER];

    RadioNode::readRadio(&header, value);
    switch (header.packetType) {
      case REGISTRATION:
        handleRegistration(header, value);
        break;
      case SWITCHEDTOGGLE_STATUS:
        switchedToggleStatusChange(header, *(SwitchedToggle *) value);
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
    // TODO Consider sending these as a topic for fault logging...
    Serial.println(F("Unable to register, out of space"));
  }
}

void switchedToggleStatusChange(RadioHeader header, SwitchedToggle sStatus)
{
  /*
   * Publish a state change caused by a SwitchedToggle being mechanically
   * changed.  In other words; someone flipped the light switch
  */
  char topic[TOPIC_LENGTH + sizeof(STATE_COMMAND)];
  bool result = ts.getTopicBase(topic, header.id);
  if (result) {
    strcat_P(topic, STATE_COMMAND);
    client.publish(topic, (char *)sStatus.status);
  } else {
    Serial.println(F("Unable to locate topic!"));
  }
}
