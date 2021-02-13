#include "Arduino.h"
#include "esp822Com.h"

// globalse -> helpers -> BTCom -> RFID -> motor -> sensors -> esp822Com
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
  double capsulSerfas = 0;// to do
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
        disableStepMotors();
        mixsingStage = 669;// high random number to get out of the while loop
        break;
      case 0://tring to interface
        spinMixser(150);
        if(moveStep(mixser,2.5,24.5) && moveStep(peripheral,2.5,capsulSerfas))mixsingStage++;
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

void stateMachine() {
  switch (stage) {
    case 'm': //mixsing
      if ((millis() - mixsingT) > 95000 + 10000/* + amount*5000*/) {
       finishMixing = true;
      }
      if (finishMixing) {
        AddToEndOfWifiData('r');
        stage = 'r';
      }
      break;
    case 'r'://Redy to mix
      if (isInWifiData('P')) {
        AddToEndOfWifiData('m');
        stage = 'm';
      }
      if ((!capsuleDetected || !trayClosed) && !isInWifiData('r')) {
        AddToEndOfWifiData('i');
        stage = 'i';
      }
      break;
    case'i'://
      if (!trayClosed)trayClosedTime = millis();
      if((millis() - trayClosedTime) > 15000) capsuleDetected = true;
      if (capsuleDetected && trayClosed && !isInWifiData('i') ) {
        AddToEndOfWifiData('r');
        stage = 'r';
      }
      break;
  }
  if (BTConected && !BTConectedFeedback ) { //BTconected
    AddToEndOfWifiData('C');
    BTConectedFeedback = true;
  }
  if (!BTConected && BTConectedFeedback) { //BTdisConected
    AddToEndOfWifiData('D');
    BTConectedFeedback = false;
  }
}
void setup() {
  motorSetup();
  SerialBT.begin("ESP32test");
  SerialBT.register_callback(callback);
  Wire.begin(); // Initiate the Wire library
  Serial.begin(9600);
  delay(1000);
  setup_register_devices();
  delay(1000);
  // Init ESP-NOW
  esp822ComSetup();
  RFSetup();
  readSensors();
  setStepMotorDirs();
  homing();
  disableStepMotors();
}
void loop() {
  switch (stage) {
    case 'm': //mixsing
      if (stageFeedback != stage) {
        mixsingT = millis();
        AddToEndOfWifiData('m');
        String ButtonsArray[] = {"Battery", "BT"};
        enabledButtonsArraySize = 2;
        copyEnableArry(ButtonsArray);
        //finishMixing = true;
      }
      break;
    case 'r'://Redy to mix
      if (stageFeedback != stage) {
        finishMixing = false;
        AddToEndOfWifiData('r');
        enabledButtonsArraySize = 6;
        String ButtonsArray[] = {"Play", "BT", "Battery", "Tray", "Add", "Sub"};
        copyEnableArry(ButtonsArray);
      }
      moveTray();
      break;
    case'i':
      if (stageFeedback != stage) {
        AddToEndOfWifiData('i');
        //mAddToEndOfWifiData('b');
        enabledButtonsArraySize = 3;
        String ButtonsArray[] = {"BT", "Battery", "Tray"};
        copyEnableArry(ButtonsArray);
      }
      moveTray();
      break;
  } 
  readSensors();
  readButtons(enabledButtonsArray);
  BTRead();
  stageFeedback = stage;
  amountFeedback = amount;
  strcpy(wifiDataFeedback.a, wifiData.a);
  if (Serial.available()) {
    char tamp = Serial.read();
    if (tamp == 'b')BTConected = false;
    if (tamp == 'B')BTConected = true;
    if (tamp == 'c')capsuleDetected = false;
    if (tamp == 'C')capsuleDetected = true;
    if (tamp == 't')trayClosed = false;
    if (tamp == 'T')trayClosed = true;
    if (tamp == 'M')finishMixing = true;
    if (tamp == 'm')finishMixing = false;
  } 
  stateMachine();
  EVERY_N_MILLISECONDS( 50 ) {
    //Serial.println(wifiData.a);
    sendToEsp8266();
    if(amountFeedback != amount) SerialBT.print(set_amount + amount +"\n");
  }
}
