#ifndef topicStorage_h
#define topicStorage_h

#include "Arduino.h"
#include <controls.h>
// If this exceeds 128 bump the int8_t to int16_t
#define MAX_TOPICS 16
#define TS_EMPTY 0xFF

const uint8_t LREGISTRATION = sizeof(Registration);

class TopicStorage
{
    public:
        int8_t getAvailableIndex();
        bool getTopicBase(char *buff, int16_t id);
        bool fromTopicBase(char *buff, Registration *reg);
        Registration getRegistration(uint8_t index);
        bool writeRegistration(Registration reg);
        void writeRegistration(uint8_t index, Registration reg);
        bool indexIsUsed(uint8_t index);
        void printAllRegistrations();
        TopicStorage();
    private:
        // Values stored as 2 nibbles.  First nibble is the nodeid, second 
        // nibble is the sensorid.  NodeId 15 (F) is reserved (gateway).  
        // TODO possible expansion to uint16_t to support many more nodes and sensors
        uint8_t eepromMap[MAX_TOPICS] = {0}; 
        int8_t indexOf(uint8_t id);
        uint16_t makeAddress(uint8_t index);
};

#endif
