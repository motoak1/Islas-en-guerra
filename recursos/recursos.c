/*#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <windows.h> //Necesario para poder posicionar en cualqueir lado de la pantalla los datos de los jugadores.

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
void IniciacionRecursos (struct Jugador *j ,char Nombre[30]){
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
void IniciacionTropa (struct Tropa *t, char Nombre[30], int Oro , int Comida, int Madera, int Piedra, int Vida, int Fuerza , int VelocidadAtaque,int DistanciaAtaque){
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
void MostrarInterfaz(struct Jugador j1, struct Jugador j2, struct Jugador j3, struct Jugador j4){
	gotoxy(0,0);
   	printf("%s ", j1.Nombre);
  	gotoxy(0,1);
  	printf("Comida:%d", j1.Comida);
  	gotoxy(0,2);
  	printf("Oro:%d ",j1.Oro);
  	gotoxy(0,3);
  	printf("Madera:%d ",j1.Madera);
  	gotoxy(0,4);
  	printf("Piedra:%d ",j1.Piedra);
  	gotoxy(0,5);
  	printf("Numero de tropas:%d\n",j1.NumeroTropas);
  	gotoxy(1,6);
  	printf("Espadas:%d ",j1.CantidadEspadas);
  	gotoxy(1,7);
  	printf("Arqueros:%d ",j1.CantidadArqueros);
  	gotoxy(1,8);
  	printf("Picas:%d",j1.CantidadPicas);
  	gotoxy(1,9);
  	printf("Caballeria:%d",j1.CantidadCaballeria);
  	
  	
  	gotoxy(100,0);
   	printf("%s ", j2.Nombre);
  	gotoxy(100,1);
  	printf("Comida:%d", j2.Comida);
  	gotoxy(100,2);
  	printf("Oro:%d ",j2.Oro);
  	gotoxy(100,3);
  	printf("Madera:%d ",j2.Madera);
  	gotoxy(100,4);
  	printf("Piedra:%d ",j2.Piedra);
  	gotoxy(102,5);
  	printf("Numero de tropas:%d ",j2.NumeroTropas);
  	gotoxy(105,6);
  	printf("Espadas:%d ",j2.CantidadEspadas);
  	gotoxy(105,7);
  	printf("Arqueros:%d ",j2.CantidadArqueros);
  	gotoxy(105,8);
  	printf("Picas:%d",j2.CantidadPicas);
  	gotoxy(105,9);
  	printf("Caballeria:%d",j2.CantidadCaballeria);
  	
  	gotoxy(0,20);
   	printf("%s ", j3.Nombre);
  	gotoxy(0,21);
  	printf("Comida:%d", j3.Comida);
  	gotoxy(0,22);
  	printf("Oro:%d ",j3.Oro);
  	gotoxy(0,23);
  	printf("Madera:%d ",j3.Madera);
  	gotoxy(0,24);
  	printf("Piedra:%d ",j3.Piedra);
  	gotoxy(0,25);
  	printf("Numero de tropas:%d\n",j3.NumeroTropas);
  	gotoxy(1,26);
  	printf("Espadas:%d ",j3.CantidadEspadas);
  	gotoxy(1,27);
  	printf("Arqueros:%d ",j3.CantidadArqueros);
  	gotoxy(1,28);
  	printf("Picas:%d",j3.CantidadPicas);
  	gotoxy(1,29);
  	printf("Caballeria:%d",j3.CantidadCaballeria);
  	
  	
  	gotoxy(100,20);
   	printf("%s ", j4.Nombre);
  	gotoxy(100,21);
  	printf("Comida:%d", j4.Comida);
  	gotoxy(100,22);
  	printf("Oro:%d ",j4.Oro);
  	gotoxy(100,23);
  	printf("Madera:%d ",j4.Madera);
  	gotoxy(100,24);
  	printf("Piedra:%d ",j4.Piedra);
  	gotoxy(102,25);
  	printf("Numero de tropas:%d ",j4.NumeroTropas);
  	gotoxy(105,26);
  	printf("Espadas:%d ",j4.CantidadEspadas);
  	gotoxy(105,27);
  	printf("Arqueros:%d ",j4.CantidadArqueros);
  	gotoxy(105,28);
  	printf("Picas:%d",j4.CantidadPicas);
  	gotoxy(105,29);
  	printf("Caballeria:%d",j4.CantidadCaballeria);
	
}
// int main(){
	
//     struct Jugador Jugador1;
//     struct Jugador Jugador2;
//     struct Jugador Jugador3;
//     struct Jugador Jugador4;
    
// 	struct Tropa Espadas;
//     struct Tropa Arqueros;
//     struct Tropa Picas;
//     struct Tropa Caballeria;
    
    
//     IniciacionRecursos(&Jugador1, "Jugador 1");
//     IniciacionRecursos(&Jugador2, "Jugador 2");
//     IniciacionRecursos(&Jugador3, "Jugador 3");
//     IniciacionRecursos(&Jugador4, "Jugador 4");
    
//     IniciacionTropa(&Espadas, "Espadas", 25,50,0,0,100,15,2,1);
//     IniciacionTropa(&Arqueros, "Arqueros",20,40,30,0,60,10,3,5);
//     IniciacionTropa(&Picas, "Picas",30,30,20,0,80,12,2,2);
//     IniciacionTropa(&Caballeria, "Caballeria",50,100,0,0,150,25,1,1);
    
//     MostrarInterfaz(Jugador1, Jugador2, Jugador3, Jugador4);
//     while(1) {
// 	//PARA QUE SE ACT A CADA RATO
//         MostrarInterfaz(Jugador1, Jugador2, Jugador3, Jugador4);
      
//     }

    
//     return 0;
// }*/
