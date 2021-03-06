#include "rfCapsule.h"

class Capsule
{
  private:
  chipMapper * mapper;
   
  public:
   //byte data[45][4];
   Capsule() { // Constructor
    this->mapper = new chipMapper();
    this->mapper->initMap();
  }
   unsigned int getParam(String key){
     DataTypeRfid * dataType;
     unsigned int dataInLocation = 0;
     dataType = this->mapper->findInMap(key);
     if(dataType){
        // Serial.println("block");
        // Serial.println(dataType->block);
        // Serial.println("offset");
        // Serial.println(dataType->offset);
        dataInLocation = (unsigned int)data[dataType->block][dataType->offset];
        return dataInLocation;
     }else{
        Serial.println(F("Uknown key"));
        return 0;
     }
  }
 int setParam(String key,unsigned int value){
     DataTypeRfid* dataType;
     int numOfBytes = 0;
     int block = 0;
     int offset = 0;
     //unsigned int datainLocation = 0;
     dataType = this->mapper->findInMap(key);
     if(dataType){
      numOfBytes = dataType->numOfBytes;
      block = dataType->block;
      offset = dataType->offset;
      for (int i = 0; i < numOfBytes ;i++){
         if (offset + i < 4){
          //saves the byte in data using bitwise
            data[dataType->block][(dataType->offset) + i ] = value>>(i*8) & 0xFF;
            // Serial.println("new value:");
            // Serial.println(data[dataType->block][(dataType->offset) + i ]);
            
         }else{
          //next block
          data[dataType->block+1][((dataType->offset) + i )%4] = value>>(i*8) & 0xFF;
         }
      }
        return 1;
    }else{
        Serial.println(F("Uknown key"));
        return -1;
     }
 }
 void createDemoChip(){
  int counter =0;
   for (int i = 0; i <= END_BLOCK; i++)
  { 
    for (int j=0;j<4;j++){
       data[i][j] = counter;
       counter = counter +1 ;
    }
  }
}
 void printDemoChip(){
   for (int i = 0; i <= END_BLOCK; i++)
  { 
    for (int j=0;j<4;j++){
        Serial.println(data[i][j]);
       
    }
  }
}
};
Capsule* cap = new Capsule();
String getParam(String param){
    String s = "";
    if (param=="UID"){
        for (int i=0 ;i<4;i++){
            s += String(mfrc522.uid.uidByte[i], HEX);
        }
        return s;
    }
    return String(cap->getParam(param)); ;
}

String createWriteBuffer() {
    String UID = getParam("UID");
    String CapType = getParam("CapType");
    String currentAmount = getParam("currentAmount");
    String height = getParam("height");
    String mixed = getParam("mixed");
    return ( "UID: "+UID+","+" CapType: "+CapType+","+" currentAmount: "+currentAmount+","+" height: "+height+","+" mixed: "+ mixed+"\n");

}
unsigned int myAtoi ( String str ) {
    unsigned int  value = 0;

    for (int i = 0; i < str.length(); i++) {
      value *=  10;
      value += (int) ((str[i]) - '0');
    }
    return value;
}
void extractUIDFromFlash(String flashBuffer,int startIndex,int endIndex) {
    int valIndex ;
    unsigned int val;
    String valBuff;
    String capsuleLine = flashBuffer.substring(startIndex,endIndex);
    // Serial.println(capsuleLine);
    int numberofelements = 0; for(auto c: flashParams) { numberofelements++; };
    for (int i=0;i<numberofelements;i++){
      valBuff = "";
      valIndex = capsuleLine.indexOf(flashParams[i]+": ");
      while( valIndex!=capsuleLine.length()&& capsuleLine[valIndex]!=' '){valIndex++;}
      valIndex++;
      while (valIndex!=capsuleLine.length()&& (capsuleLine[valIndex]!=','&&capsuleLine[valIndex]!='\n'))
        {
          valBuff += capsuleLine[valIndex];
          valIndex++;
        }
        // Serial.println("val:");
        // Serial.println(valBuff);
        val = myAtoi(valBuff);
        // Serial.println(val);
        cap->setParam(flashParams[i],val);
    }
}
