// recursos/recursos.h
#ifndef RECURSOS_H
#define RECURSOS_H
#include <stdbool.h>
#include <windows.h> // Necesario para COORD en gotoxy

// --- ESTRUCTURAS ---

struct Tropa {
    char Nombre[30];
    int CostoOro;
    int CostoComida;
    int CostoMadera;
    int CostoPiedra;
    int Vida;
    int Fuerza;
	int VelocidadAtaque;
	int DistanciaAtaque;
};
// Estados de animación/dirección
typedef enum { DIR_FRONT, DIR_BACK, DIR_LEFT, DIR_RIGHT } Direccion;

typedef struct {
    float x, y;           // Posición exacta en el mapa (píxeles)
    float destinoX, destinoY; // A donde debe ir
    bool moviendose;
    bool seleccionado;
    Direccion dir;
    int frame;            // Para animar el caminado
} UnidadObrero;

struct Jugador
{
    char Nombre[30];
    int Comida;
    int Oro;
    int Madera;
    int Piedra;
    struct Tropa *Ejercito;
    UnidadObrero obreros[6];
    int CantidadEspadas; 
    int CantidadArqueros;
    int CantidadPicas;
    int CantidadCaballeria;
    int NumeroTropas;
	int Capacidad;
};


void actualizarObreros(struct Jugador *j);
void IniciacionRecursos (struct Jugador *j ,const char *Nombre);
void IniciacionTropa (struct Tropa *t, const char *Nombre, int Oro , int Comida, int Madera, int Piedra, int Vida, int Fuerza , int VelocidadAtaque,int DistanciaAtaque);
void gotoxy(int x, int y);
void mostrarStats(struct Jugador j, int x, int y);


#endif
