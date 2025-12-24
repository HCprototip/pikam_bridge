/*
ToDo:

*/
#include <Arduino.h>
#include <main.h>  // COMENTADO TEMPORALMENTE PARA TESTEAR


void setup() {
  delay(1000); 
  Serial.begin(115200);
  delay(10);
  _spln("INICIANT...");
  EEPROM.begin(64);
  #ifdef BRIDGE_BLE
  Setup_BLE();
  #endif
  #ifdef BRIDGE_TRANSPARENTE
  Setup_BLS();
  #endif
  #ifdef TARVOS
  Setup_TRX(); 
  #endif 
  Setup_IO(); 
  TIMER_NC_RESET;
  _spln("SETUP OK");
}
void loop() {
  // EVENTS; 
  // Loop_INs();
  Loop_SER();
  Loop_TIM();
  #ifdef BRIDGE_BLE
  Loop_BLE();
  #endif
}
void SerialMalo(String motivo){
  _sp("REBUT ERROR: "); _spln(motivo);
}
void SerialRebut(){
  if (memcmp(rebut_SER_ULTIM, rebut_SER, LONG_SERIAL) == 0) if (millis() < t_BLE_OUT) return;    // si es el mateix i no hi ha timout, no processem  (evitem sobrecarrega quan hi ha el mando connectat)                                   
  #ifdef DEBUG_COM_CENT
  if (memcmp(rebut_SER_ULTIM, rebut_SER, LONG_SERIAL) != 0) Serial.printf("SER_rebut: %s\n", rebut_SER);  
  #endif 
  if (mode_func == PRINTER){
    if (ModeOf(rebut_SER[4]) == INICIO_TOTAL) peso_last = TramaToPes(rebut_SER);
    return;
  }
  ProcesaRebut(rebut_SER);
  memcpy(rebut_SER_ULTIM, rebut_SER, LONG_SERIAL);
  memset(rebut_SER, 0, LONG_SERIAL);
  RESET_t_BLE_OUT;
  RESET_t_REQUEST_OUT;
  TIMER_NC_RESET;
}
void ProcesaRebut(char rebut[LONG_SERIAL]){
  String str = "";
  char index;
  byte I;
  JSON.clear();
  ////////////////////////// PROCESA CANVIS ESTAT
  ESTAT = rebut[2] - '0';
  if (ESTAT != ESTAT_OLD){
    //_sp("ESTAT -->"); _spln(ESTAT, 2);
    //FLKG = bitRead(ESTAT, 0);
    LECTURA_NETO = bitRead(ESTAT, 1);
    //LGRO = bitRead(ESTAT, 2);
    LAL1 = bitRead(ESTAT, 3);
    LAL2 = bitRead(ESTAT, 4);
    JSON["al1"] = LAL1;
    JSON["al2"] = LAL2;
    DISPLAY_TPE(LECTURA_NETO? NETO : TOTAL);
    ESTAT_OLD = ESTAT;
  }
  ////////////////////////// PROCESA CANVIS TYPECENT
  if (visu_ext){
    byte typeCent_informado = 0; // JA CAMBIAREM PEL BO
    if (typeCent_informado != typeCent){
      typeCent = typeCent_informado;
      JSON["typeCent"] = typeCent;
      _spln("TYPECENT CAMBIADO");
    }
  }
  ////////////////////////// PROCESA CANVIS REMOLC
  remolque_sel = rebut[12] - '0' + 1;
  if (remolque_sel > 4) remolque_sel = 1;  
  if (remolque_sel != remolque_old){
    remolque_old = remolque_sel;
    JSON["rem"] = remolque_sel;
  }
  /////////////////////////  PROCES SEGONS mode i submode 
  mode modeInformat = ModeOf(rebut[4], rebut[8]);       //#E80x00000 /// AFEGIT rebut[8] per discriminar mode0 amb comunicació d'error (inteligent)
  byte opcio = SubModeOf(rebut);
  // opcions d'error:  
  if ((modeInformat == INICIO_TOTAL or modeInformat == INICIO_NETO) and (opcio == CAL_FAL_C or opcio == CAL_FAL_V or opcio == CAL_NO_CAL or opcio == CAL_FALLO)){  // MP_RECAL peroque opcio RECA està a dos llocs MP i cm a Error matematic (es mostra amb display inicial)
    modeInformat = FALTA_CAL;
  } 
  // si canvia mode canviem aspecte HTML
  if (modeAct != modeInformat){
    modeAct = modeInformat;
    if (modeAct != MAX_MODE){
      DISPLAY_CanviAspecte(modeAct);
    }
  }
    
  switch (modeAct){
    case FALTA_CAL:
      opcio -= cal_ERROR::CAL_NO_CAL;
      DISPLAY_TXT(TXT_SUPERIOR, FALTA_CALIB[0].txt_titol + (String) remolque_sel);   
      DISPLAY_TXT(TXT_MITJA, FALTA_CALIB[0].txt_opcio[opcio]);
      DISPLAY_IMG(FALTA_CALIB[0].grafic_opcio[opcio]);
      break;
    case ERROR_INT:      
      ErrorToDisp(rebut, 8);
      break;
    case INICIO_TOTAL:
    case INICIO_NETO:      
      TramaToDisp(rebut);
      if (visu_ext){
        TramaToExtras(rebut, ITEMS_PANT_INICIAL);
      }
      ENVIA_JSON();
      return;
      break;
    case MP_REMOL:
    case MP_DISPL:
      // opcio = rebut[11] - 'P';  ///// HA CANVIAT
      opcio = rebut[5] - '0' + 1;
      // if (opcio < 1 or opcio > 4) opcio = 1;
    case MENU_PRINCIPAL:
    case MP_TARA:
      DispMenu(opcio);
      ULTIM_ESTAT_MENU = opcio;
      break;
    case MP_CONFI:
      opcio = RebutToConfig(rebut[5]);
      DispMenu(opcio);
      ULTIM_ESTAT_MENU = opcio;
      ULTIM_ITEM_MENU = CODE_IN;
      DISPLAY_TXT(TXT_UNIDADES, "");
      DISPLAY_TPE("");
      break;
    case MP_DISPL_OP: //// DEVINCULAT DE SubModeOf
      opcio = rebut[5] - '0' + 1; 
      DISPLAY_TXT(TXT_SUPERIOR, MENU_DISPLAY[ULTIM_ESTAT_MENU].txt_titol);
      DISPLAY_TXT(TXT_MITJA, MENU_DISPLAY[ULTIM_ESTAT_MENU].txt_opcio[opcio]);
      DISPLAY_IMG(MENU_DISPLAY[ULTIM_ESTAT_MENU].grafic_opcio[opcio]);
      break;
    case MP_REMOL_OP: //// DEVINCULAT DE SubModeOf
      opcio = rebut[5] - '0';
      str = MENU_REMOLQUE[0].txt_titol + ULTIM_ESTAT_MENU;     
      DISPLAY_TXT(TXT_SUPERIOR, str);
      DISPLAY_TXT(TXT_MITJA, MENU_REMOLQUE[0].txt_opcio[opcio]);
      DISPLAY_IMG(MENU_REMOLQUE[0].grafic_opcio[opcio]);
      break;
    case MP_RECAL:
      DispMenu(opcio);
      OP_CAL_TIPUS = RebutToCalTipus(rebut[7]);      
      break;
    case MP_RECAL_OP:    
      DISPLAY_TXT(TXT_SUPERIOR, MENU[MP_RECAL].txt_opcio[OP_CAL_TIPUS]);
      index = rebut[5];
      if (opcio == 255) index = 'R'; 
      if (opcio == CAL_BORRA) index = '=';
      //OP_CAL_PART  = rebut[7] < '7'? 0 : rebut[7] < '='? 1:2;
      OP_CAL_PART = FasCalToPart(index);
      switch (index){ 
        case '0':
          str = MENU_CALIBRAR[0].txt_opcio[EJEC];
          DISPLAY_BOT(3);
          break;
        case '1': case '7': case '=':
          // OP_CAL_PART = qué estem calibrant R = tractora | A = CAMION
          // OP_CAL_PART = (rebut[6] == 'R')? 0:1;
          str = MENU_CALIBRAR[0].txt_opcio[SIT_T + OP_CAL_PART];
          if (opcio == CAL_BORRA) str = CONFIRME;
          ULTIM_ITEM_MENU = PESO_;
          ULTIM_ITEM_MENU += TXT_CAL_PART[OP_CAL_PART];
          DISPLAY_IMG("NoFoto");
          DISPLAY_TXT(TXT_UNIDADES, KILOS);
          DISPLAY_TPE(""); 
          DISPLAY_BOT(3);
          break;
        case '3': case '9': case '?':
          if (opcio != SENSE) {
            str = MENU_CALIBRAR[0].txt_opcio[SENSO] + " " + TXT_CAL_PART_SENSO[OP_CAL_PART];
            TramaToDisp(rebut);
          } else {
            str = MIDIENDO;
          }
          DISPLAY_TXT(TXT_UNIDADES, MILIBARS);
          DISPLAY_BOT(3);
          break;
        case '6': case '<': case 'B':
          if (opcio != cal_ERROR::CAL_FALLO){
            str = CALCULO;
            TramaToDisp(rebut);
          } else {
            str = FALLO_CALCULOS;
          }
          DISPLAY_TXT(TXT_UNIDADES, KILOS);
          DISPLAY_TPE("");
          DISPLAY_BOT(5);                   
          break;
        // case '=':
        //   str = CONFIRME;
        //   break;
        case 'R':
          str = "NO LO SE";        
          break;
        default:
          str = "NO INDEX";
          break;
      }
      DISPLAY_TXT(TXT_MITJA, str);      
      break;
    case MP_SENSO:
      I = RebutToSensor(rebut[11]);
      DISPLAY_TXT(TXT_UNIDADES, MILIBARS);
      DISPLAY_TPE("");
      DispMenu(I);
      break;
    case MP_SENSO_EXT:
      DISPLAY_MODE(true, SENSORES);
      TramaToExtras(rebut, ITEMS_PANT_SENSORES);   
      ENVIA_JSON();
      return;
      break;
    case MP_PESPA:
      /// I = RebutToPespa(rebut[7]);   ANTIC
      /// I = RebutToPespa(rebut[11]);
      I = opcio = rebut[5] - '0';
      DISPLAY_TXT(TXT_UNIDADES, KILOS);
      DISPLAY_TPE("");
      DispMenu(I);
      break;
    case MP_PESPA_EXT:
      DISPLAY_MODE(true, PESOS_PARCIALES);
      TramaToExtras(rebut, ITEMS_PANT_PESPARCIALES);   
      ENVIA_JSON();
      return;
      break;
    case MP_ALARM:
      I = rebut[11] - '0';
      //if (I > 2) I = 2;     /////////////////////// TODO: de moment limitem 
      DispMenu(I);      
      break;
    case MP_TARA_OP:
      DISPLAY_TXT(TXT_SUPERIOR, MENU_TARA[ULTIM_ESTAT_MENU].txt_titol);
      DISPLAY_TXT(TXT_MITJA, MENU_TARA[ULTIM_ESTAT_MENU].txt_opcio[opcio]);
      break;
    case INTRO_VALOR:
      //DISPLAY_TXT(TXT_MITJA, TXT_OP_PESO[N_TXT_PES]);
      if (ULTIM_ESTAT_MENU == MP_CONFI) {
        DISPLAY_TXT(TXT_SUPERIOR, CODE_IN);
        //////////////////////////////////////////////////////  TODO: treure KG o  mBars
      };
      DISPLAY_TXT(TXT_MITJA, ULTIM_ITEM_MENU);
      rebut[rebut[3] - '0' + 5] = '_';
      TramaToDisp(rebut);
      break;
    case VIS_VALOR:
      DISPLAY_TXT(TXT_SUPERIOR, "");
      DISPLAY_TXT(TXT_MITJA, ULTIM_ITEM_MENU);
      TramaToDisp(rebut);
      break;
    case REINI:
      if (opcio == SALIR) return;
      DispMenu(0);
      ULTIM_ESTAT_MENU = opcio;
      break;
    case MP_PRINTER:
      JSON["mat"] = matricula;
      JSON["alb"] = albaran;
      JSON["dt"] = time(0);      
      break;
    case MP_PRINTER_SIM:
      JSON["cont"] = "T";
      JSON["tick"] = tiket(TKT_TOTAL);
      break;
    case MP_PRINTER_COM:
      JSON["cont"] = "T";
      JSON["tick"] = tiket(TKT_COM);
      break;
    case VIS_MBAR:     
    case SENSE:
    case SALIR:
    case MAX_MODE:
      _spln("MODE NO IMPLEMENTAT");
      break;
    // default:
    //   break;
    
  }
  if (visu_ext){
    DISPLAY_MODE(false, EXTENDIDO);   ////  canviar per DISPLAY EXTENDIDO i recular modes_visu
  }
  ENVIA_JSON(); 
}
void DispMenu(int opcio){
  String str;
  str = MENU[modeAct].txt_opcio[opcio];
  DISPLAY_TXT(TXT_SUPERIOR, MENU[modeAct].txt_titol);
  DISPLAY_TXT(TXT_MITJA, str);
  DISPLAY_IMG(MENU[modeAct].grafic_opcio[opcio]);
  ULTIM_ITEM_MENU = str;
  // if (modeAct == MP_CONFI){
  //   ULTIM_ITEM_MENU = CODE_IN; // per conflicte al passar a INTRO_VALOR
  // }
}
void EnviaPeti(petis tipo){
  for (int c = 0; c <11; c++){      
    TRX_PORT.write(mandomess[tipo][c]);
  }  
  RESET_t_REQUEST_OUT;
}
void VirtualPeti(byte nBoto){
  _sp("VIRTUAL PETI /// OBSOLET????"); _spln(nBoto);
  _spln(modeAct);
  // switch (modeAct)  {
  // case MP_PRINTER:
  //   switch (nBoto){
  //   case BOT_A:
  //     JSON["cont"] = "W";
  //     ModeFuncionament(NORMAL);
  //     break;
  //   case BOT_B:  
  //     getLocalTime(&hora);
  //     peso_salida = peso_last;
  //     ProcesaRebut((char*) "#P--(--MENU-1");
  //     _spln(tiket(TKT_TOTAL));
  //     break;
  //   case BOT_C:  
  //     if (peso_entrada == 0){
  //       peso_entrada = peso_last;  
  //       getLocalTime(&hora_entrada);
  //     }
  //     ProcesaRebut((char*) "#P--)--MENU-1");
  //     break;
  //   case BOT_D:
  //     switch (last_option_print)      {
  //     case MP_PRINTER_SIM:
  //       ProcesaRebut((char*) "#P--(--MENU-1");
  //       break;
  //     case MP_PRINTER_COM:
  //       ProcesaRebut((char*) "#P--)--MENU-1");
  //       break;      
  //     default:
  //       break;
  //     }
  //   default:
  //     break;
  //   }
  //   break;
  // case MP_PRINTER_SIM:
  //   switch (nBoto){
  //   case BOT_A:
  //     ProcesaRebut((char*) "#P--@--MENU-1");
  //     break;
  //   case BOT_B:
  //     ImprimeTicket(TKT_TOTAL);
  //     last_option_print = MP_PRINTER_SIM;
  //     break;
  //   default:
  //     break;
  //   }
  //   break;
  // case MP_PRINTER_COM:
  //   switch (nBoto){
  //   case BOT_A:
  //     ProcesaRebut((char*) "#P--@--MENU-1");
  //     break;
  //   case BOT_B:
  //     peso_entrada = peso_last;
  //     peso_salida = 0;
  //     getLocalTime(&hora_entrada);
  //     ProcesaRebut((char*) "#P--)--MENU-1");
  //     break;
  //   case BOT_C:
  //     peso_salida = peso_last;
  //     getLocalTime(&hora);
  //     ProcesaRebut((char*) "#P--)--MENU-1");
  //     break;
  //   case BOT_D:
  //     ImprimeTicket(TKT_COM);     
  //     last_option_print = MP_PRINTER_COM;
  //   default:
  //     break;
  //   }
  //   break;
  // default:
  //   break;
  // }
}
void TramaToDisp(char buf[], byte index){
  PES = "";
  for (byte c = 0; c<nDIGITS_DISPLAY; c++){
    char C = buf[c + index];
    if (!isdigit(C)){
      if (C == '_'){
        PES += "<span style='color: #333333;'>8</span>";
      } else {
        PES += ' ';
      } 
    } else {
      PES += C;
    }
  }
  DISPLAY_PES(PES);
}
void ErrorToDisp(char buf[], byte index){
  // #E59000?R?T?0
  String str = "";
  str = TXT_ERROR_TRACTORA[RebutToError(buf[index], buf[index + 1])];
  str += "<br>";
  str += TXT_ERROR_REMOLQUE[RebutToError(buf[index+2], buf[index + 3])];
  DISPLAY_TXT(TXT_SUPERIOR, str);
}
int  TramaToPes(char buf[], byte index){
  String str = "";
  for (byte c = 0; c<nDIGITS_DISPLAY; c++){
    char C = buf[c + index];
    if (isdigit(C)) str += C;
  }
  return str.toInt();
}
void TramaToExtras(char buf[], byte quants, byte index){
  JsonObject txt_sen = JSON.createNestedObject("items");
  char dato[8];
  for (byte c=0; c<quants; c++){
    if (strlcpy(dato, buf+index+c*6, 7) < 6){///// CONFIABLE??
      _spln("ERROR: se esperaba trama con mas items");
      return; 
    }
    String str = "item";
    str += c;
    txt_sen[str] = dato;
    // _sp(dato);
    // _sp(" ");
  }
  // _spln();
  //txt_sen["item7"] = "A31475";   ////////////////////////////////////////////////////////// FAKE
};
byte RebutToConfig(char C){
  return C - '0' + 1;
}
void PulsatMando(byte Nboto){
  switch (mode_func){
  case NORMAL:
    EnviaPeti((petis) Nboto);
    break;
  case PRINTER:
    VirtualPeti(Nboto);
  default:
    break;
  }  
}
void PulsatMandoFunc(byte Nboto){
    ModeFuncionament((modes_func) Nboto);
    return;
}
void ModeFuncionament(modes_func modo){
  _sp("MODE FUNCIONAMENT --> "); _spln(modo);
  mode_func = modo;
  switch (modo){
  case NORMAL:
    EnviaPeti(visu_ext? REQUEST_EXT_DISPLAY : REQUEST_DISPLAY);
    break;
  case PRINTER: 
    ProcesaRebut((char*) "#P--@-MENU-1"); 
    break;
  default:
    break;
  }  
}
void ModeVisualizacion(bool modo){
  _sp("PROCESSEM CANVI VISU "); _spln(modo);
  //if (modo == visu_ext) return;   ///////////////////// no processiem si informa del mateix;
  visu_ext = modo;
  // Guardar preferencia en EEPROM si es necesario
  JSON.clear();
  DISPLAY_MODE(visu_ext, mode_visu);
  ENVIA_JSON();
}
byte RebutToCalTipus(char C){
  switch (C){
    case 'U': return 1;
    case 'A': return 2;
    case 'R': return 3;  
  }
  return 0;
}
byte RebutToSensor(char C){
  // ANTIC SISTEMA
  // switch (C){
  //   case 'S': return 1;
  //   case '1': return 2; 
  //   case '2': return 3;
  //   case 'M': return 4;     
  // }
  // return 0;
  byte R = C - '0';
  if (R > 8) R = 0;
  return R;
}
byte RebutToPespa(char C){   //// DEMODE
  switch (C){
  //   case '@': return 1;     ANTIC
  //   case 'R': return 2;
    case 'C': return 1;
    case '1': return 2;
    case '2': return 3;
    case '3': return 4;
  }
  return 0; 
}
inteligent_error RebutToError(char A, char B){
  switch (A){
    case 'R': return CAL_OK;
    case 'E': return FALLO_CAL;
    case 'T': return NO_CONECTADO;
    case 'M': switch (B){
      case 'L': return NO_CAL;
      case 'C': return FALTA_CARG;
      case 'B': return FALTA_VACIO;
    }
  }
  return NO_INFORMADO;
}
byte FasCalToPart(char FasCal){
  _sp("FASCALtoPART:");_spln(FasCal);
  if (FasCal > '0'){
    if (FasCal < '7') return 0;
    if (FasCal < '=') return 1;
    return 2;
  }
  return 0;
}
byte SubModeOf(char msg[]){
  String str = "";
  for (byte c = 7; c < 11; c++){
    str += msg[c];
  } 
  if (str == "DISP") return MP_DISPL;
  if (str == "RMML") return MP_REMOL;
  if (str == "RECA") return msg[4] == '3'? (byte) MP_RECAL : (byte) CAL_FALLO;
  if (str == "1MAV") return CAL_FALLO;
  if (str == "ENSO") return MP_SENSO;
  if (str == "ESO@") return MP_PESPA;
  if (str == "LARM") return MP_ALARM;
  if (str == "COOF") return MP_CONFI;
  if (str == "@NO@") return 1;                                      //////// originalment "@N0@"
  if (str == "@SI@") return 2;
  if (str == "UACI") return 1;
  if (str == "ARGA") return 2;
  if (str == "RESE") return 3;
  if (str == "CALU") return 0;
  if (str == "RA@U") return 0;
  if (str == "AM@U") return 1;
  if (str == "RA@C") return 0;
  if (str == "AM@C") return 1;
  if (str == "SENS") return SENSE;
  if (str == "ESET") return CAL_BORRA;
  if (str == "MML@") return CAL_NO_CAL;
  if (str == "MMC@") return CAL_FAL_C;
  if (str == "MMB@") return CAL_FAL_V;
  if (str == "3SEN") return VIS_MBAR;
  if (str == "TRAC") return 0;
  if (str == "SREM") return 0;
  if (str == "@TRA") return 1;
  if (str == "REMO") return 2;
  if (str == "GTAR") return 1;
  if (str == "VTAR") return 0;
  if (str == "ICAM") return 0;
  if (str == "SALI") return SALIR;
  
  return 0;
  // if (str == "1MAV") return CAL_FALLO;   /// ES FALLO CALCULOS O REINICIANDO SISTEMAS?????????
  /////// DEVINCULAT DE SubModeOf ////////////////
  // if (str == "ORIG") return 1;
  // if (str == "OLEF") return 2;
  // if (str == "OSPP") return 3;
  // if (str == "@PUN") return 1;
  // if (str == "@PDO") return 2;
  // if (str == "@PTR") return 3;
  // if (str == "@@@O") return 1; 
  // if (str == "@@OF") return 2;
  // if (str == "INPU") return INPUT_RQ;
  // if (str == "@TIM") return TIME_OUT;
  // if (str == "@MEN") return MENU_PRI;
  /////////////////////////////////////////////////
// SER_rebut: #E58<91MAVA0
// MODE NO IMPLEMENTAT
// SER_rebut: #E58:9ICAMM0

// SER_rebut: #EL839DISPL0
}
mode ModeOf(char m, char e){
  if (m == '0') return (e >= ':')? ERROR_INT : LECTURA_NETO? INICIO_NETO : INICIO_TOTAL;
  if (m == '3') return MENU_PRINCIPAL;
  if (m == 'L') return MP_DISPL;
  if (m == 'M') return MP_DISPL_OP;
  if (m == 'N') return MP_DISPL_OP;
  if (m == ']') return MP_DISPL_OP;
  if (m == 'S') return MP_REMOL;
  if (m == 'T') return MP_REMOL_OP;
  if (m == 'V') return MP_RECAL;
  if (m == 'U') return MP_RECAL;      
  if (m == 'W') return MP_RECAL_OP;
  if (m == 'X') return MP_RECAL_OP;
  if (m == '=') return MP_RECAL_OP;
  if (m == '2') return INTRO_VALOR;
  if (m == 'b') return MP_CONFI;
  if (m == 'O') return visu_ext? MP_SENSO_EXT : MP_SENSO;
  if (m == 'P') return VIS_VALOR;
  //if (m == 'R') return VIS_MBAR;
  if (m == '6') return visu_ext? MP_PESPA_EXT : MP_PESPA;
  if (m == '>') return MP_ALARM;
  if (m == '?') return MP_TARA;
  if (m == 'Z') return MP_TARA_OP;
  if (m == 'D') return MP_TARA_OP;
  if (m == '7') return REINI;  //  tambe es salir/volver menu
  if (m == '<') return REINI;  //segurooo?
  if (m == '@') return MP_PRINTER;
  if (m == '(') return MP_PRINTER_SIM;
  if (m == ')') return MP_PRINTER_COM;  
  //if (m == 'E') return EXT_DISP_EJES;
  //if (m == ':') return REINI; // NOP  PQ COMPARTEIX ':' AMB CANVI DE MENU
  return MAX_MODE;
}
void TypeCent(byte NtypeCent){
  
}
// ////// FUNCIONS A PASSAR A TARVOS.CPP
// bool PasaConfig(){
//   COMM_config config;
//   COMM_config config_read;
//   byte ConfigBuffer[30];
//   size_t N;
//   while(CENT.available()) {_sp("HABIA ALGO: "); _spln(CENT.read());}   //////////// BORRAMOS CARACTERES RAROS DE ARRANQUE
//   digitalWrite(PIN_LED, HIGH);
//   CENT.setTimeout(500);
//   N = CENT.readBytes(ConfigBuffer, 21);
// #ifdef ESP32
//     _sp("PASADO CONF--> ");
//     for (byte c = 0; c < N; c++){
//         _sp(ConfigBuffer[c], HEX); Serial.print(" ");
//     }
//     _spln();
// #endif
//   if (N < 8 ){
// #ifdef ESP32
//     _spln("CONFIG CORTO");
// #endif
//     return false;
//   }
//   config.adress = ConfigBuffer[0];
//   config.net = ConfigBuffer[1];
//   config.dia = ConfigBuffer[2];
//   config.mes = ConfigBuffer[3];
//   config.channel = ConfigBuffer[8];
//   config.CS = ConfigBuffer[9];
//   if (!config.CheckSumOK() or (config.adress == 0 && config.channel == 0 && config.channel == 0)){
// #ifdef ESP32
//   _spln("CS Config ERROR");
// #endif
//   return false;
//   }

//   // actualitzem nom BLE
//   // char str[20];
//   // sprintf(str, "PIKAM_%02X%02X", config.adress, config.net);
//   // BLEDevice::init(str);
//   // digitalWrite(PIN_LED, LOW);
//   // delay(50);
//   return true;
// }


/////OBSOLET///////////////////////////////////////////////////////
//// 
// bool EsperaConfig(){
//   unsigned long T_RIout = 0;
//   _sp("ESPERA CONF ");
//   bool result = false;
//   while(CENT.available()) {CENT.read();}
//   t_REQUEST_OUT = millis() + TEMPS_ESPERA_CONFIG;
//   while((millis() < t_REQUEST_OUT) & !result){     
//     if (CONFIG){
//       delay(5);
//       if (CONFIG){  
//         result = PasaConfig();
//         while(CONFIG){};
//         _sp(result? "V" : "X");
//       }
//     }
//     // MENTRES ANEM ENVIANT "REINICIANDO..."
//     if (millis() > T_RIout){
//       T_RIout = millis() + 100;
//       ProcesaRebut((char*) "#E58<91MAVA0");
// #ifdef TARVOS
//       TRX.print("#E58<91MAVA0\r");
// #endif
//     }
//     yield();
//   }
//   _spln(".");
//   return result;
// }
//
// #ifdef TARVOS
//   ConfigTAR(config);
//   EEPROM.get(EEPROM_DIR, config_read);
//   if (config_read == config){
//     _spln("Parametros ya guardados");
//   } else {
//     EEPROM.put(EEPROM_DIR, config);
//     EEPROM.commit();    
//     _spln("Escritura CONFIG OK!");
//   }
// #endif
//
// void ConfigEEPROM(){    ////////// SENSE TARVOS TE SENTIT??
//   COMM_config config;
//   TIMER_NC_RESET;
//   _spln("Configuracion desde EEPROM");
//   EEPROM.get(EEPROM_DIR, config);
//   if (!config.CheckSumOK() | (config.channel > 0x64) | ((config.adress == 0x00) & (config.net == 0x00))){
//     config = {1, 1, 21, 21};
//     _spln("ERROR datos configuracion. Por defecto 01:01 864.05");
//   }
// #ifdef TARVOS
//   ConfigTAR(config);
// #endif
// }
