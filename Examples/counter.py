from PiicoDev_Switch import PiicoDev_Switch
from PiicoDev_Unified import sleep_ms
 
button = PiicoDev_Switch(double_click_duration=2000)   # Initialise the RFID module

total_presses = 0

while True:
    print('----------------------------------------------------')
    firmware = button.firmware
    print('Firmware: ' + str(firmware[0]) + '.' + str(firmware[1]))
    print('ID: ' + str(button.whoami))
    print('Flashing LED 3 times...')
    total_presses += button.press_count
    print('Total Presses: ' + str(total_presses))
    #switch.setI2Caddr(0x20)
    sleep_ms(5000)