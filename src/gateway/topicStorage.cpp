#include "Arduino.h"
#include "topicStorage.h"
#include "controls.h"

TopicStorage::TopicStorage()
{
    /*
     * Initialize the existing topics stored in eeprom
     * All addresses in EEPROM should be initialized to 0xFF.
     * If we read the start of a topic and see a value other than 0xFF
     * then we can infer there is a topic written there.
    */
    for (int i = 0; i < MAX_TOPICS; i++) {
        uint16_t address = makeAddress(i);
        uint8_t id = EEPROM.read(address);
        if (id != TS_EMPTY) {
            eepromMap[i] = id;
        }
    }
}

TopicStorage::getAvailableIndex()
{
    int8_t index = -1;
    for (int8_t i = 0; i < MAX_TOPICS; i++) {
        if (eepromMap[i] == 0) 
            index = i;
    }
    return index;
}

TopicStorage::getTopicBase(char* buff, int id)
{
    /*
        Load the topic into buff by reading it from eeprom.
        If the id isn't found in the eepromMap then return false
    */
    index = indexOf(id);
    if (index == -1) 
        return false;
    Registration reg = getRegistration(index);
    strcpy(buff, reg.topic);
    return true;
}

TopicStorage::fromTopicBase(char* buff)
{
    /*
     Given a topic base find the matching node and control id
    */
    for (uint8_t i; i < MAX_TOPICS; i++) {
        Registration reg = getRegistration(i);
        if (strcmp(buff, reg.topic) == 0) {
            return reg.id;
        }
    }
    // impossible id when not found;
    return TS_EMPTY;
}

TopicStorage::indexOf(uint8_t id)
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

TopicStorage::getRegistration(uint8_t index)
{
    uint16_t address = makeAddress(index);
    for (int i = 0; i < LREGISTRATION; i++, adress++) {
        buff[i] = EEPROM.read(address);
    }
    return *(Registration*) buff;
}

TopicStorage:writeRegistration(uint8_t index, Registration reg)
{
    uint16_t address = makeAddress(index);
    uint8_t buff[LREGISTRATION];
    memcpy(buff, (const void*) reg);
    if (index >= 0) {
        for (int i = 0; i < LREGISTRATION; i++, address++) {
            EEPROM.write(address, buff[i]);
        }
    }
}

TopicStorage:writeRegistration(Registration reg)
{
    // Note that eepromWrite is SLOW.  Registering nodes should avoid
    // sending registrations too quickly.  3.3ms per byte * 64 bytes = 212ms
    uint8_t index = getAvailableIndex();
    writeRegistration(index, id, reg);
}

TopicStorage::addressIsUsed(uint8_t index)
{
  for (int i = 0; i < LREGISTRATION; i++, address++) {
    uint8_t val = EEPROM.read(address);
    if (val != 0xFF) {
      // 0xFF is the default cleared state for EEPROM according to 
      // Arduino docs.  It also happens to be an impossible id since
      // NodeId 15 is reserved
      return true;
    }
  }
  return false;
}

TopicStorage::makeAddress(uint8_t index)
{
    return (uint16_t) index * LREGISTRATION;
}
