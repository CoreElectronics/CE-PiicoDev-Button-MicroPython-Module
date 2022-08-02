# PiicoDev® Button (Switch) MicroPython Module and Firmware

This is the firmware repo for the [Core Electronics PiicoDev® Button](https://core-electronics.com.au/catalog/product/view/sku/CE08500)

This module depends on the [PiicoDev Unified Library](https://github.com/CoreElectronics/CE-PiicoDev-Unified), include `PiicoDev_Unified.py` in the project directory on your MicroPython device.

See the [Quickstart Guide](https://piico.dev/p21)

 # Reference

 ## Initialisation
 ```python
 switch = PiicoDev_Switch()
 ```

 ## is_pressed()
 Return a boolean of the switch status. `True` = pressed.

 ## was_pressed()
 Return a boolean of whether the switch was closed since the last query.
 (is a wrapper for `press_count()`)

 ## was_double_pressed()
Return a boolean of whether the switch was double-pressed since the last query.

## press_count()
Returns an integer of the number of times the switch was closed since last query

 ## Control onboard LED

 ## Read Device ID

 ## Change I2C address


## Properties

### .double_press_window
The number of milliseconds allowed for a double press

### .debounce_window
the number of milliseconds dead-time after an initial press

# License
This project is open source - please review the LICENSE.md file for further licensing information.

If you have any technical questions, or concerns about licensing, please contact technical support on the [Core Electronics forums](https://forum.core-electronics.com.au/).

*\"PiicoDev\" and the PiicoDev logo are trademarks of Core Electronics Pty Ltd.*
