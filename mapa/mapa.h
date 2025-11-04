#ifndef MAPA_H
#define MAPA_H

#include <windows.h>
#include <stdio.h>

#define SIZE 30

void inicializarMapa(char mapa[SIZE][SIZE]);
void mostrarMapa(char mapa[SIZE][SIZE]);
void moverJugador(char mapa[SIZE][SIZE], int *x, int *y, char direccion);
void animarAgua(char mapa[SIZE][SIZE]);
void ocultarCursor();
void moverCursor(short x, short y);
void setColor(int fondo, int texto);
void mostrarMenu();

#endif
