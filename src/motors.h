#include "RFread.h"


void moveTray(){
  traySpeed = min(max(traySpeed + 5,200),230);
  if(abs(TrayIna.readShuntCurrent()) > max(0.18,traySpeed/1000.0) && !trayClosed){
    EVERY_N_MILLISECONDS(200){
      closeTrayDirection = true^closeTrayDirection;
      traySpeed = 0;
      writeDataStatus = false;
      }
    }
  if(closeTrayDirection && !trayClosed ) {
    analogWrite(TRAY_DIR_IN,traySpeed);
    analogWrite(TRAY_DIR_OUT,0);
    if ( mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial() && readData()){
        SetCapsulParams();
        // Serial.println( cap->getParam("currentAmount"));
        // Serial.println( cap->getParam("mixed"));   
    }
    writeDataStatus = true;
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
  if(!writeDataStatus){
    writeToFlash(createWriteBuffer(),getParam("UID"));
  }
  writeDataStatus = true;
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
  int mixserStage = 0;
  if (mixser.getPos() >= 0.5)mixserStage++;
  unsigned long homingTimer = micros();
  while (mixserStage < 3 || !peripheral.getOptic() || !pistonIsHome) {
    EVERY_N_MILLISECONDS( 50 ) {
      readSensors();
    }
    if(micros() - homingTimer > 30000000){
      Serial.println("homing doesnt work");
      spinMixser(0);
      return;
    }
    if (!peripheral.getOptic()) moveStep(peripheral,-669);//random high number
    if(!pistonStuk && !pistonIsHome){
        moveStep(piston,-669);//random high number
        if(piston.isMotorStuck(200)){
          pistonStuk = true;
          piston.setPos(0);
        }
    }
    if(pistonStuk) if(moveStep(piston,0.2)) pistonIsHome = true;
    switch (mixserStage)
    {
    case 0:
      if (mixser.getOptic()) mixserStage++;
        spinMixser(-150);
        moveStep(mixser, -669);//random high number
      break;
    case 1:
      if (moveStep(mixser, 19.7)) mixserStage++;
        spinMixser(0);
        //random high number
      break;
    case 2:
      if (mixser.getOptic()){
        spinMixser(0);
        mixserStage++;
      }
        spinMixser(-150);
        moveStep(mixser, -669);//random high number
      break;
    case 3:
      break;
    }
    }
    if (!mixser.getOptic()) {
      spinMixser(-150);
      moveStep(mixser, -669);//random high number
    }
    else spinMixser(0);
  }
#define EXSIT -1
#define INTERFACE 0 
#define INTERFACE_ASSURANCE 2 
#define PUSH_EXTERNAL_MATERIALS 3 
#define MIXSING 4 
#define PENETRATION 5 
#define DELIVER_THE_CREAM 8
#define PENETRATION 5 
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
      case EXSIT:
        homing();
        mixsingStage = 669;// high random number to get out of the while loop
        break;
      case INTERFACE://tring to interface
        moveStep(peripheral,surface);
        if(moveStep(mixser,mixserInterfaceHeight))mixsingStage = INTERFACE + 1;
        break;
      case INTERFACE + 1://tring to interface
        spinMixser(200);
        delay(300);
        mixsingStage = INTERFACE_ASSURANCE;
        break;       
      case INTERFACE_ASSURANCE://chaking if interface hapend 
        spinMixser(150);
        if(moveStep(mixser,mixsingMinHight - 2)  ){
          if(capsuleInterface2 || capsuleInterface1){
            if (mixedCapsule) mixsingStage += 2; 
            else mixsingStage = PUSH_EXTERNAL_MATERIALS;
          }
          else{
            spinMixser(0);
            interfaceAtempts ++;
            if(interfaceAtempts >= 2) mixsingStage = EXSIT;//pas the max atemps so go homing
            else mixsingStage = INTERFACE;
          }
        }
        break;
      case PUSH_EXTERNAL_MATERIALS://peripheral pushes the material 
        spinMixser(0);        
        if(moveStep(peripheral,peripheralCellsHeight))mixsingStage = MIXSING;
        break;
      case MIXSING://mixsing
        spinMixser(255);
        if(curentMixsingCycles == 2*mixsingCycles)mixsingStage = PENETRATION;
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
      case PENETRATION:
        mixedCapsule = true;
        spinMixser(255);
        if(moveStep(mixser,mixsingMaxHight))mixsingStage = PENETRATION + 1;
        break;
      case PENETRATION + 1://penetration
        spinMixser(0);
        if(moveStep(mixser,mixserPenetrationHight) || mixser.isMotorStuck()) mixsingStage = PENETRATION + 2;
        break;
      case PENETRATION + 2: //exstacting
        spinMixser(-150);
        if(moveStep(mixser,mixsingMaxHight + 2))mixsingStage = DELIVER_THE_CREAM;
        break;
      case DELIVER_THE_CREAM: 
        if(piston.getPos()< pistonMaxHeight-3)spinMixser(-200);
        else spinMixser(0);
        if(moveStep(piston,(pistonMinHeight + tank*((amuntUsed+amount)/circleNumLeds)))){
          amuntUsed = amuntUsed+amount;
          mixsingStage = EXSIT;
        }
        break;
      // case 9: 
      //   spinMixser(0);
      //   moveStep(mixser,1);
      //   moveStep(piston,1);
      //   moveStep(peripheral,1);
      //   if(moveStep(mixser,0) && moveStep(piston,1) && moveStep(peripheral,0))mixsingStage=EXSIT;
      //   break;
    }
  }
  disableMotors();
}
// void mixR(){
//   homing();
//   double tank = pistonMaxHeight - pistonMinHeight;
//   double surface = 9;
//   int mixsingStage = 0;
//   int interfaceAtempts = 0;
//   int mixsingCycles = 2;
//   int curentMixsingCycles = 0;
//   if(mixedCapsule) mixsingCycles = 1;
//   while(mixsingStage < 668){
//     EVERY_N_MILLISECONDS( 100 ) {
//       printStatos();
//       // Serial.print(mixser.getPos());
//       // Serial.println("  mixser");
//       // Serial.print(piston.getPos());
//       // Serial.println("  piston");
//       // Serial.print(peripheral.getPos());
//       // Serial.println("  peripheral");
//       readSensors();
//     }
//     switch (mixsingStage) {
//       case -1://exsit stage
//         homing();
//         mixsingStage = 669;// high random number to get out of the while loop
//         break;
//       case 0://tring to interface
//         //spinMixser(200);
//         //moveStep(peripheral,surface);
//         moveStep(piston,5);
//         if(moveStep(mixser,mixserInterfaceHeight))mixsingStage++;
//         break;
//       case 1://tring to interface
//         spinMixser(200);
//         delay(300);
//         mixsingStage++;
//         break;
//       case 2://tring to interface
//         spinMixser(150);
//         moveStep(mixser,15);
//         if(moveStep(piston,11) && moveStep(mixser,15) ) mixsingStage++;
//         break;        
//       case 3://mixsing
//         spinMixser(255);
//         if(curentMixsingCycles == 2*mixsingCycles)mixsingStage++;
//         if(curentMixsingCycles%2 == 0){
//           if(moveStep(mixser,25)){
//             curentMixsingCycles++;
//           }
//         }
//         else{
//           if(moveStep(mixser,17)){
//           curentMixsingCycles++;
//           }
//         }
//         break;
//       case 4://penetration
//         mixedCapsule = true;
//         spinMixser(255);
//         if(moveStep(mixser,mixsingMaxHight))mixsingStage++;
//         break;
//       case 5://penetration
//         spinMixser(0);
//         if(moveStep(mixser,mixserPenetrationHight) || mixser.isMotorStuck()) mixsingStage++;
//         break;
//       case 6: //exstacting
//         spinMixser(-150);
//         if(moveStep(mixser,mixsingMaxHight + 2))mixsingStage++;
//         break;
//       case 7: 
//         if(piston.getPos()< pistonMaxHeight-3)spinMixser(-200);
//         else spinMixser(0);
//         if(moveStep(piston,pistonMinHeight + tank*(amuntUsed+amount)/circleNumLeds) || piston.isMotorStuck()){
//           amuntUsed = amuntUsed+amount;
//           mixsingStage++;
//         }
//         break;
//       case 8: //exstacting
//         spinMixser(0);
//         moveStep(mixser,0);
//         moveStep(piston,1);
//         moveStep(peripheral,0);
//         if(moveStep(mixser,0) && moveStep(piston,1) && moveStep(peripheral,0))mixsingStage=-1;
//         break;
//     }
//   }
//   disableMotors();
// }
void TestCupsol(){
  while(1){
    moveStep(mixser, 50000);
    EVERY_N_MILLISECONDS( 100 ) {
      Serial.println(mixser.getPos());
      readSensors();
    }
  }
}