from PiicoDev_Switch import PiicoDev_Switch
from PiicoDev_Unified import sleep_ms
 
button = PiicoDev_Switch()   # Initialise the module

while True:
    if button.was_pressed:
        print(1)
    else:
        print(0)
    sleep_ms(10)