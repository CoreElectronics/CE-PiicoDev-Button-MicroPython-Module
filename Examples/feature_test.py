from PiicoDev_Switch import PiicoDev_Switch
from PiicoDev_Unified import sleep_ms
 
switch = PiicoDev_Switch(double_click_duration=2000)   # Initialise the RFID module

while True:
    print('----------------------------------------------------')
    firmware = switch.readFirmware()
    print('Firmware: ' + str(firmware[0]) + '.' + str(firmware[1]))
    print('ID: ' + str(switch.readID()))
    print('Last Command Known: ' + str(switch.last_command_known))
    print('Last Command Success: ' + str(switch.last_command_success))
    print('Flashing LED 3 times')
#     for x in range(3):
#         switch.pwrLED(0)
#         sleep_ms(500)
#         switch.pwrLED(1)
#         sleep_ms(500)
    print(switch.read())
    
    #switch.double_click_duration = 0
    print('double_click_duration: ' + str(switch.double_click_duration))
    print('debounce_window: ' + str(switch.debounce_window))
    print('double_click_detected: ' + str(switch.double_click_detected))
    print('debug_register: ' +str(switch.readDebug()))
    #switch.setI2Caddr(0x20)
    sleep_ms(5000)