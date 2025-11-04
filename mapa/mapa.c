#include "mapa.h"
#include <stdlib.h>
#include <time.h>
#include <conio.h>

#define NUM_RECURSOS 15
#define NUM_ENEMIGOS 7

/* =============================== */
/* Utilidades de consola           */
/* =============================== */
void ocultarCursor() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hOut, &cursorInfo);
    cursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(hOut, &cursorInfo);
}

void moverCursor(short x, short y) {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD pos;
    pos.X = x;
    pos.Y = y;
    SetConsoleCursorPosition(hOut, pos);
}

void setColor(int fondo, int texto) {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hOut, fondo * 16 + texto);
}

/* =============================== */
/* Generador de islas fijas        */
/* =============================== */
void crearIsla(char mapa[SIZE][SIZE], int cx, int cy, int rx, int ry) {
    int i, j;
    int dx, dy, dist, deformacion, nx, ny;
    int maxX = rx + 2;
    int maxY = ry + 2;

    for (i = -maxY; i <= maxY; i++) {
        for (j = -maxX; j <= maxX; j++) {
            dx = j;
            dy = i;
            dist = dx * dx + dy * dy;
            deformacion = rand() % 5;
            if (dist <= (rx * ry) + deformacion) {
                nx = cy + i;
                ny = cx + j;
                if (nx >= 0 && nx < SIZE && ny >= 0 && ny < SIZE)
                    mapa[nx][ny] = '.';
            }
        }
    }
}

/* =============================== */
/* Inicializar mapa                */
/* =============================== */
void inicializarMapa(char mapa[SIZE][SIZE]) {
    int i, j, placed, rx, ry, ex, ey;

    srand((unsigned int)time(NULL));

    for (i = 0; i < SIZE; i++) {
        for (j = 0; j < SIZE; j++) {
            mapa[i][j] = '~';
        }
    }

    crearIsla(mapa, 7, 7, 6, 5);
    crearIsla(mapa, 22, 7, 7, 5);
    crearIsla(mapa, 7, 22, 7, 5);
    crearIsla(mapa, 22, 22, 6, 5);

    placed = 0;
    while (placed < NUM_RECURSOS) {
        rx = rand() % SIZE;
        ry = rand() % SIZE;
        if (mapa[rx][ry] == '.') {
            mapa[rx][ry] = '$';
            placed++;
        }
    }

    placed = 0;
    while (placed < NUM_ENEMIGOS) {
        ex = rand() % SIZE;
        ey = rand() % SIZE;
        if (mapa[ex][ey] == '.') {
            mapa[ex][ey] = 'E';
            placed++;
        }
    }
}

/* =============================== */
/* Mostrar mapa                    */
/* =============================== */
void mostrarMapa(char mapa[SIZE][SIZE]) {
    int i, j;
    ocultarCursor();
    moverCursor(0, 0);

    for (i = 0; i < SIZE; i++) {
        for (j = 0; j < SIZE; j++) {
            char c = mapa[i][j];
            if (c == '~') {
                setColor(1, 9);
                printf("~ ");
            } else if (c == '.') {
                setColor(2, 6);
                printf(". ");
            } else if (c == '$') {
                setColor(2, 14);
                printf("$ ");
            } else if (c == 'E') {
                setColor(4, 12);
                printf("E ");
            } else {
                setColor(0, 15);
                printf("%c ", c);
            }
        }
        printf("\n");
    }
    setColor(0, 15);
}

/* =============================== */
/* Mover jugador                   */
/* =============================== */
void moverJugador(char mapa[SIZE][SIZE], int *x, int *y, char direccion) {
    int nx = *x;
    int ny = *y;
    char destino;
    char msg[60];
    int i;
    char actual;

    if (direccion >= 'a' && direccion <= 'z')
        direccion -= 32;

    if (direccion == 'W') nx--;
    else if (direccion == 'S') nx++;
    else if (direccion == 'A') ny--;
    else if (direccion == 'D') ny++;
    else return;

    if (nx < 0 || nx >= SIZE || ny < 0 || ny >= SIZE) {
        moverCursor(0, SIZE + 1);
        setColor(0, 14);
        printf("No puedes salir del mapa!          ");
        setColor(0, 15);
        return;
    }

    destino = mapa[nx][ny];
    msg[0] = '\0';

    if (destino == '~') {
        moverCursor(0, SIZE + 1);
        setColor(0, 14);
        printf("No puedes nadar!                   ");
        setColor(0, 15);
        return;
    }

    if (destino == '$') {
        sprintf(msg, "Has encontrado un recurso!");
        mapa[nx][ny] = '.';
    } else if (destino == 'E') {
        sprintf(msg, "Has encontrado un enemigo!");
        mapa[nx][ny] = '.';
    }

    moverCursor((short)(*y * 2), (short)(*x));
    actual = mapa[*x][*y];
    if (actual == '~') setColor(1, 9);
    else if (actual == '.') setColor(2, 6);
    else if (actual == '$') setColor(2, 14);
    else if (actual == 'E') setColor(4, 12);
    else setColor(0, 15);
    printf("%c ", actual);

    moverCursor((short)(ny * 2), (short)(nx));
    setColor(0, 10);
    printf("P ");
    setColor(0, 15);

    *x = nx;
    *y = ny;

    moverCursor(0, SIZE + 1);
    for (i = 0; i < 60; i++) printf(" ");
    moverCursor(0, SIZE + 1);

    if (msg[0] != '\0') {
        setColor(0, 11);
        printf("%s", msg);
        setColor(0, 15);
    }
}

/* =============================== */
/* Animar agua con flujo (~ y ' ') */
/* =============================== */
void animarAgua(char mapa[SIZE][SIZE]) {
    int i, j;
    static int frame = 0;
    frame++;

    for (i = 0; i < SIZE; i++) {
        for (j = 0; j < SIZE; j++) {
            if (mapa[i][j] == '~') {
                moverCursor(j * 2, i);
                if ((i + j + frame) % 3 == 0) {
                    setColor(1, 9); printf("~ ");
                } else {
                    setColor(1, 9); printf("  ");
                }
            }
        }
    }
    setColor(0, 15);
}

/* =============================== */
/* Menú inicial                    */
/* =============================== */
void mostrarMenu() {
    int i;
    ocultarCursor();
    system("cls");

    printf("\n\n\n\n");
    printf("                    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
    printf("                      ISLAS  EN  GUERRA  \n");
    printf("                    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n\n");

    setColor(0, 14);
    printf("                 Presiona [ENTER] para comenzar\n\n");
    setColor(0, 8);
    printf("                  Desarrollado en C - 100%% CMD Edition\n\n");

    

    setColor(0, 15);
    

    while (1) {
        if (_kbhit() && _getch() == 13) break;
    }

    system("cls");
}

    