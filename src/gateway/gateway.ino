#include <RFM69.h>
#include <SPI.h>
#include "radioNode.h"

RFM69 radio;
byte ackCount=0;
uint32_t packetCount = 0;

void setup()
{
  Serial.begin(SERIAL_BAUD);
  delay(10);
  RadioNode::setupRadio();
  radio = RadioNode::getRadio();
}

void loop()
{
  // check for new wireless data
  // open connection to mqtt
  // send data to mqtt
  char input = Serial.read();
  if (Serial.available() > 0)
    RadioNode::executeCommand(input);
  if (radio.receiveDone())
    char[RF69_MAX_DATA_LEN] value;
    value = RadioNode::readRadio();
}
