/******************************************************************************
  This file contains the interrupt routines that are triggered upon an I2C write from
  master (receiveEvent), an I2C read (requestEvent), or a button state change
  (buttonInterrupt). These ISRs modify the registerMap state variable, and sometimes
  set a flag (updateFlag) that updates things in the main loop.
******************************************************************************/

//Turn on interrupts for the various pins
void setupInterrupts() {
  //Attach interrupt to switch
  attachInterrupt(digitalPinToInterrupt(switchPin), buttonInterrupt, CHANGE);
}

// Executes when data is received on I2C
// this function is registered as an event, see setup() and/or startI2C()
void receiveEvent(int numberOfBytesReceived) {
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
      Serial.print(char(incomingData[incomingDataSpot - 1]), HEX);
      //incomingData is 32 bytes. We shouldn't spill over because receiveEvent can't receive more than 32 bytes
#endif
    }
  }
  for (int regNum = 0; regNum < (sizeof(memoryMap) / sizeof(byte)); regNum++)
  {
    if (functions[regNum].registerNumber == currentRegisterNumber)
    {
      valueMap.status &= ~(1 << STATUS_LAST_COMMAND_SUCCESS); //Assume command failed
      valueMap.status |= (1 << STATUS_LAST_COMMAND_KNOWN); //Assume command is known
      functions[regNum].handleFunction(incomingData);
    }
  }


  //Update the ButtonPressed and ButtonClicked queues.
  //If the user has requested to pop the oldest event off the stack then do so!
  if (readBit(valueMap.pressedQueueStatus, BIT_QUEUE_STATUS_POP_REQUEST)) {
    //Update the register with the next-oldest timestamp
    ButtonPressed.pop();
    valueMap.pressedQueueBack = ButtonPressed.back();

    //Update the status register with the state of the ButtonPressed buffer
    writeBit(valueMap.pressedQueueStatus, BIT_QUEUE_STATUS_IS_FULL, ButtonPressed.isFull());
    writeBit(valueMap.pressedQueueStatus, BIT_QUEUE_STATUS_IS_EMPTY, ButtonPressed.isEmpty());

    //Clear the popRequest bit so we know the popping is done
    clearBit(valueMap.pressedQueueStatus, BIT_QUEUE_STATUS_POP_REQUEST);
  }

  //If the user has requested to pop the oldest event off the stack then do so!
  if (readBit(valueMap.clickedQueueStatus, BIT_QUEUE_STATUS_POP_REQUEST)) {
    //Update the register with the next-oldest timestamp
    ButtonClicked.pop();
    valueMap.clickedQueueBack = ButtonClicked.back();

    //Update the status register with the state of the ButtonClicked buffer
    writeBit(valueMap.clickedQueueStatus, BIT_QUEUE_STATUS_IS_FULL, ButtonClicked.isFull());
    writeBit(valueMap.clickedQueueStatus, BIT_QUEUE_STATUS_IS_EMPTY, ButtonClicked.isEmpty());

    //Clear the popRequest bit so we know the popping is done
    clearBit(valueMap.clickedQueueStatus, BIT_QUEUE_STATUS_POP_REQUEST);
  }

  updateFlag = true; //Update things like LED brightnesses in the main loop
}

//Respond to GET commands
//When Qwiic Button gets a request for data from the user, this function is called as an interrupt
//The interrupt will respond with bytes starting from the last byte the user sent to us
//While we are sending bytes we may have to do some calculations
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
  writeBit(valueMap.buttonStatus, BIT_STATUS_EVENT_IS_PRESSED, !digitalRead(switchPin)); //have to take the inverse of the switch pin because the switch is pulled up, not pulled down

  //Calculate time stamps before we start sending bytes via I2C
  valueMap.pressedQueueBack = millis() - ButtonPressed.back();
  valueMap.pressedQueueFront = millis() - ButtonPressed.front();

  valueMap.clickedQueueBack = millis() - ButtonClicked.back();
  valueMap.clickedQueueFront = millis() - ButtonClicked.front();

  //This will write the entire contents of the register map struct starting from
  //the register the user requested, and when it reaches the end the master
  //will read 0xFFs.

  //Wire.write((registerPointer + registerNumber), sizeof(memoryMap) - registerNumber);
}

//Called any time the pin changes state
void buttonInterrupt() {

  //Debounce
  if (millis() - lastClickTime < valueMap.buttonDebounceTime)
    return;
  lastClickTime = millis();

  setBit(valueMap.buttonStatus, BIT_STATUS_EVENT_AVAILABLE);

  //Update the ButtonPressed queue and registerMap
  writeBit(valueMap.buttonStatus, BIT_STATUS_EVENT_IS_PRESSED, !digitalRead(switchPin)); //Take the inverse of the switch pin because the switch is pulled up
  ButtonPressed.push(millis() - valueMap.buttonDebounceTime);
  writeBit(valueMap.pressedQueueStatus, BIT_QUEUE_STATUS_IS_EMPTY, ButtonPressed.isEmpty());
  writeBit(valueMap.pressedQueueStatus, BIT_QUEUE_STATUS_IS_FULL, ButtonPressed.isFull());

  //Update the ButtonClicked queue and registerMap if necessary
  if (digitalRead(switchPin) == HIGH) { //User has released the button, we have completed a click cycle
    //update the ButtonClicked queue and then registerMap
    setBit(valueMap.buttonStatus, BIT_STATUS_EVENT_HAS_BEEN_CLICKED);
    ButtonClicked.push(millis() - valueMap.buttonDebounceTime);
    writeBit(valueMap.clickedQueueStatus, BIT_QUEUE_STATUS_IS_EMPTY, ButtonClicked.isEmpty());
    writeBit(valueMap.clickedQueueStatus, BIT_QUEUE_STATUS_IS_FULL, ButtonClicked.isFull());
  }

}
