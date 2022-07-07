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
      functions[regNum].handleFunction(incomingData);
    }
  }
}

void requestEvent() {
  lastSyncTime = millis();
  Wire.write(responseBuffer, responseSize);
}

uint32_t timeBuff[3];
uint8_t pos = 0;
void switchEvent() {
  debugln("Button Pressed");
  uint32_t now = millis();
  if (now - switchPressTime > valueMap.debounceDelay) {
    valueMap.pressCount++;
    pos++;
    timeBuff[pos % 3] = now;
  }
  if (timeBuff[pos % 3] - timeBuff[(pos-1) % 3] < valueMap.doubleClickDuration){
    valueMap.doubleClickDetected = 1;
    debugln("Double-Click Detected");
  }
  else {
    debugln("here 2");
  }
  debug("Double Click Calculation: ");
  debugln(timeBuff[pos % 3]);
  debug("timeBuff[(pos-1) % 3]: ");
  debugln(timeBuff[(pos-1) % 3]);
  debug("timeBuff[pos % 3] - timeBuff[(pos-1) % 3]: ");
  debugln(timeBuff[pos % 3] - timeBuff[(pos-1) % 3]);
  debug("valueMap.doubleClickDuration: ");
  debugln(valueMap.doubleClickDuration);
  switchPressTime = millis();
}
