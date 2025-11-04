#include <stdio.h>
#include <conio.h>
#include "mapa.h"

int main() {
    char mapa[SIZE][SIZE];
    int px = SIZE / 2;
    int py = SIZE / 2;
    int i, j;
    int encontrado = 0;
    int tecla;

    inicializarMapa(mapa);

    /* Buscar terreno disponible si el centro está ocupado */
    if (mapa[px][py] != '.') {
        for (i = 0; i < SIZE && !encontrado; i++) {
            for (j = 0; j < SIZE; j++) {
                if (mapa[i][j] == '.') {
                    px = i;
                    py = j;
                    encontrado = 1;
                    break;
                }
            }
        }
    }

    mostrarMapa(mapa);

    /* Dibujar jugador inicial */
    moverCursor(py * 2, px);
    setColor(0, 10);
    printf("P ");
    setColor(0, 15);

    moverCursor(0, SIZE + 2);
    printf("Usa W, A, S, D para moverte. Presiona ESC para salir.\n");

    while (1) {
        tecla = _getch();

        if (tecla == 27) { /* ESC */
            moverCursor(0, SIZE + 3);
            setColor(0, 15);
            printf("Gracias por jugar Islas en Guerra.\n");
            break;
        }

        if (tecla == 'w' || tecla == 'W' || tecla == 'a' || tecla == 'A' ||
            tecla == 's' || tecla == 'S' || tecla == 'd' || tecla == 'D') {
            moverJugador(mapa, &px, &py, (char)tecla);
        }
    }

    return 0;
}
