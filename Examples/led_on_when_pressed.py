from PiicoDev_Switch import PiicoDev_Switch
from PiicoDev_Unified import sleep_ms
 
button = PiicoDev_Switch()   # Initialise the module

while True:
    button.led = button.is_pressed
    sleep_ms(10)
