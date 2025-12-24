#include <Arduino.h>
#include <EEPROM.h>
#include <TarvosIII.h>
//#include "c:\Users\Manel\.platformio\lib\TarvosIII\TarvosIII.h"

#define TAR_LONG_SERIAL 255

TarvosIII TRX(&TRX_PORT);

struct TARVOS_config{
    byte net;
    byte adress;
    byte channel;
    byte CS;
    byte dia;
    byte mes;
    bool CheckSumOK(){
        byte cc = channel;
        cc ^= net;
        cc ^= adress;
        return true; //////////////////// CONTROL CS DESACTIVADO
        return CS == cc;        
    }
    bool operator==(TARVOS_config B){
        return ((net == B.net) && (adress == B.adress) && (channel == B.channel));
    }
    void Print(){
        Serial.println("Tarvos config: ");
        Serial.print("  NET:     "); Serial.println(net, HEX);
        Serial.print("  ADRESS:  "); Serial.println(adress, HEX);
        Serial.print("  CHANNEL: "); Serial.println(channel);
    }
};
bool Setup_TRX();
void TRX_Init();
void TRX_Send(char *BUF, byte len);
bool TRX_Config(TARVOS_config, bool save = false);
TARVOS_config TRX_GetConfig();

byte FreqToChannel(float F);



bool TRX_CONFIGURING = false;  // Flag para bloquear Loop_SER durante configuración

char TRX_BUF[TAR_LONG_SERIAL];
char TRX_BUF_ULTIM[TAR_LONG_SERIAL];
byte TRX_BUF_INDEX;

void TRX_Init(){
    #ifdef Q7
    TRX_PORT.begin(9600);
    #else
    TRX_PORT.begin(115200);
    #endif
    TRX.Setup(TRX_PIN_MODE, TRX_PIN_RESET);
}
bool Setup_TRX(){    
    _sp("SETUP TRX: ");
    TRX_Init();
    TARVOS_config config;
    EEPROM.get(EEPROM_DIR, config);
    _sp("EN EPROM: ");
    config.Print();
    if (!config.CheckSumOK() | (config.channel > 0x64) | ((config.adress == 0x00) & (config.net == 0x00))){
        config = {1, 1, 21, 21};
        _spln("ERROR datos configuracion. Por defecto 01:01 864.05");
    }  
    TRX_Config(config, true);

    _spln("SETUP TRX: OK");
    return true; //PP
}
void TRX_Send(char *BUF, byte len){    
    for (byte c = 0; c < len; TRX_PORT.write(BUF[c++]));
}
bool TRX_Config(TARVOS_config config, bool save){
    TRX_CONFIGURING = true;  // Bloquear Loop_SER
    bool ret = true;
    // _spln("CONFIGURO TRX");
    // config.Print();
    Serial.print("COMMANDS: ");
    TRX.SetMode(COMMAND);
    delay(10);
    // TRX.FactoryReset();
    // delay(50);
    if (!TRX.SetRfProfile(RADIO_PROFILE_0)) ret = false;
    if (!TRX.SetTxPower(14)) ret = false;
    if (!TRX.SetAdressMode(3)) ret = false;
    if (!TRX.SetTimeOut(5)) ret = false;
    if (!TRX.SetOPMode(4)) ret = false;
    if (!TRX.SetRpFlags(0)) ret = false;
    if (!TRX.SetRfChannel(config.channel)) ret = false;
    if (!TRX.SetSourceNet(config.net)) ret = false;
    if (!TRX.SetSourceAddr(config.adress)) ret = false;
    if (!TRX.SetDestNet(config.net)) ret = false;
    if (!TRX.SetDestAddr(config.adress)) ret = false;
    TRX.SetMode(TRANSPARENT);
    Serial.println(ret? " -> TRX Config OK": " -> ALGO SALIO MAL :(");
    if (ret && save){
        TARVOS_config EPROM_config;
        EEPROM.get(EEPROM_DIR, EPROM_config);

        if (config == EPROM_config){} else {
            _spln("Configuracion diferente en Eprom, gravo");
            EEPROM.put(EEPROM_DIR, config);
            EEPROM.commit();
        }
    }
    TRX_CONFIGURING = false;  // Desbloquear Loop_SER
    return ret;
}
TARVOS_config TRX_GetConfig(){
    TARVOS_config config;
    TRX_CONFIGURING = true;  // Bloquear Loop_SER durante lectura
    
    TRX.SetMode(COMMAND);
    delay(10);
    
    config.channel = TRX.GetSetting(RADIO_DefaultRfChannel).values[1];
    config.net = TRX.GetSetting(MAC_SourceNetID).values[1];
    config.adress = TRX.GetSetting(MAC_SourceAddr).values[1] | (TRX.GetSetting(MAC_SourceAddr).values[2] << 8);
    
    // Calcular checksum
    config.CS = config.channel ^ config.net ^ config.adress;
    config.dia = 0;
    config.mes = 0;
    
    TRX.SetMode(TRANSPARENT);
    TRX_CONFIGURING = false;  // Desbloquear Loop_SER
    
    return config;
}
byte FreqToChannel(float F){   ///// LA PASAMOS A CLASE?
    if (F <= 140) return F;
    if (F < 863.0F || F > 870.0F) return -1;
    return (byte) round((F - 863) / 0.05F);
}

