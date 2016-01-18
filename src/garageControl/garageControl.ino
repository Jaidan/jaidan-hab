#include "secure.h"
#include "config.h"
#include "defines.h"

#include <RFM69.h>
#include <SPI.h>
#include <radioNode.h>
#include <debounced.h>
#include <controls.h>

#define LEFT_GARAGE_UP_LIMIT_ID 0
#define RIGHT_GARAGE_UP_LIMIT_ID 3
#define LEFT_GARAGE_DOWN_LIMIT_ID 1
#define RIGHT_GARAGE_DOWN_LIMIT_ID 4

RFM69 radio;

const PROGMEM char garageLeftLightTopic[] = "jaidan/home/garage/leftDoorLight";
//TODO instantiate GarageLightControl SwitchedToggle maybe??? Has a timeout?
const PROGMEM char garageLeftControl[] = "jaidan/home/garage/leftDoor";
GarageLimitControlGroup leftGarageControlGroup(
  3, 4, NODEID, LEFT_GARAGE_UP_LIMIT_ID, LEFT_GARAGE_DOWN_LIMIT_ID);

const PROGMEM char garageRightLightTopic[] = "jaidan/home/garage/rightDoorLight";
//TODO instantiate GarageLightControl SwitchedToggle maybe??? Has a timeout?
const PROGMEM char garageRightControl[] = "jaidan/home/garage/rightDoor";
GarageLimitControlGroup rightGarageControlGroup(
  5, 6, NODEID, RIGHT_GARAGE_UP_LIMIT_ID, RIGHT_GARAGE_DOWN_LIMIT_ID);

void setup() {
  Serial.begin(SERIAL_BAUD);
  delay(10);
  RadioNode::setupRadio(
    FREQUENCY,
    NODEID,
    NETWORKID,
    IS_RFM69HW,
    ENCRYPTKEY
  );
  radio = RadioNode::getRadio();
}

void loop() 
{
  char input = Serial.read();
  if (Serial.available() > 0)
    RadioNode::executeCommand(input);
  leftGarageControlGroup.loop();
  rightGarageControlGroup.loop();
  if (radio.receiveDone()) {
    RadioHeader header;
    char buff[LBODY];
    RadioNode::readRadio(&header, buff);
    uint8_t controlId = header.id & 0x0F;
    switch (controlId) {
      //case OUTDOOR_LIGHT1_ID:
       // outdoorLight1.setToggleControl((*(SwitchedToggle *)&buff).status);
        break;
    }
  }
}
