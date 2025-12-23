#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>
#include "mapa/mapa.h"
#include "mapa/menu.h"
#include <signal.h>
#include "recursos/recursos.h"


void configurarPantallaCompleta() {
    // Obtenemos el manejador de la salida estándar de la consola
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD coord;
    
    // Intentamos activar el modo de pantalla completa nativo de Windows
    // Nota: Esto funciona en el host de consola estándar de Windows
    if (!SetConsoleDisplayMode(hConsole, CONSOLE_FULLSCREEN_MODE, &coord)) {
        // Si falla el modo nativo (común en Windows 10/11 con la nueva Terminal), 
        // forzamos la maximización de la ventana mediante el manejo del HWND.
        HWND hwnd = GetConsoleWindow();
        ShowWindow(hwnd, SW_MAXIMIZE);
    }

    // Opcional: Ocultar el cursor para que no interfiera con los .bmp
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hConsole, &cursorInfo);
    cursorInfo.bVisible = FALSE; // Cambiar a TRUE si necesitas ver el cursor
    SetConsoleCursorInfo(hConsole, &cursorInfo);
}

int main() {

    // Ignorar Ctrl+C (Interrumpir)
    signal(SIGINT, SIG_IGN); 
    
    // Ignorar Ctrl+Break (Comun en Windows)
    #if defined(SIGBREAK)
        signal(SIGBREAK, SIG_IGN);
    #endif

    // Ignorar Ctrl+Z (Suspender - POSIX).
    #if defined(SIGTSTP)
        signal(SIGTSTP, SIG_IGN); 
    #endif
    
    // Ignorar Ctrl+\ (Salir/Quitar - POSIX)
    #if defined(SIGQUIT)
        signal(SIGQUIT, SIG_IGN);
    #endif

    configurarPantallaCompleta();

    char mapa[MAPA_F][MAPA_C];
    int px = 0, py = 0;
    int tecla;
    DWORD tInicio, tActual;
    struct Jugador jugador1;
    
    mostrarMenu();
    
    int accion = menuObtenerAccion();
    if (accion == 1) {
        // Cargar partida: mostrar selector
        if (!seleccionarYcargarPartida(&jugador1, mapa, &px, &py)) {
            // Fallback: iniciar nueva partida si cancela o falla la carga
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
        }
    } else {
        // Nueva partida
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
    }
    
    mostrarMapa(mapa, px, py);
    dibujarPanelEnMapa(jugador1); // Dibujar panel inicial
    
    moverCursor(0, FILAS + 2);
    printf("Usa WASD para moverte. ENTER para interactuar. ESC para opciones.\n");
    
    tInicio = GetTickCount();
    
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

            if (tecla == 27) {
                int salir = mostrarMenuOpciones(&jugador1, mapa, px, py);
                if (salir) break; // Terminar juego si elige 'Salir'
                continue; // Volver al loop del juego
            }

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
