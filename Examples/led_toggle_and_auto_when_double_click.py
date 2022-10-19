from PiicoDev_Switch import PiicoDev_Switch
from PiicoDev_Unified import sleep_ms
 
button = PiicoDev_Switch()   # Initialise the module

def auto_toggle():
    button.led = True
    for x in range(10):
        sleep_ms(100)
        button.led = False
        sleep_ms(100)
        button.led = True

button.led = True
while True:
    if button.was_pressed:
        button.led = not button.led
    
    if button.was_double_pressed:
        auto_toggle()
    
    sleep_ms(10)