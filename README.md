# PiicoDev® Button (Switch) MicroPython Module and Firmware

This is the firmware repo for the [Core Electronics PiicoDev® Button](https://core-electronics.com.au/catalog/product/view/sku/CE08500)

This module depends on the [PiicoDev Unified Library](https://github.com/CoreElectronics/CE-PiicoDev-Unified), include `PiicoDev_Unified.py` in the project directory on your MicroPython device.

See the [Quickstart Guide](https://piico.dev/p21)

 # Initialisation
 
## `PiicoDev_Switch(bus=, freq=, sda=, scl=, address=0x42, id=, double_press_duration=300, ema_parameter=63, ema_duration=20)`
| Parameter             | Type                     | Range             | Default                               | Description                                                                                                                                                                            |
| --------------------- | ------------------------ | ----------------- | ------------------------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| bus                   | int                      | 0,1               | Raspberry Pi Pico: 0, Raspberry Pi: 1 | I2C Bus.  Ignored on Micro:bit                                                                                                                                                         |
| freq                  | int                      | 100 to 1000000    | Device dependent                      | I2C Bus frequency (Hz).  Ignored on Raspberry Pi                                                                                                                                       |
| sda                   | Pin                      | Device Dependent  | Device Dependent                      | I2C SDA Pin. Implemented on Raspberry Pi Pico only                                                                                                                                     |
| scl                   | Pin                      | Device Dependent  | Device Dependent                      | I2C SCL Pin. Implemented on Raspberry Pi Pico only                                                                                                                                     |
| address               | int                      | 0x42, 0x08 - 0x77 | 0x42                                  | Manually specify the address of the connected device. For when a software address is set on the device.                                                                                |
| id                    | List[int, int, int, int] | 1=ON, 0=OFF       | [0,0,0,0]                             | Hardware switches change the device address - Abstracts the need for user to look up an address, simply input the switch positions. Alternatively, use `address` for explicit address. |
| double_press_duration | int                      |                   | 300                                   | If the button is pressed twice within this period (ms) a double-press will be registered                                                                                               |
| ema_parameter         | int                      | 0-255             | 63                                    | The tuning parameter for the button debounce [EMA Filter](https://en.wikipedia.org/wiki/Moving_average).                                                                               |
| ema_period            | int                      | 0-255             | 20                                    | The sample period in milliseconds for the button debounce [EMA Filter](https://en.wikipedia.org/wiki/Moving_average).                                                                  |

## Properties

## `.is_pressed`
 
Return a boolean of the switch status. `True` = pressed.

## `.was_pressed`
 
Return a boolean of whether the switch was closed since the last query (is a wrapper for `press_count()`)

## `.was_double_pressed`
 
Return a boolean of whether the switch was double-pressed since the last query.

## `.press_count`

Returns an integer of the number of times the switch was closed since last query.

## `.double_press_duration`

The number of milliseconds allowed for a double press.

## `.ema_parameter`

The tuning parameter for the button debounce [EMA Filter](https://en.wikipedia.org/wiki/Moving_average).

## `.ema_period`

The sample period in milliseconds for the button debounce [EMA Filter](https://en.wikipedia.org/wiki/Moving_average).

# Device Registers
The PiicoDev Potentiometer is an I2C device with default address 0x35. The register map is provided if you wish to create your own driver.
A register can be Read (R) or Write (W), but not both. For values that require Read and Write, the convention is to set bit7 for the Write-register. Eg. the status LED - Read: 0x07 and Write: 0x07 | 0x80 = 0x87

| Register Name         | Address   | Bytes  | Mode | Default Value    | Description
| --------------------- | --------- | ------ | ---- | ---------------- | -----------
| Device ID             | 0x01      | 2      | R    | 409              | I2C device ID
| Firmware Major        | 0x02      | 1      | R    | Varies           | Major firmware version
| Firmware Minor        | 0x03      | 1      | R    | Varies           | Minor firmware version
| I2C Address           | 0x04      | 1      | W    | 0x42             | Set new I2C address
| LED                   | 0x05/0x85 | 1      | R/W  | 1                | 0=OFF, 1=ON
| Is Pressed            | 0x11/0x91 | 1      | R    | 0                |
| Was Pressed           | 0x12/0x92 | 1      | R    | 0                |
| Was Double Pressed    | 0x13/0x93 | 1      | R    | 0                |
| Press Count           | 0x14/0x94 | 2      | R    | 0                |
| Double Press Duration | 0x15/0x95 | 1      | R/W  | 300              |
| EMA Parameter         | 0x16/0x96 | 1      | R/W  | 63               |
| EMA Period            | 0x17/0x97 | 1      | R/W  | 20               |

# License

This project is open source - please review the LICENSE.md file for further licensing information.

If you have any technical questions, or concerns about licensing, please contact technical support on the [Core Electronics forums](https://forum.core-electronics.com.au/).

*\"PiicoDev\" and the PiicoDev logo are trademarks of Core Electronics Pty Ltd.*
