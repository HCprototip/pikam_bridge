//#ifndef TEMPLATE_PLACEHOLDER
//#define TEMPLATE_PLACEHOLDER '~'   tret per si s'actualitza WesResponseIplm
// prova  evitar desbordament WDT https://github.com/me-no-dev/ESPAsyncWebServer/pull/621
// #define CONFIG_FREERTOS_UNICORE  // ----> No funciona amb ultima actualitzacio esp sdk!!!
//#endif
#include <Arduino.h>
//#include "C:\Users\Hector\.platformio\packages\framework-arduinoespressif32\tools\sdk\include\newlib\time.h"
//#include "C:\Users\Manel\.platformio\packages\framework-arduinoespressif32\tools\sdk\esp32\include\newlib\platform_include\time.h"
#include <sys/time.h>

#include <EEPROM.h>

#ifdef TARVOS
#include <tarvos.h>
extern bool TRX_CONFIGURING;  // Flag de configuración TRX
#endif

#if defined(BRIDGE_TRANSPARENTE)
#include <BLSerial.h>
#elif defined(BRIDGE_BLE)
#include <BLE.h>
#elif defined(WIFI)
// Incluir librerías WiFi aquí
#endif

#include <ArduinoJson.h>
#if IDIOMA == ES
    #include <menuOP.h>
#elif IDIOMA == EN
    #include <menuOP_EN.h>
#elif IDIOMA == GE
    #include <menuOP_GE.h>
#else
    #error NO HAY IDIOMA DEFINIDO
#endif
#include <printerO.h>

#ifdef MEGATRAILER
#define ITEMS_PANT_INICIAL 16
#define ITEMS_PANT_SENSORES 14
#define ITEMS_PANT_PESPARCIALES 16
#else
#define ITEMS_PANT_INICIAL 8
#define ITEMS_PANT_SENSORES 8
#define ITEMS_PANT_PESPARCIALES 8
#endif
#define nDIGITS_DISPLAY 6
#define BOT_A 4
#define BOT_B 2
#define BOT_C 3
#define BOT_D 1
#define BOTO_PCB 0  // Botón en PCB (si existe)
#define TIME_ZONE 0 //(60*60*2)
#define DEBUG_COM_CENT
#define LONG_SERIAL 250
#define INTERVAL_MSG_A_CENTRALETA 500   ///////////// si es mes petit que la cadencia del mando pot donar trames retallades (REBUT CURT)
#define INTERVAL_MSG_A_BLE 150           
#define TIMEOUT_SINCONEXION_BLE 2000
#define PRINTER_HEARDBEAT 1000
#define T_CENT_NO_TIMEOUT 1000
#define TEMPS_ESPERA_CONFIG 10000 // Temps que triga el u de la centraleta en ser alimentat (el ESP32 la tindrà permanent)


/////////////////////////////   DEFINE CODI
#define CONFIG !digitalRead(PIN_CONFIG)
#define TIMER_NC_RESET tRESET = millis() + 3600000
#define EVENTS   if (event_bandera) GestioEvents()
#define RESET_t_REQUEST_OUT t_REQUEST_OUT = millis() + INTERVAL_MSG_A_CENTRALETA
#define RESET_t_BLE_OUT t_BLE_OUT = millis() + INTERVAL_MSG_A_BLE
#define WHILE_t_REQUEST_OUT while (millis() < t_REQUEST_OUT)
#undef LED_BUILTIN
#define LED_BUILTIN 15

// _VOID   _EXFUN(tzset,	(_VOID));
// int	    _EXFUN(setenv,(const char *__string, const char *__value, int __overwrite));

#ifdef BRIDGE_BLE

#endif

StaticJsonDocument<1024> JSON;

modes_func mode_func = MANDO;
bool visu_ext = false;       
bool visu_ext_informat;
modes_visu mode_visu;
modes_visu mode_visu_informat;

byte borra = 0;

bool LECTURA_NETO;
bool FLKG;
bool LNET;
bool LGRO;
bool LAL1;
bool LAL2;
byte ESTAT;
byte ESTAT_OLD;
byte remolque_sel = 1;
byte remolque_old = 0;
boolean bol;
int Nrebut_cent;
char rebut_SER[LONG_SERIAL];
char rebut_SER_ULTIM[LONG_SERIAL];

byte CENT_BUF[100];
byte CENT_BUF_INDEX;

int printer_ID; 
bool printer_OK;

int cadencia;
bool cent_NO_TIMEOUT = false;

unsigned long t_REQUEST_OUT;    // per espera config
unsigned long t_WS_OUT;         // cadencia missatges a WS 
unsigned long tRESET;           // hard reset si no com
unsigned long t_BLE_OUT;         // cadencia missatges a BLE
unsigned long t_PRINTER_HB;     // cadencia heartbeat printer
unsigned long t_cent_NO_TIMEOUT;
bool event_bandera;
byte ULTIM_ESTAT_MENU;
mode last_option_print;

byte typeCent = 99;

struct COMM_config{             //////////////////// MIGRADA DE TARVOS.H
  byte net;
  byte adress;
  byte channel;
  byte dia;
  byte mes;
  byte CS;
  bool CheckSumOK(){
      byte cc = channel;
      cc ^= net;
      cc ^= adress;
      cc ^= dia;
      cc ^= mes;
      return CS == cc;        
  }    
};
bool operator==(COMM_config A, COMM_config B){
  return ((A.adress == B.adress) && (A.channel == B.channel) && (A.CS == B.CS));
}

void GestioEvents();
void Loop_INs();
void Loop_SER();
void Loop_TIM();
void Loop_BLE();
void ModeFuncionament(modes_func modo);
void ModeVisualizacion(bool modo);
void EnviaPeti(petis peti);
void SetupIO();
void Setup_SER();
void Setup_BLE();
void Loop_SER();
void Loop_EVENTS();
void Loop_TIM();
void SerialMalo(String motivo);
void SerialRebut();
void REBUT_JSON(char *payload);
void ProcesaRebut(char rebut[13]);
void PulsatMando(byte Nboto);
void PulsatMandoFunc(byte Nboto);
void VirtualPeti(byte Nboto);
void TramaToDisp(char buf[], byte index = 6);
void ErrorToDisp(char buf[], byte index = 6);
void TramaToExtras(char buf[], byte quants, byte index = 18);
int  TramaToPes(char buf[], byte index = 6);
byte FasCalToPart(char FasCal);
byte SubModeOf(char msg[]);
void TypeCent(byte NtypeCent);

mode ModeOf(char c, char e = '0');
void DispMenu(int opcio);
byte RebutToCalTipus(char C);
byte RebutToSensor(char C);
byte RebutToPespa(char C);
byte RebutToConfig(char C);
void ENVIA_JSON();
void DISPLAY_CanviAspecte(enum mode modo);
void DISPLAY_PES(String str);
void DISPLAY_TXT(ID_TXT id, String str);
void DISPLAY_BOT(byte modo);
void DISPLAY_IMG(String str);
void DISPLAY_TPE(String str);
void DISPLAY_MODE(bool ext, modes_visu mod);
void SetDateTime(time_t T);
void SetPrinter();
void PulsatPrinter(byte Nboto);
void Printer_Estat(bool estat);
void JsonToCommand(char *BLS_BUF, byte BLS_BUF_INDEX);

inteligent_error RebutToError(char A, char B);



#pragma region SETUPs               /////////////////////////////   SETUPs
void Setup_SERIAL(){
  //CENT.begin(CENT_BAUD, SERIAL_8N1, PIN_RX, PIN_TX, false, 1000);
  Serial.begin(115200);
}
void Setup_IO(){
  pinMode(BOTO_PCB, INPUT_PULLUP);
  pinMode(PIN_CONFIG, INPUT);
}
#ifdef BRIDGE_BLE
// BLE Server Callbacks (conexión/desconexión)
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };
    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};

// BLE Characteristic Callbacks (recibir datos del cliente)
class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string rxValue = pCharacteristic->getValue();

      if (rxValue.length() > 0) {
        if (rxValue.length() > BLE_MAX_LONG) {
          _sp("[ERROR] overflow en BLE. Bytes recibidos: ");
          _spln(rxValue.length());
          return;
        }
        
        memcpy(rebut_BLE, rxValue.c_str(), rxValue.length());
        rebut_BLE[rxValue.length()] = '\0';
        
        #ifdef DEBUG_COM_CENT
        Serial.printf("BLE_rebut: %s\n", rebut_BLE);
        #endif
        
        // Enviar comando a cola para procesar en loop principal
        if (jsonCommandQueue != NULL) {
          xQueueSend(jsonCommandQueue, rebut_BLE, 0);
        }
      }
    }
};

void Setup_BLE(){ 
  // Crear cola para comandos JSON (capacidad 5 mensajes)
  jsonCommandQueue = xQueueCreate(5, BLE_MAX_LONG);
  
  // Inicializar BLE
  BLEDevice::init("PIKAM_BLE_BRIDGE");
  
  // Crear servidor BLE
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  
  // Crear servicio BLE
  BLEService *pService = pServer->createService(SERVICE_UUID);
  
  // Crear característica TX (notificaciones al cliente)
  pTxCharacteristic = pService->createCharacteristic(
      CHARACTERISTIC_UUID_TX,
      BLECharacteristic::PROPERTY_NOTIFY
  );
  pTxCharacteristic->addDescriptor(new BLE2902());
  
  // Crear característica RX (recibir datos del cliente)
  BLECharacteristic *pRxCharacteristic = pService->createCharacteristic(
      CHARACTERISTIC_UUID_RX,
      BLECharacteristic::PROPERTY_WRITE
  );
  pRxCharacteristic->setCallbacks(new MyCallbacks());
  
  // Iniciar servicio
  pService->start();
  
  // Iniciar advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  
  _spln("BLE SETUP OK - Esperando conexion...");
}

void Loop_BLE(){
  // Procesar comandos BLE de la cola
  char bleBuffer[BLE_MAX_LONG];
  if (jsonCommandQueue != NULL) {
    while (xQueueReceive(jsonCommandQueue, bleBuffer, 0) == pdTRUE) {
      switch (bleBuffer[0]){
        case 'P': {
          byte petiNum = bleBuffer[1] - '0';
          if (petiNum <= REQUEST_EXT_DISPLAY) {
            EnviaPeti((petis)petiNum);
          }
          break;
        }
        case 'F':
          PulsatMandoFunc(bleBuffer[1] - '0');
          break;
        case '{':
          REBUT_JSON(bleBuffer);
          break;
        default:
          break;
      }
    }
  }
  
  // Gestionar reconexión BLE
  if (!deviceConnected && oldDeviceConnected) {
    delay(500); 
    pServer->startAdvertising();
    _spln("BLE: Reiniciando advertising...");
    oldDeviceConnected = deviceConnected;
  }
  if (deviceConnected && !oldDeviceConnected) {
    oldDeviceConnected = deviceConnected;
    _spln("BLE: Cliente conectado");
    remolque_old = 0;
    ESTAT_OLD = 0;
    modeAct = mode::MAX_MODE;
    mode_func = NORMAL;
    JSON.clear();
    JSON["check_value"] = visu_ext;
    JSON["typeCent"] = typeCent;
    ENVIA_JSON();
    delay(100);
    JSON.clear();
    DISPLAY_MODE(visu_ext, mode_visu);
    ENVIA_JSON();
    EnviaPeti(RESET_CENT_SOFT);
    TIMER_NC_RESET;
  }
}

#endif

#pragma endregion SETUPs
#pragma region LOOPs                /////////////////////////////   LOOPs
void Loop_SER(){
#ifdef TARVOS
    if (TRX_CONFIGURING) return;  // No procesar serie durante configuración TRX
#endif
#ifdef BRIDGE_TRANSPARENTE
////////  DE BT A TRX
if (SerialBT.available()) {
  char D = SerialBT.read();
  BLS_BUF[BLS_BUF_INDEX++] = D;
  if ((BLS_BUF_INDEX >= BLS_LONG_SERIAL) | (D == 13)){
    //TRX_PORT.println("$␀␁XD00000");
    /////// DEBUB
    _sp("B-> ");_sp(BLS_BUF_INDEX);(_sp("bytes: "));
    for (byte c = 0; c < BLS_BUF_INDEX; c++){
      Serial.write(BLS_BUF[c]);
    };
    _spln();
    ///////

    switch (BLS_BUF[0])
    {
    case '$':
      MESSAGE_TYPE = PETI;
      TRX_Send(BLS_BUF, BLS_BUF_INDEX);
      break;
    case '{':
      MESSAGE_TYPE = COMANDO;
      JsonToCommand(BLS_BUF, BLS_BUF_INDEX);
      break;
    default:
      MESSAGE_TYPE = BLS_MESSAGE_ERROR;
      break;
    }
    BLS_BUF_INDEX = 0;
  }    
  // Serial.print(D, HEX); Serial.print(" ");
  // switch (D)
  // {
  //   case 't':
  //   case 'T':
  //     SerialBT.println(millis()/1000);
  //     break;
  //   case '+':
  //     TRX_PORT.println("$01XD0000");
  //     Serial.println("EMULO $01XD0000");
  //     break;
  //   default:
  //     TRX_PORT.write(reb);
  //     break;
  // }
    }
//////// DE TRX A BT
if (TRX_PORT.available()) { 
  byte D = TRX_PORT.read();
  TRX_BUF[TRX_BUF_INDEX++] = D;
  if ((TRX_BUF_INDEX >= 20) | (D == 13)){
    for (byte c = 0; c < TRX_BUF_INDEX; SerialBT.write(TRX_BUF[c++]));
    /////// DEBUB
    Serial.print("T->: ");
    for (byte c = 0; c < TRX_BUF_INDEX; c++){
      Serial.write(TRX_BUF[c]);
    };
    Serial.println();
    ///////
    TRX_BUF_INDEX = 0;
  }    
}
#endif
#ifdef BRIDGE_BLE
    if (TRX_PORT.available()) { 
        byte D = TRX_PORT.read();
        TRX_BUF[TRX_BUF_INDEX++] = D;
        if ((TRX_BUF_INDEX >= 20) | (D == 13)){
          ProcesaRebut((char *)TRX_BUF);
          RESET_t_BLE_OUT;
          TIMER_NC_RESET;

            /////// DEBUB
            Serial.print("T->: ");
            for (byte c = 0; c < TRX_BUF_INDEX; c++){
                Serial.write(TRX_BUF[c]);
            };
            Serial.println();
            ///////
            TRX_BUF_INDEX = 0;
        }    
    }
#endif
#ifdef EN_ST
  byte data = 0;  
  while (CENT.available()){
    data = CENT.read();
    rebut_SER[Nrebut_cent++] = (char)data;
    if (Nrebut_cent > LONG_SERIAL){
      SerialMalo("OVERFLOW");
      for (byte c = 0; c < Nrebut_cent; _sp(rebut_SER[c++]));
      _spln();
      Nrebut_cent = 0;
    }
    //_sp(data);_sp(":");
    if (data == 13){
      if (Nrebut_cent > 11){
        SerialRebut();
      }
      else{            
        SerialMalo("CORTO");
        for (byte c = 0; c < Nrebut_cent; _sp(rebut_SER[c++]));
        _spln();
      }
      Nrebut_cent = 0;
    }
#ifdef TARVOS
    if (CENT_BUF_INDEX == 5 and data < 20) data += '0'; // 28MAR evitem pes en blanc
    CENT_BUF[CENT_BUF_INDEX++] = data;
    if (CENT_BUF_INDEX > 20 or data == 13)
    {
      //Serial.print("S");
      for (byte c = 0; c < CENT_BUF_INDEX; TRX_PORT.write(CENT_BUF[c++]));
      CENT_BUF_INDEX = 0;
    }
#endif
  } 
#ifdef TARVOS
  if (TRX_PORT.available()){
    byte D = TRX_PORT.read();
    TRX_BUF[TRX_BUF_INDEX++] = D;
    if ((TRX_BUF_INDEX >= 20) | (D == 13)){
      if (mode_func == NORMAL) for (byte c = 0; c < TRX_BUF_INDEX; CENT.write(TRX_BUF[c++]));
      TRX_BUF_INDEX = 0;
      RESET_t_REQUEST_OUT;
      RESET_t_BLE_OUT;
      TIMER_NC_RESET;
    }
  }
#endif

#endif
}
void Loop_TIM(){
  unsigned long MILLIS = millis();
  // PER REUBICAR SEGONS CAS
  // if (mode_func != PRINTER){
  #if defined(BRIDGE_BLE) || defined(WIFI)
  if (true){  //deviceConnected
    if (MILLIS > t_REQUEST_OUT){        
      EnviaPeti(visu_ext? REQUEST_EXT_DISPLAY : REQUEST_DISPLAY);
    }
  }
  #endif
  // }
  // 
  if (MILLIS > tRESET) {
    _spln("REEESET!!");
    ESP.restart();
  } 
  // if (printer_OK){ 
  //   if (MILLIS > t_PRINTER_HB){
  //     t_PRINTER_HB = MILLIS + PRINTER_HEARDBEAT;
  //     EnviaPeti(REQUEST_DISPLAY);   // anem preguntant el pes
  //     /// i vaig enviant pulsos al terminal, si no entra en SIN DATOS
  //     JSON.clear();
  //     JSON["HB"] = true;
  //     ENVIA_JSON();
  //   }
  // }
  if (cent_NO_TIMEOUT){ 
    if (MILLIS > t_cent_NO_TIMEOUT){
      t_cent_NO_TIMEOUT = MILLIS + T_CENT_NO_TIMEOUT;
      EnviaPeti(P5);  
      _spln("RESET CENT TO");
    }
  }
}
#pragma endregion LOOPs
#pragma region F_TARVOS             /////////////////////////////   TARVOS
#ifdef TARVOS
extern bool TRX_CONFIGURING;
// void Setup_TRX(){
//     TRX_PORT.begin(115200);
//     TAR.Setup(PIN_MODE, PIN_RESET);
// }
// void ConfigTAR(COMM_config config){
//     TAR.SetMode(COMMAND);
//     delay(10);
//     TAR.Command(TARVOS_COMMAND::CMD_FACTORY_RESET_REQ);
//     delay(50);
//     TAR.SetRfProfile(RADIO_PROFILE_0);
//     TAR.SetTxPower(14);    
//     TAR.SetAdressMode(3);
//     TAR.SetTimeOut(5);
//     TAR.SetOPMode(4);
//     TAR.SetRpFlags(0);
//     TAR.SetRfChannel(config.channel);    
//     TAR.SetSourceNet(config.net);
//     TAR.SetSourceAddr(config.adress);
//     TAR.SetDestNet(config.net);
//     TAR.SetDestAddr(config.adress);
//     TAR.SetMode(TRANSPARENT);
// }
#endif
#pragma endregion F_TARVOS
#pragma region PRINTER              /////////////////////////////   PRINTER
// void PulsatPrinter(byte Nboto){
//   _sp("PRINTER BOTO "); _spln(Nboto);
// }
// void Printer_Estat(bool estat){
//   printer_OK = estat;
//   if (printer_OK){
//     _spln("PRINTER CONNECTADA");
//   } else {
//     _spln("PRINTER DESCONNECTADA");
//   }
//   if (modeAct == INICIO_TOTAL or modeAct == INICIO_NETO){
//     DISPLAY_CanviAspecte(modeAct);
//     ENVIA_JSON();
//   }
// }


#pragma endregion PRINTER
#pragma region GESTIO_EVENTS        /////////////////////////////   GESTIO EVENTS
struct event {
  bool bandera;
  bool estat;
};
event event_Socket;
event event_Wifi;

void GestioEvents(){
  event_bandera = false;
  // Simplificado para BLE - eventos manejados directamente en callbacks
}
#pragma endregion GESTIO_EVENTS
#pragma region F_DISPLAY            /////////////////////////////   FUNCIONS DISPLAY
void DISPLAY_CanviAspecte(mode modo){
  // textos teclas:
  _sp("CANVI ASPECTE "); _spln(modo);
  JSON["foot"] = "no";
  switch (modo){
    case INICIO_TOTAL:
    case INICIO_NETO:    
      DISPLAY_IMG("");
      DISPLAY_TXT(TXT_SUPERIOR, "");
      DISPLAY_TXT(TXT_MITJA, "");
      DISPLAY_TXT(TXT_UNIDADES, KILOS);
      DISPLAY_MODE(visu_ext, EXTENDIDO);
      DISPLAY_TPE(LECTURA_NETO? NETO : TOTAL);
      JSON["foot"] = "si";
      JSON["cont"] = visu_ext? "E" : "W";
      break;
    case ERROR_INT:        
      DISPLAY_IMG("");
      DISPLAY_TXT(TXT_SUPERIOR, "");
      DISPLAY_TXT(TXT_MITJA, "");
      DISPLAY_TXT(TXT_UNIDADES, "");
      JSON["tpe"] = "";
      DISPLAY_MODE(false, EXTENDIDO);   ////  canviar per DISPLAY EXTENDIDO i recular modes_visu
      JSON["foot"] = "si, EXTENDIDO"; 
    break;
    case VIS_VALOR:
    case SENSE:
    case INTRO_VALOR:
      DISPLAY_IMG("");
    break;
    case MP_PRINTER:
      JSON["cont"] = "P";
    break;
    // case EXT_DISP_EJES:
    //   JSON["cont"] = "E";
    // break;

  
  }
  DISPLAY_BOT(modo_BOT[modo]);
}
void DISPLAY_PES(String str){
  JSON["pes"] = str;  
  //JSON["pes"] = "47775";     //////////////////////////////////////////////////// FAKE
}
void DISPLAY_TXT(ID_TXT id, String str){
  String txt = "txt" + (String)id;
  JSON[txt] = str;
}
void DISPLAY_BOT(byte modo){
  JsonObject txt_bot = JSON.createNestedObject("txt_bot");  
  txt_bot["bot1"] = textos_teclas[modo][0];
  txt_bot["bot2"] = textos_teclas[modo][1];
  txt_bot["bot3"] = textos_teclas[modo][2];
  txt_bot["bot4"] = textos_teclas[modo][3];
  txt_bot["bot5"] = printer_OK? textos_teclas[modo][4] : "";
}
void DISPLAY_IMG(String str){
  JSON["img"] = str;
}
void DISPLAY_TPE(String str){
  JSON["tpe"] = str;
}
void DISPLAY_MODE(bool ext, modes_visu mod){
  if (visu_ext_informat != ext or mode_visu_informat != mod){
    visu_ext_informat = ext;
    mode_visu_informat = mod;
    JSON["cont"] = ext? "E" : "W";
    if (ext) {
      JSON["Etype"] = (byte) mod;
      JSON["VisuFons"] = remolque_sel;
      ENVIA_JSON();           //// si hi ha canvi de Etype, primer envio i dono temps per que canvii l'html i segeuixo amb la resta de parametres.
      delay(100);           // mal mal
      JSON.clear();
      // Activem pulsacions fantasma per evitar TimeOUT de centraleta
      if (mod == SENSORES | mod == PESOS_PARCIALES) cent_NO_TIMEOUT = true;
    } else {
      cent_NO_TIMEOUT = false;
    }
  }
}
#pragma endregion F_DISPLAY
#pragma region F_JSON               /////////////////////////////   JSON
void ENVIA_JSON(){
#ifdef BRIDGE_BLE
  //if (!deviceConnected) return;
  
  String output;
  serializeJson(JSON, output);
  
  // Fragmentar si es necesario (BLE MTU típico ~512 bytes)
  if (output.length() <= 512) {
    pTxCharacteristic->setValue(output.c_str());
    pTxCharacteristic->notify();
  } else {
    // Fragmentar en paquetes
    _sp("JSON largo, fragmentando: "); _spln(output.length());
    int chunks = (output.length() / 512) + 1;
    for (int i = 0; i < chunks; i++) {
      String chunk = output.substring(i * 512, min((i + 1) * 512, (int)output.length()));
      pTxCharacteristic->setValue(chunk.c_str());
      pTxCharacteristic->notify();
      delay(10); // pequeña pausa entre fragmentos
    }
  }
#endif
}
void REBUT_JSON(char *payload){
  DeserializationError error = deserializeJson(JSON, payload);
  
  serializeJsonPretty(JSON, Serial);
  if (error){
    _spln("ERROR JSON REBUT");
    return;
  }
  // if (JSON.containsKey("alb")) albaran = JSON["alb"] | "00000";
  // if (JSON.containsKey("mat")) matricula = JSON["mat"] | "0000ABC";
  // if (JSON.containsKey("dt")){  
  //   String str = JSON["dt"];
  //   time_t T = (time_t) str.toInt();
  //   timeval now = { .tv_sec = T };
  //   settimeofday(&now, NULL);
  //   time_t rawtime;
  //   struct tm * timeinfo;
  //   time (&rawtime);
  //   timeinfo = localtime (&rawtime);
  //   Serial.printf ("Data: %s", asctime(timeinfo));
  //   // PENDENT REINTRODUIR PRINTER
  //   // if (printer_OK){
  //   //   JSON.clear();
  //   //   JSON["set"] = true;
  //   //   JSON["dt"] = str;
  //   //   String output;
  //   //   serializeJson(JSON, output);
  //   //   pTxCharacteristic->setValue(output.c_str());
  //   //   pTxCharacteristic->notify();
  //   // }
  // }
  // if (JSON.containsKey("MV")){
  //   bool visu_ext_new = JSON["MV"] | false;
  //   ModeVisualizacion(visu_ext_new);
  //   JSON.clear();
  //   JSON["check_value"] = visu_ext;
  //   ENVIA_JSON();
  // }
  if (JSON.containsKey("GET_RC")){
        TARVOS_config config = TRX_GetConfig();
        config.Print();
        JSON.clear();
        char netStr[3], addrStr[3];
        sprintf(netStr, "%02X", config.net);
        sprintf(addrStr, "%02X", config.adress);
        float freq = 863.0 + (config.channel * 0.05);
        
        JSON["GET_RC"]["NET"] = netStr;
        JSON["GET_RC"]["ADRESS"] = addrStr;
        JSON["GET_RC"]["CHANNEL"] = freq;
#ifdef BRIDGE_BLE
        ENVIA_JSON();
#endif
#ifdef BRIDGE_TRANSPARENTE
        serializeJson(JSON, SerialBT); 
#endif
    }
  if (JSON.containsKey("SET_RC")){
        TARVOS_config config;        
        char * p;
        config.adress = (byte) strtol(JSON["SET_RC"]["ADRESS"], &p, 16);
        config.net = (byte) strtol(JSON["SET_RC"]["NET"], &p, 16);
        config.channel = FreqToChannel(JSON["SET_RC"]["CHANNEL"]);
        config.Print();
        bool result = TRX_Config(config, true);
        JSON.clear();
        JSON["SET_RC_RESP"] = result;
#ifdef BRIDGE_BLE
        ENVIA_JSON();
#endif
#ifdef BRIDGE_TRANSPARENTE
        serializeJson(JSON, SerialBT); 
#endif
    }
  // if (printer_OK){
  //   ProcesaRebut((char *) "#P--@-MENU-1");
  // } else {
  //   SetPrinter();
  // } 
}
void JsonToCommand(char *BLS_BUF, byte BLS_BUF_INDEX){
    DeserializationError error = deserializeJson(JSON, BLS_BUF);
    if (error){
        _spln("ERROR JSON REBUT BLE");
        return;
    }
    _spln("REBUT JSON: ");
    serializeJsonPretty(JSON, Serial);
    _spln();
    if (JSON.containsKey("SET_RC")){
        TARVOS_config config;        
        char * p;
        config.adress = (byte) strtol(JSON["SET_RC"]["ADRESS"], &p, 16);
        config.net = (byte) strtol(JSON["SET_RC"]["NET"], &p, 16);
        config.channel = FreqToChannel(JSON["SET_RC"]["CHANNEL"]);
        config.Print();
        bool result = TRX_Config(config, true);
        
        // Enviar respuesta
        JSON.clear();
        JSON["RESP"] = result;
        
        #ifdef BRIDGE_TRANSPARENTE
        serializeJson(JSON, SerialBT);
        #endif
        
        #ifdef BRIDGE_BLE
        ENVIA_JSON();
        #endif
        
        _spln(result ? "CONFIG OK - Enviada respuesta" : "CONFIG ERROR - Enviada respuesta");
    }
}
#pragma endregion F_JSON

/// QUE HAREMOS CON ESTO ??????
/////////////////////////////////////////////
// void SetPrinter(){
//   String output;
//   JSON.clear();
//   JSON["set"] = true;
//   JSON["alb"] = albaran;
//   JSON["mat"] = matricula;
//   serializeJson(JSON, output);
//   if (deviceConnected && printer_OK) {
//     pTxCharacteristic->setValue(output.c_str());
//     pTxCharacteristic->notify();
//   }
// }
// void ImprimeTicket(TKT_TIPO tipo){
//   String output;
//   JSON.clear();
//   JSON["print"] = true;
//   JSON["TKT_TIPO"] = (byte) tipo;
//   JSON["alb"] = albaran;
//   JSON["mat"] = matricula;
//   JSON["PE"] = peso_entrada;
//   JSON["PS"] = peso_salida;
//   JSON["HE"] = mktime(&hora_entrada);
//   JSON["HS"] = mktime(&hora);
//   serializeJson(JSON, output);
//   if (deviceConnected && printer_OK) {
//     pTxCharacteristic->setValue(output.c_str());
//     pTxCharacteristic->notify();
//   }
// }
// void SetDateTime(time_t T){   ////////////// OBSOLEEET???
//   timeval now = { .tv_sec = T + TIME_ZONE };
//   settimeofday(&now, NULL);   
//   struct tm timeinfo;
//   getLocalTime(&timeinfo);
//   Serial.printf ("Data: %s", _asctime(&timeinfo));
//   if (printer_OK){
//     JSON.clear();
//     JSON["set"] = true;
//     JSON["dt"] = T;
//     String output;
//     serializeJson(JSON, output);
//     pTxCharacteristic->setValue(output.c_str());
//     pTxCharacteristic->notify();
//   }
// }
