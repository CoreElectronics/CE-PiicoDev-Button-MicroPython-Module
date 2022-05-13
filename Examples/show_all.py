from PiicoDev_Switch import PiicoDev_Switch
from PiicoDev_Unified import sleep_ms
 
button = PiicoDev_Switch()   # Initialise the RFID module


print('begin')
print(button.readID())
print(button.readFirmware())
print(button.read())
sleep_ms(1000)
