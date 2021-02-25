#ifndef RFIDINOUT_H
#define RFIDINOUT_H
#include "RFID/flash.h"

byte* readblock(uint8_t pageAddr) {
  //data in 4 block is readed at once.
  status = (MFRC522::StatusCode) mfrc522.MIFARE_Read(pageAddr, buff, &size1);
  if (status != MFRC522::STATUS_OK) {
    Serial.println(mfrc522.GetStatusCodeName(status));
    return NULL;
  }
  //Serial.print(F("Readed data: "));
  //Dump a byte array to Serial
  for (byte i = 0; i < 16; i++) {
    value[i] = buff[i];
  }
  return value;
}
void readData() {
  Serial.println("Start reading:");
  uint8_t pageAddr = 0;
  for (int i = 0; i <= END_BLOCK; i++)
  { 
    blockRes = readblock(pageAddr + i);
    if (blockRes != NULL) {
      //data in 4 block is readed at once.
      blockaAscii = "";
      for (int j = 0; j < 4; j++)
      {
        //Serial.print(blockRes[j],HEX);
        data[i][j]= blockRes[j];
        // if (j != 3) {
        //   Serial.print(",");
        // }else{
        //     Serial.println();
        // }
      }
    } else {
      Serial.println(F( "failed reading data: "));
      return;
    }
  }
  Serial.println("Read all the data succesfully");
}
bool writeBytesToBlock(uint8_t pageAddr, byte buff[]) {
    // Serial.print(F("WRITTEN: "));
    //  Serial.print(buff[0]);
    //  Serial.print(buff[1]);
    //  Serial.print(buff[2]);
    //  Serial.print(buff[3]);
  //data is writen in blocks of 4 bytes (4 bytes per page)
  status = (MFRC522::StatusCode) mfrc522.MIFARE_Ultralight_Write(pageAddr, buff, 4);
  if (status != MFRC522::STATUS_OK) {
      Serial.print(F("MIFARE_write() failed: "));
      Serial.println(mfrc522.GetStatusCodeName(status));
      return false;
  }
  Serial.println(F("MIFARE_Ultralight_Write() OK "));
  // Serial.println();
  return true;
}
void writeToFlash(String params, String uid){
    insertUID(params,uid);
}
void readFromFlash(){
    Serial.println(readFlash());
}
bool writeData(){
  uint8_t pageAddr1 = 0x04;
  for (int i = 4; i <=END_BLOCK; i++)
  { 
    if (i==4){
          Serial.print(F("WRITTEN: "));
          Serial.print(data[i][0]);
          Serial.print(data[i][1]);
          Serial.print(data[i][2]);
          Serial.print(data[i][3]);
        if (!writeBytesToBlock( pageAddr1 + (i -4), data[i])){
            Serial.println("Error writing");
            return false;
        }
    }
    
  }
  return true;
}


bool writeByGivenByte(uint8_t pageAddr, int numOfByte, byte newByte) {
  // we are reading the whole block . in our case : only 4 bytes.
  byte* res = readblock(pageAddr);
  if (res) {
    //change the value to the wanted byte
    res[numOfByte] = newByte;
    //override this block
    if (writeBytesToBlock(pageAddr, res ) == false){
      return false;
    }
    return true;
  } else {
    return false;
  }

}

#endif