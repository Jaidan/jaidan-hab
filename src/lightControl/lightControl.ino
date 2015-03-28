#include "../common/secure.h"
#include "config.h"
#include "../common/defines.h"

#include <RFM69.h>
#include <SPI.h>
#include "radioNode.h"
#include "debounced.h"

Debounced lightSwitch = Debounced(4, &switchAction);
bool enableLight = RELAY_OFF;
const int HEADER = sizeof(RadioHeader);
RFM69 radio;

void setup() {
  Serial.begin(SERIAL_BAUD);
  delay(10)
  RadioNode::setupRadio();
  RadioNode::registerNode();
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
