#include "Arduino.h"
#include <avr/EEPROM.h>
#include "topicStorage.h"
#include <controls.h>

TopicStorage::TopicStorage()
{
    /*
     * Initialize the existing topics stored in eeprom
     * All addresses in EEPROM should be initialized to 0xFF.
     * If we read the start of a topic and see a value other than 0xFF
     * then we can infer there is a topic written there.
    */
    for (uint8_t i = 0; i < MAX_TOPICS; i++) {
        uint16_t address = makeAddress(i);
        uint8_t id = eeprom_read_byte((uint8_t *)address);
        if (id != TS_EMPTY) {
            eepromMap[i] = id;
        }
    }
}

int8_t TopicStorage::getAvailableIndex()
{
    int8_t index = -1;
    for (int8_t i = 0; i < MAX_TOPICS; i++) {
        if (eepromMap[i] == 0) 
            index = i;
    }
    return index;
}

bool TopicStorage::getTopicBase(char* buff, int id)
{
    /*
        Load the topic into buff by reading it from eeprom.
        If the id isn't found in the eepromMap then return false
    */
    int8_t index = indexOf(id);
    if (index == -1) 
        return false;
    Registration reg = getRegistration(index);
    strcpy(buff, reg.topic);
    return true;
}

bool TopicStorage::fromTopicBase(char* buff, Registration *reg)
{
    /*
     Given a topic base find the matching node and control id
    */
    for (uint8_t i; i < MAX_TOPICS; i++) {
        *reg = getRegistration(i);
        if (strcmp(buff, reg->topic) == 0) {
            return true;
        }
    }
    return false;
}

int8_t TopicStorage::indexOf(uint8_t id)
{
    // Naive search 
    int8_t index = -1;
    for (uint8_t i = 0; i < MAX_TOPICS; i++) {
        if (eepromMap[i] == id) {
            index = i;
            break;
        }
    }
    return index;
}

Registration TopicStorage::getRegistration(uint8_t index)
{
    uint16_t address = makeAddress(index);
    uint8_t buff[LREGISTRATION];
    eeprom_read_block((void *)buff, (const void *)address, LREGISTRATION);
    return *(Registration*) buff;
}

void TopicStorage::writeRegistration(uint8_t index, Registration reg)
{
    uint16_t address = makeAddress(index);
    uint8_t buff[LREGISTRATION];
    memcpy(buff, &reg, LREGISTRATION);
    eeprom_update_block((const void *)&reg, (void *)address, LREGISTRATION);
}

bool TopicStorage::writeRegistration(Registration reg)
{
    // Note that eepromWrite is SLOW.  Registering nodes should avoid
    // sending registrations too quickly.  3.3ms per byte * 64 bytes = 212ms
    int8_t index = getAvailableIndex();
    if (index < 0)
        return false;
    writeRegistration(index, reg);
    return true;
}

bool TopicStorage::indexIsUsed(uint8_t index)
{
    uint16_t address = makeAddress(index);
    for (int i = 0; i < LREGISTRATION; i++, address++) {
        uint8_t val = eeprom_read_byte((uint8_t *)address);
        if (val != 0xFF) {
            // 0xFF is the default cleared state for EEPROM according to 
            // Arduino docs.  It also happens to be an impossible id since
            // NodeId 15 is reserved
            return true;
        }
    }
    return false;
}

uint16_t TopicStorage::makeAddress(uint8_t index)
{
    return (uint16_t) index * LREGISTRATION;
}
