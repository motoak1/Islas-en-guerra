#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>
#include "mapa.h"

int main() {
    char mapa[MAPA_F][MAPA_C];
    int px = 0, py = 0;
    int tecla;
    DWORD tInicio, tActual;

    mostrarMenu();  /* Menú de inicio con animación */

    inicializarMapa(mapa);
    srand((unsigned int)time(NULL));

    // Busca una posición inicial válida para el jugador en cualquier isla.
    do {
        px = rand() % MAPA_F;
        py = rand() % MAPA_C;
    } while (mapa[px][py] != '.');

    // Inicializar offset centrado en el jugador
    offset_f = px - FILAS / 2;
    offset_c = py - COLUMNAS / 2;
    if (offset_f < 0) offset_f = 0;
    if (offset_f > MAPA_F - FILAS) offset_f = MAPA_F - FILAS;
    if (offset_c < 0) offset_c = 0;
    if (offset_c > MAPA_C - COLUMNAS) offset_c = MAPA_C - COLUMNAS;

    mostrarMapa(mapa);

    moverCursor((py - offset_c) * 2, px - offset_f);
    setColor(0, 10);
    printf("P ");
    setColor(0, 15);

    moverCursor(0, FILAS + 2);
    printf("Usa W, A, S, D para moverte. ESC para salir.\n");

    tInicio = GetTickCount();

    while (1) {
        /* Animar el agua cada 300ms */
        tActual = GetTickCount();
        if (tActual - tInicio >= 300) {
            animarAgua(mapa);
            tInicio = tActual;
        }

        if (_kbhit()) {
            tecla = _getch();
            if (tecla == 27) break;
            if (tecla == 'w' || tecla == 'W' || tecla == 'a' || tecla == 'A' ||
                tecla == 's' || tecla == 'S' || tecla == 'd' || tecla == 'D') {
                moverJugador(mapa, &px, &py, (char)tecla);
            }
        }
    }

    moverCursor(0, FILAS + 3);
    setColor(0, 15);
    printf("Gracias por jugar ISLAS EN GUERRA ⚔️\n");
    return 0;
}
