#ifndef FLASH_H
#define FLASH_H
#include <Arduino.h>
#include "globals.h"

void writeFile(String data){
  if(data==""){
    return;
  }
  File file = SPIFFS.open("/UnsavedCapsules.txt",FILE_WRITE);
  if(file.print(data)){
    //Serial.println("File was written");
  }else{
    Serial.println("File write failed");
  }
  file.close();
}
/**
  *finds UID in Flash 
  @param String UID: pointer file
  @param String txtFile: Current data in Flash
  @param  int* startIndex: update start pointer of UID
  @param  int* endIndex: update END pointer of UID
*/
int findUID( String UID, String txtFile, int* startIndex, int * endIndex){
  int i;
  *startIndex = txtFile.indexOf("UID: "+UID+", ");
  //Serial.println(*startIndex);
  if (*startIndex <0){
   Serial.println("Error: can not find capsule in cache");
   return 0;
  }
  for (i = *startIndex;i < txtFile.length();i++){
    if (txtFile[i] == '\n'){
      break;
    }
  }
  *endIndex = i+1;
  return 1;
}
/**
  *Deletes UID from file 
  @param File* fileptr: pointer file
  @param String txtFile: Current data in Flash
  @param String UID: the UID to delete
*/
void DeleteUIDFromFile( String txtFile, String UID){
  //File file = SPIFFS.open("/UnsavedCapsules.txt",FILE_WRITE);
  //Serial.println("Before delete: " + txtFile);
  int startIndex, endIndex;
  // Updates the indexes
  if(findUID(UID, txtFile, &startIndex, &endIndex)){
      String capsuleLine = txtFile.substring(startIndex,endIndex);
      // Serial.println("The capsule is: " + capsuleLine);
      txtFile.remove(startIndex,endIndex - startIndex); //remove(startIndex,leght)
      writeFile(txtFile);
  }
}
String readFlash(){
   String txtFile = "";
   File file = SPIFFS.open("/UnsavedCapsules.txt");
   while(file.available()){
    txtFile += char(file.read());
  }
  file.close();
  return txtFile;
}
void insertUID(String UIDDATA,String UID){
  if(UID=="0000"){
    return;//not supposed to be considered
  }
  int startIndex, endIndex;
  String flashBuffer = readFlash();
  // Updates the indexes
  if(findUID(UID, flashBuffer, &startIndex, &endIndex)){
    if (flashBuffer.indexOf(UIDDATA)>0){
      // Serial.println(UIDDATA);
      // Serial.println(flashBuffer);
      // Serial.println("sameDataInFlash!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!11");
      return;
    }else{
      //deletes the prev
      //  Serial.println("updateUID!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1");
      DeleteUIDFromFile( flashBuffer, UID);
    }
  }
  //saves the new one
  writeFile(UIDDATA + readFlash());
}
void clearFlash(){
  writeFile("Flash:\n");
}
#endif