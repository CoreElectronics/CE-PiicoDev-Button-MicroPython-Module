// Executes when data is received on I2C
// this function is registered as an event, see setup() and/or startI2C()
void receiveEvent(uint16_t numberOfBytesReceived)
{
  lastSyncTime = millis();
  incomingDataSpot = 0;

  memset(incomingData, 0, sizeof(incomingData));
  while (Wire.available())
  {
    currentRegisterNumber = Wire.read();
    while (Wire.available())
    {
      incomingData[incomingDataSpot++] = Wire.read();
      #if DEBUG
      Serial.print(char(incomingData[incomingDataSpot-1]), HEX);
      //incomingData is 32 bytes. We shouldn't spill over because receiveEvent can't receive more than 32 bytes
      #endif
    }
  }
  for (uint16_t regNum = 0; regNum < (sizeof(memoryMap) / sizeof(uint8_t)); regNum++)
  {
    if (functions[regNum].registerNumber == currentRegisterNumber)
    {
      valueMap.status &= ~(1 << STATUS_LAST_COMMAND_SUCCESS); //Assume command failed
      valueMap.status |= (1 << STATUS_LAST_COMMAND_KNOWN); //Assume command is known
      functions[regNum].handleFunction(incomingData);
    }
  }
}

void requestEvent() {
  lastSyncTime = millis();
  switch (responseType)
  {
    case RESPONSE_STATUS:
      // Respond with the system status byte
      Wire.write(valueMap.status);

      // Once read, clear the last command known and last command success bits
      valueMap.status &= ~(1 << STATUS_LAST_COMMAND_SUCCESS);
      valueMap.status &= ~(1 << STATUS_LAST_COMMAND_KNOWN);
      break;

    case RESPONSE_VALUE:
      // Respond to other queries eg. firmware version, device ID
      Wire.write(responseBuffer, responseSize);

      responseType = RESPONSE_STATUS; //Return to default state
      break;

    default:
      Wire.write(0x80); //Unknown response state
      break;
  }
}

uint32_t timeBuff[3];
uint8_t pos = 0;
void buttonEvent() {
  debugln("Button Pressed");
  uint32_t now = millis();
  if (now - buttonPressTime > valueMap.debounceDelay) {
    valueMap.pressCount++;
    pos++;
    timeBuff[pos % 3] = now;
  }
  if (timeBuff[pos % 3] - timeBuff[(pos-1) % 3] < valueMap.doubleClickDuration){
    valueMap.status |= (1 << STATUS_DOUBLE_CLICK);
    valueMap.debug = valueMap.doubleClickDuration;
    debugln("here 1");
  }
  else {
    
    valueMap.debug = 12;
    debugln("here 2");
  }
  debug("Double Click Calculation: ");
  debugln(timeBuff[pos % 3]);
  debugln(timeBuff[(pos-1) % 3]);
  debugln(timeBuff[pos % 3] - timeBuff[(pos-1) % 3]);
  debugln(valueMap.doubleClickDuration);
  //valueMap.debug = pos;
  buttonPressTime = millis();
}
