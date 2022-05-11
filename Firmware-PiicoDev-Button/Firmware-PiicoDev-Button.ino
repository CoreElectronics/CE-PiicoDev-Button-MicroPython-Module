/*
 * PiicoDev Button Firmware
 * Written by Peter Johnston @ Core Electronics
 * Based off the Qwiic Button Project https://github.com/sparkfun/Qwiic_Button
 * Date: May 2022
 * An I2C based module that reads the PiicoDev Button
 *
 * Feel like supporting PiicoDev? Buy a module here: https://core-electronics.com.au/catalog/product/view/sku/CE08500
 *
 */

#define DEBUG 1

#include <Wire.h>
#include <EEPROM.h>
#include "nvm.h"
#include "queue.h"
#include "registers.h"
#include "led.h"

// TRY THIS
// Disconnect the interrupt form the pin
// Re-attach the interrupt after a time triggered by another interrupt
//#include "PinChangeInterrupt.h" 1.2.9 //Nico Hood's library: https://github.com/NicoHood/PinChangeInterrupt/
//Used for pin change interrupts on ATtinys (encoder button causes interrupt)
  /*** NOTE, PinChangeInterrupt library NEEDS a modification to work with this code.
  *** you MUST comment out this line: 
  *** https://github.com/NicoHood/PinChangeInterrupt/blob/master/src/PinChangeInterruptSettings.h#L228
  */

#include <avr/sleep.h> //Needed for sleep_mode
#include <avr/power.h> //Needed for powering down perihperals such as the ADC/TWI and Timers

#define DEVICE_ID 0x5D
#define FIRMWARE_MAJOR 0x00 //Firmware Version. Helpful for tech support.
#define FIRMWARE_MINOR 0x01

#define DEFAULT_I2C_ADDRESS 0x42

#define SOFTWARE_ADDRESS true
#define HARDWARE_ADDRESS false

uint8_t oldAddress;

//Hardware connections
// Prototyping with Arduino Uno
#if defined(__AVR_ATmega328P__)
  const uint8_t powerLedPin = 3;
  const uint16_t potentiometerPin = 0;
  const int addressPin1 = 8;
  const int addressPin2 = 7;
  const int addressPin3 = 6;
  const int addressPin4 = 5;
  const uint8_t buttonPin = 2;
#else
  // ATTINY 8x6 or 16x6
  const uint8_t powerLedPin = PIN_PA2;
  const uint16_t potentiometerPin = PIN_PA7;
  
  const uint8_t addressPin1 = PIN_PA1;
  const uint8_t addressPin2 = PIN_PC3;
  const uint8_t addressPin3 = PIN_PC2;
  const uint8_t addressPin4 = PIN_PC1;
  const uint8_t buttonPin = PIN_PA7;
#endif

//Global variables
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//These are the defaults for all settings

// Initialise the virtual I2C registers
volatile memoryMap registerMap {
  DEVICE_ID,            //id
  FIRMWARE_MINOR,       //firmwareMinor
  FIRMWARE_MAJOR,       //firmwareMajor
  0x01,                  //power LED state
  DEFAULT_I2C_ADDRESS, //i2cAddress
  {0, 0, 0},            //buttonStatus {eventAvailable, hasBeenClicked, isPressed}
  {1, 1},              //interruptConfig {clickedEnable, pressedEnable}
  0x000A,              //buttonDebounceTime
  {0, 1, 0},           //pressedQueueStatus {popRequest, isEmpty, isFull}
  0x00000000,          //pressedQueueFront
  0x00000000,          //pressedQueueBack
  {0, 1, 0},           //clickedQueueStatus {popRequest, isEmpty, isFull}
  0x00000000,          //clickedQueueFront
  0x00000000,          //clickedQueueBack
};

// Define which registers are user-modifiable. (0 = protected, 1 = modifiable)
memoryMap protectionMap = {
  0x42,       //id
  0x00,       //firmwareMinor
  0x00,       //firmwareMajor
   0xFF,       //power LED state 
    0xFF,                                             //i2cAddress
  {1, 1, 1},  //buttonStatus {eventAvailable, hasBeenClicked, isPressed}
  {1, 1},     //interruptConfig {clickedEnable, pressedEnable}
  0xFFFF,     //buttonDebounceTime
  {1, 0, 0},  //pressedQueueStatus {popRequest, isEmpty, isFull}
  0x00000000, //pressedQueueFront
  0x00000000, //pressedQueueBack
  {1, 0, 0},  //clickedQueueStatus {popRequest, isEmpty, isFull}
  0x00000000, //clickedQueueFront
  0x00000000, //clickedQueueBack
  0xFF,       //i2cAddress

};

//Cast 32bit address of the object registerMap with uint8_t so we can increment the pointer
uint8_t *registerPointer = (uint8_t *)&registerMap;
uint8_t *protectionPointer = (uint8_t *)&protectionMap;

volatile uint8_t registerNumber; //Gets set when user writes an address. We then serve the spot the user requested.

volatile boolean updateFlag = true; //Goes true when we receive new bytes from user. Causes LEDs and things to update in main loop.

volatile Queue ButtonPressed, ButtonClicked; //Init FIFO buffer for storing timestamps associated with button presses and clicks

volatile unsigned long lastClickTime = 0; //Used for debouncing

LEDconfig onboardLED; //init the onboard LED

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

void setup() {
  // Pull up address pins
  pinMode(addressPin1, INPUT_PULLUP);
  pinMode(addressPin2, INPUT_PULLUP);
  pinMode(addressPin3, INPUT_PULLUP);
  pinMode(addressPin4, INPUT_PULLUP);
  pinMode(powerLedPin, OUTPUT);
  powerLed(true); // enable Power LED by default on every power-up

  pinMode(switchPin, INPUT_PULLUP); //GPIO with internal pullup, goes low when button is pushed
#if defined(__AVR_ATmega328P__)
  pinMode(interruptPin, INPUT_PULLUP);     //High-impedance input until we have an int and then we output low. Pulled high with 10k with cuttable jumper.
#else
  pinMode(interruptPin, INPUT);     //High-impedance input until we have an int and then we output low. Pulled high with 10k with cuttable jumper.
#endif

  //Disable ADC
  ADCSRA = 0;

  //Disable Brown-Out Detect
  MCUCR = bit(BODS) | bit(BODSE);
  MCUCR = bit(BODS);

  //Power down various bits of hardware to lower power usage
  //set_sleep_mode(SLEEP_MODE_PWR_DOWN); //May turn off millis
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

#if defined(__AVR_ATmega328P__)
  //Debug values
  registerMap.ledBrightness = 255;     //Max brightness
  registerMap.ledPulseGranularity = 1; //Amount to change LED at each step

  registerMap.ledPulseCycleTime = 500; //Total amount of cycle, does not include off time. LED pulse disabled if zero.
  registerMap.ledPulseOffTime = 500;   //Off time between pulses
#endif

  onboardLED.update(&registerMap); //update LED variables, get ready for pulsing
  setupInterrupts();               //Enable pin change interrupts for I2C, switch, etc
  startI2C(&registerMap);          //Determine the I2C address we should be using and begin listening on I2C bus
  oldAddress = registerMap.i2cAddress;


  digitalWrite(statusLedPin, HIGH); //turn on the status LED to notify that we've setup everything properly
}

void loop() {
  // Check to see if the I2C address has been updated by software, set the appropriate address-type flag
  if (oldAddress != registerMap.i2cAddress)
  {
    oldAddress = registerMap.i2cAddress;
    EEPROM.put(LOCATION_ADDRESS_TYPE, SOFTWARE_ADDRESS);
  }
  
  //update interruptPin output
  if ((registerMap.buttonStatus.isPressed && registerMap.interruptConfigure.pressedEnable) ||
      (registerMap.buttonStatus.hasBeenClicked && registerMap.interruptConfigure.clickedEnable))
  { //if the interrupt is triggered
    pinMode(interruptPin, OUTPUT); //make the interrupt pin a low-impedance connection to ground
    digitalWrite(interruptPin, LOW);
  }
  else
  { //go to high-impedance mode on the interrupt pin if the interrupt is not triggered
#if defined(__AVR_ATmega328P__)
    pinMode(interruptPin, INPUT_PULLUP);
#else
    pinMode(interruptPin, INPUT);
#endif
  }

  if (updateFlag) {
    // Power LED - open drain so toggle between output-low and high-impedance input
    static bool lastPowerLed = true;
    if (registerMap.pwrLedCtrl != lastPowerLed) {
      lastPowerLed = registerMap.pwrLedCtrl;
      powerLed(registerMap.pwrLedCtrl);
    }




    //Record anything new to EEPROM (like new LED values)
    //It can take ~3.4ms to write a byte to EEPROM so we do that here instead of in an interrupt
    recordSystemSettings(&registerMap);

    //Calculate LED values based on pulse settings if anything has changed
    onboardLED.update(&registerMap);

    updateFlag = false; //clear flag
  }
  
  sleep_mode();             //Stop everything and go to sleep. Wake up if I2C event occurs.
  onboardLED.pulse(powerLedPin); //update the brightness of the LED
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


// Control the power LED - open drain output so toggle between enable (output, low) and disable (high-impedance input)
void powerLed(bool enable) {
  if (enable) {
    pinMode(powerLedPin, OUTPUT);
    digitalWrite(powerLedPin, HIGH);
  } else {
    pinMode(powerLedPin, INPUT);
    
  //Read the interrupt bits
  EEPROM.get(LOCATION_INTERRUPTS, map->interruptConfigure.byteWrapped);
  if (map->interruptConfigure.byteWrapped == 0xFF)
  {
    map->interruptConfigure.byteWrapped = 0x03; //By default, enable the click and pressed interrupts
    EEPROM.put(LOCATION_INTERRUPTS, map->interruptConfigure.byteWrapped);
  }

  EEPROM.get(LOCATION_LED_PULSEGRANULARITY, map->ledPulseGranularity);
  if (map->ledPulseGranularity == 0xFF)
  {
    map->ledPulseGranularity = 0; //Default to none
    EEPROM.put(LOCATION_LED_PULSEGRANULARITY, map->ledPulseGranularity);
  }

  EEPROM.get(LOCATION_LED_PULSECYCLETIME, map->ledPulseCycleTime);
  if (map->ledPulseCycleTime == 0xFFFF)
  {
    map->ledPulseCycleTime = 0; //Default to none
    EEPROM.put(LOCATION_LED_PULSECYCLETIME, map->ledPulseCycleTime);
  }

  EEPROM.get(LOCATION_LED_PULSEOFFTIME, map->ledPulseOffTime);
  if (map->ledPulseOffTime == 0xFFFF)
  {
    map->ledPulseOffTime = 0; //Default to none
    EEPROM.put(LOCATION_LED_PULSECYCLETIME, map->ledPulseOffTime);
  }

  EEPROM.get(LOCATION_BUTTON_DEBOUNCE_TIME, map->buttonDebounceTime);
  if (map->buttonDebounceTime == 0xFFFF)
  {
    map->buttonDebounceTime = 10; //Default to 10ms
    EEPROM.put(LOCATION_BUTTON_DEBOUNCE_TIME, map->buttonDebounceTime);
  }

  //Read the starting value for the LED
  EEPROM.get(LOCATION_LED_BRIGHTNESS, map->ledBrightness);
  if (map->ledPulseCycleTime > 0)
  {
    //Don't turn on LED, we'll pulse it in main loop
    analogWrite(ledPin, 0);
  }
  else
  { //Pulsing disabled
    //Turn on LED to setting
    analogWrite(ledPin, map->ledBrightness);
  }
}



  EEPROM.put(LOCATION_INTERRUPTS, map->interruptConfigure.byteWrapped);
  EEPROM.put(LOCATION_LED_BRIGHTNESS, map->ledBrightness);
  EEPROM.put(LOCATION_LED_PULSEGRANULARITY, map->ledPulseGranularity);
  EEPROM.put(LOCATION_LED_PULSECYCLETIME, map->ledPulseCycleTime);
  EEPROM.put(LOCATION_LED_PULSEOFFTIME, map->ledPulseOffTime);
  EEPROM.put(LOCATION_BUTTON_DEBOUNCE_TIME, map->buttonDebounceTime);
}
