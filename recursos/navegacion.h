#ifndef NAVEGACION_H
#define NAVEGACION_H

#include "recursos.h"
#include <windows.h>

// Verifica si un punto está dentro del barco (para detección de clicks)
bool barcoContienePunto(Barco* barco, float mundoX, float mundoY);

// Desembarca las tropas del barco en tierra cercana
void desembarcarTropas(Barco* barco, struct Jugador* j);

// Reinicia recursos y estado al llegar a una isla desconocida
void reiniciarIslaDesconocida(struct Jugador* j);

// NUEVO: Viaja directamente a una isla sin animación
void viajarAIsla(struct Jugador* j, int islaDestino);

#endif

