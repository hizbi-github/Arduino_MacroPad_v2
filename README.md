


# About

<p align="center">
  <img src="Demonstration_Images_and_Gifs\Cover_Image.jpg" />
</p>

This is the second version of my previous MacroPad (https://github.com/hizbi-github/Arduino_MacroPad). 

The code has been rewritten from the ground-up with new features. It now supports a rotary encoder instead of a potentiometer for 
controlling the volume level on a PC. It also shows the volume level on 128x64 dislay. For scrolling it 
has an analog stick. And now instead of normal buttons, there's a 4x4 matrix keypad. This macropad is reprogrammable, although there 
is no software UI available to do so from a PC; you have do it on your own in C/C++ and upload the compiled code to the MacroPad. An 
Arduino Pro Micro is used as a microcontroller in the MacroPad to provide the HID input to the computer.

If you are confused about what even is this thing, here's a short explanation:

    A macropad is essentially a small external harware device that can be programmed to input any 
    user command. This command may be a keyboard keystroke, a mouse movement, etc. The user is 
    able to combine several commands and map them to a single button on the macropad. This makes 
    it easy to perform multiple PC commands with a single press of a button from the macrpad. 
    Volume knobs and small displays are also included to further add to the user experience and 
    increase productivity.

<p align="center">
  <img src="Demonstration_Images_and_Gifs\Comparison_Banner.png" />
</p>

# Features

This MacroPad has some basic hotkeys that I programmed for my own needs. These are:

### 1. Volume Control:

        The rotary encoder can be used to increase/decrease the PC's volume. Pressing the 
        encoder's button mutes the voulme. A volume bar is also shown on the small display.

<p align="center">
  <img src="Demonstration_Images_and_Gifs\Volume_Knob.gif" />
</p>

### 2. Auto Login:

        Enters your PC password for you and then presses the "Enter" key to complete the login process.

<p align="center">
  <img src="Demonstration_Images_and_Gifs\Auto_Login.gif" />
</p>

### 3. Monitor/Screen Output Switch:

        This a sequence of multiple key presses to change the output to "Extend" and back to 
        "PC screen only" as in Windows 10. This combo is quite useful for me since I had to 
        manualy do this everytime I wanted to connect some other device to my monitor.

<p align="center">
  <img src="Demonstration_Images_and_Gifs\Screen_Output_Control.gif" />
</p>

### 4. Desktop Switch:

        Inputs the "Win + D" combo and displays the desktop on a Windows 10 machine. Pressing 
        it again will take you back to the your previuos window.

<p align="center">
  <img src="Demonstration_Images_and_Gifs\Switching_to_Desktop.gif" />
</p>

### 5. Scrolling with the Analog Stick:

        Tired of scrolling with your scoll wheel? Well the analog stick in the MacroPad is able 
        to act as a scroll wheel. It allows for smooth scrolling with two modes: fast and slow. 
        Pushing the analog stick a little allows for a slower scroll, while pushing it completely 
        enables fast scolling so you can zoom through documents and webpages!

<p align="center">
  <img src="Demonstration_Images_and_Gifs\Scrolling_with_Analog_Stick.gif" />
</p>

Similarly, the remaining buttons on the keypad can also be programmed for automating mundane tasks. I would add more shortcuts as my need arises!

# Components

<p align="center">
  <img src="Demonstration_Images_and_Gifs\Multiple_Images.png" />
</p>

1. Arduino Pro Micro (5V version)
2. A Micro-USB to USB-A Cable
3. KY-040 Rotary Encoder
4. 128x64 I2C OLED Display
5. Generic Analog Stick 
6. 4x4 Matrix Keypad Buttons
7. Double-Sided 8x12cm Prototype Board (acting as a motherboard for all the components) 
8. Female Header Pin Slots
9. Lots of Female-Female Jumper Wires
10. A Small Box (for placing the finished macropad)

# Changelog / Revision History

## Rev_1

1. Rotary Encoder works perfectly for controlling the PC volume.

2. The Encoder's button is used for muting the volume.

3. Since only one interrupt pin is used for Rotary Encoder, its more sensitive on one direction.

4. Debouncing yet to be implemented for the Rotary Encoder.

5. 4x4 Matrix Keypad works now, although still needs more optimization.

6. Keypad Matrix code might need complete overhaul :/

7. Some sample HID fucntions and Macros also added for the Keypad buttons.

8. Flash size is 100% (28658 Bytes). Need to replace the U8glib with its lighter version.

## Rev_2

1. Almost complete code overhaul. 

2. Keypad Matrix now works perfectly with debounce, using the simple two loops implementation. 

3. Rotary Encoder debounce issue has also been fixed and its much more responsive now. 

4. Added the new U8g2 Display Library, which contains the U8x8 Display Library. 

5. U8x8 Library is the light version of U8g2 Library, which is the new version of U8g Library.

6. This new library is very light on size, fast in execution and simple in setting up its object.

7. Although the new library can only display simple characters/strings, with less font choices. 

8. This also means that the 100% Flash size problem has been solved. The Flash size has been reduced to 60%. 

9. But its at 68% now, since new code (more features) have also been added. 

10. The display will now show the output/descriptiom of some (will add more later) keypad buttons as they are pressed. 

11. A function has been written to show keypad updates on the display. The funtion takes 2 strings and their coordinates as input. 

12. By default, the display will show the volume level. 

13. Some fun keypad functions such as auto-login and monitor-switch has also been added, with proper debouncing. 

14. Another keypad functions layer has also been added, but not implemented yet. Total 2 layers now. 

15. The volume level display function has also been completely revamped to use with the new U8x8 library. 

16. The volume level is now shown in 10 "|" slashes, one for each 10 ticks of volume. 

17. Previously there were 50 "|" slashes, one for each 2 ticks of volume. 

18. The slash reduction was due to the U8x8 library having no font options for small fonts. 

20. Still, the lightness and speed of this new library is more than enough to justify the slash reduction.  

21. Overall, code has been tidied up and extra code removed. 

22. Next revision will feature the addition of Analog Stick and probably Mouse HID library. 

## Rev_3

1. Added the support for the Analog Stick. 

2. The Analog Stick mimicks the movement of a mouse's scroll wheel.

3. There are two modes for scrolling; slow and fast.

4. For slow scrolling, push the Analog Stick slightly upwards or downwards.

5. Push harder on the Analog Stick to scroll quickly.

6. Added new hotkeys/functions for window-switch and desktop-switch.

7. Performed some code cleanup. 

<p align="center">
    ==============================================================
</p>