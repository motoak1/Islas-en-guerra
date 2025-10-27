#include "raylib.h"
#include "mapa.h"

// Definimos el tamaño de la ventana basado en el mapa
const int screenWidth = COLUMNAS * TAM_CELDA;
const int screenHeight = FILAS * TAM_CELDA;

int main(void) {
    InitWindow(screenWidth, screenHeight, "Islas en guerra - Demo Mapa");

    Mapa miMapa;
    inicializarMapa(&miMapa);

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        // --- 1. Actualización (Update) ---
        if (IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP))
            moverJugador(&miMapa, 'w');
        if (IsKeyPressed(KEY_S) || IsKeyPressed(KEY_DOWN))
            moverJugador(&miMapa, 's');
        if (IsKeyPressed(KEY_A) || IsKeyPressed(KEY_LEFT))
            moverJugador(&miMapa, 'a');
        if (IsKeyPressed(KEY_D) || IsKeyPressed(KEY_RIGHT))
            moverJugador(&miMapa, 'd');

        // --- 2. Dibujado (Draw) ---
        BeginDrawing();

        ClearBackground(BLACK); // Fondo negro
        
        // ¡Llamamos a nuestra nueva función de dibujado!
        dibujarMapa(&miMapa);

        // Opcional: Dibujar FPS
        DrawFPS(10, 10);

        EndDrawing();
    }

    CloseWindow();

    return 0;
}