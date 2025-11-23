#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <windows.h> //Necesario para poder posicionar en cualqueir lado de la pantalla los datos de los jugadores.
#include "menu.h"
#include "mapa.h"
#include "recursos.h"


void IniciacionRecursos (struct Jugador *j ,const char *Nombre){
	strcpy(j->Nombre,Nombre);
	j->Comida=200;
	j->Oro=100;
	j->Madera=150;
	j->Piedra=100;	
	j->Ejercito= NULL;
	j->NumeroTropas=0;
	j->Capacidad=0;
	j->CantidadCaballeria=0;
	j->CantidadArqueros=0;
	j->CantidadPicas=0;
	j->CantidadEspadas=0;
}
void IniciacionTropa (struct Tropa *t, const char *Nombre, int Oro , int Comida, int Madera, int Piedra, int Vida, int Fuerza , int VelocidadAtaque,int DistanciaAtaque){
	strcpy (t->Nombre,Nombre);
	t->CostoComida= Comida;
	t->CostoOro= Oro;
	t->CostoMadera=Madera;
	t->CostoPiedra=Piedra;
	t->Vida=Vida;
	t->Fuerza=Fuerza;
	t->VelocidadAtaque=VelocidadAtaque;
	t->DistanciaAtaque=DistanciaAtaque;
	
	
}
void gotoxy(int x, int y) {
    COORD coord;
    coord.X = x;
    coord.Y = y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}
void mostrarStats(struct Jugador j, int x, int y) {
    // Ahora usa la funcion de mapa.c
    dibujarPanelEnMapa(j);
}
void dibujarPanelFondo() {
    // Ya no hace nada, el panel se dibuja con mostrarStats
}
