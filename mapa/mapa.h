#ifndef MAPA_H
#define MAPA_H

#include "../recursos/recursos.h"
#include "../recursos/ui_compra.h"
#include <windows.h>

// Forward declaration de la estructura
struct MenuCompra;

// --- CONSTANTES DE DIMENSIÓN ---
#define MAPA_SIZE 2048
#define TILE_SIZE 32                      // Tamaño lógico (celda de matriz)
#define GRID_SIZE (MAPA_SIZE / TILE_SIZE) 
#define SPRITE_ARBOL 128                  // Tamaño visual del BMP de árbol

typedef struct {
  int x;      // Posicion X en el mapa 2048
  int y;      // Posicion Y en el mapa 2048
  float zoom; // Nivel de zoom
} Camara;

// --- COLISIONES / GRID (matriz dinámica con punteros) ---
// Retorna una matriz GRID_SIZE x GRID_SIZE (int**) donde 1 = ocupado.
int **mapaObtenerCollisionMap(void);
// Reconstruye la matriz en base a los árboles registrados en mapaObjetos.
void mapaReconstruirCollisionMap(void);
// Marca un edificio en el collision map como impasable
void mapaMarcarEdificio(float x, float y, int ancho, int alto);
// Detecta automáticamente una posición válida en la orilla del mapa
void mapaDetectarOrilla(float *outX, float *outY, int *outDir);
// Libera la memoria del collisionMap dinámico.
void mapaLiberarCollisionMap(void);

// Dibuja el mundo (terreno, árboles, obreros) en el DC especificado
// Ahora acepta el menú para dibujarlo dentro del mismo buffer (evitar parpadeo)
void dibujarMundo(HDC hdc, RECT rect, Camara cam, struct Jugador *pJugador,
                  struct MenuCompra *menu);
void cargarRecursosGraficos();
void dibujarObreros(HDC hdcBuffer, struct Jugador *j, Camara cam, int anchoP,
                    int altoP);
void mapaSeleccionarIsla(int isla);

#endif