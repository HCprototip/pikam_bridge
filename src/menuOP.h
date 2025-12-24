#include <Arduino.h>

#define MILIBARS        "mbar"
#define KILOS           "Kg"
#define CONFIRME        "CONFIRME"
#define CALCULO         "CALCULO " + TXT_CAL_PART[OP_CAL_PART] + " SI COINCIDE, PULSAR OK"
#define FALLO_CALCULOS  "FALLO CALCULOS"
#define PESO_           "PESO "
#define CODE_IN         "CODE IN"
#define MIDIENDO        "MIDIENDO"
#define NETO            "NETO"
#define TOTAL           "TOTAL"

byte N_TXT_PES;
String PES = "00000";
String ULTIM_ITEM_MENU = "";
byte OP_CAL_TIPUS;
byte OP_CAL_PART;

static const String type_cent_JSON{""};
enum modes_visu{ EXTENDIDO, SENSORES, PESOS_PARCIALES};
enum modes_func{ PRINTER , NORMAL, MANDO, VIS_EJES, VIS_SENSORES};
struct OP_MENU{
  const String txt_titol;
  const String txt_opcio[20];
  const String grafic_opcio[20];
};
enum cal_ERROR{ CAL_BORRA = 240, CAL_NO_CAL = 250, CAL_FAL_C, CAL_FAL_V, CAL_FALLO};
enum mode {
  MENU_PRINCIPAL, MP_DISPL, MP_CONFI, MP_REMOL, MP_RECAL, MP_SENSO,  MP_PESPA, MP_ALARM, MP_TARA, REINI, MP_DISPL_OP, MP_REMOL_OP, MP_RECAL_OP, MP_TARA_OP, INTRO_VALOR, VIS_VALOR, INICIO_TOTAL, INICIO_NETO, SENSE, FALTA_CAL, VIS_MBAR, SALIR, MP_PRINTER, MP_PRINTER_SIM , MP_PRINTER_COM, EXT_DISP, MP_SENSO_EXT, MP_PESPA_EXT, EXT_DISP_SENSORES, ERROR_INT, MAX_MODE
}; /////////////      !!ESTAN VINCULATS mode i  modo_BOT |||
static const byte modo_BOT[]{
    1,              2,      2,        2,        2,        2,          2,        2,        2,        8,       2,          2,            3,          3,          4,            3,        0,            6,          0,      7,        0,        0,       9,          10,               11,           0,       12,           12,           0,                  7};
static const String textos_teclas[][5]{
  {"MENU", "", "", "NETO","IMP"},
  {"PREVIO", "&#9650", "&#9660", "OK", ""},
  {"PREVIO", "&#9650", "", "OK", ""},
  {"PREVIO", "", "", "OK", ""},
  {"PREVIO", "&#9650", "&#10140", "OK", ""},
  {"PREVIO", "REPETIR", "", "OK", ""},
  {"MENU", "", "TARA", "TOTAL", "IMP"},
  {"MENU", "", "", "OK", ""},
  {"MENU", "", "", "", ""},
  {"SALIR", "SIMPLE", "ENT/SAL", "REPETIR"},
  {"VOLVER", "IMPRIMIR"},
  {"VOLVER", "ENTRADA", "SALIDA", "IMPRIMIR"},
  {"VOLVER", "", "", "", ""}
};
//// TODO: Cal separar més opcions inteligent/normal?
static const OP_MENU MENU[]{
  {"MENU PRINCIPAL", {"#?", "DISPLAY", "CONFIGURATION", "REMOLQUE", "RECALIBRAR", "SENSORES", "PESOS PARCIALES", "ALARMAS"},
                    {"NoFoto", "MP_displ", "MP_recal",  "MP_remol", "MP_recal",   "MP_senso", "MP_pesos",        "MP_alarm"}},
  {"MENU DISPLAY", {"#?", "BLOQUEO ULTIMO 0", "ANULAR CEROS", "WEIGHT SPEED"},
                    {"#?", "MD_block",        "MD_nozr",      "MD_speed"}},
  {"MENU CONFIGURACION", {"#?", "REG1",     "REG2",     "REG3",     "REG4",     "REG5",     "REG6",     "REG7",     "REG8",     "CONF. DEFAULT"},
                    {"NoFoto",  "MP_recal", "MP_recal", "MP_recal", "MP_recal", "MP_recal", "MP_recal", "MP_recal", "MP_recal", "MP_alarm"}},
  {"MENU REMOLQUE", {"REMOLQUE #?", "REMOLQUE 1", "REMOLQUE 2", "REMOLQUE 3", "REMOLQUE 4"},
                    {"NoFoto",         "MR_rem1",      "MR_rem2",      "MR_rem3",      "MR_rem4"}},  
  {"MENU RECALIBRAR", {"#?", "CALIBRAR VACIO", "CALIBRAR CARGADO", "BORRAR CALIBRACION"},
                      {"NoFoto","MC_vacio",            "MC_cargado",           "MC_borrar"}},
#ifdef INTELIGENTE
  {"MENU SENSORES", {"SENSOR #?","S1",       "S2",       "S3",       "S4",       "S5",       "S6",       "S7"},
                    {"NoFoto", "MP_senso", "MP_senso", "MP_senso", "MP_senso", "MP_senso", "MP_senso", "MP_senso"}},
  {"MENU PESOS PARCIALES", {"EJE TRACTORA", "EJES REMOLQUE 1", "EJES REMOLQUE 2", "EJES REMOLQUE 3", "REMOLQUE 1 EJE 3", "REMOLQUE 2 EJE 1", "REMOLQUE 2 EJE 2", "REMOLQUE 2 EJE 3"},
                           {"ML_trac",       "ML_rem",         "ML_rem",          "ML_rem",          "ML_rem",           "ML_rem",           "ML_rem",           "ML_rem"}},
  {"MENU ALARMAS", {"No DEFINIDO", "ALARMA 1", "ALARMA 2", "ALARMA EJE 1", "ALARMA EJE 2", "ALARMA EJE 3", "ALARMA EJE 4", "ALARMA EJE 5", "ALARMA EJE 6", "ALARMA EJE 7", "ALARMA EJE 8", "ALARMA EJE 9", "ALARMA EJE 10", "ALARMA EJE 11", "ALARMA EJE 12", "ALARMA EJE 13", "ALARMA EJE 14", "ALARMA EJE 15", "ALARMA EJE 16"},
                  {"NoFoto",       "MA_al1",   "MA_al2",   "MA_aleje",     "MA_aleje",     "MA_aleje",     "MA_aleje",     "MA_aleje",     "MA_aleje",     "MA_aleje",     "MA_aleje",     "MA_aleje",     "MA_aleje",      "MA_aleje",      "MA_aleje",      "MA_aleje",      "MA_aleje",      "MA_aleje",      "MA_aleje"}},
#else          
  {"MENU SENSORES", {"SENSOR #?","SENSOR TRACTORA 1", "SENSOR TRACTORA 2", "SENSOR REMOLQUE 1", "SENSOR REMOLQUE 2"},
                      {"NoFoto",       "MS_trac1",           "MS_trac2",              "MS_rem1",            "MS_rem2"}},          
  {"MENU PESOS PARCIALES", {"PESO #?", "EJE TRACTORA", "EJE REMOLQUE"},
                           {"nop",    "ML_trac",        "ML_rem"}},
  {"MENU ALARMAS", {"ALARMA #?", "ALARMA 1", "ALARMA 2"},
                  {"NoFoto",    "MA_al1",    "MA_al2"}},
#endif
  {"MENU TARA", {"BORRAR TARA" , "TARAR CAMION"},
                  {"MT_borra",   "MT_tarar"}},
  {"REINICIANDO SISTEMAS",{""},
                        {"MP_reini"}}
};
static const OP_MENU MENU_DISPLAY[]{
  {},
  {"CEROS FIJOS", {"#?","ACTIVAR", "DESACTIVAR"},
                  {"NoFoto","SEL_on", "SEL_off"}},
  {"ANULAR CEROS", {"#?","ACTIVAR", "DESACTIVAR"},
                  {"NoFoto","SEL_on", "SEL_off"}},
  {"WEIGHT SPEED", {"#?","FAST", "MEDIUM", "SLOW"},
                  {"NoFoto","MD_fast", "MD_medium", "MD_slow"}}
};
static const OP_MENU MENU_REMOLQUE[]{
  {"SEL. REMOLQUE ", {"DESACTIVAR", "ACTIVAR"},
                            {"SEL_off", "SEL_on"}}
};


static const OP_MENU MENU_TARA[]{
  {"BORRADO TARA", {"EJECUTAR"},
                  {"NoFoto"}},
  {"TARAR CAMION", {"TARANDO", "  ", "TRES"},
                  {"NoFoto"}},
  {".......", {"MAS  ABAJO"},
                  {"NoFoto"}}
};
static const OP_MENU FALTA_CALIB[]{
  {"REMOLQUE n",  {"NO CALIBRADO", "FALTA CARGADO", "FALTA VACIO", "FALLO CALCULOS"},
                  {"MP_Nocal",     "MP_Nocal",      "MP_Nocal",    "MP_Emth"}}
};

enum petis {
  REQUEST_DISPLAY, P1, P2, P3, P4, P5, RESET_CENT_SOFT, RESET_CENT_HARD, REQUEST_EXT_DISPLAY
} ;
enum ID_TXT {TXT_SUPERIOR, TXT_MITJA, TXT_UNIDADES};
static const char mandomess[][11] = { 
    { 36, 48, 49, 88, 68, 48, 48, 48, 48, 48, 13 }, //REQUEST_DISPLAY  
		{ 36, 48, 49, 80, 55, 48, 48, 48, 48, 48, 13 }, //REQUEST_KEY0
		{ 36, 48, 49, 80, 56, 48, 48, 48, 48, 48, 13 }, //REQUEST_KEY1
		{ 36, 48, 49, 80, 57, 48, 48, 48, 48, 48, 13 }, //REQUEST_KEY2
		{ 36, 48, 49, 80, 58, 48, 48, 48, 48, 48, 13 }, //REQUEST_KEY3
		{ 36, 48, 49, 80, 59, 48, 48, 48, 48, 48, 13 }, //REQUEST_KEY4
    { 36, 48, 49, 77, 77, 49, 55, 55, 54, 48, 13 }, //SOFT_RESET_CENT
    { 36, 48, 49, 84, 85, 48, 48, 48, 48, 48, 13 }, //RESET_CENT
    { 36, 48, 49, 85, 85, 48, 48, 48, 48, 48, 13 }, //REQUEST_EXT_DISPLAY 
};
#ifdef MEGATRAILER
static const String TXT_CAL_PART[] = {"TRACTORA", "TRACTORA Y 1er REMOLQUE", "SEGUNDO REMOLQUE"};
static const String TXT_CAL_PART_SENSO[] = {"TRACTORA", "PRIMER REMOLQUE", "SEGUNDO REMOLQUE"};
static const OP_MENU MENU_CALIBRAR[]{
  {"CALIBRAR ", {"EJECUTAR", "Situar tractora en la báscula. Desfrenada, anotar peso<br> Pulsar OK", "Situar la tractora y primer remolque en la báscula. Desfrenado, anotar peso<br> Pulsar OK", "Situar el segundo remolque en la báscula. Desfrenado, anotar peso<br> Pulsar OK", "PRESION", "CONFIRME", "REMOLQUE", "PRESION EN mB"}}
enum mode_CALIBRAR {EJEC,     SIT_T,                                                                  SIT_C,                                                                                       SIT_D,                                                                             SENSO,     ESET0,       NOCAL,      PRESION};
};
#else
static const String TXT_CAL_PART[] = {"TRACTORA", "CAMION"};
static const String TXT_CAL_PART_SENSO[] = {"TRACTORA", "REMOLQUE"};
static const OP_MENU MENU_CALIBRAR[]{
  {"CALIBRAR ", {"EJECUTAR", "Situar tractora en la báscula. Desfrenada, anotar peso<br> Pulsar OK", "Situar el camión en la báscula. Desfrenado, anotar peso<br> Pulsar OK", "PRESION", "CONFIRME", "REMOLQUE", "PRESION EN mB"}}
};
enum mode_CALIBRAR {EJEC,     SIT_T,                                                                  SIT_C,                                                                   SENSO,     ESET0,       NOCAL,      PRESION};


#endif
enum inteligent_error {                      CAL_OK,            FALLO_CAL,                 NO_CAL,                  FALTA_CARG,               FALTA_VACIO,            NO_CONECTADO,            NO_INFORMADO};
static const String TXT_ERROR_TRACTORA[] = {"TRACTORA CAL OK", "TRACTORA FALLO CÁLCULOS", "TRACTORA NO CALIBRADA", "TRACTORA FALTA CARGADO", "TRACTORA FALTA VACIO", "",                      ""};
static const String TXT_ERROR_REMOLQUE[] = {"REMOLQUE CAL OK", "REMOLQUE FALLO CÁLCULOS", "REMOLQUE NO CALIBRADO", "REMOLQUE FALTA CARGADO", "REMOLQUE FALTA VACIO", "REMOLQUE DESCONECTADO", ""};
mode modeAct = mode::MAX_MODE;