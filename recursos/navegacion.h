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
bool viajarAIsla(struct Jugador* j, int islaDestino);

// Registrar la isla inicial
void navegacionRegistrarIslaInicial(int isla);

// Posición fija del barco por isla (exportada para uso externo)
void navegacionObtenerPosicionBarcoIsla(int isla, float *outX, float *outY, int *outDir);

// Acceso a enemigos activos en la isla actual
Unidad *navegacionObtenerEnemigosActivos(int *cantidad);

// Obtiene la isla inicial registrada al inicio de la partida
int navegacionObtenerIslaInicial(void);

// Consulta si una isla está conquistada
bool navegacionIsIslaConquistada(int isla);

// True si la isla actual del jugador NO está conquistada
bool navegacionIslaActualNoConquistada(const struct Jugador *j);

#endif

