#include "RFread.h"
void moveTray(){
  if(!capsuleInterface1 && !capsuleInterface2){
    if(trayDirection && !trayClosed  ) {
      analogWrite(TRAY_DIR_IN,200);
    if ( mfrc522.PICC_IsNewCardPresent()){
      if ( mfrc522.PICC_ReadCardSerial()){
        RFreadData(size1);
        mfrc522.PICC_HaltA();
        mfrc522.PCD_StopCrypto1();
        capsuleDetected = true;
        Serial.println(RFIntID);
      }
    }
   }
    else analogWrite(TRAY_DIR_IN,0); 
    if(!trayDirection && !trayOpen){
      analogWrite(TRAY_DIR_OUT,200);
    }
    else analogWrite(TRAY_DIR_OUT,0);
  }
  else{
    analogWrite(TRAY_DIR_OUT,0);
    analogWrite(TRAY_DIR_IN,0);
  }
 //if(abs(TrayIna.readShuntCurrent() > 0.16)) trayClosed = true;
 //if(!trayDirection) trayClosed = false;
}
void spinMixser(int sped){
  if(sped > 0){
   analogWrite(mixserDCPin1,sped);
   analogWrite(mixserDCPin2,0);
  }
  else{
   analogWrite(mixserDCPin1,0);
   analogWrite(mixserDCPin2,abs(sped));
  }
}
// set all step motor pins
void setStepMotorDirs() {
  if (mixser.getDir() != mixser.getDirFeedback() || peripheral.getDirFeedback() != peripheral.getDir() ||  piston.getDirFeedback() != piston.getDir()) {
    int PCAdata = 56;
    if (mixser.getDir())PCAdata -= 8;
    if (peripheral.getDir())PCAdata -= 16;
    if (piston.getDir())PCAdata -= 32 ;
    Wire.beginTransmission(0x1A); // Begin transmission to the Sensor
    Wire.write(0x01); //write to uotput register
    Wire.write(PCAdata); //configure all bits p0-p7 as high output, 0X00 = LOW
    Wire.endTransmission(); // Ends the transmission and transmits the data from the two registers
    mixser.setDirFeedback(mixser.getDir());
    peripheral.setDirFeedback(peripheral.getDir());
    piston.setDirFeedback(piston.getDir());
  }
}
bool moveStep(StepMotor &motor,double speeed, double TargetPos ){
  motor.readEncoder();
  motor.culcMotorDir(TargetPos);
  setStepMotorDirs();
  return motor.Move(speeed, TargetPos);
}
void readSensors() {
  //int Rghx1A = readWireData(0x1A);
  int Rghx18 = readWireData(sensorAddress);
  trayClosed =(true);//(Rghx18 >> 3) % 2);
  trayOpen = ((Rghx18 >> 4) % 2);
  capsuleInterface1 = ((Rghx18 >> 5) % 2);
  capsuleInterface2 = ((Rghx18 >> 6) % 2);
  piston.setOptic((Rghx18 >> 1) % 2);
  peripheral.setOptic((Rghx18 >> 2) % 2);
  mixser.setOptic((Rghx18 >> 0) % 2);
}
void homing() {
  while (!mixser.getOptic() || !peripheral.getOptic() || !piston.getOptic()) {
    EVERY_N_MILLISECONDS( 100 ) {
    readSensors();}
    if (!peripheral.getOptic())moveStep(peripheral,2.5, -20000);
    if (!piston.getOptic()) moveStep(piston,2.5, -20000);
    if (!mixser.getOptic()) {
      spinMixser(-220);
      moveStep(mixser,2.5, -20000);
    }
    else spinMixser(0);
  }
}

void mix(){
  //homing();
  bool mixedCapsule = false;
  double peripheralCellsHeight = 20;
  bool redSensors = true;
  int mixserLestMove = micros();
  double mixserPosFeedback = 0;
  int mixsingStage = 0;
  int interfaceAtempts = 0;
  int mixsingCycles = 2;
  int curentMixsingCycles = 0;
  if(mixedCapsule) mixsingCycles= 1;
  while(mixsingStage < 668){
    EVERY_N_MILLISECONDS( 100 ) {
    //Serial.println(mixsingStage);
    readSensors();
    redSensors = true;
    }
    switch (mixsingStage) {
      case -1:
        homing();
        mixsingStage = 669;// high random number to get out of the while loop
        break;
      case 0://tring to interface
        spinMixser(150);
        if(moveStep(mixser,2.5,24.5))mixsingStage++;
        break;
      case 1://chaking if interface hapend 
        spinMixser(0);
        if(moveStep(mixser,2.5,6)){
          if(capsuleInterface1){
            if (mixedCapsule) mixsingStage += 2; 
            else mixsingStage++;
          }
          else{
            interfaceAtempts ++;
            if(interfaceAtempts >= 2) mixsingStage = -1;//pas the max atemps so go homing
            else mixsingStage--;
          }
        }
        break;
      case 2://Pushes the material 
        if(moveStep(peripheral,2.5,peripheralCellsHeight))mixsingStage++;
        break;
      case 3://mixsing
        spinMixser(255);
        if(curentMixsingCycles == 2*mixsingCycles)mixsingStage++;
        if(curentMixsingCycles%2 == 0){
          if(moveStep(mixser,2.5,23)){
            curentMixsingCycles++;
          }
        }
        else{
          if(moveStep(mixser,2.5,12)){
          curentMixsingCycles++;
          }
        }
        break;
      case 4://penetration
        spinMixser(150);
        if(moveStep(mixser,2.5,23))mixsingStage++;
        break;
      case 5://penetration
        spinMixser(0);
        moveStep(mixser,1,669);//random high number
        if(mixserPosFeedback != mixser.getPos())mixserLestMove = micros();
        if(micros()-mixserLestMove > 100000 && !redSensors)mixsingStage++;
        mixserPosFeedback = mixser.getPos();
        break;
      case 6: 
        if(moveStep(mixser,2.5,23))mixsingStage++;
        break;
      case 7: 
        if(moveStep(piston,2.5,pistonCurHeight + amount*(pistonMaxHeight - pistonMinHeight)))mixsingStage=669;
        break;
    }
    redSensors = false;
  }
}