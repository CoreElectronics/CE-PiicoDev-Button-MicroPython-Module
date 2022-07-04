from PiicoDev_Switch import PiicoDev_Switch
from PiicoDev_Unified import sleep_ms
 
button = PiicoDev_Switch(double_click_duration=2000)   # Initialise the RFID module

while True:
    print('----------------------------------------------------')
    firmware = button.readFirmware()
    print('Firmware: ' + str(firmware[0]) + '.' + str(firmware[1]))
    print('ID: ' + str(button.readID()))
    print('Last Command Known: ' + str(button.last_command_known))
    print('Last Command Success: ' + str(button.last_command_success))
    print('Flashing LED 3 times')
    for x in range(3):
        button.pwrLED(0)
        sleep_ms(500)
        button.pwrLED(1)
        sleep_ms(500)
    print(button.read())
    
    #switch.double_click_duration = 0
    print('double_click_duration: ' + str(button.double_click_duration))
    print('debounce_window: ' + str(button.debounce_window))
    print('button pressed: ' + str(button.read_state()))
    print('double_click_detected: ' + str(button.double_click_detected()))
    print('debug_register: ' +str(button.readDebug()))
    #switch.setI2Caddr(0x20)
    sleep_ms(5000)