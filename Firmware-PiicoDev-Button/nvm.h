//Location in EEPROM for each thing we want to store between power cycles
enum eepromLocations {
  LOCATION_I2C_ADDRESS = 0x00, //Device's address
  LOCATION_INTERRUPTS = 0x01,
  LOCATION_BUTTON_DEBOUNCE_TIME = 0x08,
  LOCATION_ADDRESS_TYPE = 0x0A, // Address type can be either hardware defined (jumpers/switches), or software defined by user.
};
