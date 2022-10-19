from PiicoDev_Switch import PiicoDev_Switch
from PiicoDev_Unified import sleep_ms
 
button = PiicoDev_Switch()   # Initialise the module

total_presses = 0

while True:
    total_presses += button.press_count
    print('Total Presses: ' + str(total_presses))
    sleep_ms(1000)