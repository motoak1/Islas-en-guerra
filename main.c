#include <windows.h>
#include <stdio.h>
#include <stdbool.h>
#include "mapa/menu.h"
#include "mapa/mapa.h"

// Variables para el control del mapa
// Se inicializa con zoom 2.5f para que la imagen se vea de cerca y cubra la pantalla 
Camara camara = {0, 0, 2.5f}; 
bool enJuego = false;
POINT mouseAnterior;
bool arrastrando = false;

void configurarPantallaCompleta() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD coord;
    // Forzar pantalla completa nativa o maximizar ventana [cite: 10]
    if (!SetConsoleDisplayMode(hConsole, CONSOLE_FULLSCREEN_MODE, &coord)) {
        ShowWindow(GetConsoleWindow(), SW_MAXIMIZE);
    }
}

int main() {
    configurarPantallaCompleta();
    
    HWND hwnd = GetConsoleWindow();
    HDC hdc = GetDC(hwnd);
    RECT rectPantalla;
    GetClientRect(hwnd, &rectPantalla);

    // Carga de la imagen BMP de 1024x1024 [cite: 11]
    cargarRecursosGraficos(); 

    bool ejecutando = true;
    while (ejecutando) {
        if (enJuego) {
    // 1. Detectar si el botón izquierdo está presionado
    if (GetAsyncKeyState(VK_LBUTTON) & 0x8000) {
        POINT mouseActual;
        GetCursorPos(&mouseActual); // Obtener posición real del mouse

        if (!arrastrando) {
            // Primer clic: guardar posición inicial
            mouseAnterior = mouseActual;
            arrastrando = true;
        } else {
            // Calcular desplazamiento (Delta)
            int dx = mouseActual.x - mouseAnterior.x;
            int dy = mouseActual.y - mouseAnterior.y;

            // Mover la cámara en sentido opuesto al mouse (Drag natural)
            // Ajustamos por el zoom para que el arrastre sea preciso
            camara.x -= (int)(dx / camara.zoom);
            camara.y -= (int)(dy / camara.zoom);

            mouseAnterior = mouseActual; // Actualizar para el siguiente frame
        }
    } else {
        arrastrando = false; // Soltó el clic
    }

    // 2. Validar límites para no salirse del mapa de 1024x1024 [cite: 120]
    if (camara.x < 0) camara.x = 0;
    if (camara.y < 0) camara.y = 0;
    if (camara.x > 1024 - (rectPantalla.right / camara.zoom)) 
        camara.x = 1024 - (rectPantalla.right / camara.zoom);
    if (camara.y > 1024 - (rectPantalla.bottom / camara.zoom))
        camara.y = 1024 - (rectPantalla.bottom / camara.zoom);

    // 3. Dibujar frame suave
    dibujarMundo(hdc, rectPantalla, camara);
} else {
            // Mostrar menú principal
            mostrarMenu();
            int accion = menuObtenerAccion();
            if (accion == 0) {
                // Nueva partida
                enJuego = true;
            } else if (accion == 1) {
                // Cargar partida (no implementado)
                ejecutando = false; // Salir por ahora
            } else {
                // Salir
                ejecutando = false;
            }
        }
        
        // Pequena pausa para no saturar el procesador
        Sleep(10); 
    }

    ReleaseDC(hwnd, hdc);
    return 0;
}