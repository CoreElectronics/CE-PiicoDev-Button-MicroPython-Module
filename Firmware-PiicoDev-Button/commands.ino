/*
  User accessible functions
*/

void idReturn(char *data) {
  responseType = RESPONSE_VALUE;
  loadArray((byte)valueMap.id);
  valueMap.status |= (1 << STATUS_LAST_COMMAND_SUCCESS); //Command success
}

void firmwareVersionReturn(char *data) {
  responseType = RESPONSE_VALUE;
  loadArray((byte)valueMap.firmwareVersion);
  valueMap.status |= (1 << STATUS_LAST_COMMAND_SUCCESS); //Command success
}

// Control the power LED - open drain output so toggle between enable (output, low) and disable (high-impedance input)
void setPowerLed(char *data) {
  powerLed( (data[0] == 1) );
}
void powerLed(bool state) {
  if (state) {
    pinMode(powerLedPin, OUTPUT);
    digitalWrite(powerLedPin, HIGH);
  } else {
    pinMode(powerLedPin, INPUT);
  }
}

void setAddress(char *data) {
  byte tempAddress = data[0];

  if (tempAddress < 0x08 || tempAddress > 0x77)
    return; //Command failed. This address is out of bounds.
  valueMap.i2cAddress = tempAddress;

  EEPROM.put(LOCATION_ADDRESS_TYPE, SOFTWARE_ADDRESS);
  valueMap.status |= (1 << STATUS_LAST_COMMAND_SUCCESS);
  updateFlag = true; // will trigger a I2C re-initalise and save custom address to EEPROM
}

//Loads a long into the start of the responseBuffer
void loadArray(unsigned long myNumber)
{
  for (byte x = 0 ; x < sizeof(myNumber) ; x++)
    responseBuffer[x] = (myNumber >> (((sizeof(myNumber) - 1) - x) * 8)) & 0xFF;
  responseSize = sizeof(myNumber);
}

void loadArray(long myNumber)
{
  for (byte x = 0 ; x < sizeof(myNumber) ; x++)
    responseBuffer[x] = (myNumber >> (((sizeof(myNumber) - 1) - x) * 8)) & 0xFF;
  responseSize = sizeof(myNumber);
}

//Loads an int into the start of the responseBuffer
void loadArray(int myNumber)
{
  for (byte x = 0 ; x < sizeof(myNumber) ; x++)
    responseBuffer[x] = (myNumber >> (((sizeof(myNumber) - 1) - x) * 8)) & 0xFF;
  responseSize = sizeof(myNumber);
}

void loadArray(unsigned int myNumber)
{
  for (byte x = 0 ; x < sizeof(myNumber) ; x++)
    responseBuffer[x] = (myNumber >> (((sizeof(myNumber) - 1) - x) * 8)) & 0xFF;
  responseSize = sizeof(myNumber);
}

//Loads an byte into the start of the responseBuffer
void loadArray(byte myNumber)
{
  for (byte x = 0 ; x < sizeof(myNumber) ; x++)
    responseBuffer[x] = (myNumber >> (((sizeof(myNumber) - 1) - x) * 8)) & 0xFF;
  responseSize = sizeof(myNumber);
}

//Loads a bool into the start of the responseBuffer
void loadArray(boolean myStatus)
{
  responseBuffer[0] = myStatus;
  responseSize = sizeof(myStatus);
}

uint8_t readBit(uint8_t x, int n) {
  return x & 1 << n != 0;
}

uint8_t setBit(uint8_t x, int n) {
    return x | (1 << n);
}

uint8_t clearBit(uint8_t x, int n) {
    return x & ~(1 << n);
}

uint8_t writeBit(uint8_t x, int n, bool b) {
    if (b == 0) {
        return clearBit(x, n);
    }
    else {
        return setBit(x, n);
    }
}