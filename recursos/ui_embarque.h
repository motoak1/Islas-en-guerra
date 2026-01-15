#ifndef UI_EMBARQUE_H
#define UI_EMBARQUE_H

#include "recursos.h"
#include <windows.h>

typedef struct MenuEmbarque {
  bool activo;
  bool eligiendoIsla;
  int obrerosSeleccionados;
  int caballerosSeleccionados;
  int guerrerosSeleccionados;
  int totalSeleccionados;
  bool mostrandoDesconocido; // Nuevo flag para submenú
  
  // Posición y tamaño de la UI
  int x, y;
  int ancho, alto;
} MenuEmbarque;

void menuEmbarqueInicializar(MenuEmbarque* menu);
void menuEmbarqueAbrir(MenuEmbarque* menu, int anchoVentana, int altoVentana);
void menuEmbarqueCerrar(MenuEmbarque* menu);
void menuEmbarqueDibujar(HDC hdc, MenuEmbarque* menu, struct Jugador* j);
bool menuEmbarqueClick(MenuEmbarque* menu, struct Jugador* j, int x, int y);
void menuEmbarqueEmbarcar(MenuEmbarque* menu, struct Jugador* j);

#endif
