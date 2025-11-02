// mapa.h
#ifndef MAPA_H
#define MAPA_H

#define FILAS 40
#define COLUMNAS 80

typedef enum {
    AGUA,
    TIERRA,
    JUGADOR,
    COMIDA,
    MADERA,
    RECURSO  // Oro
} TipoCelda;

typedef struct {
    TipoCelda celdas[FILAS][COLUMNAS];
    int jugadorX;
    int jugadorY;
    TipoCelda celdaBajoJugador;
} Mapa;

// Declaraciones de funciones
void inicializarConsola(int ancho, int alto);
void restaurarCursorVisible();
void moverCursor(int x, int y);
void setColor(int color);
void prepararFrame();
void inicializarMapa(Mapa *mapa);
void moverJugador(Mapa *mapa, char dir);
void dibujarMapa(Mapa *mapa, double tiempo);

#endif