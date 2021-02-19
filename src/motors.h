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
    Wire.write(0xff); //configure all bits p0-p7 as high output, 0X00 = LOW
    Wire.endTransmission(); // Ends the transmission and transmits the data from the two registers
    mixser.setDirFeedback(mixser.getDir());
    peripheral.setDirFeedback(peripheral.getDir());
    piston.setDirFeedback(piston.getDir());
    mixser.setDirFeedback(!mixser.getDir());
    piston.setDirFeedback(!piston.getDir());
}
void setStepMotorDirs() {
  if (mixser.getDir() != mixser.getDirFeedback() || peripheral.getDirFeedback() != peripheral.getDir() ||  piston.getDirFeedback() != piston.getDir()) {
    int PCAdata = 0b111110;
    if (mixser.getDir())PCAdata -= 0b1000;
    if (peripheral.getDir())PCAdata -= 0b10000;
    if (piston.getDir())PCAdata -= 0b100000 ;
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

void homing() {
  bool redSensors = true;
  double pistonPosFeedback = piston.getPos();
  double pistonLestMove = micros();
  bool pistonStuk = false;
  bool pistonIsHome = false;
  while (!mixser.getOptic() || !peripheral.getOptic() || !pistonIsHome) {
    EVERY_N_MILLISECONDS( 100 ) {
    readSensors();
    redSensors = true;}
    if (!peripheral.getOptic()) moveStep(peripheral,2.5, -669);//random high number
    if(!pistonStuk && !pistonIsHome){
        moveStep(piston,2,-669);//random high number
        if(pistonPosFeedback != piston.getPos())pistonLestMove = micros();
        if(micros() - pistonLestMove > 30000 && !redSensors){
        pistonStuk = true;
        piston.setPos(0);
        }
        pistonPosFeedback = piston.getPos();
    }
    if(pistonStuk) if(moveStep(piston,2.5,1)) pistonIsHome = true;
    if (!mixser.getOptic()) {
      spinMixser(-150);
      moveStep(mixser,2.5, -669);//random high number
    }
    else spinMixser(0);
    redSensors = false;
  }
}

void mix(){
  homing();
  double peripheralCellsHeight = 19.15;
  bool redSensors = true;
  int mixserLestMove = micros();
  double mixserPosFeedback = 0;
  int mixsingStage = 4;
  int interfaceAtempts = 0;
  int mixsingCycles = 2;
  int curentMixsingCycles = 0;
  if(mixedCapsule) mixsingCycles= 1;
  while(mixsingStage < 668){
    EVERY_N_MILLISECONDS( 100 ) {
    Serial.println(mixser.getPos());
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
        moveStep(peripheral,2.5,10);
        moveStep(piston,2.5,5);
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
        if(moveStep(peripheral,2.5,peripheralCellsHeight))mixsingStage++;
        break;
      case 3://mixsing
        spinMixser(255);
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
        if(moveStep(mixser,2.5,29.5))mixsingStage++;
        break;
      case 5://penetration
        spinMixser(0);
        moveStep(mixser,2.5,669);//random high number
        if(mixserPosFeedback != mixser.getPos())mixserLestMove = micros();
        if(micros()-mixserLestMove > 1500000 && !redSensors)mixsingStage++;
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