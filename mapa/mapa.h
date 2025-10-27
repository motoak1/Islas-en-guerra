// mapa.h
#ifndef MAPA_H
#define MAPA_H

// Ajusta esto al tamaño de tu ventana y fuente
#define FILAS 40
#define COLUMNAS 80
#define TAM_CELDA 10 // Tamaño en píxeles de cada celda (para el renderizado)

typedef enum {
    AGUA,
    TIERRA,
    COMIDA,
    MADERA,
    RECURSO,
    JUGADOR
} TipoCelda;

typedef struct {
    TipoCelda celdas[FILAS][COLUMNAS];
    int jugadorX;
    int jugadorY;
    TipoCelda celdaBajoJugador; // Para recordar qué había debajo del jugador
} Mapa;

void inicializarMapa(Mapa *mapa);
void moverJugador(Mapa *mapa, char dir);
// ¡La nueva función de dibujado!
void dibujarMapa(Mapa *mapa);

#endif