#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <windows.h> //Necesario para poder posicionar en cualqueir lado de la pantalla los datos de los jugadores.
#include "../mapa/menu.h"
#include "recursos.h"
#include "stdbool.h"
#include <math.h>

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

for (int i = 0; i < 6; i++) {
        j->obreros[i].x = 400.0f + (i * 70.0f); // Aparecen cerca en el mapa
        j->obreros[i].y = 400.0f;
        j->obreros[i].destinoX = j->obreros[i].x;
        j->obreros[i].destinoY = j->obreros[i].y;
        j->obreros[i].moviendose = false;
        j->obreros[i].seleccionado = false;
        j->obreros[i].dir = DIR_FRONT;
    }

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
// void mostrarStats(struct Jugador j, int x, int y) {
//     // Ahora usa la funcion de mapa.c
//     dibujarPanelEnMapa(j);
// }

void actualizarObreros(struct Jugador *j) {
    float vel = 3.5f; // Velocidad de caminata
    for (int i = 0; i < 6; i++) {
        UnidadObrero *o = &j->obreros[i];
        if (o->moviendose) {
            float dx = o->destinoX - o->x;
            float dy = o->destinoY - o->y;
            float dist = sqrt(dx*dx + dy*dy);

            if (dist > vel) {
                o->x += (dx / dist) * vel;
                o->y += (dy / dist) * vel;
                // Cambio de dirección visual
                if (fabs(dx) > fabs(dy)) o->dir = (dx > 0) ? DIR_RIGHT : DIR_LEFT;
                else o->dir = (dy > 0) ? DIR_FRONT : DIR_BACK;
            } else {
                o->x = o->destinoX; o->y = o->destinoY;
                o->moviendose = false;
            }
        }
    }
}