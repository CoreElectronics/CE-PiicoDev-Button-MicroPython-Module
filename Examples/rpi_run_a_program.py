# Use a PiicoDev Button to launch a program on the Raspberry Pi
# This can be used to launch any program or run a script.

from PiicoDev_Switch import PiicoDev_Switch
from PiicoDev_Unified import sleep_ms
import subprocess # subprocess is used for executing shell commands
 
button = PiicoDev_Switch()   # Initialise the module

while True:
    if button.was_pressed: # execute a command (open the Chromium web browser
        p = subprocess.Popen("chromium-browser", stdout=subprocess.PIPE, shell=True)
    sleep_ms(100)
