from PiicoDev_Switch import PiicoDev_Switch
from PiicoDev_Unified import sleep_ms
 
button = PiicoDev_Switch(debounce_window=90, double_click_duration=2000)   # Initialise the RFID module

while True:
    if button.press_count:
        print(1)
    else:
        print(0)
    sleep_ms(10)