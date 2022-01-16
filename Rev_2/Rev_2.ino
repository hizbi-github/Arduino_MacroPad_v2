

///// Rev_2 /////
///// Almost complete code overhaul. /////
///// Keypad Matrix now works perfectly with debounce, using the simple two loops implementation. /////
///// Rotary Encoder debounce issue has also been fixed and its much more responsive now. /////
///// Added the new U8g2 Display Library, which contains the U8x8 Display Library. /////
///// U8x8 Library is the light version of U8g2 Library, which is the new version of U8g Library. /////
///// This new library is very light on size, fast in execution and simple in setting up its object. /////
///// Although the new library can only display simple characters/strings, with less font choices. /////
///// This also means that the 100% Flash size problem has been solved. The Flash size has been reduced to 60%. /////
///// But its at 68% now, since new code (more features) have also been added. /////
///// The display will now show the output/descriptiom of some (will add more later) keypad buttons as they are pressed. /////
///// A function has been written to show keypad updates on the display. The funtion takes 2 strings and their coordinates as input. /////
///// By default, the display will show the volume level. /////
///// Some fun keypad functions such as auto-login and monitor-switch has also been added, with proper debouncing. /////
///// Another keypad functions layer has also been added, but not implemented yet. Total 2 layers now. /////
///// The volume level display function has also been completely revamped to use with the new U8x8 library. /////
///// The volume level is now shown in 10 "|" slashes, one for each 10 ticks of volume. /////
///// Previously there were 50 "|" slashes, one for each 2 ticks of volume. /////
///// The slash reduction was due to the U8x8 library having no font options for small fonts. /////
///// Still, the lightness and speed of this new library is more than enough to justify the slash reduction. ///// 
///// Overall, code has been tidied up and extra code removed. /////
///// Next revision will feature the addition of Analog Stick and probably Mouse HID library. /////


///// GitHub: @Hizbi-K /////



// #include <Arduino.h>
#include <Wire.h>
#include "U8x8lib.h"
#include "HID-Project.h"



U8X8_SSD1306_128X64_NONAME_HW_I2C displayOLED;      // Creating object of the new U8x8 display library.
                                                    // Note, the default constructor is being called here.


int encoder_Clk = 0;
volatile bool current_Clk_State = HIGH;

int encoder_Data = 1;
volatile bool current_Data_State = HIGH;

#define INTERRUPT 3        // Setting up Digital Pin 1 for Interrupts from Rotary Encoder

volatile bool encoderStatus = HIGH;
volatile bool encoderBusy = false;

int encoder_BUT = 7;
bool butState_3 = LOW;
bool keyPressed_3 = false;


volatile int volumeLevel = 0;

bool setVolumeLevel = true;


String stringFullVolume = "||||||||||";
String stringCurrentVol = stringFullVolume;


byte keypadRows [5] = {0, 18, 19, 20, 21};         // Keypad row pins, "0" is for array padding.
byte keypadColumns [5] = {0, 15, 14, 16, 10}; //{0, 10, 16, 14, 15};      // Keypad column pins, "0" is for array padding.

byte rows = 1;
byte columns = 1;


struct structKeypadMatrix
{
  byte rows = 0;
  byte columns = 0;
  boolean keyPressed = false;

} keypadMatrix;


struct structMonitorSwitch
{
  boolean change = false;       // For activating/deactivating the function call in main loop.

  boolean inMenu = true;        // Allowing to bring up the screen projection menu. (Required to be set "true" to execute the chain of commands).
  boolean movingDown = false;
  boolean selectMode = false;

  unsigned long previousMillis_MS = 0;   

} monitorSwitchActions;


struct structAutoPCLogin
{
  boolean doAutoLogin = false;

  boolean bringUpTextField = true;
  boolean enterPassword = false;
  boolean pressLogin = false;

  unsigned long previousMillis_AL = 0;  

} autoPCLoginActions;


boolean showVolumePage = true;              // By default, the Volume Level page is shown on the OLED Display.
boolean clearDisplayforNewPage = false;     // Checking to clear the display after showing keypad updates. Need this
                                            // for some artficial delay so that the keypad update can actually be read.

unsigned long currentMillis = 0;
unsigned long previousMillis_RE = 0;
unsigned long previousMillis_KP = 0;    // For Button 1
unsigned long previousMillis_DP = 0;    // For LED (Left)
unsigned long previousMillis_2 = 0;     // For Button 2
unsigned long previousMillis_3 = 0;     // For Button 3 (Rotary Encoder's Button)
unsigned long previousMillis_4 = 0;     // For LED (Right)
unsigned long previousMillis_5 = 0;     // For LED (Left)
unsigned long previousMillis_6 = 0;     // Common for calling many different Functions
unsigned long previousMillis_7 = 0;     // For Analog Stick Reception
unsigned long previousMillis_8 = 0;     // For Analog Stick Swiping

unsigned long debounceInterval = 10;
unsigned long encoderDebounce = 1;
unsigned long keypadDebounce = 180;
unsigned long displayUpdateInterval = 500;
unsigned long analogStickSwipeInterval = 250;




void pressEncoderBUT ();

void encoderISR ();           // Reacting quickly to Rotary Encoder's Change in Direction usign Interrupts.

void volumeKnob ();           // Increasing/Decreasing the PC volume depending upon the direction of Encoder's rotation.

void setVolume ();            // Sets the volume at a pre-determined level, so that pseudo real volume level can displayed at the OLED Display.
                              
void readMatrixKeypad ();     // Reads the button presses from Keypad matrix by polling it.

void keypadFunctionsLayer_1 ();      // Performs the respective function assigned to Keypad's button for layer 1.

void keypadFunctionsLayer_2 ();      // Performs the respective function assigned to Keypad's button for layer 2.

void monitorSwitch ();        // Switches the screen projection mode over HDMI.

void autoPCLogin ();          // (UNSAFE) Does the automatic password entry and login.


void drawPage_KeypadButtonUpdates (byte firstLine_x, byte firstLine_y, char firstLine, byte secondLine_x, byte secondLine_y, char secondLine);


void drawPage_Volume ();       // Displaying the volume bar graph and level.
void drawPage_General ();






void setup ()
{ 
  
  pinMode(encoder_Clk, INPUT_PULLUP);
  pinMode(encoder_Data, INPUT_PULLUP);
  pinMode(encoder_BUT, INPUT_PULLUP);


  attachInterrupt(INTERRUPT, encoderISR, CHANGE);

  
  pinMode(keypadRows[1], INPUT_PULLUP);
  pinMode(keypadRows[2], INPUT_PULLUP);
  pinMode(keypadRows[3], INPUT_PULLUP);
  pinMode(keypadRows[4], INPUT_PULLUP);


  pinMode(keypadColumns[1], INPUT_PULLUP);
  pinMode(keypadColumns[2], INPUT_PULLUP);
  pinMode(keypadColumns[3], INPUT_PULLUP);
  pinMode(keypadColumns[4], INPUT_PULLUP);


  previousMillis_RE = millis();      // Initializing the timer for the common always-running Functions.
  previousMillis_KP = millis();      // Initializing the timer for the common always-running Functions.
  previousMillis_DP = millis();      // Initializing the timer for the common always-running Functions.

  // Wire.begin();

  displayOLED.begin();

  Keyboard.begin();
  Consumer.begin();

  setVolume();           // Sets the volume at a pre-determined level, so that volume level can displayed at the OLED Display.

}  
  


void loop ()
{
    
  currentMillis = millis();
  
  
  pressEncoderBUT();


  if (currentMillis - previousMillis_RE >= encoderDebounce)
  {
    volumeKnob();

    previousMillis_RE = currentMillis;
  }

  
  if (currentMillis - previousMillis_KP >= keypadDebounce)
  {
    readMatrixKeypad();

    previousMillis_KP = currentMillis;
  }


  if (keypadMatrix.keyPressed == false)
  {
    keypadFunctionsLayer_1();

    keypadMatrix.keyPressed = true;
  }

  if (monitorSwitchActions.change == true)
  {
    monitorSwitch();
  }

  if (autoPCLoginActions.doAutoLogin == true)
  {
    autoPCLogin();
  }
  

  if ((showVolumePage == true) && (currentMillis - previousMillis_DP >= displayUpdateInterval))
  {
    drawPage_Volume();         // Volume Level page
  }

} 



///////////////////////////////////////////////////////////////////////////////////////////




  


///////////////////////////////////////////////////////////////////////////////////////////



void encoderISR()     // ISR for taking in the Encoder readings quickly
{
    
    current_Clk_State = digitalRead(encoder_Clk);
    current_Data_State = digitalRead(encoder_Data);


    if (current_Clk_State == LOW)
    { 
      encoderStatus = current_Data_State;
    }

    else if (current_Clk_State == HIGH)
    {
      encoderStatus = !current_Data_State;
    }

    encoderBusy = true;
  
}



///////////////////////////////////////////////////////////////////////////////////////////



void pressEncoderBUT ()       // Using the button from the Rotatory Encoder
{
    
  
  if (butState_3 != digitalRead(encoder_BUT))
  {  
    
    if (butState_3 == HIGH)
    {
      butState_3 = digitalRead(encoder_BUT);
      keyPressed_3 = true;

      previousMillis_3 = currentMillis;
    }
    else if (butState_3 == LOW)
    {
      butState_3 = HIGH;
      keyPressed_3 = false;
    }

  }


  if ((butState_3 == LOW) && (keyPressed_3 == true) && ((currentMillis - previousMillis_3) > debounceInterval))
  {
      
    Consumer.write(MEDIA_VOL_MUTE);

    
    if (digitalRead(encoder_BUT) == LOW)      // No toggling until the finger has been lifted off
    {
      keyPressed_3 =  false;
      butState_3 = LOW;
    }
    else if (digitalRead(encoder_BUT) == HIGH)
    {
      keyPressed_3 =  true;
      butState_3 = HIGH;
    }
    

  } 


}



///////////////////////////////////////////////////////////////////////////////////////////



void volumeKnob()
{
  if (encoderBusy == true)
    {
      
      if (encoderStatus == HIGH)         // Up counter for clockwise rotation
      {
        volumeLevel = volumeLevel + 2;
        
        Consumer.write(MEDIA_VOL_UP);

        if (volumeLevel == 102)     // Resetting the volume level on OLED Display
        {
          volumeLevel = 100;
        }
      }


      else if (encoderStatus == LOW)      // Down counter for anit-clockwise rotation
      {
        volumeLevel = volumeLevel - 2;
        
        Consumer.write(MEDIA_VOL_DOWN);
        
        if (volumeLevel == -2)      // Resetting the counter
        {
          volumeLevel = 0;
        }
      }

      encoderBusy = false;             // Allowing Arduino to read the Rotary Encoder again

    }
}



///////////////////////////////////////////////////////////////////////////////////////////



void setVolume ()
{ 
  if (setVolumeLevel == true)
  {

    Consumer.write(MEDIA_VOL_MUTE);       // There should be no need to do this, but alas, the volume reset will not work otherwise!
    Consumer.write(MEDIA_VOL_MUTE);

    for (int down = 0; down <= 100; down++)
    {
      Consumer.write(MEDIA_VOL_DOWN);
    delay(10);                            // I am so sorry for using delays, but this is my only hope since this function
    }                                     // in setup().

    for (int up = 0; up < 35; up++)
    {
      Consumer.write(MEDIA_VOL_UP);      
      delay(10);                          // Sorry again :/
    }

    volumeLevel = 70;
    setVolumeLevel = false;

  }
}



///////////////////////////////////////////////////////////////////////////////////////////



void readMatrixKeypad ()
{

  for (rows = 1; rows <= 4; rows ++)
  {
    pinMode(keypadRows[rows], OUTPUT);
    digitalWrite(keypadRows[rows], LOW); 

    for (columns = 1; columns <= 4; columns++)
    {
      
      if (digitalRead(keypadColumns[columns]) == LOW)
      {
        keypadMatrix.rows = rows;
        keypadMatrix.columns = columns;

        keypadMatrix.keyPressed = false;        // That means the function in the main loop() will
      }                                         // perform the action for the keypress. after which
                                                // this boolean will be set to true.
    }

    pinMode(keypadRows[rows], INPUT_PULLUP);

  }

}
        


///////////////////////////////////////////////////////////////////////////////////////////



void drawPage_KeypadButtonUpdates (byte firstLine_x, byte firstLine_y, String firstLine, byte secondLine_x, byte secondLine_y, String secondLine)
{

  showVolumePage = false;          // Disabling the Volume Level display page momentarily to show kypad updates.
  
  displayOLED.clearDisplay();
  displayOLED.setFont(u8x8_font_amstrad_cpc_extended_f);

  displayOLED.setCursor(firstLine_x, firstLine_y);
  displayOLED.print(firstLine);

  displayOLED.setCursor(secondLine_x, secondLine_y);
  displayOLED.print(secondLine);

  clearDisplayforNewPage = true;    // Adding some delay before clearingthe keypad updates from the display.
                                    // This will increase readibillity. Delay is implemented in the "if" statement
                                    // calling the "drawPage_Volume" function.
}



///////////////////////////////////////////////////////////////////////////////////////////







///////////////////////////////////////////////////////////////////////////////////////////







///////////////////////////////////////////////////////////////////////////////////////////







///////////////////////////////////////////////////////////////////////////////////////////



void keypadFunctionsLayer_1 ()
{

  switch (keypadMatrix.rows)
  {
  
    case 1:
    {

      switch (keypadMatrix.columns)      // Switching between the four columns.
      {

        case 1:
        {
          autoPCLoginActions.doAutoLogin = true;

          break;
        }

        case 2:
        {
          drawPage_KeypadButtonUpdates(1, 2, "Window-Switch", 1, 4, "Alt + TAB");

          Keyboard.press(KEY_LEFT_ALT);
          Keyboard.press(KEY_TAB);
          Keyboard.releaseAll();

          showVolumePage = true;                 // Reactivating the "drawPage_Volume" function disabled by keypad updates.
          previousMillis_DP = currentMillis;     // Artficial delay for keypad update readibility. 

          break;
        }

        case 3:
        {
          monitorSwitchActions.change = true;     // Calling the Monitor switch function to

          break;
        }

        case 4:
        {
          drawPage_KeypadButtonUpdates(1, 2, "Desktop-Toggle", 1, 4, "Win + D");

          Keyboard.press(KEY_LEFT_WINDOWS);
          Keyboard.press(KEY_D);
          Keyboard.releaseAll();

          showVolumePage = true;                 // Reactivating the "drawPage_Volume" function disabled by keypad updates.
          previousMillis_DP = currentMillis;     // Artficial delay for keypad update readibility.   

          break;
        }

      }

      break;

    }


    case 2:
    {
      
      switch (keypadMatrix.columns)      // Switching between the four columns.
      {

        case 1:
        {
          Keyboard.print("5");
          
          break;
        }

        case 2:
        {
          Keyboard.print("6");

          break;
        }

        case 3:
        {
          Keyboard.print("7");
          
          break;
        }

        case 4:
        {
          Keyboard.print("8");
        
          break;
        }

      }

      break;
    }

    case 3:
    {
      
      switch (keypadMatrix.columns)      // Switching between the four columns.
      {

        case 1:
        {
          Keyboard.print("9");

          break;
        }

        case 2:
        {
          Keyboard.print("10");

          break;
        }

        case 3:
        {
          Keyboard.print("11");
          
          break;
        }

        case 4:
        {
          Keyboard.print("12");
        
          break;
        }

      }

      break;
    }

    case 4:
    {
      
      switch (keypadMatrix.columns)      // Switching between the four columns.
      {

        case 1:
        {
          Keyboard.print("13");

          break;
        }

        case 2:
        {
          Keyboard.print("14");

          break;
        }

        case 3:
        {
          Keyboard.print("15");
          
          break;
        }

        case 4:
        {
          Keyboard.print("16");
        
          break;
        }

      }

      break;
    }    

  }

}



///////////////////////////////////////////////////////////////////////////////////////////



void keypadFunctionsLayer_2 ()
{

  switch (keypadMatrix.rows)
  {
  
    case 1:
    {

      switch (keypadMatrix.columns)      // Switching between the four columns.
      {

        case 1:
        {
          Keyboard.print("decent12");

          break;
        }

        case 2:
        {
          Keyboard.write(KEY_ENTER);

          break;
        }

        case 3:
        {
          Keyboard.press(KEY_LEFT_WINDOWS);
          Keyboard.press(KEY_P);
          Keyboard.releaseAll();
          
          break;
        }

        case 4:
        {
          Keyboard.press(KEY_LEFT_WINDOWS);
          Keyboard.press(KEY_D);
          Keyboard.releaseAll();

          break;
        }

      }

      break;

    }


    case 2:
    {
      
      switch (keypadMatrix.columns)      // Switching between the four columns.
      {

        case 1:
        {
          Keyboard.print("5");
          
          break;
        }

        case 2:
        {
          Keyboard.print("6");

          break;
        }

        case 3:
        {
          Keyboard.write(KEY_DOWN_ARROW);
          Keyboard.write(KEY_DOWN_ARROW);
          Keyboard.write(KEY_ENTER); 
          
          break;
        }

        case 4:
        {
          Keyboard.print("8");
        
          break;
        }

      }

      break;
    }

    case 3:
    {
      
      switch (keypadMatrix.columns)      // Switching between the four columns.
      {

        case 1:
        {
          Keyboard.print("9");

          break;
        }

        case 2:
        {
          Keyboard.print("10");

          break;
        }

        case 3:
        {
          Keyboard.print("11");
          
          break;
        }

        case 4:
        {
          Keyboard.print("12");
        
          break;
        }

      }

      break;
    }

    case 4:
    {
      
      switch (keypadMatrix.columns)      // Switching between the four columns.
      {

        case 1:
        {
          Keyboard.print("13");

          break;
        }

        case 2:
        {
          Keyboard.print("14");

          break;
        }

        case 3:
        {
          Keyboard.print("15");
          
          break;
        }

        case 4:
        {
          Keyboard.print("16");
        
          break;
        }

      }

      break;
    }    

  }

}



///////////////////////////////////////////////////////////////////////////////////////////



void monitorSwitch ()
{
  
  if (monitorSwitchActions.inMenu == true)
  {
    showVolumePage = false;
    drawPage_KeypadButtonUpdates(1, 2, "Monitor", 1, 4, "Switch");
    
    
    Keyboard.press(KEY_LEFT_WINDOWS);
    Keyboard.press(KEY_P);
    Keyboard.releaseAll();

    monitorSwitchActions.inMenu = false;
    monitorSwitchActions.movingDown = true;

    monitorSwitchActions.previousMillis_MS = currentMillis;
  }

  if ((monitorSwitchActions.movingDown == true) && (currentMillis - monitorSwitchActions.previousMillis_MS >= 500))
  {    
    Keyboard.write(KEY_DOWN_ARROW);
    Keyboard.write(KEY_DOWN_ARROW);

    monitorSwitchActions.movingDown = false;
    monitorSwitchActions.selectMode = true;

    monitorSwitchActions.previousMillis_MS = currentMillis;
  }

  if ((monitorSwitchActions.selectMode == true) && (currentMillis - monitorSwitchActions.previousMillis_MS >= 500))
  {
    Keyboard.write(KEY_ENTER); 
    Keyboard.write(KEY_ESC);

    monitorSwitchActions.selectMode = false;
    monitorSwitchActions.inMenu = true;

    monitorSwitchActions.change = false;

    showVolumePage = true;                 // Reactivating the "drawPage_Volume" function disabled by keypad updates.
    previousMillis_DP = currentMillis;     // Artficial delay for keypad update readibility. 
  }
  
}



///////////////////////////////////////////////////////////////////////////////////////////



void autoPCLogin ()
{

  if (autoPCLoginActions.bringUpTextField == true)
  {
    drawPage_KeypadButtonUpdates(1, 2, "Auto", 1, 4, "Login");
    
    Keyboard.write(KEY_ENTER);

    autoPCLoginActions.bringUpTextField = false;
    autoPCLoginActions.enterPassword = true;

    autoPCLoginActions.previousMillis_AL = currentMillis;
  }


  if ((autoPCLoginActions.enterPassword == true) && (currentMillis - autoPCLoginActions.previousMillis_AL >= 200))
  {    
    Keyboard.print("decent12");

    autoPCLoginActions.enterPassword = false;
    autoPCLoginActions.pressLogin = true;

    monitorSwitchActions.previousMillis_MS = currentMillis;
  }


  if ((autoPCLoginActions.pressLogin == true) && (currentMillis - autoPCLoginActions.previousMillis_AL >= 200))
  {
    Keyboard.write(KEY_ENTER);

    autoPCLoginActions.pressLogin = false;
    autoPCLoginActions.bringUpTextField = true;

    autoPCLoginActions.doAutoLogin = false;

    showVolumePage = true;                 // Reactivating the "drawPage_Volume" function disabled by keypad updates.
    previousMillis_DP = currentMillis;     // Artficial delay for keypad update readibility. 
  }

}



///////////////////////////////////////////////////////////////////////////////////////////



void drawPage_Volume ()       // For displaying PC Volume page
{                        // Very specific positional values have been used for proper alignment. The coordinates can be modified...at your own risk! :p

  static int levelToBarGraphSegments = 0;
  static int previouslevelToBarGraphSegments = 0;
  

  static boolean oneDigitVolume = false;
  static boolean twoDigitVolume = false;
  static boolean threeDigitVolume = false;



  if (clearDisplayforNewPage == true)       // Adding some delay before clearning the screen right after showing keypad updates.
  {                                         // The delay is backed into the "if" statement inside which this "drawPage_Volume" function.
    displayOLED.clearDisplay();             // is called.

    clearDisplayforNewPage = false;
  }


  
  displayOLED.setFont(u8x8_font_amstrad_cpc_extended_f);
  displayOLED.setCursor(2, 0);
  displayOLED.print(rows);
  displayOLED.drawString(5, 0, "Volume");


  if (volumeLevel == 100)               // Centering the volume number text when its at 100.
  {
    displayOLED.setCursor(5, 2);

    if (threeDigitVolume == false)      // Clear only once to prevent flickering.
    {
      displayOLED.clearLine(2);
      displayOLED.clearLine(3);
      displayOLED.clearLine(4);

      oneDigitVolume = false;
      twoDigitVolume = false;
      threeDigitVolume = true;
    }
  }

  else if ((volumeLevel >= 10) && (volumeLevel < 100))        // Centering the volume number text when its between 10 and 99 (inclusive).
  {
    displayOLED.setCursor(6, 2);

    if (twoDigitVolume == false)        // Clear only once to prevent flickering.
    {
      displayOLED.clearLine(2);
      displayOLED.clearLine(3);
      displayOLED.clearLine(4);
      
      oneDigitVolume = false;
      twoDigitVolume = true;
      threeDigitVolume = false;
    }
  }

  else if (volumeLevel < 10)            // Centering the volume number text when its between 9 and 0 (inclusive).                
  {
    displayOLED.setCursor(7, 2);

    if (oneDigitVolume == false)        // Clear only once to prevent flickering.
    {
      displayOLED.clearLine(2);
      displayOLED.clearLine(3);
      displayOLED.clearLine(4);

      oneDigitVolume = true;
      twoDigitVolume = false;
      threeDigitVolume = false;
    }
  }



  displayOLED.setFont(u8x8_font_courR18_2x3_n);

  displayOLED.print(volumeLevel);



  previouslevelToBarGraphSegments = levelToBarGraphSegments;      // Storing the previous value to check whether the difference is 10 ticks or not.
                                                                  // Will be used to clear the volume level on the display once per 10 ticks,
                                                                  // or the volumer level flickers which doesn't look good.

  levelToBarGraphSegments = volumeLevel / 10;     // Converting the dynamic volume level into 10 ticks.
                                                                           
    

  stringCurrentVol = stringFullVolume.substring(0, levelToBarGraphSegments);      // Displaying the Volume Bar Graph for different 
                                                                                  //levels by modifying the string.   
  
  if (abs(previouslevelToBarGraphSegments - levelToBarGraphSegments) >= 1)      // Clearing the display only for 10 ticks,
  {                                                                             // otherwise it causes flickering.
    displayOLED.clearLine(6);
  }


  displayOLED.setFont(u8x8_font_amstrad_cpc_extended_f);
  displayOLED.drawString(3, 6, stringCurrentVol.c_str());     // Strings can't be accessed directly using "U8x8 or u8g2" library, 
                                                              //hence ".c_str()" is used to pass the string reference.
 
  displayOLED.drawString(1, 6, ">");        // Symbol showing the direction for volume increase.     
  displayOLED.drawString(14, 6, ">");       // Symbol showing the direction for volume decrease.
  
}



///////////////////////////////////////////////////////////////////////////////////////////







///////////////////////////////////////////////////////////////////////////////////////////





/* HUGE EXPLANATION THAT BECAME OUTDATED AS SOON AS I FOUND THE SIMPLE WAY TO SOLVE THE PROBLEM
   DON'T CODE ON EMPTY STOMACH! */

    /*     10 ticks have a difference of 0.
            2 ticks have a difference of 8.

          Note that after displaying the increased volume level bar, the "clearLevelBar" is set at true, and it only becomes false after the volume is 
    increased or decreased by less than 10 ticks. No problem so far, untill we also consider properly display the volume level for descreasing volume. 
    This is done by capturing the difference of 8 when decreasing from a previous 10 ticks. 

          The "if" statement above includes the condition for catching this volume decrease. Naturally, the "clearLevelBar" would have set itself false due 
    to the "else" statement associated with the above "if" statement. But since 2 ticks and 10 ticks are consecutive, that is followed by one another when 
    decreasing the volume, the "else" statement isn't satisfied and "clearLevelBar" isn't set to false.

          Consecutiveness is explained as:

          volumeLevel is at 100. levelToBarGraphSegments is at 10. levelUpdateCheck will be 0.
          When decreasing the volume, the program checks for a unit decrease in volume, which is 2 ticks. 
          (It's 2 ticks to match Windows 10 volume update ticks).

          Decreasing the volume, volumeLevel is now at 98. levelToBarGraphSegments is 9. levelUpdateCheck is 8.
          At this stage, we want to set the volume bar at correct level, since previously it would only update at volume level 90, 80, 70, ... so on.

          (Incomplete...)

          That's why when clearing the display for volume decrease, the "else-if" statement checks for "clearLevelBar" as true, unlike for volume increase,
    where it checks for "clearLevelBar" as false.   */