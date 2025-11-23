#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>
#include "mapa.h"
#include "menu.h"
#include "../recursos/recursos.h"

int main() {
    char mapa[MAPA_F][MAPA_C];
    int px = 0, py = 0;
    int tecla;
    DWORD tInicio, tActual;
    struct Jugador jugador1;
    
    mostrarMenu();
    
    IniciacionRecursos(&jugador1, "Jugador 1");
    inicializarMapa(mapa);
    srand((unsigned int)time(NULL));
    
    do {
        px = rand() % MAPA_F;
        py = rand() % MAPA_C;
    } while (mapa[px][py] != '.');
    
    offset_f = px - FILAS / 2;
    offset_c = py - COLUMNAS / 2;
    
    if (offset_f < 0) offset_f = 0;
    if (offset_f > MAPA_F - FILAS) offset_f = MAPA_F - FILAS;
    if (offset_c < 0) offset_c = 0;
    if (offset_c > MAPA_C - COLUMNAS) offset_c = MAPA_C - COLUMNAS;
    
    mostrarMapa(mapa, px, py);
    dibujarPanelEnMapa(jugador1); // Dibujar panel inicial
    
    moverCursor(0, FILAS + 2);
    printf("Usa WASD para moverte. ENTER para interactuar. ESC para salir.\n");
    
    tInicio = GetTickCount();
    
    // Bucle principal del juego
    // Bucle principal del juego
// Bucle principal del juego
// Bucle principal del juego
while (1) {
    // Animar el agua cada 300 milisegundos
    tActual = GetTickCount();
    if (tActual - tInicio >= 300) {
        animarAgua(mapa);
        // Redibujar panel después de animar agua (siempre forzado)
        forzarRedibujoPanelEnMapa(jugador1);
        tInicio = tActual;
    }
    
    // Detectar entrada del teclado
    if (_kbhit()) {
        tecla = _getch();
        
        if (tecla == 27) break;
        
        if (tecla == 'w' || tecla == 'W' || tecla == 'a' || tecla == 'A' ||
            tecla == 's' || tecla == 'S' || tecla == 'd' || tecla == 'D' ||
            tecla == 13) {
            
            // Ejecutar accion
            explorarMapa(&jugador1, mapa, &px, &py, (char)tecla);
            
            // SIEMPRE redibujar panel después de cualquier acción
            forzarRedibujoPanelEnMapa(jugador1);
        }
    }
}

    
    moverCursor(0, FILAS + 3);
    setColor(0, 15);
    printf("Gracias por jugar ISLAS EN GUERRA\n");
    return 0;
}
