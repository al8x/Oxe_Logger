/**
Explain everytrhing here
 */
 
#include <SPI.h>
#include <SdFat.h>
#include <SdFatUtil.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <TimeLib.h>
#include <DS1307RTC.h>  // a basic DS1307 library that returns time as a time_t

#include "UserDataType.h"  // Edit this include file to change data_t.

#define CS_3204 9
const uint8_t SD_CS_PIN = 10; // SD chip select pin.

const int hallEffectSensorPin = 0;
const int rightButtonPin = 1;
const int middleButtonPin = 2;
const int leftButtonPin = 3;

int rightButtonPressed = false; //boolean for the button 
int middleButtonPressed = false; //boolean for the button 
int leftButtonPressed = false; //boolean for the button 
int magnetDetected = 0 ; //counter for hall effect sensor
int logging=false; //Affect the interrupt, to prevent error when recording

//------------------------------------------------------------------------------
//---------------------------------------------------------- SCREEEN REQUIEREMENT
//------------------------------------------------------------------------------


#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

#define NUMFLAKES 10
#define XPOS 0
#define YPOS 1
#define DELTAY 2

#define LOGO16_GLCD_HEIGHT 16 
#define LOGO16_GLCD_WIDTH  16 
static const unsigned char PROGMEM logo16_glcd_bmp[] =
{ B00000000, B11000000,
  B00000001, B11000000,
  B00000001, B11000000,
  B00000011, B11100000,
  B11110011, B11100000,
  B11111110, B11111000,
  B01111110, B11111111,
  B00110011, B10011111,
  B00011111, B11111100,
  B00001101, B01110000,
  B00011011, B10100000,
  B00111111, B11100000,
  B00111111, B11110000,
  B01111100, B11110000,
  B01110000, B01110000,
  B00000000, B00110000 };

#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif
//------------------------------------------------------------------------------
// ------------------------------------------------   END OF SCREEN DECLARATIONS
//------------------------------------------------------------------------------


//==============================================================================
// Start of configuration constants.
//==============================================================================
// Pin definitions.
//
//
// Digital pin to indicate an error, set to -1 if not used.
// The led blinks for fatal errors. The led goes on solid for SD write
// overrun errors and logging continues.
const int8_t ERROR_LED_PIN = -1;
//------------------------------------------------------------------------------
// File definitions.
//
// Maximum file size in blocks.
// The program creates a contiguous file with FILE_BLOCK_COUNT 512 byte blocks.
// This file is flash erased using special SD commands.  The file will be
// truncated if logging is stopped early.
const uint32_t FILE_BLOCK_COUNT = 256000;

// log file base name.  Must be six characters or less.
#define FILE_BASE_NAME "data"
//------------------------------------------------------------------------------
// Buffer definitions.
//
// The logger will use SdFat's buffer plus BUFFER_BLOCK_COUNT additional
// buffers.
//
#ifndef RAMEND
// Assume ARM. Use total of nine 512 byte buffers.
const uint8_t BUFFER_BLOCK_COUNT = 8;
//
#elif RAMEND < 0X8FF
#error Too little SRAM
//
#elif RAMEND < 0X10FF
// Use total of two 512 byte buffers.
const uint8_t BUFFER_BLOCK_COUNT = 1;
//
#elif RAMEND < 0X20FF
// Use total of five 512 byte buffers.
const uint8_t BUFFER_BLOCK_COUNT = 4;
//
#else  // RAMEND
// Use total of 13 512 byte buffers.
const uint8_t BUFFER_BLOCK_COUNT = 12;
#endif  // RAMEND
//==============================================================================
// End of configuration constants.
//==============================================================================
// Temporary log file.  Will be deleted if a reset or power failure occurs.
#define TMP_FILE_NAME "tmp_log.bin"

// Size of file base name.  Must not be larger than six.
const uint8_t BASE_NAME_SIZE = sizeof(FILE_BASE_NAME) - 1;

SdFat sd;

SdBaseFile binFile;

char binName[13] = FILE_BASE_NAME "00.bin";

// Number of data records in a block.
const uint16_t DATA_DIM = (512 - 4)/sizeof(data_t);

//Compute fill so block size is 512 bytes.  FILL_DIM may be zero.
const uint16_t FILL_DIM = 512 - 4 - DATA_DIM*sizeof(data_t);

struct block_t {
  uint16_t count;
  uint16_t overrun;
  data_t data[DATA_DIM];
  uint8_t fill[FILL_DIM];
};

const uint8_t QUEUE_DIM = BUFFER_BLOCK_COUNT + 2;

block_t* emptyQueue[QUEUE_DIM];
uint8_t emptyHead;
uint8_t emptyTail;

block_t* fullQueue[QUEUE_DIM];
uint8_t fullHead;
uint8_t fullTail;

// Advance queue index.
inline uint8_t queueNext(uint8_t ht) {
  return ht < (QUEUE_DIM - 1) ? ht + 1 : 0;
}
//==============================================================================
// Error messages stored in flash.
#define error(msg) errorFlash(F(msg))
//------------------------------------------------------------------------------
void errorFlash(const __FlashStringHelper* msg) {
  sd.errorPrint(msg);
  ErrorOnScreen(msg);
}
//------------------------------------------------------------------------------
//
void fatalBlink() {
  while (true) {
    if (ERROR_LED_PIN >= 0) {
      digitalWrite(ERROR_LED_PIN, HIGH);
      delay(200);
      digitalWrite(ERROR_LED_PIN, LOW);
      delay(200);
    }
  }
}
//------------------------------------------------------------------------------

// max number of blocks to erase per erase call
uint32_t const ERASE_SIZE = 262144L;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void setup(void) {
  // Here maintain the power supply
  
  //----------------------SCREEN Initialization
  
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3D (for the 128x64)
  display.clearDisplay();
  
   //-------------------------------TIME 
  // set the Time library to use Teensy 3.0's RTC to keep time
  setSyncProvider(getTeensy3Time);
  
  if (timeStatus()!= timeSet) {
    display.println("Unable to sync with the RTC");
    delay(1000);
    } 
 
//-------------------------------------- Pin declaration 

  pinMode(rightButtonPin, INPUT);
  pinMode(middleButtonPin, INPUT);
  pinMode(leftButtonPin, INPUT);
  pinMode(hallEffectSensorPin, INPUT);

  attachInterrupt(hallEffectSensorPin,OneWheelRotation,FALLING);
  attachInterrupt(rightButtonPin,RightBouton,FALLING);
  attachInterrupt(middleButtonPin,MiddleBouton,FALLING);
  attachInterrupt(leftButtonPin,LeftBouton,FALLING);

  pinMode(SD_CS_PIN,OUTPUT);
  pinMode(CS_3204,OUTPUT);
  digitalWrite(SD_CS_PIN,HIGH);
  digitalWrite(CS_3204,HIGH);

  //------------------------------------------SD card system
  if (sizeof(block_t) != 512) {
    error("Invalid block size");
  }
   
  SPI.begin();
  // initialize file system.
  if (!sd.begin(SD_CS_PIN, SPI_FULL_SPEED)) {
    sd.initErrorPrint();
//    while(1){
//      PasDeCarteSD();
//    }
  }
  // Choix a l'entrée de l'interface !!
  OnEstPret();
  
  leftButtonPressed=false;
  middleButtonPressed=false;
  rightButtonPressed=false;
  
}
//------------------------------------------------------------------------------
void loop(void) {
  digitalClockDisplay();

  //Partie setup du programme
  //----------------------------------------------------SETUP
  while (!rightButtonPressed){
    OnEstPret();
    delay(10);
    
    if (leftButtonPressed){
      FormatTheCard();
      leftButtonPressed=false;
 
    }
    else if (middleButtonPressed){
      middleButtonPressed=false;
      setFrequency();
    }  
  }
  rightButtonPressed = false;
  display.clearDisplay();
  
  // Partie logging
  //----------------------------------------------------logging
  HomeScreen();
  while (!leftButtonPressed){
    HomeScreen();
    
    if (rightButtonPressed) { // Launch logging prgramm
      rightButtonPressed = false; // Put back the switch false
      magnetDetected = 0; // We reset the wheel lap sensor to prevent detcteion that occured before we want to record
      logging=true;
      logData();                // The programm logData is stopped from inside by pressing the button
      logging=false;
      
      leftButtonPressed=false; // In case of not desired action during logging
      middleButtonPressed=false;
      rightButtonPressed=false;
    }
  }
  leftButtonPressed = false;
  display.clearDisplay();
}

//**************************************************************************************
//------------------------------- Bouton interrupt ------------------------------------
//**************************************************************************************
void OneWheelRotation(){
  magnetDetected+=1;
}

void RightBouton (){
  if (!logging){
    delay (100);
  }
  if (digitalRead(rightButtonPin) == LOW)
  {
    rightButtonPressed=true;
  }
}

void MiddleBouton (){
  if (!logging){
    delay (100);
  }
  if (digitalRead(middleButtonPin) == LOW)
  {
    middleButtonPressed=true;
  }
}

void LeftBouton (){
 if (!logging){
    delay (100);
  }
  if (digitalRead(leftButtonPin) == LOW)
  {
    leftButtonPressed=true;
  }
}

