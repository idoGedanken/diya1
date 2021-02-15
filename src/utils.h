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