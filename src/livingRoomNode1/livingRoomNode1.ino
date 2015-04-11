#include "secure.h"
#include "config.h"
#include "defines.h"

#include <RFM69.h>
#include <SPI.h>
#include <radioNode.h>
#include <debounced.h>
#include <controls.h>

#define OUTDOOR_LIGHT1_ID 0

RFM69 radio;

const PROGMEM char outdoorLight1Topic[] = "jaidan/home/outdoors/light1";
SwitchedToggleControl outdoorLight1 = SwitchedToggleControl(1, 2, NODEID, OUTDOOR_LIGHT1_ID);  // Front house light

void setup()
{
  Serial.begin(SERIAL_BAUD);
  delay(10);
  RadioNode::setupRadio(FREQUENCY, NODEID, NETWORKID, IS_RFM69HW, ENCRYPTKEY);
  radio = RadioNode::getRadio();
}

void loop()
{
  outdoorLight1.loop();
  if (radio.receiveDone()) {
    RadioHeader header;
    char buff[LBODY];
    RadioNode::readRadio(&header, buff);
    uint8_t controlId = header.id & 0x0F;
    switch (controlId) {
      case OUTDOOR_LIGHT1_ID:
        outdoorLight1.setToggleControl((*(SwitchedToggle *)&buff).status);
        break;
    }
  }
}
