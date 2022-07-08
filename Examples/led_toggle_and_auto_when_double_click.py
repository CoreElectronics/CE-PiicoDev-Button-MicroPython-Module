from PiicoDev_Switch import PiicoDev_Switch
from PiicoDev_Unified import sleep_ms
 
button = PiicoDev_Switch()   # Initialise the module

while True:
    while button.was_pressed == False:
        sleep_ms(10)
    button.led = False
    while button.was_pressed == False:
        sleep_ms(10)
    button.led = True