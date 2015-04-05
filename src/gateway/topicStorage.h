#ifndef topicStorage_h
#define topicStorage_h

#include "Arduino.h"

// If this exceeds 128 bump the int8_t to int16_t
#define MAX_TOPICS 16
#define TS_EMPTY 0xFF

const uint8_t LREGISTRATION = sizeof(Registration);

class TopicStorage
{
    public:
        int8_t getAvailableIndex();
        bool getTopicBase(char* buff, int16_t id);
        uint8_t fromTopicBase(char* buff);
        Registration getRegistration(uint8_t index);
        void writeRegistration(RegistrationRequest registration);
        void writeRegistration(uint8_t index, RegistrationRequest registration);
        bool indexIsUsed(uint8_t index);
    private:
        uint8_t eepromMap[MAX_TOPICS] = {0}; 
        int8_t indexOf(uint8_t id);
        uint16_t makeAddress(uint8_t index);
};

#endif
