#include "mapa.h"
#include <stdlib.h>
#include <time.h>
#include <conio.h>

#define NUM_ISLAS 4
#define NUM_RECURSOS 15
#define NUM_ENEMIGOS 8

/* ============================= */
/*  Oculta el cursor de consola  */
/* ============================= */
void ocultarCursor() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hOut, &cursorInfo);
    cursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(hOut, &cursorInfo);
}

/* ============================= */
/*  Mueve el cursor a (x, y)     */
/* ============================= */
void moverCursor(short x, short y) {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD pos;
    pos.X = x;
    pos.Y = y;
    SetConsoleCursorPosition(hOut, pos);
}

/* ============================= */
/*  Cambia color de texto/fondo  */
/* ============================= */
void setColor(int colorFondo, int colorTexto) {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hOut, colorFondo * 16 + colorTexto);
}

/* ============================= */
/*  Inicializa el mapa base      */
/* ============================= */
void inicializarMapa(char mapa[SIZE][SIZE]) {
    int i, j;
    int cx, cy, w, h, ix, iy;
    int placed, rx, ry, ex, ey, patch;

    srand((unsigned int)time(NULL));

    /* Llenar todo con agua */
    for (i = 0; i < SIZE; i++) {
        for (j = 0; j < SIZE; j++) {
            mapa[i][j] = '~';
        }
    }

    /* Crear islas: regiones de '.' con distinto tamaño y posición */
    for (patch = 0; patch < NUM_ISLAS; patch++) {
        cx = rand() % SIZE;
        cy = rand() % SIZE;
        w = 3 + rand() % 5;
        h = 2 + rand() % 4;

        for (ix = cx; ix < cx + h && ix < SIZE; ix++) {
            for (iy = cy; iy < cy + w && iy < SIZE; iy++) {
                if (ix >= 0 && iy >= 0)
                    mapa[ix][iy] = '.';
            }
        }
    }

    /* Colocar recursos ($) */
    placed = 0;
    while (placed < NUM_RECURSOS) {
        rx = rand() % SIZE;
        ry = rand() % SIZE;
        if (mapa[rx][ry] == '.') {
            mapa[rx][ry] = '$';
            placed++;
        }
    }

    /* Colocar enemigos (E) */
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

/* ============================= */
/*  Mostrar mapa en colores      */
/* ============================= */
void mostrarMapa(char mapa[SIZE][SIZE]) {
    int i, j;
    ocultarCursor();
    moverCursor(0, 0);

    for (i = 0; i < SIZE; i++) {
        for (j = 0; j < SIZE; j++) {
            char c = mapa[i][j];
            if (c == '~') {
                setColor(1, 9);    /* fondo azul, texto azul claro */
                printf("~ ");
            } else if (c == '.') {
                setColor(2, 6);    /* fondo verde, texto amarillo claro */
                printf(". ");
            } else if (c == '$') {
                setColor(2, 14);   /* fondo verde, texto amarillo brillante */
                printf("$ ");
            } else if (c == 'E') {
                setColor(4, 12);   /* fondo rojo oscuro, texto rojo claro */
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

/* ============================= */
/*  Mover al jugador (WASD)      */
/* ============================= */
void moverJugador(char mapa[SIZE][SIZE], int *x, int *y, char direccion) {
    int nx = *x;
    int ny = *y;
    char destino;
    char msg[60];
    int i;

    if (direccion >= 'a' && direccion <= 'z')
        direccion -= 32;

    if (direccion == 'W') nx--;
    else if (direccion == 'S') nx++;
    else if (direccion == 'A') ny--;
    else if (direccion == 'D') ny++;
    else return;

    /* Validar límites */
    if (nx < 0 || nx >= SIZE || ny < 0 || ny >= SIZE) {
        moverCursor(0, SIZE + 1);
        setColor(0, 14);
        printf("No puedes salir del mapa!        ");
        setColor(0, 15);
        return;
    }

    destino = mapa[nx][ny];
    msg[0] = '\0';

    if (destino == '$') {
        sprintf(msg, "Has encontrado un recurso!");
        mapa[nx][ny] = '.';
    } else if (destino == 'E') {
        sprintf(msg, "Has encontrado un enemigo!");
        mapa[nx][ny] = '.';
    }

    /* Redibujar celda anterior */
    moverCursor((short)(*y * 2), (short)(*x));
    char actual = mapa[*x][*y];
    if (actual == '~') setColor(1, 9);
    else if (actual == '.') setColor(2, 6);
    else if (actual == '$') setColor(2, 14);
    else if (actual == 'E') setColor(4, 12);
    else setColor(0, 15);
    printf("%c ", actual);

    /* Dibujar jugador */
    moverCursor((short)(ny * 2), (short)(nx));
    setColor(0, 10); /* texto verde brillante */
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
