from PiicoDev_Switch import PiicoDev_Switch
from PiicoDev_Unified import sleep_ms
 
button = PiicoDev_Switch(double_click_duration=2000)   # Initialise the RFID module

while True:
    print('----------------------------------------------------')
    firmware = button.firmware
    print('Firmware: ' + str(firmware[0]) + '.' + str(firmware[1]))
    print('ID: ' + str(button.whoami))
    print('Flashing LED 3 times...')
    for x in range(3):
        button.led = False
        print("LED ON: " + str(button.led))
        sleep_ms(1000)
        button.led = True
        print("LED ON: " + str(button.led))
        sleep_ms(1000)
    print('Button press count: ' + str(button.press_count))
    #switch.double_click_duration = 0
    print('double_click_duration: ' + str(button.double_click_duration))
    print('debounce_window: ' + str(button.debounce_window))
    print('Is Pressed: ' + str(button.is_pressed))
    print('double_click_detected: ' + str(button.double_click_detected))
    #switch.setI2Caddr(0x20)
    sleep_ms(5000)