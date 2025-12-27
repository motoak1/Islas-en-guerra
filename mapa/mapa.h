#ifndef MAPA_H
#define MAPA_H

#include <windows.h>
#include "../recursos/recursos.h"

// --- CONSTANTES DE DIMENSIÓN ---
#define MAPA_SIZE 2048    //
#define TILE_SIZE 64      // Tamaño lógico (celda de matriz)
#define GRID_SIZE (MAPA_SIZE / TILE_SIZE) // 32x32 celdas
#define SPRITE_ARBOL 128  // Tamaño visual del BMP de árbol

typedef struct {
    int x;       // Posicion X en el mapa 2048
    int y;       // Posicion Y en el mapa 2048
    float zoom;  // Nivel de zoom
} Camara;

// --- COLISIONES / GRID (matriz dinámica con punteros) ---
// Retorna una matriz GRID_SIZE x GRID_SIZE (int**) donde 1 = ocupado.
int **mapaObtenerCollisionMap(void);
// Reconstruye la matriz en base a los árboles registrados en mapaObjetos.
void mapaReconstruirCollisionMap(void);
// Libera la memoria del collisionMap dinámico.
void mapaLiberarCollisionMap(void);

// Funciones Gráficas
// Actualizamos el prototipo para incluir al jugador
void dibujarMundo(HDC hdc, RECT rectPantalla, Camara cam, struct Jugador *pJugador);
void cargarRecursosGraficos();
void dibujarObreros(HDC hdcBuffer, struct Jugador *j, Camara cam, int anchoP, int altoP);

#endif