#ifndef MAPA_H
#define MAPA_H

#include <windows.h>

// Definimos la estructura con typedef para poder usar 'Camara' directamente
typedef struct {
    int x;       // Posicion X en el mapa 1024
    int y;       // Posicion Y en el mapa 1024
    float zoom;  // Nivel de zoom
} Camara;

// Funciones Gráficas
void cargarRecursosGraficos();
void dibujarMundo(HDC hdc, RECT rectPantalla, Camara cam);
void explorarMapaGrafico(Camara *cam, int dx, int dy);

#endif