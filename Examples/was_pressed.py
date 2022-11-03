# Check if a PiicoDev Button was pressed

from PiicoDev_Switch import PiicoDev_Switch
from PiicoDev_Unified import sleep_ms
 
button = PiicoDev_Switch()   # Initialise the module

while True:
    if button.was_pressed: # was the button pressed since the last time we checked?
    # Remix idea: try changing .was_pressed to .is_pressed
        print(1) # print a result that can be plotted
    else:
        print(0)
        
    sleep_ms(100)