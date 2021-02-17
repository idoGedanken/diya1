#include "RFread.h"
void moveTray(){
  //if(!capsuleInterface1 && !capsuleInterface2){
    if(trayDirection && !trayClosed  ) {
      analogWrite(TRAY_DIR_IN,200);
      analogWrite(TRAY_DIR_OUT,0);
   } 
   else if(!trayDirection && !trayOpen){
      analogWrite(TRAY_DIR_OUT,200);
      analogWrite(TRAY_DIR_IN,0); 
    }
  // }
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
void disableMotors(){
    Wire.beginTransmission(0x1A); // Begin transmission to the Sensor
    Wire.write(0x01); //write to uotput register
    Wire.write(0); //configure all bits p0-p7 as high output, 0X00 = LOW
    Wire.endTransmission(); // Ends the transmission and transmits the data from the two registers
    mixser.setDirFeedback(mixser.getDir());
    peripheral.setDirFeedback(peripheral.getDir());
    piston.setDirFeedback(piston.getDir());
}
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
  trayClosed =((Rghx18 >> 3) % 2);
  trayOpen = ((Rghx18 >> 4) % 2);
  capsuleInterface1 = ((Rghx18 >> 5) % 2);
  capsuleInterface2 = ((Rghx18 >> 6) % 2);
  piston.setOptic((Rghx18 >> 1) % 2);
  peripheral.setOptic((Rghx18 >> 2) % 2);
  mixser.setOptic((Rghx18 >> 0) % 2);
}
void homing() {
  bool redSensors = true;
  double pistonPosFeedback = piston.getPos();
  double pistonLestMove = micros();
  bool pistonStuk = false;
  bool pistonIsHome = false;
  while (!mixser.getOptic() || !peripheral.getOptic() || !pistonIsHome) {
    EVERY_N_MILLISECONDS( 100 ) {
    //Serial.println(peripheral.getPos());
    readSensors();
    redSensors = true;}
    if (!peripheral.getOptic()) moveStep(peripheral,2.5, -20000);
    if(!pistonStuk && !pistonIsHome){
        moveStep(piston,2,-20000);//random high number
        if(pistonPosFeedback != piston.getPos())pistonLestMove = micros();
        if(micros() - pistonLestMove > 50000 && !redSensors){
        pistonStuk = true;
        piston.setPos(0);
        }
        pistonPosFeedback = piston.getPos();
    }
    if(pistonStuk) if(moveStep(piston,2.5,2)) pistonIsHome = true;
    if (!mixser.getOptic()) {
      spinMixser(-150);
      moveStep(mixser,2.5, -20000);
    }
    else spinMixser(0);
    redSensors = false;
  }
}

void mix(){
  //homing();
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
    //Serial.println(piston.getPos());
    readSensors();
    redSensors = true;
    }
    switch (mixsingStage) {
      case -1://exsit stage
        homing();
        mixsingStage = 669;// high random number to get out of the while loop
        break;
      case 0://tring to interface
        spinMixser(150);
        if(moveStep(mixser,2.5,27))mixsingStage++;
        break;
      case 1://chaking if interface hapend 
        spinMixser(150);
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
        spinMixser(0);
        mixsingStage++;
        if(moveStep(peripheral,2.5,peripheralCellsHeight))mixsingStage++;
        break;
      case 3://mixsing
        spinMixser(150);
        if(curentMixsingCycles == 2*mixsingCycles)mixsingStage++;
        if(curentMixsingCycles%2 == 0){
          if(moveStep(mixser,2.5,24)){
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
      case 6: //exstacting
        spinMixser(-150);
        if(moveStep(mixser,2.5,19))mixsingStage++;
        break;
      case 7: 
        spinMixser(0);
        if(moveStep(piston,2.5,pistonCurHeight + amount*((pistonMaxHeight - pistonMinHeight)/circleNumLeds))){
          pistonCurHeight = piston.getPos();
          mixsingStage=-1;
        }
        break;
    }
    redSensors = false;
  }
  disableMotors();
}