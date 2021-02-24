#include "RFread.h"
void moveTray(){
  //if(!capsuleInterface1 && !capsuleInterface2){
    if(trayDirection != trayDirectionFeedback ) traySpeed = 50;
    traySpeed = min(traySpeed + 1,200);
    // Serial.println(traySpeed);
    // Serial.println(trayDirection);

    if(trayDirection && !trayClosed ) {
      analogWrite(TRAY_DIR_IN,traySpeed);
      analogWrite(TRAY_DIR_OUT,0);
   } 
   else if(!trayDirection && !trayOpen){
     if(0){
      analogWrite(TRAY_DIR_OUT,0);
      analogWrite(TRAY_DIR_IN,0); 
      //shirscode();
     }
     else{
      analogWrite(TRAY_DIR_OUT,traySpeed);
      analogWrite(TRAY_DIR_IN,0); 
     }
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
    int PCAdata = 0b111000;
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
  bool pistonStuk = false;
  bool pistonIsHome = false;
  while (!mixser.getOptic() || !peripheral.getOptic() || !pistonIsHome) {
    EVERY_N_MILLISECONDS( 100 ) {
    readSensors();
    redSensors = true;}
    if (!peripheral.getOptic()) moveStep(peripheral,2.5, -669);//random high number
    if(!pistonStuk && !pistonIsHome){
        moveStep(piston,2.5,-669);//random high number
        if(piston.isMotorStuck(-1,redSensors)){
          pistonStuk = true;
          piston.setPos(0);
        }
    }
    if(pistonStuk) if(moveStep(piston,2.5,0.2)) pistonIsHome = true;
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
  double surface = 9;
  bool redSensors = true;
  int mixsingStage = 0;
  int interfaceAtempts = 0;
  int mixsingCycles = 2;
  int curentMixsingCycles = 0;
  if(mixedCapsule) mixsingCycles = 1;
  while(mixsingStage < 668){
    EVERY_N_MILLISECONDS( 100 ) {
    printStatos();
    // Serial.print(mixser.getPos());
    // Serial.println("  mixser");
    // Serial.print(piston.getPos());
    // Serial.println("  piston");
    // Serial.print(peripheral.getPos());
    // Serial.println("  peripheral");
    readSensors();
    redSensors = true;
    }
    switch (mixsingStage) {
      case -1://exsit stage
        homing();
        mixsingStage = 669;// high random number to get out of the while loop
        break;
      case 0://tring to interface
        //spinMixser(200);
        moveStep(peripheral,2.5,surface);
        if(moveStep(mixser,2.5,mixserInterfaceHeight))mixsingStage++;
        break;
      case 1://tring to interface
        spinMixser(200);
        delay(300);
        mixsingStage++;
        break;
      case 2://chaking if interface hapend 
        spinMixser(150);
        if(moveStep(mixser,2.5,mixsingMinHight - 2)){
          if(capsuleInterface2 || capsuleInterface1){
            if (mixedCapsule) mixsingStage += 2; 
            else mixsingStage++;
          }
          else{
            interfaceAtempts ++;
            if(interfaceAtempts >= 2) mixsingStage = -1;//pas the max atemps so go homing
            else mixsingStage -= 2;
          }
        }
        break;
      case 3://Pushes the material 
        //moveStep(mixser,2.5,1);
        spinMixser(0);        
        if(peripheral.isMotorStuck(-1,redSensors)||moveStep(peripheral,2.5,peripheralCellsHeight))mixsingStage++;
        break;
      case 4://mixsing
        spinMixser(255);
        if(curentMixsingCycles == 2*mixsingCycles)mixsingStage++;
        if(curentMixsingCycles%2 == 0){
          if(moveStep(mixser,2.5,mixsingMaxHight)){
            curentMixsingCycles++;
          }
        }
        else{
          if(moveStep(mixser,2.5,mixsingMinHight)){
          curentMixsingCycles++;
          }
        }
        break;
      case 5://penetration
        finishMixing = true;
        spinMixser(255);
        if(moveStep(mixser,2.5,mixsingMaxHight))mixsingStage++;
        break;
      case 6://penetration
        spinMixser(0);
        moveStep(mixser,2.5,669);//random high number
        if(mixser.isMotorStuck(-1,redSensors))mixsingStage++;
        break;
      case 7: //exstacting
        spinMixser(-150);
        if(moveStep(mixser,2.5,mixsingMaxHight + 2))mixsingStage++;
        break;
      case 8: 
        if(piston.getPos()< pistonMaxHeight-2)spinMixser(-200);
        else spinMixser(0);
        if(moveStep(piston,2.5,pistonCurHeight + amount*((pistonMaxHeight - pistonMinHeight)/circleNumLeds)) || piston.isMotorStuck(-1,redSensors)){
          pistonCurHeight = piston.getPos();
          mixsingStage++;
        }
        break;
      case 9: //exstacting
        spinMixser(-150);
        moveStep(mixser,2.5,0);
        moveStep(piston,2.5,1);
        moveStep(peripheral,2.5,0);
        if(moveStep(mixser,2.5,0) && moveStep(piston,2.5,1) && moveStep(peripheral,2.5,0))mixsingStage=-1;
        break;
    }
    redSensors = false;
  }
  disableMotors();
}
void TestCupsol(){
  homing();
  while(!moveStep(peripheral, 2.5,50)){
    EVERY_N_MILLISECONDS( 100 ) {
    Serial.println(peripheral.getPos());
    readSensors();
  }
  }
  spinMixser(255);
  delay(500);
  spinMixser(0);

  while(!moveStep(piston, 2.5,6)){
    EVERY_N_MILLISECONDS( 100 ) {
    Serial.println(piston.getPos());
    // Serial.println("mixser.getPos()");
    // Serial.print(piston.getPos());
    // Serial.println("piston.getPos()");
    // Serial.println(peripheral.getPos());
    // Serial.println("peripheral.getPos()");
    readSensors();
  }
  }
}