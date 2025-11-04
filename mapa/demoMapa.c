#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>
#include "mapa.h"

int main() {
    char mapa[SIZE][SIZE];
    int px = 0, py = 0;
    int tecla;
    DWORD tInicio, tActual;

    mostrarMenu();  /* Menú de inicio con animación */

    inicializarMapa(mapa);
    srand((unsigned int)time(NULL));

    // Busca una posición inicial válida para el jugador en cualquier isla.
    do {
        px = rand() % SIZE;
        py = rand() % SIZE;
    } while (mapa[px][py] != '.');

    mostrarMapa(mapa);

    moverCursor(py * 2, px);
    setColor(0, 10);
    printf("P ");
    setColor(0, 15);

    moverCursor(0, SIZE + 2);
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

    moverCursor(0, SIZE + 3);
    setColor(0, 15);
    printf("Gracias por jugar ISLAS EN GUERRA ⚔️\n");
    return 0;
}
