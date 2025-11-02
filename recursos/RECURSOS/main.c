// juego.c - MAPA ORIGINAL 40x80
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <time.h>
#include <windows.h>

#define FILAS 40
#define COLUMNAS 80

typedef enum {
    AGUA, TIERRA, JUGADOR, COMIDA, MADERA, RECURSO
} TipoCelda;

typedef struct {
    TipoCelda celdas[FILAS][COLUMNAS];
    int jugadorX, jugadorY;
    TipoCelda celdaBajoJugador;
} Mapa;

struct Jugador {
    char Nombre[30];
    int Comida, Oro, Madera, Piedra, NumeroTropas;
    int CantidadEspadas, CantidadArqueros, CantidadPicas, CantidadCaballeria;
};

void IniciacionRecursos(struct Jugador *j, char Nombre[30]) {
    strcpy(j->Nombre, Nombre);
    j->Comida = 200; j->Oro = 100; j->Madera = 150; j->Piedra = 100; j->NumeroTropas = 0;
    j->CantidadEspadas = j->CantidadArqueros = j->CantidadPicas = j->CantidadCaballeria = 0;
}

static float ruidoPerlin(int x, int y, int semilla) {
    int n = x + y * 57 + semilla * 131;
    n = (n << 13) ^ n;
    return (1.0f - ((n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0f);
}

void inicializarMapa(Mapa *mapa) {
    srand((unsigned)time(NULL));
    int i, j, n;
    
    for (i = 0; i < FILAS; i++)
        for (j = 0; j < COLUMNAS; j++)
            mapa->celdas[i][j] = AGUA;
    
    int centros[4][2] = {{10, 20}, {10, 60}, {30, 20}, {30, 60}};
    
    for (n = 0; n < 4; n++) {
        int cx = centros[n][0], cy = centros[n][1];
        int radio = 8 + rand() % 5;
        int semilla1 = rand() % 1000;
        int semilla2 = rand() % 1000;
        
        for (i = -radio * 2; i <= radio * 2; i++) {
            for (j = -radio * 2; j <= radio * 2; j++) {
                int x = cx + i;
                int y = cy + j;
                if (x < 0 || x >= FILAS || y < 0 || y >= COLUMNAS)
                    continue;
                
                float dist = (i * i + j * j) / (float)(radio * radio);
                float deform1 = (ruidoPerlin(x, y, semilla1) + 1.0f) / 2.0f;
                float deform2 = (ruidoPerlin(x / 3, y / 3, semilla2) + 1.0f) / 2.0f;
                float ruido_total = (deform1 * 0.3f) + (deform2 * 0.7f);
                
                if (dist + ruido_total * 0.8f < 1.0f) {
                    int r = rand() % 100;
                    if (r < 5)
                        mapa->celdas[x][y] = COMIDA;
                    else if (r < 10)
                        mapa->celdas[x][y] = MADERA;
                    else if (r < 15)
                        mapa->celdas[x][y] = RECURSO;
                    else
                        mapa->celdas[x][y] = TIERRA;
                }
            }
        }
    }
    
    mapa->jugadorX = centros[0][0];
    mapa->jugadorY = centros[0][1];
    mapa->celdaBajoJugador = mapa->celdas[mapa->jugadorX][mapa->jugadorY];
    mapa->celdas[mapa->jugadorX][mapa->jugadorY] = JUGADOR;
}

void recolectarRecurso(struct Jugador *j, TipoCelda tipo) {
    if (tipo == COMIDA) j->Comida += 25;
    else if (tipo == MADERA) j->Madera += 25;
    else if (tipo == RECURSO) j->Oro += 25;
}

void moverJugador(Mapa *mapa, char dir, struct Jugador *jugador) {
    int nx = mapa->jugadorX, ny = mapa->jugadorY;
    
    if (dir == 'w' || dir == 'W') nx--;
    if (dir == 's' || dir == 'S') nx++;
    if (dir == 'a' || dir == 'A') ny--;
    if (dir == 'd' || dir == 'D') ny++;
    
    if (nx < 0 || nx >= FILAS || ny < 0 || ny >= COLUMNAS) return;
    
    if (mapa->celdas[nx][ny] != AGUA) {
        mapa->celdas[mapa->jugadorX][mapa->jugadorY] = mapa->celdaBajoJugador;
        mapa->celdaBajoJugador = mapa->celdas[nx][ny];
        recolectarRecurso(jugador, mapa->celdaBajoJugador);
        mapa->jugadorX = nx;
        mapa->jugadorY = ny;
        mapa->celdas[nx][ny] = JUGADOR;
    }
}

void setColor(int color) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, (WORD)color);
}

void dibujarTodo(Mapa *mapa, struct Jugador J1, struct Jugador J2, 
                 struct Jugador J3, struct Jugador J4, double tiempo) {
    
    
    // ========== JUGADORES 1 Y 2 (ARRIBA) ==========
    setColor(15); 
    printf("%-12s", J1.Nombre);
    printf("                                                            ");
    printf("%-12s\n", J2.Nombre);
    
    setColor(10); 
    printf("Comida: %-5d", J1.Comida);
    printf("                                                            ");
    printf("Comida: %-5d\n", J2.Comida);
    
    setColor(14); 
    printf("Oro:    %-5d", J1.Oro);
    printf("                                                            ");
    printf("Oro:    %-5d\n", J2.Oro);
    
    setColor(2); 
    printf("Madera: %-5d", J1.Madera);
    printf("                                                            ");
    printf("Madera: %-5d\n", J2.Madera);
    
    setColor(7); 
    printf("Piedra: %-5d", J1.Piedra);
    printf("                                                            ");
    printf("Piedra: %-5d\n", J2.Piedra);
    
    setColor(12); 
    printf("Tropas: %-3d  ", J1.NumeroTropas);
    printf("                                                            ");
    printf("Tropas: %-3d\n", J2.NumeroTropas);
    
    setColor(8); 
    printf(" E:%-2d A:%-2d ", J1.CantidadEspadas, J1.CantidadArqueros);
    printf("                                                            ");
    printf(" E:%-2d A:%-2d\n", J2.CantidadEspadas, J2.CantidadArqueros);
    
    printf(" P:%-2d C:%-2d ", J1.CantidadPicas, J1.CantidadCaballeria);
    printf("                                                            ");
    printf(" P:%-2d C:%-2d\n", J2.CantidadPicas, J2.CantidadCaballeria);
    
    printf("\n");
    
    // ========== MAPA ==========
    for (int i = 0; i < FILAS; i++) {
        for (int j = 0; j < COLUMNAS; j++) {
            char simbolo = ' ';
            int color = 7;
            
            switch (mapa->celdas[i][j]) {
                case AGUA: {
                    int fase = ((int)(tiempo * 3.0) + (i / 2) + j) % 3;
                    if (fase == 0) simbolo = '~';
                    else if (fase == 1) simbolo = '-';
                    else simbolo = '.';
                    color = 9;
                    break;
                }
                case TIERRA: simbolo = '#'; color = 6; break;
                case JUGADOR: simbolo = '@'; color = 15; break;
                case COMIDA: simbolo = 'f'; color = 10; break;
                case MADERA: simbolo = 'T'; color = 2; break;
                case RECURSO: simbolo = '$'; color = 14; break;
                default: simbolo = '?'; color = 12; break;
            }
            
            setColor(color);
            printf("%c", simbolo);
        }
        printf("\n");
    }
    
    // ========== JUGADORES 3 Y 4 (ABAJO) ==========
    printf("\n");
    
    setColor(15); 
    printf("%-12s", J3.Nombre);
    printf("                                                            ");
    printf("%-12s\n", J4.Nombre);
    
    setColor(10); 
    printf("Comida: %-5d", J3.Comida);
    printf("                                                            ");
    printf("Comida: %-5d\n", J4.Comida);
    
    setColor(14); 
    printf("Oro:    %-5d", J3.Oro);
    printf("                                                            ");
    printf("Oro:    %-5d\n", J4.Oro);
    
    setColor(2); 
    printf("Madera: %-5d", J3.Madera);
    printf("                                                            ");
    printf("Madera: %-5d\n", J4.Madera);
    
    setColor(7); 
    printf("Piedra: %-5d", J3.Piedra);
    printf("                                                            ");
    printf("Piedra: %-5d\n", J4.Piedra);
    
    setColor(12); 
    printf("Tropas: %-3d  ", J3.NumeroTropas);
    printf("                                                            ");
    printf("Tropas: %-3d\n", J4.NumeroTropas);
    
    setColor(8); 
    printf(" E:%-2d A:%-2d ", J3.CantidadEspadas, J3.CantidadArqueros);
    printf("                                                            ");
    printf(" E:%-2d A:%-2d\n", J4.CantidadEspadas, J4.CantidadArqueros);
    
    printf(" P:%-2d C:%-2d ", J3.CantidadPicas, J3.CantidadCaballeria);
    printf("                                                            ");
    printf(" P:%-2d C:%-2d\n", J4.CantidadPicas, J4.CantidadCaballeria);
    
    // Controles
    setColor(7);
    printf("\n                         WASD: Mover | Q: Salir\n");
}

int main() {
    struct Jugador J1, J2, J3, J4;
    IniciacionRecursos(&J1, "Jugador 1");
    IniciacionRecursos(&J2, "Jugador 2");
    IniciacionRecursos(&J3, "Jugador 3");
    IniciacionRecursos(&J4, "Jugador 4");
    
    Mapa mapa;
    inicializarMapa(&mapa);
    
    char tecla;
    double tiempo = 0.0;
    
    while(1) {
        system("cls");  // ✅ Como pediste
        dibujarTodo(&mapa, J1, J2, J3, J4, tiempo);
        
        if (_kbhit()) {
            tecla = _getch();
            if (tecla == 'q' || tecla == 'Q' || tecla == 27) break;
            moverJugador(&mapa, tecla, &J1);
        }
        
        tiempo += 0.05;
        Sleep(50);
    }
    
    system("cls");
    setColor(14);
    printf("\n\n     ===================================\n");
    printf("          GRACIAS POR JUGAR!\n");
    printf("     ===================================\n\n");
    setColor(7);
    
    return 0;
}