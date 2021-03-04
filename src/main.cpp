#include "Arduino.h"
#include "tests.h"
// wireCom -> globals -> flash-> rfidInOut->rfCapsule-> utils -> BTcom  -> motors -> buttons -> tests -> main



void sendToEsp8266() {
  if (isInWifiData('A'))amount += 1;
  if (isInWifiData('S')) amount -= 1;
  amount = min(maxAmount,amount);
  amount = max(0,amount);
  wifiData.fill = amount;
  wifiData.mixsingTime = 95000 + 10000 ;//+ amount*5000;
  wifiData.BTcolor = BTcolor;
  wifiData.color = RFIDColor[0];//findNumInArray(RFIntID,RFIntIDArray)
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &wifiData, sizeof(wifiData));
  if (result == ESP_OK)clenWifiData();
  else Serial.println("Error sending wifi data");
}
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  wifisStatus = status == ESP_NOW_SEND_SUCCESS ? "S" : "F";
}
void stateMachine() {
  switch (stage) {
    case 'm': //mixsing
      if (finishMixing) {
        AddToEndOfWifiData('r');
        stage = 'r';
      }
      break;
    case 'r'://Redy to mix
      if (isInWifiData('P') && maxAmount != 0 && amount != 0) {
        AddToEndOfWifiData('m');
        stage = 'm';
      }
      if ((!capsuleDetected || !trayClosed) && !isInWifiData('r')) {
        AddToEndOfWifiData('i');
        stage = 'i';
      }
      break;
    case 'i'://
      // if (!trayClosed)trayClosedTime = millis();
      // if((millis() - trayClosedTime) > 15000) capsuleDetected = true; // RFID overide 
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
  Serial.begin(9600);                                           // Initialize serial communications with the PC
  SPI.begin(14,12,13,15);                                                  // Init SPI bus
  mfrc522.PCD_Init();
  if (!SPIFFS.begin(true)){
    Serial.println("Error: mounting SPIFFS");
    return;
  }        
   //******************* Testmode *************************
  if (Serial.available()) {
  delay(2);
  if ((char)Serial.read() == 'Q'){
    Serial.print('Q');
    test();}
  }
  //******************* motors SETUP *************************
  SetCapsulParams();
  Serial.println("SETUP COMPLITED");
  readSensors();
  setStepMotorDirs();
  //TestCupsol();
  homing();
  disableMotors();
  Serial.println("HOMING COMPLITED");
  

}
void loop() {
  // test();

  switch (stage) {
    case 'm': //mixing
      if (stageFeedback != stage) {
        AddToEndOfWifiData('m');
        String ButtonsArray[] = {"Battery", "BT"};
        enabledButtonsArraySize = 2;
        copyEnableArry(ButtonsArray);
        sendToEsp8266();
        mix();
        finishMixing = true;
        maxAmount = circleNumLeds - amuntUsed ;
        cap->setParam("currentAmount",(unsigned int)amuntUsed);
        if( mixedCapsule == true)cap->setParam("mixed",(unsigned int)1);
        else cap->setParam("mixed",(unsigned int)0);
      }
      break;
    case 'r'://Ready to mix
      if (stageFeedback != stage) {
        finishMixing = false;
        AddToEndOfWifiData('r');
        enabledButtonsArraySize = 6;
        String ButtonsArray[] = {"Play", "BT", "Battery", "Tray", "Add", "Sub"};
        if(maxAmount == 0)ButtonsArray[0] = "BT";
        copyEnableArry(ButtonsArray);
      }
      moveTray();
      break;
    case 'i'://init
      if (stageFeedback != stage) {
        AddToEndOfWifiData('i');
        //mAddToEndOfWifiData('b');
        enabledButtonsArraySize = 3;
        String ButtonsArray[] = {"BT", "Battery", "Tray"};
        copyEnableArry(ButtonsArray);
      }
      //setCapsul(1);
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
    delay(2);
    char temp = (char)Serial.read();
    Serial.println(temp);
    if (temp == 'Q'){
      Serial.print('Q');
      test();}
    if (temp == 'b')BTConected = false;
    if (temp == 'B')BTConected = true;
    if (temp == 'c')capsuleDetected = false;
    if (temp == 'C')capsuleDetected = true;
    if (temp == 't')trayClosed = false;
    if (temp == 'T')trayClosed = true;
    if (temp == 'M')finishMixing = true;
    if (temp == 'm')finishMixing = false;
  } 
  stateMachine();
  EVERY_N_MILLISECONDS( 50 ) {
    //setCapsul(0);
    //Serial.println(cap->getParam("CapType"));
    printStatos();
    sendToEsp8266();
    if(amountFeedback != amount) SerialBT.print(set_amount + amount +"\n");
  }
}
