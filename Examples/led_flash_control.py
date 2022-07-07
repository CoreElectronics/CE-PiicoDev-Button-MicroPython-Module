from PiicoDev_Switch import PiicoDev_Switch
from PiicoDev_Unified import sleep_ms
 
button = PiicoDev_Switch()   # Initialise the module

while True:
    value = int(button.count)
    while button.count == 0:
        sleep_ms(10)
    print('Value: ' + str(value))
    button.led = False
    while button.count == 0:
        sleep_ms(10)
    button.led = True