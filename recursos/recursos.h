// recursos/recursos.h
#ifndef RECURSOS_H
#define RECURSOS_H

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

struct Jugador
{
    char Nombre[30];
    int Comida;
    int Oro;
    int Madera;
    int Piedra;
    struct Tropa *Ejercito;
    int CantidadEspadas; 
    int CantidadArqueros;
    int CantidadPicas;
    int CantidadCaballeria;
    int NumeroTropas;
	int Capacidad;
};

void IniciacionRecursos (struct Jugador *j ,const char *Nombre);
void IniciacionTropa (struct Tropa *t, const char *Nombre, int Oro , int Comida, int Madera, int Piedra, int Vida, int Fuerza , int VelocidadAtaque,int DistanciaAtaque);
void gotoxy(int x, int y);
void mostrarStats(struct Jugador j, int x, int y);
void dibujarPanelFondo();
void dibujarPanelEnMapa(struct Jugador j);  // ✅ AÑADIR ESTA LÍNEA

#endif
