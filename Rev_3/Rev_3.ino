


///// Rev_3 /////
///// Added the support for the analog stick. /////
///// Analog Stick is used for scrolling up and down. /////
///// Performed some code cleanup. /////

///// GitHub: @hizbi-github /////



#include <Arduino.h>
#include "U8x8lib.h"
#include "HID-Project.h"



U8X8_SSD1306_128X64_NONAME_HW_I2C displayOLED;      // Creating object of the new U8x8 display library.
                                                    // Note, the default constructor is being called here.


int encoder_Clk = 1;
volatile bool current_Clk_State = HIGH;

int encoder_Data = 0;
volatile bool current_Data_State = HIGH;


#define INTERRUPT_CLK 2        // Setting up Digital Pin 0 for Interrupts from Rotary Encoder


volatile bool encoderStatus = HIGH;
volatile bool encoderBusy = false;

int encoder_BUT = 7;
bool butState_3 = LOW;
bool keyPressed_3 = false;


volatile int volumeLevel = 0;

volatile char volumeDirection = ' ';

bool setVolumeLevel = true;


String stringFullVolume = "||||||||||";
String stringCurrentVol = stringFullVolume;


byte keypadRows [5] = {0, 18, 19, 20, 21};         // Keypad row pins, "0" is for array padding.
byte keypadColumns [5] = {0, 15, 14, 16, 10};      // Keypad column pins, "0" is for array padding.

byte rows = 1;
byte columns = 1;


struct structKeypadMatrix
{
  byte rows = 0;
  byte columns = 0;
  boolean keyPressed = false;

} keypadMatrix;


struct analogStickStruct
{
  int buttonPin = 5;
  int xPin = A9;
  int yPin = A8;
  
  double x = 0;              
  double y = 0;              

  boolean butState = LOW;
  boolean keyPressed = false;
  boolean stickMoved = false;

} analogStick;

volatile bool analogReadOn = false;     // Turning off the Analog Stick reading Fucntion at initial startup


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
unsigned long encoderDebounce = 5;
unsigned long keypadDebounce = 180;
unsigned long displayUpdateInterval = 500;
unsigned long analogStickSwipeInterval = 40;





void pressEncoderBUT ();

void encoderISR ();           // Reacting quickly to Rotary Encoder's Change in Direction usign Interrupts.

void volumeKnob ();           // Increasing/Decreasing the PC volume depending upon the direction of Encoder's rotation.

void setVolume ();            // Sets the volume at a pre-determined level, so that pseudo real volume level can displayed at the OLED Display.


                              
void readMatrixKeypad ();     // Reads the button presses from Keypad matrix by polling it.

void keypadFunctionsLayer_1 ();      // Performs the respective function assigned to Keypad's button for layer 1.

void keypadFunctionsLayer_2 ();      // Performs the respective function assigned to Keypad's button for layer 2.

void monitorSwitch ();        // Switches the screen projection mode over HDMI.

void autoPCLogin ();          // (UNSAFE) Does the automatic password entry and login.



void analogStickReader ();      // Reads the Analog Stick Data from the analog pins.

void moveMouse ();

void analogStickSwipe ();         // For switching the display menu by swiping the Analog Stick

void pressAnalogStick_Btn ();

double rangeMapper (double value, double fromMin, double fromMax, double toMin, double toMax);     // For Mapping two different data ranges



void drawPage_KeypadButtonUpdates (byte firstLine_x, byte firstLine_y, char firstLine, byte secondLine_x, byte secondLine_y, char secondLine);

void drawPage_Volume ();       // Displaying the volume bar graph and level.

void drawPage_General ();



void setup ()
{ 
  
  pinMode(encoder_Clk, INPUT_PULLUP);
  pinMode(encoder_Data, INPUT_PULLUP);
  pinMode(encoder_BUT, INPUT_PULLUP);


  attachInterrupt(INTERRUPT_CLK, encoderISR, CHANGE);

  
  pinMode(keypadRows[1], INPUT_PULLUP);
  pinMode(keypadRows[2], INPUT_PULLUP);
  pinMode(keypadRows[3], INPUT_PULLUP);
  pinMode(keypadRows[4], INPUT_PULLUP);


  pinMode(keypadColumns[1], INPUT_PULLUP);
  pinMode(keypadColumns[2], INPUT_PULLUP);
  pinMode(keypadColumns[3], INPUT_PULLUP);
  pinMode(keypadColumns[4], INPUT_PULLUP);


  //pinMode(analogStick.buttonPin, INPUT_PULLUP);
  pinMode(analogStick.xPin, INPUT);
  pinMode(analogStick.yPin, INPUT);


  previousMillis_RE = millis();      // Initializing the timer for the common always-running Functions.
  previousMillis_KP = millis();      // Initializing the timer for the common always-running Functions.
  previousMillis_DP = millis();      // Initializing the timer for the common always-running Functions.
  previousMillis_8 = millis();

  // Wire.begin();

  displayOLED.begin();

  Keyboard.begin();
  Consumer.begin();

  AbsoluteMouse.begin();
  //Mouse.begin();

  setVolume();           // Sets the volume at a pre-determined level, so that volume level can displayed at the OLED Display.

}  
  


void loop ()
{
    
  currentMillis = millis();
  
  
  pressEncoderBUT();


  analogStickReader ();
  

  if ((currentMillis - previousMillis_8) > analogStickSwipeInterval)
  {
    analogStickSwipe();

    previousMillis_8 = currentMillis;
  }


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


  
  //if (analogStick.stickMoved == true)
  //{
  //  moveMouse();

  //  analogStick.stickMoved = false;
  //}


  if ((showVolumePage == true) && (currentMillis - previousMillis_DP >= displayUpdateInterval))
  {
    drawPage_Volume();         // Volume Level page
  }

} 



///////////////////////////////////////////////////////////////////////////////////////////



// Empty code block for adding code later...
  


///////////////////////////////////////////////////////////////////////////////////////////



void encoderISR()     // ISR for taking in the Encoder readings quickly
{
  
    current_Clk_State = digitalRead(encoder_Clk);
    current_Data_State = digitalRead(encoder_Data);


    if (current_Clk_State == LOW)
    { 
      encoderStatus = !current_Data_State;
    }

    else if (current_Clk_State == HIGH)
    {
      encoderStatus = current_Data_State;
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

    volumeDirection = 'X';

    
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
        volumeLevel = volumeLevel + 5;
              
        Consumer.write(MEDIA_VOL_UP);

        volumeDirection = '>';

        if (volumeLevel == 105)     // Resetting the volume level on OLED Display
        {
          volumeLevel = 100;
        }
      }


      else if (encoderStatus == LOW)      // Down counter for anit-clockwise rotation
      {
        volumeLevel = volumeLevel - 5;
        
        Consumer.write(MEDIA_VOL_DOWN);

        volumeDirection = '<';
        
        if (volumeLevel == -5)      // Resetting the counter
        {
          volumeLevel = 0;
        }
      }

    }

    encoderBusy = false;             // Allowing Arduino to read the Rotary Encoder again

}



///////////////////////////////////////////////////////////////////////////////////////////



void setVolume ()
{ 
  if (setVolumeLevel == true)
  {

    Consumer.write(MEDIA_VOL_MUTE);       // There should be no need to do this, but alas, the volume reset will not work otherwise!
    Consumer.write(MEDIA_VOL_MUTE);

    for (int down = 0; down <= 50; down++)
    {
      Consumer.write(MEDIA_VOL_DOWN);
      delay(50);                            // I am so sorry for using delays, but this is my only hope since this function
    }                                     // in setup().

    for (int up = 0; up <= 13; up++)
    {
      Consumer.write(MEDIA_VOL_UP);      
      delay(50);                          // Sorry again :/
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



void analogStickReader ()     // For reading the Analog Stick Data when requested by the Master Arduino
{
  
  //static int xPrevious = 0;
  //static int yPrevious = 0;


  analogStick.x = analogRead(analogStick.xPin);
  analogStick.y = analogRead(analogStick.yPin);

  //analogStick.x = analogStick.x - 9;        // Applying the offsets to get the accurate data
  //analogStick.y = analogStick.y + 12;       // Applying the offsets to get the accurate data


  analogStick.x = rangeMapper(analogStick.x, -9, 1014, 10, 64);     // Range values determined experimently

  analogStick.y = rangeMapper(analogStick.y, 0, 1023, 100, 0);     // Range values determined experimently


  //if (abs(analogStick.x - xPrevious) >= 10 || abs(analogStick.y - yPrevious) >= 10)
  //{
    //analogStick.stickMoved = true;

  //  xPrevious = analogStick.x;
  //  yPrevious = analogStick.y;
  //}

}



///////////////////////////////////////////////////////////////////////////////////////////



void analogStickSwipe ()        // Funtion that allows to change display pages by swiping the Analog Stick
{

  if ((analogStick.y > 50) && (analogStick.y <= 95))     // Scrolling Up Slow
  {
    analogStickSwipeInterval = 40;
    AbsoluteMouse.move(0, 0, 1);
  }
  else if ((analogStick.y > 95) && (analogStick.y <= 100))     // Scrolling Up Fast
  {
    analogStickSwipeInterval = 50;
    AbsoluteMouse.move(0, 0, 4);
  }


  if ((analogStick.y < 45) && (analogStick.y >= 5))       // Scrolling Down Slow
  {
    analogStickSwipeInterval = 40;
    AbsoluteMouse.move(0, 0, -1);
  }
  else if ((analogStick.y < 5) && (analogStick.y >= 0))       // Scrolling Down Fast
  {
    analogStickSwipeInterval = 50;
    AbsoluteMouse.move(0, 0, -4);
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
  

  //static boolean oneDigitVolume = false;
  //static boolean twoDigitVolume = false;
  //static boolean threeDigitVolume = false;



  if (clearDisplayforNewPage == true)       // Adding some delay before clearning the screen right after showing keypad updates.
  {                                         // The delay is backed into the "if" statement inside which this "drawPage_Volume" function.
    displayOLED.clearDisplay();             // is called.

    clearDisplayforNewPage = false;
  }


  displayOLED.setFont(u8x8_font_amstrad_cpc_extended_f);
  displayOLED.setCursor(2, 0);
  //displayOLED.print(rows);
  displayOLED.drawString(5, 0, "Volume");



  displayOLED.setFont(u8x8_font_courR18_2x3_r);       // Avoid a rookie mistake like me, must use the correct font characters for printing.

  switch(volumeDirection)
  {
    case '>':
    {
      displayOLED.clearLine(2);
      displayOLED.clearLine(3);
      displayOLED.clearLine(4);
            
      displayOLED.drawString(6, 2, ">");

      volumeDirection = ' ';                    // Reseting the indicator for the next Rotary Encoder reading.

      break;
    }

    case '<':
    {
      displayOLED.clearLine(2);
      displayOLED.clearLine(3);
      displayOLED.clearLine(4);
      
      displayOLED.drawString(6, 2, "<");

      volumeDirection = ' ';                    // Reseting the indicator for the next Rotary Encoder reading.

      break;
    }

    case 'X':
    {
      displayOLED.clearLine(2);
      displayOLED.clearLine(3);
      displayOLED.clearLine(4);
      
      displayOLED.drawString(3, 2, "MUTE");

      volumeDirection = ' ';                    // Reseting the indicator for the next Rotary Encoder reading.

      break;
    }
  }



  //if (volumeLevel == 100)               // Centering the volume number text when its at 100.
  //{
  //  displayOLED.setCursor(5, 2);

  //  if (threeDigitVolume == false)      // Clear only once to prevent flickering.
  //  {
  //    displayOLED.clearLine(2);
  //    displayOLED.clearLine(3);
  //    displayOLED.clearLine(4);

  //    oneDigitVolume = false;
  //    twoDigitVolume = false;
  //    threeDigitVolume = true;
  //  }
  //}

  //else if ((volumeLevel >= 10) && (volumeLevel < 100))        // Centering the volume number text when its between 10 and 99 (inclusive).
  //{
  //  displayOLED.setCursor(6, 2);

  //  if (twoDigitVolume == false)        // Clear only once to prevent flickering.
  //  {
  //    displayOLED.clearLine(2);
  //    displayOLED.clearLine(3);
  //    displayOLED.clearLine(4);
      
  //    oneDigitVolume = false;
  //    twoDigitVolume = true;
  //    threeDigitVolume = false;
  //  }
  //}

  //else if (volumeLevel < 10)            // Centering the volume number text when its between 9 and 0 (inclusive).                
  //{
  //  displayOLED.setCursor(7, 2);

  //  if (oneDigitVolume == false)        // Clear only once to prevent flickering.
  //  {
  //    displayOLED.clearLine(2);
  //    displayOLED.clearLine(3);
  //    displayOLED.clearLine(4);

  //    oneDigitVolume = true;
  //    twoDigitVolume = false;
  //    threeDigitVolume = false;
  //  }
  //}



  //displayOLED.setFont(u8x8_font_courR18_2x3_n);

  //displayOLED.print(volumeLevel);



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



double rangeMapper (double value, double fromMin, double fromMax, double toMin, double toMax)     // Just a mapping functon
{                                                                                                 // that can be modified  
  double fromRange = fromMax - fromMin;                                                           // to accept more data types
  double toRange = toMax - toMin;

  double converted = 0.0;

  converted = (value - fromMin) * (toRange / fromRange) + toMin;

  return (converted);
}



///////////////////////////////////////////////////////////////////////////////////////////



// Empty code block for adding code later...



///////////////////////////////////////////////////////////////////////////////////////////





