# PiicoDev® Button (Switch) MicroPython Module and Firmware

This is the firmware repo for the [Core Electronics PiicoDev® Button](https://core-electronics.com.au/catalog/product/view/sku/CE08500)

This module depends on the [PiicoDev Unified Library](https://github.com/CoreElectronics/CE-PiicoDev-Unified), include `PiicoDev_Unified.py` in the project directory on your MicroPython device.

See the [Quickstart Guide](https://piico.dev/p21)

 # Initialisation
 
## `PiicoDev_Switch(bus=, freq=, sda=, scl=, address=0x42, id=, double_click_duration=300, ema_parameter=63, ema_duration=20)`
| Parameter             | Type                     | Range             | Default                               | Description                                                                                                                                                                            |
| --------------------- | ------------------------ | ----------------- | ------------------------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| bus                   | int                      | 0,1               | Raspberry Pi Pico: 0, Raspberry Pi: 1 | I2C Bus.  Ignored on Micro:bit                                                                                                                                                         |
| freq                  | int                      | 100 to 1000000    | Device dependent                      | I2C Bus frequency (Hz).  Ignored on Raspberry Pi                                                                                                                                       |
| sda                   | Pin                      | Device Dependent  | Device Dependent                      | I2C SDA Pin. Implemented on Raspberry Pi Pico only                                                                                                                                     |
| scl                   | Pin                      | Device Dependent  | Device Dependent                      | I2C SCL Pin. Implemented on Raspberry Pi Pico only                                                                                                                                     |
| address               | int                      | 0x42, 0x08 - 0x77 | 0x42                                  | Manually specify the address of the connected device. For when a software address is set on the device.                                                                                |
| id                    | List[int, int, int, int] | 1=ON, 0=OFF       | [0,0,0,0]                             | Hardware switches change the device address - Abstracts the need for user to look up an address, simply input the switch positions. Alternatively, use `address` for explicit address. |
| double_click_duration | int                      |                   | 300                                   | If the button is pressed twice within this period (ms) a double-click will be registered                                                                                               |
| ema_parameter         | int                      | 0-255             | 63                                    | Parameter for the Exponential Weighted Moving Average                                                                                                                                  |
| ema_duration          | int                      | 0-255             | 20                                    | Milliseconds for the Exponential Weighted Moving Average                                                                                                                               |

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

Parameter for the Exponential Weighted Moving Average

## `.ema_duration`

Milliseconds for the Exponential Weighted Moving Average

# License

This project is open source - please review the LICENSE.md file for further licensing information.

If you have any technical questions, or concerns about licensing, please contact technical support on the [Core Electronics forums](https://forum.core-electronics.com.au/).

*\"PiicoDev\" and the PiicoDev logo are trademarks of Core Electronics Pty Ltd.*
