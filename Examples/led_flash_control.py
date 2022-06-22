from PiicoDev_Potentiometer import PiicoDev_Potentiometer
from PiicoDev_Unified import sleep_ms
 
pot = PiicoDev_Potentiometer(min=300, max=50)   # Initialise the module

while True:
    value = int(pot.read())
    print('Value: ' + str(value))
    pot.pwrLED(False)
    sleep_ms(value)
    pot.pwrLED(True)
    sleep_ms(value)