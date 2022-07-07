/*
   PiicoDev Button Firmware
   Written by Peter Johnston @ Core Electronics
   Based off the Core Electronics Potentiometer module https://github.com/CoreElectronics/CE-PiicoDev-Buzzer-MicroPython-Module
   Date: 2022-07-04
   An I2C based module that reads the PiicoDev Button

   Feel like supporting PiicoDev? Buy a module here:
   PiicoDev Button: https://core-electronics.com.au/catalog/product/view/sku/CE08500

*/

#define DEBUG false

#if DEBUG == true
#define debug(x)     Serial.print(x)
#define debugln(x)   Serial.println(x)
#else
#define debug(x)
#define debugln(x)
#endif

#include <Wire.h>
#include <EEPROM.h>
#include <stdint.h>
#include <avr/sleep.h> // For sleep_mode
#include <avr/power.h> // For powering-down peripherals such as ADC and Timers

#define FIRMWARE_MAJOR 0x01
#define FIRMWARE_MINOR 0x00

#define DEVICE_ID 409
#define DEFAULT_I2C_ADDRESS 0x42    // The default address when all switches are off
#define I2C_ADDRESS_POOL_START 0x08 // The start of the 'smart module address pool' minus 1 - addresses settable by switches
#define SOFTWARE_ADDRESS true
#define HARDWARE_ADDRESS false
#define I2C_BUFFER_SIZE 32 // For ATmega328 based Arduinos, the I2C buffer is limited to 32 bytes
#define DOUBLE_CLICK_DURATION 300
#define DEBOUNCE_DELAY 40
#define EMA_PARAMETER 62
#define EMA_PERIOD 20 // ms

enum eepromLocations {
  LOCATION_I2C_ADDRESS = 0x00,  // Device's address
  LOCATION_ADDRESS_TYPE = 0x01, // Address type can be either hardware defined (jumpers/switches), or software defined by user.
};

uint8_t oldAddress;

// EMA Variables
float switchAvg;
bool switchOn;
bool switchOnPrev;

// Double-click detection variables
uint32_t timeBuff[3];
uint8_t pos = 0;

// Hardware Connectins
// Prototyping with Arduino Uno
#if defined(__AVR_ATmega328P__)
const uint8_t powerLedPin = 13;
const uint16_t switchPin = 3;
const uint16_t addressPin1 = 8;
const uint16_t addressPin2 = 7;
const uint16_t addressPin3 = 6;
const uint16_t addressPin4 = 5;
#else
// ATTINY 8x6 or 16x6
const uint8_t powerLedPin = PIN_PA3;
const uint16_t switchPin = PIN_PA7;
const uint8_t addressPin1 = PIN_PA1;
const uint8_t addressPin2 = PIN_PC3;
const uint8_t addressPin3 = PIN_PC2;
const uint8_t addressPin4 = PIN_PC1;
#endif

// System global variables
volatile bool updateFlag = true; // Goes true when new data received. Cause LEDs to update
volatile uint32_t lastSyncTime = 0;

#define LOCAL_BUFFER_SIZE 20 // bytes
uint8_t incomingData[LOCAL_BUFFER_SIZE]; // Local buffer to record I2C bytes before committing to file, add 1 for 0 character on end
volatile uint16_t incomingDataSpot = 0; // Keeps track of where we are in the incoming buffer

uint8_t responseBuffer[I2C_BUFFER_SIZE]; // Used to pass data back to master
volatile uint8_t responseSize = 1; // Defines how many bytes of relevant data is contained in the responseBuffer

struct memoryMap {
  uint16_t id;
  uint8_t firmwareMajor;
  uint8_t firmwareMinor;
  uint8_t i2cAddress;
  uint16_t pressCount;
  uint8_t led;
  uint8_t state;
  uint8_t doubleClickDetected;
  uint8_t wasPressed;
  uint16_t doubleClickDurationOut;
  uint16_t debounceDelayOut;
  uint8_t ledWrite;
  uint16_t doubleClickDuration;
  uint16_t debounceDelay;
};

// Register addresses.
const memoryMap registerMap = {
  .id = 0x11,
  .firmwareMajor = 0x02,
  .firmwareMinor = 0x03,
  .i2cAddress = 0x04,
  .pressCount = 0x05,
  .led = 0x07,
  .state = 0x08,
  .doubleClickDetected = 0x09,
  .wasPressed = 0x10,
  .doubleClickDurationOut = 0x21,
  .debounceDelayOut = 0x23,
  .ledWrite = 0x87,
  .doubleClickDuration = 0xA1,
  .debounceDelay = 0xA3,

};

volatile memoryMap valueMap = {
  .id = DEVICE_ID,
  .firmwareMajor = FIRMWARE_MAJOR,
  .firmwareMinor = FIRMWARE_MINOR,
  .i2cAddress = DEFAULT_I2C_ADDRESS,
  .pressCount = 0x00,
  .led = 0x01,
  .state = 0x00,
  .doubleClickDetected = 0x00,
  .wasPressed = 0,
  .doubleClickDurationOut = DOUBLE_CLICK_DURATION,
  .debounceDelayOut = DEBOUNCE_DELAY,
  .ledWrite = 0x01,
  .doubleClickDuration = DOUBLE_CLICK_DURATION,
  .debounceDelay = DEBOUNCE_DELAY,
};

uint8_t currentRegisterNumber;

struct functionMap {
  uint8_t registerNumber;
  void (*handleFunction)(char *myData);
};

void idReturn(char *data);
void firmwareMajorReturn(char *data);
void firmwareMinorReturn(char *data);
void setAddress(char *data);
void readPressCount(char *data);
void getPowerLed(char *data);
void readState(char *data);
void readDoubleClickDetected(char *data);
void readWasPressed(char *data);
void getDoubleClickDuration(char *data);
void getDebounceDelay(char *data);
void setPowerLed(char *data);
void setDoubleClickDuration(char *data);
void setDebounceDelay(char *data);

functionMap functions[] = {
  {registerMap.id, idReturn},
  {registerMap.firmwareMajor, firmwareMajorReturn},
  {registerMap.firmwareMinor, firmwareMinorReturn},
  {registerMap.i2cAddress, setAddress},
  {registerMap.pressCount, readPressCount},
  {registerMap.led, getPowerLed},
  {registerMap.state, readState},
  {registerMap.doubleClickDetected, readDoubleClickDetected},
  {registerMap.wasPressed, readWasPressed},
  {registerMap.doubleClickDurationOut, getDoubleClickDuration},
  {registerMap.debounceDelayOut, getDebounceDelay},
  {registerMap.ledWrite, setPowerLed},
  {registerMap.doubleClickDuration, setDoubleClickDuration},
  {registerMap.debounceDelay, setDebounceDelay},
};

void setup() {

#if DEBUG
  Serial.begin(115200);
#endif
  debugln("Begin");
  debugln(switchPin);

  // Pull up address pins
  pinMode(addressPin1, INPUT_PULLUP);
  pinMode(addressPin2, INPUT_PULLUP);
  pinMode(addressPin3, INPUT_PULLUP);
  pinMode(addressPin4, INPUT_PULLUP);
  pinMode(powerLedPin, OUTPUT);
  pinMode(switchPin, INPUT_PULLUP);
  powerLed(true); // enable Power LED by default on every power-up

  set_sleep_mode(SLEEP_MODE_IDLE);
  sleep_enable();
  readSystemSettings(); //Load all system settings from EEPROM


  startI2C();          //Determine the I2C address we should be using and begin listening on I2C bus
  oldAddress = valueMap.i2cAddress;
}

void loop() {
  float a = 0.25;
  if (updateFlag) {
    startI2C(); // reinitialise I2C with new address, update EEPROM with custom address as necessary
    updateFlag = false;
  }
  switchAvg = a * digitalRead(switchPin) + (1 - a) * switchAvg;
  if (switchAvg > 0.5) {
    switchOn = true;
  }
  if (switchAvg < 0.5) {
    switchOn = false;
  }
  if ((switchOn == false) && (switchOnPrev == true)) {
    switchEvent();
  }
  switchOnPrev = switchOn;

//TODO 
  //delay(valueMap.emaPeriod);
  delay(20);
}

void switchEvent() {
  debugln("Button Pressed");
  uint32_t now = millis();
  valueMap.pressCount++;
  valueMap.wasPressed = true;
  pos++;
  timeBuff[pos % 3] = now;
  if (timeBuff[pos % 3] - timeBuff[(pos - 1) % 3] < valueMap.doubleClickDuration) {
    valueMap.doubleClickDetected = 1;
    debugln("Double-Click Detected");
  }
}

// Begin listening on I2C bus as I2C slave using the global variable valueMap.i2cAddress
// ToDo don't use globals ie. pass in value map =>  void startI2C(memoryMap *map)
void startI2C()
{
  uint8_t address;
  uint8_t addressType;
  EEPROM.get(LOCATION_ADDRESS_TYPE, addressType);
  if (addressType == 0xFF) {
    EEPROM.put(LOCATION_ADDRESS_TYPE, SOFTWARE_ADDRESS);
  }

  // Add hardware address jumper values to the default address
  uint8_t IOaddress = DEFAULT_I2C_ADDRESS;
  uint8_t switchPositions = 0;
  bitWrite(switchPositions, 0, !digitalRead(addressPin1));
  bitWrite(switchPositions, 1, !digitalRead(addressPin2));
  bitWrite(switchPositions, 2, !digitalRead(addressPin3));
  bitWrite(switchPositions, 3, !digitalRead(addressPin4));
#if DEBUG
  Serial.print("switchPositions");
  Serial.println(switchPositions);
#endif
  if (switchPositions != 0) IOaddress = I2C_ADDRESS_POOL_START + switchPositions; // use the "smart-module address pool" when any hardware address is set

  // If any of the address jumpers are set, we use jumpers
  if ((IOaddress != DEFAULT_I2C_ADDRESS) || (addressType == HARDWARE_ADDRESS))
  {
    address = IOaddress;
    EEPROM.put(LOCATION_ADDRESS_TYPE, HARDWARE_ADDRESS);
  }
  // If none of the address jumpers are set, we use registerMap (but check to make sure that the value is legal first)
  else
  {
    // if the value is legal, then set it
    if (valueMap.i2cAddress > 0x07 && valueMap.i2cAddress < 0x78)
      address = valueMap.i2cAddress;

    // if the value is illegal, default to the default I2C address for our platform
    else
      address = DEFAULT_I2C_ADDRESS;
  }

  // save new address to the register map
  valueMap.i2cAddress = address;

  debug("I2C Address: ");
  debugln(address);
  recordSystemSettings(); // save the new address to EEPROM

  // reconfigure Wire instance
  Wire.end();          //stop I2C on old address
  Wire.begin(address); //rejoin the I2C bus on new address

  // The connections to the interrupts are severed when a Wire.begin occurs, so here we reattach them
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);
}
