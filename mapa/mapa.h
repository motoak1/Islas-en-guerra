/// mapa.h
#ifndef MAPA_H
#define MAPA_H
#include <windows.h>
#include <stdio.h>
#include "../recursos/recursos.h"

#define FILAS 26
#define COLUMNAS 41
#define MAPA_F 150
#define MAPA_C 180

extern int offset_f;
extern int offset_c;

void inicializarMapa(char mapa[MAPA_F][MAPA_C]);
void mostrarMapa(char mapa[MAPA_F][MAPA_C], int px, int py);
void animarAgua(char mapa[MAPA_F][MAPA_C]);
int explorarMapa(struct Jugador *j, char mapa[MAPA_F][MAPA_C], int *x, int *y, char direccion);

void animarTalado(int x, int y);
void mostrarMenuCampamento(struct Jugador *j, char mapa[MAPA_F][MAPA_C], int px, int py);  // ✅ LÍNEA NUEVA
void mostrarMenuCasaCentral(struct Jugador *j, char mapa[MAPA_F][MAPA_C], int px, int py, int casa_x, int casa_y);
int hayRecursoAdyacente(char mapa[MAPA_F][MAPA_C], int px, int py, char recurso, int *rx, int *ry);
void dibujarPanelStats(CHAR_INFO *buffer, struct Jugador j);
void dibujarPanelEnMapa(struct Jugador j);
void forzarRedibujoPanelEnMapa(struct Jugador j);  // ✅ AÑADIR ESTA LÍNEA
void limpiarMensajes();

#endif
