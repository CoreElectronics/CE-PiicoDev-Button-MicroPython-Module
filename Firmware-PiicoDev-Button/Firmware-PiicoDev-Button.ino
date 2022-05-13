/*
   PiicoDev Button Firmware
   Written by Peter Johnston @ Core Electronics
   Based off the Qwiic Button Project https://github.com/sparkfun/Qwiic_Button
   Date: May 2022
   An I2C based module that reads the PiicoDev Button

   Feel like supporting PiicoDev? Buy a module here: https://core-electronics.com.au/catalog/product/view/sku/CE08500

*/

// ToDo: Make mamory map a seperate struct to valuemap
#define DEBUG 1

#include <Wire.h>
#include <EEPROM.h>
#include "queue.h"
#include <avr/sleep.h> //Needed for sleep_mode
#include <avr/power.h> //Needed for powering down perihperals such as the ADC/TWI and Timers

#define DEVICE_ID 409
#define FIRMWARE_VERSION 1
#define DEFAULT_I2C_ADDRESS 0x42
#define SOFTWARE_ADDRESS true
#define HARDWARE_ADDRESS false
#define I2C_BUFFER_SIZE 32 //For ATmega328 based Arduinos, the I2C buffer is limited to 32 bytes
#define BIT_STATUS_EVENT_AVAILABLE 0
#define BIT_STATUS_EVENT_HAS_BEEN_CLICKED 1
#define BIT_STATUS_EVENT_IS_PRESSED 2
#define BIT_QUEUE_STATUS_POP_REQUEST 0
#define BIT_QUEUE_STATUS_IS_EMPTY 1
#define BIT_QUEUE_STATUS_IS_FULL 2
#define BIT_INTERRUPT_CONFIG_CLICKED_ENABLE 0
#define BIT_INTERRUPT_CONFIG_PREDDED_ENABLE 1

//Location in EEPROM for each thing we want to store between power cycles
enum eepromLocations {
  LOCATION_I2C_ADDRESS = 0x00, //Device's address
  LOCATION_INTERRUPTS = 0x01,
  LOCATION_BUTTON_DEBOUNCE_TIME = 0x08,
  LOCATION_ADDRESS_TYPE = 0x0A, // Address type can be either hardware defined (jumpers/switches), or software defined by user.
};

uint8_t oldAddress;

//Hardware connections
// Prototyping with Arduino Uno
#if defined(__AVR_ATmega328P__)
const uint8_t powerLedPin = 3;
const int switchPin = 0;
const int addressPin1 = 8;
const int addressPin2 = 7;
const int addressPin3 = 6;
const int addressPin4 = 5;
#else
// ATTINY 8x6 or 16x6
const uint8_t powerLedPin = PIN_PA2;
const uint8_t switchPin = PIN_PA7;
const uint8_t addressPin1 = PIN_PA1;
const uint8_t addressPin2 = PIN_PC3;
const uint8_t addressPin3 = PIN_PC2;
const uint8_t addressPin4 = PIN_PC1;
#endif


// struct statusRegisterBitField {
//     bool eventAvailable : 1; //This is bit 0. User mutable, gets set to 1 when a new event occurs. User is expected to write 0 to clear the flag.
//     bool hasBeenClicked : 1; //Defaults to zero on POR. Gets set to one when the button gets clicked. Must be cleared by the user.
//     bool isPressed : 1;  //Gets set to one if button is pushed.
//     bool: 5;
// };
// //  uint8_t byteWrapped;
// //} statusRegisterBitField;
// //
// //typedef union {
// //  struct {
// //    bool clickedEnable : 1; //This is bit 0. user mutable, set to 1 to enable an interrupt when the button is clicked. Defaults to 0.
// //    bool pressedEnable : 1; //user mutable, set to 1 to enable an interrupt when the button is pressed. Defaults to 0.
// //    bool: 6;
// //  };
// //  uint8_t byteWrapped;
// //} interruptConfigBitField;
// //
// union queueStatusBitField {
//   struct {
//     bool popRequest : 1; //This is bit 0. User mutable, user sets to 1 to pop from queue, we pop from queue and set the bit back to zero.
//     bool isEmpty : 1; //user immutable, returns 1 or 0 depending on whether or not the queue is empty
//     bool isFull : 1; //user immutable, returns 1 or 0 depending on whether or not the queue is full
//     bool: 5;
//   };
//  uint8_t byteWrapped;
// };

//These are the different types of data the device can respond with
enum Response {
  RESPONSE_STATUS, //1 byte containing status bits
  RESPONSE_VALUE, //Value byte containing measurements etc.
};

volatile Response responseType = RESPONSE_STATUS; //State engine that let's us know what the master is asking for
byte responseBuffer[I2C_BUFFER_SIZE]; //Used to pass data back to master
volatile byte responseSize = 1; //Defines how many bytes of relevant data is contained in the responseBuffer

#define STATUS_LAST_COMMAND_SUCCESS 1
#define STATUS_LAST_COMMAND_KNOWN 2

struct memoryMap {
  uint16_t id;
  uint8_t status;
  uint16_t firmwareVersion;
  uint8_t i2cAddress;
  uint8_t led;
  uint8_t interruptConfigure;
  uint16_t buttonDebounceTime;
  uint8_t buttonStatus;
  uint8_t pressedQueueStatus;
  unsigned long pressedQueueFront;
  unsigned long pressedQueueBack;
  uint8_t clickedQueueStatus;
  unsigned long clickedQueueFront;
  unsigned long clickedQueueBack;
};

// Register addresses.
const memoryMap registerMap = {
  .id = 0x00,
  .status = 0x01,
  .firmwareVersion = 0x02,
  .i2cAddress = 0x03,
  .led = 0x04,
  .interruptConfigure = 0x10,
  .buttonDebounceTime = 0x11,
  .buttonStatus = 0x20,
  .pressedQueueStatus = 0x21,
  .pressedQueueFront = 0x22,
  .pressedQueueBack = 0x23,
  .clickedQueueStatus = 0x24,
  .clickedQueueFront = 0x25,
  .clickedQueueBack = 0x26,
};

volatile memoryMap valueMap = {
  .id = DEVICE_ID,
  .status = 0x00,
  .firmwareVersion = FIRMWARE_VERSION,
  .i2cAddress = DEFAULT_I2C_ADDRESS,
  .led = 0x01,
  .interruptConfigure = 0x03, //{clickedEnable, pressedEnable}
  .buttonDebounceTime = 0x000A,
  .buttonStatus = 0x00, //  {eventAvailable, hasBeenClicked, isPressed}
  .pressedQueueStatus = 0x02, //{popRequest, isEmpty, isFull}
  .pressedQueueFront = 0x00,
  .pressedQueueBack = 0x00,
  .clickedQueueStatus = 0x02, //{popRequest, isEmpty, isFull}
  .clickedQueueFront = 0x00,
  .clickedQueueBack = 0x00,
};

uint8_t currentRegisterNumber;

struct functionMap {
  byte registerNumber;
  void (*handleFunction)(char *myData);
};

void idReturn(char *data);
void firmwareVersionReturn(char *data);
void setAddress(char *data);
void setPowerLed(char *data);

functionMap functions[] = {
  {registerMap.id, idReturn},
  {registerMap.firmwareVersion, firmwareVersionReturn},
  {registerMap.i2cAddress, setAddress},
  {registerMap.led, setPowerLed},
};

//volatile uint8_t registerNumber; //Gets set when user writes an address. We then serve the spot the user requested.

// System global variables
volatile boolean updateFlag = true; //Goes true when we receive new bytes from user. Causes LEDs and things to update in main loop.
volatile unsigned long lastSyncTime = 0;

#define LOCAL_BUFFER_SIZE 20 // bytes
byte incomingData[LOCAL_BUFFER_SIZE]; //Local buffer to record I2C bytes before committing to file, add 1 for 0 character on end
volatile int incomingDataSpot = 0; //Keeps track of where we are in the incoming buffer

volatile Queue ButtonPressed, ButtonClicked; //Init FIFO buffer for storing timestamps associated with button presses and clicks

volatile unsigned long lastClickTime = 0; //Used for debouncing

void setup() {
  // Pull up address pins
  pinMode(addressPin1, INPUT_PULLUP);
  pinMode(addressPin2, INPUT_PULLUP);
  pinMode(addressPin3, INPUT_PULLUP);
  pinMode(addressPin4, INPUT_PULLUP);
  pinMode(powerLedPin, OUTPUT);
  powerLed(true); // enable Power LED by default on every power-up
  pinMode(switchPin, INPUT_PULLUP); //GPIO with internal pullup, goes low when button is pushed

  //Power down various bits of hardware to lower power usage
  set_sleep_mode(SLEEP_MODE_IDLE);
  sleep_enable();

#if defined(__AVR_ATmega328P__)
  for (int x = 0; x < 100; x++) {
    EEPROM.put(x, 0xFF);
  }
  Serial.begin(115200);
  Serial.println("Qwiic Button");
  Serial.print("Address: 0x");
  Serial.println(registerMap.i2cAddress, HEX);
  Serial.print("Device ID: 0x");
  Serial.println(registerMap.id, HEX);
#endif

  readSystemSettings(&registerMap); //Load all system settings from EEPROM

  setupInterrupts();               //Enable pin change interrupts for I2C, switch, etc
  startI2C(&registerMap);          //Determine the I2C address we should be using and begin listening on I2C bus
  oldAddress = registerMap.i2cAddress;
}

void loop() {
  // Check to see if the I2C address has been updated by software, set the appropriate address-type flag
  if (oldAddress != registerMap.i2cAddress)
  {
    oldAddress = registerMap.i2cAddress;
    EEPROM.put(LOCATION_ADDRESS_TYPE, SOFTWARE_ADDRESS);
  }
  if (updateFlag) {
    //Record anything new to EEPROM (like new LED values)
    //It can take ~3.4ms to write a byte to EEPROM so we do that here instead of in an interrupt
    recordSystemSettings(&registerMap);

    updateFlag = false; //clear flag
  }

  sleep_mode();             //Stop everything and go to sleep. Wake up if I2C event occurs.
}

//Update own I2C address to what's configured with registerMap.i2cAddress and/or the address jumpers.
void startI2C(memoryMap *map)
{
  uint8_t address;
  uint8_t addressType;
  EEPROM.get(LOCATION_ADDRESS_TYPE, addressType);

  if (addressType == 0xFF)
  {
    EEPROM.put(LOCATION_ADDRESS_TYPE, SOFTWARE_ADDRESS);
  }

  // Add hardware address jumper values to the default address
  uint8_t IOaddress = DEFAULT_I2C_ADDRESS;
  uint8_t switchPositions = 0;
  bitWrite(switchPositions, 0, !digitalRead(addressPin1));
  bitWrite(switchPositions, 1, !digitalRead(addressPin2));
  bitWrite(switchPositions, 2, !digitalRead(addressPin3));
  bitWrite(switchPositions, 3, !digitalRead(addressPin4));
  IOaddress += switchPositions;

  //If any of the address jumpers are set, we use jumpers
  if ((IOaddress != DEFAULT_I2C_ADDRESS) || (addressType == HARDWARE_ADDRESS))
  {
    address = IOaddress;
    EEPROM.put(LOCATION_ADDRESS_TYPE, HARDWARE_ADDRESS);
  }
  //If none of the address jumpers are set, we use registerMap (but check to make sure that the value is legal first)
  else
  {
    //if the value is legal, then set it
    if (map->i2cAddress > 0x07 && map->i2cAddress < 0x78)
      address = map->i2cAddress;

    //if the value is illegal, default to the default I2C address for our platform
    else
      address = DEFAULT_I2C_ADDRESS;
  }

  //save new address to the register map
  map->i2cAddress = address;

  //reconfigure Wire instance
  Wire.end();          //stop I2C on old address
  Wire.begin(address); //rejoin the I2C bus on new address

  //The connections to the interrupts are severed when a Wire.begin occurs, so here we reattach them
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);
}
