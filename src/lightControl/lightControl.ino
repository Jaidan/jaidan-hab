#include "../common/secure.h"
#include "config.h"
#include "defines.h"

#include <RFM69.h>
#include <SPI.h>
#include "radioNode.h"
#include "debounced.h"

const int HEADER = sizeof(RadioHeader);
RFM69 radio;

SwitchedToggleControl[] = {}; // customize for each node

void setup() {
  Serial.begin(SERIAL_BAUD);
  delay(10)
  RadioNode::setupRadio();
  // TODO replace with sensor registrations
  //RadioNode::registerNode();
  radio = RadioNode::getRadio();
}

void loop() {
  lightSwtich.readButton();
  if (radio.receiveDone()) {
    RadioHeader header;
    LightStatus value;
    RadioNode::readRadio(&header, (char*)value);
    enableLight = value.status ? RELAY_ON : RELAY_OFF;
  }
}

void switchAction(int state)
{
  enableLight = !enableLight;
  uint8_t[RF69_MAX_DATA_LEN] data;
  getPacket(data);
  RadioNode::sendData(data)
}

void getPacket(uint8_t *buff)
{
  RadioHeader header = (RadioHeader) { NODEID, millis(), LIGHT_STATUS };
  memcpy(buff, &header, HEADER);

  LightStatus body = (LightStatus) { enableLight };
  memcpy(&buff[HEADER], &body, sizeof(LightStatus));
}
