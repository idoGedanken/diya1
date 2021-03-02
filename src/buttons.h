#include "motors.h"


char butNameToProtocolString(String bName) {
  if (bName == "Tray")return 'T';
  if (bName == "Play")return 'P';
  if (bName == "Add")return 'A';
  if (bName == "BT")return 'B';
  if (bName == "phone")return 'p';
  if (bName == "Battery")return 'b';
  if (bName == "Sub")return 'S';
  return '?';
}
String hxToButtonName(int hx, String unableArry[]) {
  String result = "";
  for (int butIndex = 0 ; butIndex < enabledButtonsArraySize; butIndex++) {
    char temp = butNameToProtocolString(unableArry[butIndex]);
    if ((hx >> 6) % 2 && temp == 'T')result += "T";
    if ((hx >> 3) % 2 && temp == 'P')result += "P";
    if ((hx >> 5) % 2 && temp == 'A')result +=  "A";
    if ((hx >> 0) % 2 && temp == 'B')result +=  "B";
    //if ((hx >> 1) % 2 && temp == 'p')result +=  "p";
    //if ((hx >> 2) % 2 && temp == 'b')result +=  "b";
    if ((hx >> 4) % 2 && temp == 'S')result +=  "S";
  }
  //Serial.println(result);
  return result;
}
String buttonsPreesed(String unableArry[]){
  String result ="";
  String buttons = hxToButtonName(readWireData(buttonsAddress), unableArry);
  //Serial.println(buttons);
  if(buttons.length() >= 2) return result;
  if(isInString(buttons ,'T')&& (trayClosed ||trayOpen)){
    if( millis() - TPressedTimer >= buttonPreesedTime){
      TPressedTimer = millis();
      closeTrayDirection = closeTrayDirection^true;
      trayClosed = false;
      result += "T";
    }
  }
  else TPressedTimer = millis();  
  if(isInString(buttons ,'P')){
    if( millis() - PPressedTimer >= buttonPreesedTime){
      PPressedTimer = millis();
      result += "P";
    }
  }
  else PPressedTimer = millis(); 
  if(isInString(buttons ,'A')){
    if( millis() - APressedTimer >= buttonPreesedTime + ASecondPressedTimer){
      APressedTimer = millis()  ;
      ASecondPressedTimer = 300;
      result += "A";
    }
  }
  else {
    ASecondPressedTimer = 0;
    APressedTimer = millis(); 
  }
  if(isInString(buttons ,'S')){
    if(millis() - SPressedTimer >= buttonPreesedTime + SSecondPressedTimer){
      SPressedTimer = millis();
      SSecondPressedTimer = 300;
      result += "S";
    }
  }
  else{
    SSecondPressedTimer = 0;
    SPressedTimer = millis();
  }
  if(isInString(buttons ,'B')){
    if( millis() - BPressedTimer >= buttonPreesedTime){
      BPressedTimer = millis();
      result += "B";
    }
  }
  else BPressedTimer = millis(); 
  if(isInString(buttons ,'p')){
    if( millis() - pPressedTimer >= buttonPreesedTime + 100){
      pPressedTimer = millis();
      result += "p";
    }
  }
  else pPressedTimer = millis(); 
  if(isInString(buttons ,'b')){
    if( millis() - bPressedTimer >= buttonPreesedTime + 300){
      bPressedTimer = millis();
      result += "b";
    }
  }
  else bPressedTimer = millis(); 
  return result;
}
void readButtons(String unableArry[]) {
  AddStringToEndOfWifiData(buttonsPreesed(unableArry));
}