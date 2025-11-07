#include "mapa.h"
#include "menu.h"
#include <stdlib.h>
#include <time.h>
#include <conio.h>
#include <stdbool.h>

#define NUM_RECURSOS 15
#define NUM_ENEMIGOS 7

// Variables globales para el offset de la vista (scrolling)
int offset_f = 0;
int offset_c = 0;

/* =============================== */
/* Utilidades de consola           */
/* =============================== */


/* =============================== */
/* Generador de islas fijas        */
/* =============================== */
void crearIsla(char mapa[MAPA_F][MAPA_C], int cx, int cy, int rx, int ry) {
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
                if (nx >= 0 && nx < MAPA_F && ny >= 0 && ny < MAPA_C)
                    mapa[nx][ny] = '.';
            }
        }
    }
}

/* =============================== */
/* Crear puente entre dos puntos  */
/* =============================== */
void crearPuente(char mapa[MAPA_F][MAPA_C], int x1, int y1, int x2, int y2) {
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = x1 < x2 ? 1 : -1;
    int sy = y1 < y2 ? 1 : -1;
    int err = dx - dy;

    while (true) {
        if (x1 >= 0 && x1 < MAPA_F && y1 >= 0 && y1 < MAPA_C)
            mapa[x1][y1] = '.';
        if (x1 == x2 && y1 == y2) break;
        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x1 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y1 += sy;
        }
    }
}

/* =============================== */
/* Inicializar mapa                */
/* =============================== */
void inicializarMapa(char mapa[MAPA_F][MAPA_C]) {
    int i, j, placed, rx, ry, ex, ey;

    srand((unsigned int)time(NULL));

    for (i = 0; i < MAPA_F; i++) {
        for (j = 0; j < MAPA_C; j++) {
            mapa[i][j] = '~';
        }
    }

    // Crear 5 islas: 4 en esquinas y 1 central ajustada para no cortarse
    crearIsla(mapa, 12, 12, 10, 8);  // Esquina superior izquierda
    crearIsla(mapa, 85, 12, 10, 8);  // Esquina superior derecha
    crearIsla(mapa, 12, 85, 10, 8);  // Esquina inferior izquierda
    crearIsla(mapa, 85, 85, 10, 8);  // Esquina inferior derecha
    crearIsla(mapa, 40, 50, 10, 8);  // Isla central ajustada (no se corta)

    // Crear puentes ortogonales para conectar las islas (sin diagonales)
    crearPuente(mapa, 12, 12, 85, 12); // Puente horizontal superior (fila 12)
    crearPuente(mapa, 12, 12, 12, 85); // Puente vertical izquierdo (columna 12)
    crearPuente(mapa, 85, 12, 85, 85); // Puente vertical derecho (columna 85)
    crearPuente(mapa, 12, 85, 85, 85); // Puente horizontal inferior (fila 85)
    crearPuente(mapa, 40, 50, 85, 50); // Puente horizontal central a derecha (fila 50)
    crearPuente(mapa, 40, 50, 12, 50); // Puente horizontal central a izquierda (fila 50)
    crearPuente(mapa, 40, 50, 40, 12); // Puente vertical central a arriba (columna 50)
    crearPuente(mapa, 40, 50, 40, 85); // Puente vertical central a abajo (columna 50)

    placed = 0;
    while (placed < NUM_RECURSOS) {
        rx = rand() % MAPA_F;
        ry = rand() % MAPA_C;
        if (mapa[rx][ry] == '.') {
            mapa[rx][ry] = '$';
            placed++;
        }
    }

    placed = 0;
    while (placed < NUM_ENEMIGOS) {
        ex = rand() % MAPA_F;
        ey = rand() % MAPA_C;
        if (mapa[ex][ey] == '.') {
            mapa[ex][ey] = 'E';
            placed++;
        }
    }
}

/* =============================== */
/* Dibujar marco del mapa          */
/* =============================== */
void dibujarMarcoMapa() {
    int i;
    setColor(0, 8); // Color gris oscuro para el marco

    // Esquinas y líneas horizontales
    moverCursor(0, 0);
    printf("%c", 201); // ╔
    for (i = 1; i < COLUMNAS * 2 + 2; i++) { // Ajustado el ancho
        printf("%c", 205); // ═
    }
    printf("%c", 187); // ╗

    moverCursor(0, FILAS + 1);
    printf("%c", 200); // ╚
    for (i = 1; i < COLUMNAS * 2 + 2; i++) { // Ajustado el ancho
        printf("%c", 205); // ═
    }
    printf("%c", 188); // ╝

    // Líneas verticales
    for (i = 1; i < FILAS + 1; i++) {
        moverCursor(0, i);
        printf("%c", 186); // ║
        moverCursor(COLUMNAS * 2 + 2, i); // Ajustada la posición
        printf("%c", 186); // ║
    }
    setColor(0, 15); // Restaurar color por defecto
}


/* =============================== */
/* Mostrar mapa                    */
/* =============================== */
void mostrarMapa(char mapa[MAPA_F][MAPA_C]) {
    int i, j;
    ocultarCursor();
    // system("cls"); // Eliminado para evitar parpadeo
    dibujarMarcoMapa();
    moverCursor(2, 1); // Posicionar cursor dentro del marco

    for (i = 0; i < FILAS; i++) {
        moverCursor(2, i + 1);
        for (j = 0; j < COLUMNAS; j++) {
            char c = mapa[offset_f + i][offset_c + j];
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
    }
    setColor(0, 15);
}

/* =============================== */
/* Mover jugador                   */
/* =============================== */
void moverJugador(char mapa[MAPA_F][MAPA_C], int *x, int *y, char direccion) {
    int nx = *x;
    int ny = *y;
    char destino;
    char msg[60];
    int i;
    char actual;
    int offset_changed = 0;

    if (direccion >= 'a' && direccion <= 'z')
        direccion -= 32;

    if (direccion == 'W') nx--;
    else if (direccion == 'S') nx++;
    else if (direccion == 'A') ny--;
    else if (direccion == 'D') ny++;
    else return;

    if (nx < 0 || nx >= MAPA_F || ny < 0 || ny >= MAPA_C) {
        moverCursor(0, FILAS + 2);
        setColor(0, 14);
        printf("No puedes salir del mapa!          ");
        setColor(0, 15);
        return;
    }

    destino = mapa[nx][ny];
    msg[0] = '\0';

    if (destino == '~') {
        moverCursor(0, FILAS + 2);
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

    // Calcular nuevo offset para centrar la vista en el jugador
    int new_offset_f = offset_f;
    int new_offset_c = offset_c;
    new_offset_f = nx - FILAS / 2;
    new_offset_c = ny - COLUMNAS / 2;
    if (new_offset_f < 0) new_offset_f = 0;
    if (new_offset_f > MAPA_F - FILAS) new_offset_f = MAPA_F - FILAS;
    if (new_offset_c < 0) new_offset_c = 0;
    if (new_offset_c > MAPA_C - COLUMNAS) new_offset_c = MAPA_C - COLUMNAS;

    if (new_offset_f != offset_f || new_offset_c != offset_c) {
        offset_f = new_offset_f;
        offset_c = new_offset_c;
        offset_changed = 1;
    }
    
    // Solo redibujamos la celda anterior si NO hubo cambio de offset.
    // Si hubo cambio de offset, la llamada a mostrarMapa(mapa) se encargará de esto.
    if (!offset_changed) {
        // Redibujar la celda anterior del jugador
        moverCursor((short)((*y - offset_c) * 2 + 2), (short)(*x - offset_f + 1));
        actual = mapa[*x][*y];
        if (actual == '~') setColor(1, 9);
        else if (actual == '.') setColor(2, 6);
        else if (actual == '$') setColor(2, 14);
        else if (actual == 'E') setColor(4, 12);
        else setColor(0, 15);
        printf("%c ", actual);
    }


    *x = nx;
    *y = ny;

    // Si el offset cambió, redibujar todo el mapa
    if (offset_changed) {
        mostrarMapa(mapa);
    }

    // Dibujar el jugador en la nueva posición (siempre después de cualquier redibujado)
    moverCursor((short)((ny - offset_c) * 2 + 2), (short)(nx - offset_f + 1));
    setColor(0, 10);
    printf("P ");
    setColor(0, 15);

    moverCursor(0, FILAS + 2);
    for (i = 0; i < 60; i++) printf(" ");
    moverCursor(0, FILAS + 2);

    if (msg[0] != '\0') {
        setColor(0, 11);
        printf("%s", msg);
        setColor(0, 15);
    }
}

/* =============================== */
/* Animar agua con flujo (~ y ' ') */
/* =============================== */
void animarAgua(char mapa[MAPA_F][MAPA_C]) {
    int i, j;
    static int frame = 0;
    frame++;

    for (i = 0; i < FILAS; i++) {
        for (j = 0; j < COLUMNAS; j++) {
            if (mapa[offset_f + i][offset_c + j] == '~') {
                moverCursor((j * 2) + 2, i + 1);
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


