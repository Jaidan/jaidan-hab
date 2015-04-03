#include "../common/secure.h"
#include "config.h"
#include "../common/defines.h"
#include <EEPROM.h>

#include <RFM69.h>
#include <SPI.h>
#include <PubSubClient.h>
#include <Ethernet.h>
#include "radioNode.h"

#define MAX_NODES 16
#define TOPIC_LENGTH

uint8_t mac[] = {  0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0xED }; // get real mac
uint8_t server[] = { 192, 168, 0, 1 };
uint8_t[4] ip;

const uint8_t HEADER = sizeof(RadioHeader);
uint8_t knownNodes = 0;
uint8_t[MAX_NODES] nodes;

RFM69 radio;
EthernetClient ethClient;
PubSubClient client(server, 1883, callback, ethClient);

void callback(char* topic, byte* payload, unsigned int length) {
    // handle message arrived
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
  loadNodes();
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
    char[64] topic;

    RadioNode::readRadio(&header, value);
    getTopicBase(topic, header.nodeId - 1);
    switch header.packetType {
      case REGISTRATION:
        handleRegistration(header, (Registration*) value);
        break;
      case LIGHT_STATUS:
        lightStatusChange(header, (LightStatus*) value);
        break;
    }
  }
}

void loadNodes()
  /*
     All addresses in EEPROM should be initialized to 0xFF.
     If we read the start of a topic and see a value other than 0xFF
     then we can infer there is a topic written there.
  */
{
  for (int i = 0; i < MAX_NODES; i++) {
    uint8_t address = i * TOPIC_LENGTH;
    if (addressIsUsed()) {
      knownNodes++;
    }
  }
}

void getTopicBase(char *buff, int index)
{
  /*
     The topic are 64 bytes each max occupying 2 pages each.
     the start address is based on the index being accessed
     and the data is read into the provided buffer
  */
  int address = TOPIC_LENGTH * index; 
  for (int i = 0; i < TOPIC_LENGTH; i++, adress++) {
    buff[i] = EEPROM.read(address);
  }
}

void handleRegistration(RadioHeader header, Registration registration) 
{
  int address = TOPIC_LENGTH * (header.nodeId - 1);
  if (!addressIsUsed()) {
    for (int i = 0; i < TOPIC_LENGTH; i++, address++) {
      EEPROM.write(address, registration.topic[i]);
    }
    client.subscribe(registartion.topic);
    for (int i = 0; i < registration.commandCount; i++) {
      // Subscribe to command topics
      String topic = String(registration.topic);
      topic +=  String(registration.commands[i]);
      client.subscribe(topic);
    }
  } else {
    Serial.writeln(F("DUPLICATE REGISTRATION DETECTED"));
  }
}

void addressIsUsed(int address)
{
  for (int i = 0; i < TOPIC_LENGTH; i++, address++) {
    uint8_t val = EEPROM.read(address);
    if (val != 0xFF) {
      return true;
    }
  }
  return false;
}

void lightStatus(RadioHeader header, LightStatus lStatus)
{
  char[TOPIC_LENGTH] topic;
  getTopicBase(topic, header.nodeId - 1);
  client.publish(topic, (char*)lstatus.status);
}

