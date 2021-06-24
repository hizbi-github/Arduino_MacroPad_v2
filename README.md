

# To Be Updated Soon!

# :)


# Revision History

## Rev_1

1. Rotary Encoder works perfectly for controlling the PC volume.

2. The Encoder button is used for muting the volume.

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


==================================================================================================