#include "RFID.h"
void motorSetup(){
  pinMode(mixserDCPin1, OUTPUT);
  pinMode(mixserDCPin2, OUTPUT);
  pinMode(TRAY_DIR_IN, OUTPUT);
  pinMode(TRAY_DIR_OUT, OUTPUT);
  // Default INA226 address is 0x40
  TrayIna.begin();
  TrayIna.configure(INA226_AVERAGES_1, INA226_BUS_CONV_TIME_1100US, INA226_SHUNT_CONV_TIME_1100US, INA226_MODE_SHUNT_BUS_CONT);
  TrayIna.calibrate(0.01, 4);
  }
void disableStepMotors() {
    Wire.beginTransmission(0x1A); // Begin transmission to the Sensor
    Wire.write(0x01); //write to uotput register
    Wire.write(0xff); //configure all bits p0-p7 as high output, 0X00 = LOW
    Wire.endTransmission(); // Ends the transmission and transmits the data from the two registers
}
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
