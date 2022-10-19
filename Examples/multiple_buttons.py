from PiicoDev_Switch import PiicoDev_Switch
from PiicoDev_Unified import sleep_ms
 
button_up = PiicoDev_Switch()   # Initialise the 1st module
button_down = PiicoDev_Switch(id=[1,0,0,0])   # Initialise the 2nd module

value = 0
press_count_up_prev = 0
press_count_down_prev = 0

while True:
    press_count_up = button_up.press_count
    press_count_down = button_down.press_count
    value = value + press_count_up - press_count_down
    if press_count_up == 0 and press_count_up_prev == 0 and button_up.is_pressed:
        value = value + 10  # holding button increments faster
    if press_count_down == 0 and press_count_down_prev == 0 and button_down.is_pressed:
        value = value - 10 # holding button decrements faster
    print(value)
    press_count_up_prev = press_count_up
    press_count_down_prev = press_count_down
    sleep_ms(200)