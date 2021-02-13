#include "Arduino.h"
#include "globals.h"


int findNumInArray(int num, int arr[]){
  for(int i = 0;i < 3; i++) if(arr[i] == num) return i;
  return 0;
}
bool isInWifiData(char a) {
  for (int i = 0; i < 30; i++)if (wifiData.a[i] == a)return true;
  return false;
}
void clenWifiData(){
  for (int i = 0; i < 30; i++)wifiData.a[i] = (char)0;
}
bool isInString(String s ,char a) {
  for (int i = 0; i < 30; i++)if (s[i] == a) return true;
  return false;
}
void copyEnableArry( String notMain[]) {
  for ( int i = 0; i < enabledButtonsArraySize; i++)enabledButtonsArray[i] = notMain[i];
}
bool AddToEndOfWifiData(char a) {
  if (!isInWifiData(a)) {
    for (int i = 0; i < 30; i++) {
      if (!wifiData.a[i]) {
        wifiData.a[i] = a;
        return true;
      }
    }
  }
  return false;
}
void AddStringToEndOfWifiData(String s) {
  for (int i = 0; i < s.length(); i++)AddToEndOfWifiData(s[i]);
}
void callback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param){
  if(event == ESP_SPP_SRV_OPEN_EVT )BTcolor =CRGB ::White;
  else if(event == ESP_SPP_CLOSE_EVT)BTcolor =CRGB::Blue;
}
String BTRead() {
  if (SerialBT.available()) {
   while (SerialBT.available()){
     temp_bt[BTStage] = SerialBT.read();
     BTStage++;
   }
   temp_bt[BTStage]='\0';
   String temp = String(temp_bt);
   temp.trim();
 if (String(temp).equals("createAmountMinus")) 
   AddToEndOfWifiData('S');
 if (String(temp).equals("createAmountPlus")) 
    AddToEndOfWifiData('A');
 if (String(temp).equals("getCID")) {
      SerialBT.print(set_cid);
      SerialBT.print("\n");
      temp="";
  }
 if (String(temp).equals("createMix")) {
   AddToEndOfWifiData('m');
   temp="";
  }
  BTStage=0;  
 }
 return "";
}

//*************************************************************Read RFID tag data **********************************************************
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
char butNameToProtocolString(String bName) {
  if (bName == "Tray")return 'T';
  if (bName == "Play")return 'P';
  if (bName == "Add")return 'A';
  if (bName == "BT")return 'B';
  if (bName == "phone")return 'p';
  if (bName == "Battery")return 'b';
  if (bName == "Sub")return 'S';
  return '?';
}
String hxToButtonName(int hx, String unableArry[]) {
  String result = "";
  for (int butIndex = 0 ; butIndex < enabledButtonsArraySize; butIndex++) {
    char temp = butNameToProtocolString(unableArry[butIndex]);
    if ((hx >> 6) % 2 && temp == 'T')result += "T";
    if ((hx >> 3) % 2 && temp == 'P')result += "P";
    if ((hx >> 5) % 2 && temp == 'A')result +=  "A";
    //if ((hx >> 0) % 2 && temp == 'B')result +=  "B";
    if ((hx >> 1) % 2 && temp == 'p')result +=  "p";
    if ((hx >> 2) % 2 && temp == 'b')result +=  "b";
    if ((hx >> 4) % 2 && temp == 'S')result +=  "S";
  }
  return result;
}
String buttonsPreesed(String unableArry[]){
  String result ="";
  String buttons = hxToButtonName(readWireData(buttonsAddress), unableArry);
  if(buttons.length() >= 2) return result;
  if(isInString(buttons ,'T')&& (trayClosed ||trayOpen)){
    if( millis() - TPressedTimer >= buttonPreesedTime){
      TPressedTimer = millis();
      trayDirection = trayDirection^true;
      trayClosed = false;
      result += "T";
    }
  }
  else TPressedTimer = millis();  
  if(isInString(buttons ,'P')){
    if( millis() - PPressedTimer >= buttonPreesedTime){
      PPressedTimer = millis();
      result += "P";
    }
  }
  else PPressedTimer = millis(); 
  if(isInString(buttons ,'A')){
    if( millis() - APressedTimer >= buttonPreesedTime + ASecondPressedTimer){
      APressedTimer = millis()  ;
      ASecondPressedTimer = 300;
      result += "A";
    }
  }
  else {
    ASecondPressedTimer = 0;
    APressedTimer = millis(); 
  }
  if(isInString(buttons ,'S')){
    if(millis() - SPressedTimer >= buttonPreesedTime + SSecondPressedTimer){
      SPressedTimer = millis();
      SSecondPressedTimer = 300;
      result += "S";
    }
  }
  else{
    SSecondPressedTimer = 0;
    SPressedTimer = millis();
  }
  if(isInString(buttons ,'B')){
    if( millis() - BPressedTimer >= buttonPreesedTime){
      BPressedTimer = millis();
      result += "B";
    }
  }
  else BPressedTimer = millis(); 
  if(isInString(buttons ,'p')){
    if( millis() - pPressedTimer >= buttonPreesedTime + 100){
      pPressedTimer = millis();
      result += "p";
    }
  }
  else pPressedTimer = millis(); 
  if(isInString(buttons ,'b')){
    if( millis() - bPressedTimer >= buttonPreesedTime){
      bPressedTimer = millis();
      result += "b";
    }
  }
  else bPressedTimer = millis(); 
  return result;
}
void readButtons(String unableArry[]) {
  AddStringToEndOfWifiData(buttonsPreesed(unableArry));
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
//*************************************************************Handling the Esp8266 communication data **********************************************************
void sendToEsp8266() {
//  if(wifiData.a != " ")Serial.println(wifiData.a);
  if (isInWifiData('A'))amount += 1;
  if (isInWifiData('S')) amount -= 1;
  amount = min(maxAmount,max(0,amount));
  wifiData.fill = amount;
  wifiData.mixsingTime = 95000 + 10000 ;//+ amount*5000;
  wifiData.BTcolor = BTcolor;
  wifiData.color = RFIDColor[findNumInArray(RFIntID,RFIntIDArray)];
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &wifiData, sizeof(wifiData));
  if (result == ESP_OK)clenWifiData();
  else Serial.println("Error sending the data");
}
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  //Serial.println(wifiData.a);
  wifisStatus = status == ESP_NOW_SEND_SUCCESS ? "S" : "F";
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
  pinMode(mixserDCPin1, OUTPUT);
  pinMode(mixserDCPin2, OUTPUT);
  pinMode(TRAY_DIR_IN, OUTPUT);// set tray pin as output
  pinMode(TRAY_DIR_OUT, OUTPUT);// set tray pin as output
  // Default INA226 address is 0x40
  TrayIna.begin();
  TrayIna.configure(INA226_AVERAGES_1, INA226_BUS_CONV_TIME_1100US, INA226_SHUNT_CONV_TIME_1100US, INA226_MODE_SHUNT_BUS_CONT);
  TrayIna.calibrate(0.01, 4);
  //strcpy(wifiDataFeedback.a,"");
  SerialBT.begin("ESP32test");
  SerialBT.register_callback(callback);
  Wire.begin(); // Initiate the Wire library
  Serial.begin(9600);
  delay(1000);
  setup_register_devices();
  delay(1000);
  // Init ESP-NOW
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_register_send_cb(OnDataSent);
  esp_now_peer_info_t peerInfo;
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("wifi Failed to add peer");
    return;
  }
   //******************* RFID SETUP *************************
  SPI.begin(14,12,13,15);                                                  // Init SPI bus
  mfrc522.PCD_Init();                                              // Init MFRC522 card
  Serial.println(F("Read personal data on a MIFARE PICC:")); 
  Serial.println("Setup Complete.");
  Serial.println("");
  delay(1000);
  readSensors();
  setStepMotorDirs();
  //homing();
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
