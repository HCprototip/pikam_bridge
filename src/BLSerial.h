#include <Arduino.h>
#include <BluetoothSerial.h>

#define BLS_LONG_SERIAL 255

BluetoothSerial SerialBT;

char BLS_BUF[BLS_LONG_SERIAL];
char BLS_BUF_ULTIM[BLS_LONG_SERIAL];
byte BLS_BUF_INDEX;

enum BLS_MESSAGE_TYPE {PETI, COMANDO, BLS_MESSAGE_ERROR = -1} MESSAGE_TYPE;

void nada(){
}
void BTAuthCompleteCallback(boolean success) {
  if (success) {
    Serial.println("DISPOSITIVO EMPAREJADO");
  } else {
    Serial.println("DESEMPAREJADO");
  }
}
bool Setup_BLS(){    
  SerialBT.begin("ESP32_BT_BRIDGE");  
  SerialBT.onAuthComplete(BTAuthCompleteCallback); 
  SerialBT.isClosed();
  _sp("BT ADRESS: ");_spln(SerialBT.getBtAddressString());
  return true;
}