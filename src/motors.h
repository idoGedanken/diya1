#include "RFread.h"
void moveTray(){
  traySpeed = min(max(traySpeed + 1,180),255);
  if(abs(TrayIna.readShuntCurrent()) > max(0.18,traySpeed/1000.0) && !trayClosed){EVERY_N_MILLISECONDS(200){
    closeTrayDirection = true^closeTrayDirection;
    traySpeed = 0;
    }
    }
  if(closeTrayDirection && !trayClosed ) {
    analogWrite(TRAY_DIR_IN,traySpeed);
    analogWrite(TRAY_DIR_OUT,0);
    if ( mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial() && readData()){
        SetCapsulParams();
        // Serial.println( cap->getParam("CapType"));
        // Serial.println( cap->getParam("currentAmount"));
        // Serial.println( cap->getParam("mixed"));
        
    }
    writeDataStatus = false;
  } 
  else if(!closeTrayDirection && !trayOpen){
    analogWrite(TRAY_DIR_OUT,traySpeed);
    analogWrite(TRAY_DIR_IN,0); 
    if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()){
      writeDataStatus = writeData() || writeDataStatus;
      }
    }
// }
else{
  if(!writeDataStatus){writeToFlash(createWriteBuffer(),getParam("UID"));
    Serial.println(readFlash());}
    writeDataStatus = true;
    traySpeed = 0;
    analogWrite(TRAY_DIR_OUT,0);
    analogWrite(TRAY_DIR_IN,0);
  }
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
bool moveStep(StepMotor &motor, double TargetPos,double speeed=3.4 ){
  motor.readEncoder();
  motor.calcMotorDir(TargetPos);
  setStepMotorDirs();
  return motor.move(TargetPos, speeed);
}

void homing() {
  bool pistonStuk = false;
  bool pistonIsHome = false;
  unsigned long homingTimer = micros();
  while (!mixser.getOptic() || !peripheral.getOptic() || !pistonIsHome) {
    EVERY_N_MILLISECONDS( 50 ) {
    readSensors();
    }
    if(micros() - homingTimer > 20000000){
      Serial.println("homing doesnt work");
      spinMixser(0);
      return;
    }
    if (!peripheral.getOptic()) moveStep(peripheral,-669);//random high number
    if(!pistonStuk && !pistonIsHome){
        moveStep(piston,-669);//random high number
        if(piston.isMotorStuck()){
          pistonStuk = true;
          piston.setPos(0);
        }
    }
    if(pistonStuk) if(moveStep(piston,0.2)) pistonIsHome = true;
    if (!mixser.getOptic()) {
      spinMixser(-150);
      moveStep(mixser, -669);//random high number
    }
    else spinMixser(0);
  }
}


void mix(){
  homing();
  double tank = pistonMaxHeight - pistonMinHeight;
  double surface = 9;
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
    }
    switch (mixsingStage) {
      case -1://exsit stage
        homing();
        mixsingStage = 669;// high random number to get out of the while loop
        break;
      case 0://tring to interface
        //spinMixser(200);
        //moveStep(peripheral,surface);
        if(moveStep(mixser,mixserInterfaceHeight))mixsingStage++;
        break;
      case 1://tring to interface
        spinMixser(200);
        delay(300);
        mixsingStage++;
        break;
      case 2://tring to interface
        spinMixser(0);
        moveStep(mixser,15);
        if(moveStep(piston,10) && moveStep(mixser,10) ) mixsingStage++;
        break;        
      case 15://chaking if interface hapend 
        spinMixser(150);
        if(moveStep(mixser,mixsingMinHight - 2)  ){
          if(capsuleInterface2 || capsuleInterface1){
            if (mixedCapsule) mixsingStage += 2; 
            else mixsingStage++;
          }
          else{
            spinMixser(0);
            interfaceAtempts ++;
            if(interfaceAtempts >= 2) mixsingStage = -1;//pas the max atemps so go homing
            else mixsingStage -= 2;
          }
        }
        break;
      case 3://peripheral pushes the material 
        //moveStep(mixser,1);
        spinMixser(0);        
        if(moveStep(peripheral,peripheralCellsHeight))mixsingStage++;
        break;
      case 4://mixsing
        spinMixser(255);
        if(curentMixsingCycles == 2*mixsingCycles)mixsingStage++;
        if(curentMixsingCycles%2 == 0){
          if(moveStep(mixser,mixsingMaxHight)){
            curentMixsingCycles++;
          }
        }
        else{
          if(moveStep(mixser,mixsingMinHight)){
          curentMixsingCycles++;
          }
        }
        break;
      case 5://penetration
        mixedCapsule = true;
        spinMixser(255);
        if(moveStep(mixser,mixsingMaxHight))mixsingStage++;
        break;
      case 6://penetration
        spinMixser(0);
        if(moveStep(mixser,mixserPenetrationHight) || mixser.isMotorStuck()) mixsingStage++;
        break;
      case 7: //exstacting
        spinMixser(-150);
        if(moveStep(mixser,mixsingMaxHight + 2))mixsingStage++;
        break;
      case 8: 
        if(piston.getPos()< pistonMaxHeight-3)spinMixser(-200);
        else spinMixser(0);
        if(moveStep(piston,pistonMinHeight + tank*(amuntUsed+amount)/circleNumLeds) || piston.isMotorStuck()){
          amuntUsed = amuntUsed+amount;
          mixsingStage++;
        }
        break;
      case 9: //exstacting
        spinMixser(0);
        moveStep(mixser,0);
        moveStep(piston,1);
        moveStep(peripheral,0);
        if(moveStep(mixser,0) && moveStep(piston,1) && moveStep(peripheral,0))mixsingStage=-1;
        break;
    }
  }
  disableMotors();
}
void TestCupsol(){
  for(int i = 0 ; i < 15000;i++){
    digitalWrite(27, HIGH);
    digitalWrite(27, LOW);
    delay(1);
  }
  while(1){
    // moveStep(mixser, 50000);
    // EVERY_N_MILLISECONDS( 100 ) {
    //   Serial.println(mixser.getPos());
    //   readSensors();
    // }
  }
}