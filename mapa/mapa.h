// mapa.h

#ifndef MAPA_H
#define MAPA_H

#include <windows.h>
#include <stdio.h>
#include "../recursos/recursos.h" // Sube al directorio padre y entra a recursos/

#define FILAS 26

// ¡¡MOVER ESTAS LÍNEAS AQUÍ ARRIBA!!
// Coordenadas de inicio del panel (Columna 85, Fila 1)
#define STATS_X 85 
#define STATS_Y 1

// ¡¡ESTA ES LA CORRECCIÓN CLAVE!!
// COLUMNAS ahora representa el ANCHO DEL MAPA VISIBLE (en tiles)
// (STATS_X - 2 (por el marco izq)) / 2 (porque cada tile es 'X ')
#define COLUMNAS ((STATS_X - 2) / 2) // Esto da 41 tiles

#define PANEL_WIDTH 18  // Ancho necesario para el texto del panel
#define PANEL_HEIGHT 6  // Alto necesario (Título + 4 recursos + 1 espacio)
#define MAPA_F 60  // Filas del mapa virtual
#define MAPA_C 150 // Columnas del mapa virtual

// Coordenadas de inicio del panel (Columna 85, Fila 1)
#define STATS_X 85
#define STATS_Y 1

// Variables globales para el offset de la vista (scrolling)
extern int offset_f;
extern int offset_c;

void inicializarMapa(char mapa[MAPA_F][MAPA_C]);
// CAMBIO: Ahora acepta px y py para dibujar al jugador dentro del buffer
void mostrarMapa(char mapa[MAPA_F][MAPA_C]);
void animarAgua(char mapa[MAPA_F][MAPA_C]);

// **CORRECCIÓN DE LA FIRMA:** Ahora acepta 'struct Jugador *' y devuelve 'int'
int moverJugador(struct Jugador *j, char mapa[MAPA_F][MAPA_C], int *x, int *y, char direccion);

#endif
