#include "radioNode.h"
#include "debounced.h"

#define GATEWAYID 1;

Debounced lightSwitch = Debounced(4, &switchAction);
bool enableLight = false;
const int HEADER = sizeof(RadioHeader);

void setup() {
  Serial.begin(SERIAL_BAUD);
  delay(10)
  RadioNode::setupRadio();
}

void loop() {
  // read radio for commands
  // read switch for changes
  lightSwtich.readButton();
}

void switchAction(int state)
{
  enableLight = !enableLight;
  byte[RF69_MAX_DATA_LEN] data;
  getPacket(data);
  RadioNode::sendData()
}

void getPacket(byte *buff)
{
  RadioHeader header = (RadioHeader) { NODEID, millis(), LIGHT_STATUS };
  memcpy(buff, &header, HEADER);

  LightStatus body = (LightStatus) { enableLight };
  memcpy(&buff[HEADER], &body, sizeof(LightStatus));
}
