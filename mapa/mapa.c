#include <stdlib.h>
#include <time.h>
#include "mapa.h"
#include "raylib.h" // ¡Necesario para GetTime() y dibujar!
#include <math.h>     // Para la animación del agua

// Tu función de ruido (sin cambios)
static float ruidoPerlin(int x, int y, int semilla) {
    int n = x + y * 57 + semilla * 131;
    n = (n << 13) ^ n;
    return (1.0f - ((n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0f);
}

void inicializarMapa(Mapa *mapa) {
    srand(time(NULL));

    for (int i = 0; i < FILAS; i++)
        for (int j = 0; j < COLUMNAS; j++)
            mapa->celdas[i][j] = AGUA;

    int centros[4][2] = {{10, 20}, {10, 60}, {30, 20}, {30, 60}}; // Ajustados a FILAS/COLUMNAS

    for (int n = 0; n < 4; n++) {
        int cx = centros[n][0], cy = centros[n][1];
        int radio = 8 + rand() % 5;
        int semilla1 = rand() % 1000;
        int semilla2 = rand() % 1000; // Semilla para la segunda capa de ruido

        for (int i = -radio * 2; i <= radio * 2; i++) {
            for (int j = -radio * 2; j <= radio * 2; j++) {
                int x = cx + i;
                int y = cy + j;
                if (x < 0 || x >= FILAS || y < 0 || y >= COLUMNAS)
                    continue;

                float dist = (i * i + j * j) / (float)(radio * radio);

                // *** MEJORA DE ISLA ***
                // Combinamos dos "octavas" de ruido para más detalle
                // Ruido de alta frecuencia (pequeños detalles)
                float deform_alta_freq = (ruidoPerlin(x, y, semilla1) + 1.0f) / 2.0f; 
                // Ruido de baja frecuencia (grandes formas)
                float deform_baja_freq = (ruidoPerlin(x / 3, y / 3, semilla2) + 1.0f) / 2.0f; 

                // Combinamos el ruido, dando más peso al de baja frecuencia
                float ruido_total = (deform_alta_freq * 0.3f) + (deform_baja_freq * 0.7f);

                // Aplicamos el ruido combinado. Aumentamos su influencia (de 0.3f a 0.8f)
                if (dist + ruido_total * 0.8f < 1.0f) {
                    int r = rand() % 100;
                    if (r < 5)
                        mapa->celdas[x][y] = COMIDA;
                    else if (r < 10)
                        mapa->celdas[x][y] = MADERA;
                    else if (r < 15)
                        mapa->celdas[x][y] = RECURSO;
                    else
                        mapa->celdas[x][y] = TIERRA;
                }
            }
        }
    }

    mapa->jugadorX = centros[0][0];
    mapa->jugadorY = centros[0][1];
    // Guardamos lo que hay debajo del jugador antes de ponerlo
    mapa->celdaBajoJugador = mapa->celdas[mapa->jugadorX][mapa->jugadorY];
    mapa->celdas[mapa->jugadorX][mapa->jugadorY] = JUGADOR;
}

// *** FUNCIÓN DE MOVER CORREGIDA ***
void moverJugador(Mapa *mapa, char dir) {
    int nx = mapa->jugadorX, ny = mapa->jugadorY;
    if (dir == 'w') nx--;
    if (dir == 's') nx++;
    if (dir == 'a') ny--;
    if (dir == 'd') ny++;

    if (nx < 0 || nx >= FILAS || ny < 0 || ny >= COLUMNAS)
        return;

    // Solo se mueve si no es agua
    if (mapa->celdas[nx][ny] != AGUA) {
        // 1. Deja en la celda anterior lo que había guardado
        mapa->celdas[mapa->jugadorX][mapa->jugadorY] = mapa->celdaBajoJugador;
        
        // 2. Guarda lo que hay en la nueva celda
        mapa->celdaBajoJugador = mapa->celdas[nx][ny];
        
        // 3. Se mueve a la nueva celda
        mapa->jugadorX = nx;
        mapa->jugadorY = ny;
        mapa->celdas[nx][ny] = JUGADOR;
    }
}

// *** NUEVA FUNCIÓN DE DIBUJADO ***
void dibujarMapa(Mapa *mapa) {
    // GetTime() nos da el tiempo para las animaciones
    double tiempo = GetTime();

    for (int i = 0; i < FILAS; i++) {
        for (int j = 0; j < COLUMNAS; j++) {
            const char* simbolo = "?";
            Color color = MAGENTA; // Color de error

            // Posición en píxeles
            int posX = j * TAM_CELDA;
            int posY = i * TAM_CELDA;

            TipoCelda celda = mapa->celdas[i][j];

            // Si es tierra, dibujamos un fondo para que los recursos resalten
            if (celda == TIERRA || celda == COMIDA || celda == MADERA || celda == RECURSO) {
                DrawRectangle(posX, posY, TAM_CELDA, TAM_CELDA, (Color){ 139, 69, 19, 255 }); // Marrón
            }

            switch (celda) {
                case AGUA:
                    color = (Color){ 0, 121, 241, 255 }; // Azul
                    
                    // *** ANIMACIÓN DE AGUA ***
                    // (i + j) escalona la animación
                    // (tiempo * 3.0) controla la velocidad
                    // % 3 da 3 fases de animación: '~', '-', '.'
                    int fase = ((int)(tiempo * 3.0) + (i / 2) + j) % 3;
                    
                    if (fase == 0) simbolo = "~";
                    else if (fase == 1) simbolo = "-";
                    else simbolo = "."; // Trough of the wave
                    break;

                case TIERRA:
                    simbolo = " "; // La tierra es solo el fondo marrón
                    break;
                
                case JUGADOR:
                    simbolo = "@";
                    color = WHITE;
                    break;

                case COMIDA:
                    simbolo = "f"; // 'f' de food
                    color = (Color){ 124, 252, 0, 255 }; // Verde lima
                    break;

                case MADERA:
                    simbolo = "T"; // 'T' de tree
                    color = (Color){ 34, 139, 34, 255 }; // Verde bosque
                    break;

                case RECURSO:
                    simbolo = "$"; // Símbolo de "tesoro"
                    color = (Color){ 255, 215, 0, 255 }; // Dorado
                    break;
            }

            // Usamos DrawText en lugar de DrawRectangle
            // Centramos el símbolo en la celda
            int anchoTexto = MeasureText(simbolo, TAM_CELDA);
            DrawText(simbolo, posX + (TAM_CELDA - anchoTexto) / 2, posY, TAM_CELDA, color);
        }
    }
}