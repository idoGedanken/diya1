#include "sensors.h"
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  //Serial.println(wifiData.a);
  wifisStatus = status == ESP_NOW_SEND_SUCCESS ? "S" : "F";
}
void esp822ComSetup(){
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
}
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
  if (result == ESP_OK) clenWifiData();
  //else Serial.println("Error sending the data");
}
