#include <FastLED.h>
#include <esp_now.h>
#include <WiFi.h>
#include <Wire.h>
#include <BluetoothSerial.h>
#include <analogWrite.h>
#include <EEPROM.h> //esp
#include <SPI.h>
#include <MFRC522.h>
#include <INA226.h>
#include <StepMotor.cpp>
#include "wireCom.h"

StepMotor mixser(25,33);
StepMotor peripheral(26,32);
StepMotor piston(27,35);

double  peripheralCellsHeight = 13.5 ;
double  mixserInterfaceHeight = 24;
double  mixsingMaxHight = 20 ;
double  mixsingMinHight = 8 ;
bool mixedCapsule = false;
double pistonMinHeight = 14;
double pistonCurHeight = pistonMinHeight;
double pistonMaxHeight = 25.3; 
double mixserMaxHeight = pistonMaxHeight + 4; 
double circleNumLeds = 8; 
int amount = 1;
int amountFeedback = 1;
int maxAmount = ((pistonMaxHeight - pistonCurHeight)/(pistonMaxHeight - pistonMinHeight))*circleNumLeds ;
//MOTOR PINS
#define TRAY_DIR_IN 17
#define TRAY_DIR_OUT 23
INA226 TrayIna;
#define mixserDCPin1 18
#define mixserDCPin2 19
int buttonsAddress = 0x19; 
int sensorAddress = 0x18;
bool capsuleDetected = true;
bool finishMixing = false;
bool buttonPreest;
String enabledButtonsArray[7];
int enabledButtonsArraySize = 0;
char stage = 'i';
char stageFeedback = 'f';
//******************* RFID CONFIG *************************
#define RST_PIN         16    // Configuration
#define SS_PIN          15   
#define MAP_SIZE 50 // Amount of bits to write to the rfid
#define END_BLOCK 44 // End of user-data
MFRC522 mfrc522(SS_PIN, RST_PIN);// Create MFRC522 instance
MFRC522::MIFARE_Key key;
MFRC522::StatusCode status;
byte buff[18];
//byte writeBlock[18] = {0x68, 0x65, 0x79,0x02};
byte data[45][4]; // data on chip , used for reading,writing
byte value[18];
byte* blockRes; // result of reading
//String blockaAscii;// result of reading in ascii
uint8_t pageAddr = 0x03;// page we start our reading from
byte size1 = sizeof(buff);
int RFIntID = 2201;
int RFIntIDArray[] ={2201,2185,2187};
CRGB RFIDColor[] = {CRGB(0, 0, 255),CRGB(255, 102, 255),CRGB(204, 255, 51)};
//******************TIMING************************************
unsigned int timeMixsing = 0;
unsigned int TPressedTimer = 0;
unsigned int PPressedTimer = 0;
unsigned int APressedTimer = 0;
unsigned int ASecondPressedTimer = 0;
unsigned int BPressedTimer = 0;
unsigned int pPressedTimer = 0;
unsigned int bPressedTimer = 0;
unsigned int SPressedTimer = 0;
unsigned int SSecondPressedTimer = 0;
unsigned int buttonPreesedTime = 150;
unsigned long trayClosedTime  = 0;
unsigned long mixsingT ;
unsigned long addSubTime  = 0;
//******************SENSORS************************************
bool mixserOptic;
bool InPistonOptic;
bool OutPistonOptic;
bool trayClosed  = true;
bool trayOpen;
bool capsuleInterface1;
bool capsuleInterface2;
bool trayDirection = true;
//******************BT************************************
bool BTConected = false;
bool BTConectedFeedback = false;
String set_amount = String("CreateSetAmount_");
String set_cid = String ("CID_00112233");
char character;
char* temp_bt = new char[20];
int BTStage=0;
bool sentConect = false;
bool sentdisconect = false;
CRGB BTcolor =CRGB ::Blue;
BluetoothSerial SerialBT;
String BTData = "";
//******************* WIFI *************************
String wifisStatus = "";
typedef struct struct_message {
  char a[32];
  CRGB color = CRGB ::White;
  double fill = 0.1;
  double battary = 1;
  int mixsingTime ;
  CRGB BTcolor =CRGB ::Blue;
} struct_message;
struct_message wifiData;
struct_message wifiDataFeedback;
// REPLACE WITH YOUR RECEIVER MAC Address  84:F3:EB:DC:E4:26
uint8_t broadcastAddress[] = {0x84, 0xF3, 0xEB, 0xDC, 0xE4, 0x26};
// { 0x84, 0xF3, 0xEB, 0xDC, 0xE4, 0x26} - black
//{0x84, 0xCC, 0xA8, 0x9F, 0xEE, 0x62} - white








// 9 calls
// double  peripheralCellsHeight = 17 ;
// double  mixserInterfaceHeight = 28;
// double  mixsingMaxHight = 27 ;
// double  mixsingMinHight = 8 ;
// bool mixedCapsule = false;
// double pistonMinHeight = 18.5;
// double pistonCurHeight = pistonMinHeight;
// double pistonMaxHeight = 33; 
// double mixserMaxHeight = pistonMaxHeight + 4; 