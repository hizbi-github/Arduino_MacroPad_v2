

///// Rev_1 /////
///// Rotary Encoder works perfectly for controlling the PC volume. /////
///// The Encoder button is used for muting the volume. /////
///// Since only one interrupt pin is used for Rotary Encoder, its more sensitive on one direction. /////
///// Debouncing yet to be implemented for the Rotary Encoder. /////
///// 4x4 Matrix Keypad works now, although still needs more optimization. /////
///// Keypad Matrix code might need complete overhaul :/ /////
///// Some sample HID fucntions and Macros also added for the Keypad buttons. /////
///// Flash size is 100% (28658 Bytes). Need to replace the U8glib with its lighter version. /////


///// GitHub: @Hizbi-K /////



#include <Arduino.h>
#include <Wire.h>
#include <U8glib.h>
#include "HID-Project.h"


U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_DEV_0|U8G_I2C_OPT_NO_ACK|U8G_I2C_OPT_FAST);  // Fast I2C / TWI


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

String stringFullVolume = "||||||||||||||||||||||||||||||||||||||||||||||||||";
String stringCurrentVol = stringFullVolume;


byte keypadRows [5] = {0, 18, 19, 20, 21};         // Keypad row pins, "0" is for array padding.
byte keypadColumns [5] = {0, 15, 14, 16, 10}; //{0, 10, 16, 14, 15};      // Keypad column pins, "0" is for array padding.

byte rows = 1;
byte columns = 1;

boolean rowGroundedInCol_1 = false;
boolean rowGroundedInCol_2 = false;
boolean rowGroundedInCol_3 = false;
boolean rowGroundedInCol_4 = false;


struct structKeypadMatrix
{
  byte rows = 0;
  byte columns = 0;
} keypadMatrix;

struct structGenericButtonPress
{
  bool butState = LOW;
  bool keyPressed = false;

  unsigned long previousMillis = 0;
};



unsigned long currentMillis = 0;
unsigned long previousMillis_1 = 0;     // For Button 1
unsigned long previousMillis_2 = 0;     // For Button 2
unsigned long previousMillis_3 = 0;     // For Button 3 (Rotary Encoder's Button)
unsigned long previousMillis_4 = 0;     // For LED (Right)
unsigned long previousMillis_5 = 0;     // For LED (Left)
unsigned long previousMillis_6 = 0;     // Common for calling many different Functions
unsigned long previousMillis_7 = 0;     // For Analog Stick Reception
unsigned long previousMillis_8 = 0;     // For Analog Stick Swiping

unsigned long debounceInterval = 10;
unsigned long encoderDebounce = 6;
unsigned long analogStickSwipeInterval = 250;
unsigned long timeOffMPU = 125;




void pressEncoderBUT ();

void encoderISR ();           // Reacting quickly to Rotary Encoder's Change in Direction usign Interrupts.

void volumeKnob ();

void setVolume ();            // Sets the volume at a pre-determined level, so that volume level can displayed at the OLED Display.

                              // Reads the button presses from Keypad matrix by polling it.

void keypadFunctions ();      // Performs the respective function assigned to Keypad's button.

void pressColBut_1 (int BUT);       // Reading respective Column button presses for Row 1.

void pressColBut_2 (int BUT);

void pressColBut_3 (int BUT);

void pressColBut_4 (int BUT);


void drawPage_1 ();           // Displaying the volume bar graph and level.





void setup ()
{

  if ( u8g.getMode() == U8G_MODE_R3G3B2 )           // Assigning default color values.
  {
    u8g.setColorIndex(255);     // white
  }
  else if ( u8g.getMode() == U8G_MODE_GRAY2BIT ) 
  {
    u8g.setColorIndex(3);         // max intensity
  }
  else if ( u8g.getMode() == U8G_MODE_BW )          // I think we are using this, since our display
  {                                                 // is Black & Blue, so Index(1) means to turn on
    u8g.setColorIndex(1);         // pixel on       // the blue pixels (or white pixels in B&W diplays).
  }
  else if ( u8g.getMode() == U8G_MODE_HICOLOR ) 
  {
    u8g.setHiColorByRGB(255,255,255);
  }
  
  
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


  previousMillis_1 = millis();      // Initializing the timer for the common always-running Functions.
  previousMillis_2 = millis();      // Initializing the timer for the common always-running Functions.



  Wire.begin();

  Keyboard.begin();
  Consumer.begin();

  setVolume();           // Sets the volume at a pre-determined level, so that volume level can displayed at the OLED Display.

}  
  


void loop ()
{
  
  
  currentMillis = millis();
  
  
  pressEncoderBUT();

  volumeKnob();


  pinMode(keypadRows[rows], OUTPUT);
  digitalWrite(keypadRows[rows], LOW); 


  pressColBut_1(keypadColumns[1]);
  pressColBut_2(keypadColumns[2]);
  pressColBut_3(keypadColumns[3]);
  pressColBut_4(keypadColumns[4]);


  if (rowGroundedInCol_1 == false && rowGroundedInCol_2 == false && rowGroundedInCol_3 == false && rowGroundedInCol_4 == false)
  {
    pinMode(keypadRows[1], INPUT_PULLUP); 
    pinMode(keypadRows[2], INPUT_PULLUP); 
    pinMode(keypadRows[3], INPUT_PULLUP); 
    pinMode(keypadRows[4], INPUT_PULLUP); 
  }
  else
  {
    rowGroundedInCol_1 = false;
    rowGroundedInCol_2 = false;
    rowGroundedInCol_3 = false;
    rowGroundedInCol_4 = false;
  }

  if (currentMillis - previousMillis_2 >= 100)
  {
    rows++;
    previousMillis_2 = currentMillis;

    if (rows > 4)
    {
      rows = 1;
    }
  }

  u8g.firstPage();  

  do 
  {  
    drawPage_1();         // LED Status page
  
  } 
  while (u8g.nextPage());


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

    previousMillis_4 = currentMillis;
  }

  else if (current_Clk_State == HIGH)
  {
    encoderStatus = !current_Data_State;

    previousMillis_5 = currentMillis;
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
      
      if ((encoderStatus == HIGH) && ((currentMillis - previousMillis_4) > encoderDebounce))          // Up counter for clockwise rotation
      {
        volumeLevel = volumeLevel + 2;
        
        Consumer.write(MEDIA_VOL_UP);

        if (volumeLevel == 102)     // Resetting the volume level on OLED Display
        {
          volumeLevel = 100;
        }
      }


      else if ((encoderStatus == LOW) && ((currentMillis - previousMillis_5) > encoderDebounce))      // Down counter for anit-clockwise rotation
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
      delay(10);
    }

    for (int up = 0; up < 35; up++)
    {
      Consumer.write(MEDIA_VOL_UP);
      delay(10);
    }

    volumeLevel = 70;
    setVolumeLevel = false;

  }

}



///////////////////////////////////////////////////////////////////////////////////////////




        


///////////////////////////////////////////////////////////////////////////////////////////



void pressColBut_1 (int BUT)       // Using the button from the Rotatory Encoder
{

  static structGenericButtonPress genericBut;  
  
  if (genericBut.butState != digitalRead(BUT))
  {  
    
    if (genericBut.butState == HIGH)
    {
      genericBut.butState = digitalRead(BUT);
      genericBut.keyPressed = true;

      genericBut.previousMillis = currentMillis;

      keypadMatrix.rows = rows;
      keypadMatrix.columns = 1;

    }
    else if (genericBut.butState == LOW)
    {
      genericBut.butState = HIGH;
      genericBut.keyPressed = false;
    }

  }



  if ((genericBut.butState == LOW) && (genericBut.keyPressed == true) && ((currentMillis - genericBut.previousMillis) > debounceInterval))
  {   

    keypadFunctions(); 

    pinMode(keypadRows[keypadMatrix.rows], INPUT_PULLUP); 

    rowGroundedInCol_1 = true;
    
    if (digitalRead(BUT) == LOW)      // No toggling until the finger has been lifted off
    {
      genericBut.keyPressed =  false;
      genericBut.butState = LOW;
    }
    else if (digitalRead(BUT) == HIGH)
    {
      genericBut.keyPressed =  true;
      genericBut.butState = HIGH;
    }
    

  } 


}



///////////////////////////////////////////////////////////////////////////////////////////



void pressColBut_2 (int BUT)       // Using the button from the Rotatory Encoder
{

  static structGenericButtonPress genericBut;  
  
  if (genericBut.butState != digitalRead(BUT))
  {  
    
    if (genericBut.butState == HIGH)
    {
      genericBut.butState = digitalRead(BUT);
      genericBut.keyPressed = true;

      genericBut.previousMillis = currentMillis;

      keypadMatrix.rows = rows;
      keypadMatrix.columns = 2;

    }
    else if (genericBut.butState == LOW)
    {
      genericBut.butState = HIGH;
      genericBut.keyPressed = false;
    }

  }



  if ((genericBut.butState == LOW) && (genericBut.keyPressed == true) && ((currentMillis - genericBut.previousMillis) > debounceInterval))
  {   

    keypadFunctions();
   
    pinMode(keypadRows[keypadMatrix.rows], INPUT_PULLUP); 

    rowGroundedInCol_2 = true;

    if (digitalRead(BUT) == LOW)      // No toggling until the finger has been lifted off
    {
      genericBut.keyPressed =  false;
      genericBut.butState = LOW;
    }
    else if (digitalRead(BUT) == HIGH)
    {
      genericBut.keyPressed =  true;
      genericBut.butState = HIGH;
    }
    

  } 


}



///////////////////////////////////////////////////////////////////////////////////////////



void pressColBut_3 (int BUT)       // Using the button from the Rotatory Encoder
{

  static structGenericButtonPress genericBut;  
  
  if (genericBut.butState != digitalRead(BUT))
  {  
    
    if (genericBut.butState == HIGH)
    {
      genericBut.butState = digitalRead(BUT);
      genericBut.keyPressed = true;

      genericBut.previousMillis = currentMillis;

      keypadMatrix.rows = rows;
      keypadMatrix.columns = 3;
      
    }
    else if (genericBut.butState == LOW)
    {
      genericBut.butState = HIGH;
      genericBut.keyPressed = false;
    }

  }



  if ((genericBut.butState == LOW) && (genericBut.keyPressed == true) && ((currentMillis - genericBut.previousMillis) > debounceInterval))
  {   

    keypadFunctions();
    
    pinMode(keypadRows[keypadMatrix.rows], INPUT_PULLUP); 

    rowGroundedInCol_3 = true;

    if (digitalRead(BUT) == LOW)      // No toggling until the finger has been lifted off
    {
      genericBut.keyPressed =  false;
      genericBut.butState = LOW;
    }
    else if (digitalRead(BUT) == HIGH)
    {
      genericBut.keyPressed =  true;
      genericBut.butState = HIGH;
    }
    

  } 


}



///////////////////////////////////////////////////////////////////////////////////////////



void pressColBut_4 (int BUT)       // Using the button from the Rotatory Encoder
{

  static structGenericButtonPress genericBut;  
  
  if (genericBut.butState != digitalRead(BUT))
  {  
    
    if (genericBut.butState == HIGH)
    {
      genericBut.butState = digitalRead(BUT);
      genericBut.keyPressed = true;

      genericBut.previousMillis = currentMillis;

      keypadMatrix.rows = rows;
      keypadMatrix.columns = 4;

    }
    else if (genericBut.butState == LOW)
    {
      genericBut.butState = HIGH;
      genericBut.keyPressed = false;
    }

  }



  if ((genericBut.butState == LOW) && (genericBut.keyPressed == true) && ((currentMillis - genericBut.previousMillis) > debounceInterval))
  {   

    keypadFunctions();

    pinMode(keypadRows[keypadMatrix.rows], INPUT_PULLUP); 

    rowGroundedInCol_4 = true;
    
    if (digitalRead(BUT) == LOW)      // No toggling until the finger has been lifted off
    {
      genericBut.keyPressed =  false;
      genericBut.butState = LOW;
    }
    else if (digitalRead(BUT) == HIGH)
    {
      genericBut.keyPressed =  true;
      genericBut.butState = HIGH;
    }
    

  } 


}




///////////////////////////////////////////////////////////////////////////////////////////



void keypadFunctions ()
{

  switch (keypadMatrix.rows)
  {
  
    case 1:
    {

      switch (keypadMatrix.columns)      // Switching between the four columns.
      {

        case 1:
        {
          // Keyboard.print("1");
          Keyboard.print("decent12");

          break;
        }

        case 2:
        {
          // Keyboard.print("2");
          Keyboard.write(KEY_ENTER);

          break;
        }

        case 3:
        {
          // Keyboard.print("3");
          Keyboard.press(KEY_LEFT_WINDOWS);
          Keyboard.press(KEY_P);
          Keyboard.releaseAll();
          
          break;
        }

        case 4:
        {
          // Keyboard.print("4");
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
          // Keyboard.print("decent12");
          
          break;
        }

        case 2:
        {
          Keyboard.print("6");
          // Keyboard.write(KEY_ENTER);

          break;
        }

        case 3:
        {
          // Keyboard.print("7");
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
          // Keyboard.print("decent12");

          break;
        }

        case 2:
        {
          Keyboard.print("10");
          // Keyboard.write(KEY_ENTER);

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
          // Keyboard.print("decent12");

          break;
        }

        case 2:
        {
          Keyboard.print("14");
          // Keyboard.write(KEY_ENTER);

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







///////////////////////////////////////////////////////////////////////////////////////////






///////////////////////////////////////////////////////////////////////////////////////////







///////////////////////////////////////////////////////////////////////////////////////////



void drawPage_1 ()       // For displaying PC Volume page
{                        // Very specific positional values have been used for proper alignment. The coordinates can be modified...at your own risk! :p

  u8g.setFont(u8g_font_courR12); 
  u8g.setPrintPos(5, 11);
  u8g.print(rows);
  u8g.drawStr(36, 11, "Volume");


  u8g.setFont(u8g_font_courB18);
  u8g.setPrintPos(50, 40); 

  if (volumeLevel == 100)    
  {
    u8g.setPrintPos(44, 40);
  }
  else if (volumeLevel < 10)
  {
    u8g.setPrintPos(58, 40);
  }

  u8g.print(volumeLevel);
   

  if ((volumeLevel >= 1) && (volumeLevel <= 100))       // Displaying the Volume Bar Graph for different levels by modifying the string.
  {
    stringCurrentVol = stringFullVolume.substring(0, (volumeLevel/2) - 1);
  }


  u8g.setFont(u8g_font_babyr);
  u8g.drawStr(16, 58, stringCurrentVol.c_str());        // Strings can't be accessed directly using "u8g" library, hence ".c_str()" is used to pass the string reference.


  u8g.setFont(u8g_font_courB10r);
  u8g.drawStr(2, 60, "-");
  u8g.drawStr(119, 60, "+");
  
}



///////////////////////////////////////////////////////////////////////////////////////////



/*void monitorSwitch()
{
  
  if ((digitalRead(monSwitch_1) == LOW) && (monPressed_1 == false))       /// Initially, the toggle switch will be at this position, but it won't be detected.
  {
    stateMon_1 = digitalRead(monSwitch_1);
    monPressed_1 = true;
    monPressed_2 = false;

    previousMillis_Def = currentMillis;
  }

  if ((digitalRead(monSwitch_2) == LOW) && (monPressed_2 == false))
  {
    stateMon_2 = digitalRead(monSwitch_2);
    monPressed_2 = true;
    monPressed_1 = false;

    previousMillis_Def = currentMillis;
  }

///////////////////////////////////////

  if ((stateMon_1 == LOW) && ((currentMillis - previousMillis_Def) > debounceInterval_M))
  {
    Keyboard.press(KEY_LEFT_WINDOWS);
    Keyboard.write(KEY_P);
    Keyboard.release(KEY_LEFT_WINDOWS);
    keyPressed_M1 = true;
    stateMon_1 = HIGH;

    previousMillis_M_1 = currentMillis;
  } 


  if ((keyPressed_M1 == true) && ((currentMillis - previousMillis_M_1) > debounceInterval_M))
  {
    Keyboard.write(KEY_P);
    Keyboard.write(KEY_P);
    Keyboard.write(KEY_ENTER);
    Keyboard.write(KEY_ESC);
    keyPressed_M1 = false;
  }

///////////////////////////////////////

  if ((stateMon_2 == LOW) && ((currentMillis - previousMillis_Def) > debounceInterval_M))
  {
    Keyboard.press(KEY_LEFT_WINDOWS);
    Keyboard.write(KEY_P);
    Keyboard.release(KEY_LEFT_WINDOWS);
    keyPressed_M2 = true;
    stateMon_2 = true;

    previousMillis_M_2 = currentMillis;
  }


  if ((keyPressed_M2 == true) && ((currentMillis - previousMillis_M_2) > debounceInterval_M))
  {  
    Keyboard.write(KEY_P);
    Keyboard.write(KEY_P);
    Keyboard.write(KEY_ENTER);
    Keyboard.write(KEY_ESC);
    keyPressed_M2 = false;
  }

///////////////////////////////////////

}*/



///////////////////////////////////////////////////////////////////////////////////////////