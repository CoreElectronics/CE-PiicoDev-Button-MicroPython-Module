# Set a new (software) address
from PiicoDev_Switch import PiicoDev_Switch
from PiicoDev_Unified import sleep_ms
 
button = PiicoDev_Switch()   # Initialise the module

new_address = 0x70

print("Setting new address to {}".format(new_address))
button.setI2Caddr(new_address)
