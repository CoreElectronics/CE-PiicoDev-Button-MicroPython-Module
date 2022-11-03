# Use a PiicoDev Button to scroll images on the micro:bit display
from microbit import display, Image
from PiicoDev_Switch import PiicoDev_Switch
from PiicoDev_Unified import sleep_ms
 
button = PiicoDev_Switch()   # Initialise the module

index = 0
images = [Image.HEART, Image.HAPPY, Image.SAD] # a pool of images to display

while True:
    if button.was_pressed:          # change the image by
        index += 1                  # incrementing the index
        if index >= len(images):    # wrap the index back to zero when it goes out of bounds
            index=0
        print("index: ",index)
        
    display.show( images[index] )   # Show the selected image
    
    sleep_ms(100)