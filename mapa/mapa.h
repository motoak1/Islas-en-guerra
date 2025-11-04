#ifndef MAPA_H
#define MAPA_H

#include <windows.h>
#include <stdio.h>

#define SIZE 20

void inicializarMapa(char mapa[SIZE][SIZE]);
void mostrarMapa(char mapa[SIZE][SIZE]);
void moverJugador(char mapa[SIZE][SIZE], int *x, int *y, char direccion);
void ocultarCursor();
void moverCursor(short x, short y);
void setColor(int colorFondo, int colorTexto);

#endif
