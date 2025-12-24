
#include <Arduino.h>

#define DOBLE   //str += "\e!\x20"
#define SIMPLE  //str += "\e!\x01"
#define NL  str += "\n"

String matricula, albaran;
int peso_last, peso_entrada, peso_salida;
struct tm hora, hora_entrada;

enum TKT_TIPO {TKT_TOTAL, TKT_ENTRADA, TKT_SALIDA, TKT_COM};

char* _asctime(const struct tm *timeptr);

String tiket(TKT_TIPO tipo){
    String str = "";// "\e@";
    int dif;
    switch (tipo)  {
    case TKT_TOTAL:
      SIMPLE;
      str += "-------------------------\n"; DOBLE;
      str += "ID: "; str += matricula; SIMPLE; NL;
      str += "-------------------------\n";
      str += "Albaran: "; DOBLE; str += albaran; SIMPLE; NL; 
      str += "Peso Total:\n"; DOBLE;
      str += (String) peso_salida; str += "Kg\n"; SIMPLE;
      str += _asctime(&hora);
      str += "-------------------------\n";
      break;
    case TKT_COM:
      str += "-------------------------\n"; DOBLE;
      str += "ID: "; str += matricula; SIMPLE; NL;
      str += "-------------------------\n";
      str += "Albaran: "; DOBLE; str += albaran; SIMPLE; NL; 
      str += "Peso Entrada:\n"; DOBLE;
      str += (String) peso_entrada; str += "Kg\n"; SIMPLE;
      str += _asctime(&hora_entrada);
      dif = peso_salida - peso_entrada;
      SIMPLE;        
      str += "Peso Salida:\n"; DOBLE;
      if (peso_salida != 0){
        str += peso_salida; str += "Kg\n"; SIMPLE;
        str += "Peso "; str += dif < 0? "Descargado" : "Cargado:"; NL; DOBLE;
        str += (String) abs(dif); str += "Kg\n"; SIMPLE;
        str += _asctime(&hora);
        } else {
            str += "\n\n\n\n";
        }
      str += "-------------------------\n";
      break;
    case TKT_ENTRADA:
    case TKT_SALIDA:
    default:
        break;
    }	
    return str;
}

char* _asctime(const struct tm *timeptr){
//   static const char wday_name[][4] = {
//     "Dom", "Lun", "Mar", "Mie", "Jue", "Vie", "Sab"
//   };
  static const char mon_name[][4] = {
    "Ene", "Feb", "Mar", "Abr", "May", "Jun",
    "Jul", "Ago", "Sep", "Oct", "Nov", "Dic"
  };
  static char result[26];
  sprintf(result, "%.2d:%.2d:%.2d %3d %.3s %d\n",
    timeptr->tm_hour,
    timeptr->tm_min,
    timeptr->tm_sec,
    //wday_name[timeptr->tm_wday],
    timeptr->tm_mday, 
    mon_name[timeptr->tm_mon],
    timeptr->tm_year + 1900);
  return result;
}   