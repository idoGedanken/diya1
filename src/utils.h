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
void readSensors() {
  //int Rghx1A = readWireData(0x1A);
  int Rghx18 = readWireData(sensorAddress);
  trayClosed =((Rghx18 >> 3) % 2);
  trayOpen = ((Rghx18 >> 4) % 2);
  capsuleInterface1 = ((Rghx18 >> 5) % 2);
  capsuleInterface2 = ((Rghx18 >> 6) % 2);
  piston.setOptic((Rghx18 >> 1) % 2);
  peripheral.setOptic((Rghx18 >> 2) % 2);
  mixser.setOptic((Rghx18 >> 0) % 2);

}