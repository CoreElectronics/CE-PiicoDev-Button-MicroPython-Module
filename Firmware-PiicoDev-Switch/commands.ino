/*
  User accessible functions
*/

// Macro for number of elements in an array
#define COUNT_OF(x) ((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))

void readPressCount(char *data) {
  loadArray((uint16_t)valueMap.pressCount);
  valueMap.pressCount = 0;
}

void idReturn(char *data) {
  loadArray((uint16_t)valueMap.id);
}

void firmwareMajorReturn(char *data) {
  loadArray((uint8_t)valueMap.firmwareMajor);
}

void firmwareMinorReturn(char *data) {
  loadArray((uint8_t)valueMap.firmwareMinor);
}

void readState(char *data) {
  valueMap.state = digitalRead(switchPin);
  loadArray((uint8_t)valueMap.state);
}

void readDoubleClickDetected(char *data) {
  loadArray((uint8_t)valueMap.doubleClickDetected);
  valueMap.doubleClickDetected = 0;
}

void readWasPressed(char *data) {
  loadArray((uint8_t)valueMap.wasPressed);
  valueMap.wasPressed = false;
}

void powerLed(bool state) {
  if (state) {
    digitalWrite(powerLedPin, true);
  } else {
    digitalWrite(powerLedPin, false);
  }
}

void getPowerLed(char *data) {
  valueMap.led = digitalRead(powerLedPin);
  loadArray((uint8_t)valueMap.led);
}

void getDoubleClickDuration(char *data) {
  loadArray(valueMap.doubleClickDuration);
}

void getEMAParameter(char *data) {
  loadArray(valueMap.emaParameter);
}

void getEMAPeriod(char *data) {
  loadArray(valueMap.emaPeriod);
}

// Control the power LED
void setPowerLed(char *data) {
  powerLed( (data[0] == 1) );
}

void setDoubleClickDuration(char *data) {
  valueMap.doubleClickDurationWrite = (uint8_t(data[0]) << 8) + uint8_t(data[1]);
  valueMap.doubleClickDuration = valueMap.doubleClickDurationWrite;
}

void setEMAParameter(char *data) {
  valueMap.emaParameterWrite = data[0];
  valueMap.emaParameter = valueMap.emaParameterWrite;
}

void setEMAPeriod(char *data) {
  valueMap.emaPeriodWrite = data[0];
  valueMap.emaPeriod = valueMap.emaPeriodWrite;
}

void setAddress(char *data) {
  uint8_t tempAddress = data[0];

  if (tempAddress < 0x08 || tempAddress > 0x77)
    return; // Command failed. This address is out of bounds.
  valueMap.i2cAddress = tempAddress;

  EEPROM.put(LOCATION_ADDRESS_TYPE, SOFTWARE_ADDRESS);
  updateFlag = true; // will trigger a I2C re-initalise and save custom address to EEPROM
}

// Functions to load data into the response buffer
void loadArray(uint8_t myNumber)
{
  for (uint8_t x = 0 ; x < sizeof(myNumber) ; x++)
    responseBuffer[x] = (myNumber >> (((sizeof(myNumber) - 1) - x) * 8)) & 0xFF;
  responseSize = sizeof(myNumber);
}

void loadArray(uint16_t myNumber)
{
  for (uint8_t x = 0 ; x < sizeof(myNumber) ; x++)
    responseBuffer[x] = (myNumber >> (((sizeof(myNumber) - 1) - x) * 8)) & 0xFF;
  responseSize = sizeof(myNumber);
}
