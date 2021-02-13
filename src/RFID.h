#include "BTCom.h"
//*************************************************************Read RFID tag data **********************************************************
  void RFSetup(){
  SPI.begin(14,12,13,15);                                                  // Init SPI bus
  mfrc522.PCD_Init();                                              // Init MFRC522 card
  Serial.println(F("Read personal data on a MIFARE PICC:")); 
  Serial.println("Setup Complete.");
  Serial.println("");
  delay(1000);
  }

byte* readblock(uint8_t pageAddr, byte size1 ) {
  //Serial.println(F("Reading data ... "));
  //data in 4 block is readed at once.
  status = (MFRC522::StatusCode) mfrc522.MIFARE_Read(pageAddr, buff, &size1);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("MIFARE_Read() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return NULL;
  }
  for (byte i = 0; i < 16; i++) {
    value[i] = buff[i];
  }
  return value;
}
bool RFreadData(byte size1) {
  uint8_t pageAddr = 0;
  int intVal = 0;
  for (int i = 0; i <= END_BLOCK; i++){ 
    blockRes = readblock( pageAddr + i, size1 );
    if (blockRes != NULL) {
      for (int j = 0; j < 4; j++){
        intVal += blockRes[j];
        data[i][j]= blockRes[j];
        if (j != 3) {}
      }
    } else return false;
  }
  RFIntID = intVal;
  Serial.println(intVal);
  return true;
  Serial.println("Read RF data succesfully");
}
