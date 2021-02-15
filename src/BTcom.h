#include "utils.h"

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
