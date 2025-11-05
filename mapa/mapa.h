#ifndef MAPA_H
#define MAPA_H

#include <windows.h>
#include <stdio.h>

#define FILAS 26
#define COLUMNAS 50
#define MAPA_F 50  // Filas del mapa virtual
#define MAPA_C 100 // Columnas del mapa virtual

// Variables globales para el offset de la vista (scrolling)
extern int offset_f;
extern int offset_c;

void inicializarMapa(char mapa[MAPA_F][MAPA_C]);
void mostrarMapa(char mapa[MAPA_F][MAPA_C]);
void moverJugador(char mapa[MAPA_F][MAPA_C], int *x, int *y, char direccion);
void animarAgua(char mapa[MAPA_F][MAPA_C]);
void ocultarCursor();
void moverCursor(short x, short y);
void setColor(int fondo, int texto);
void mostrarMenu();

#endif
