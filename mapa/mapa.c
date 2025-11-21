#include "mapa.h"
#include "menu.h"
#include <stdlib.h>
#include <time.h>
#include <conio.h>
#include "../recursos/recursos.h" // Sube al directorio padre y entra a recursos/ 
#include <stdbool.h>

#define NUM_RECURSOS 15
#define NUM_ENEMIGOS 5
#define NUM_COMIDA 10
#define NUM_MADERA 10
#define NUM_PIEDRA 10

// Nuevos Esquemas de Color (Texto brillante sobre fondo oscuro/tierra)
#define COLOR_AGUA_BG 1		// Azul Oscuro para el fondo del agua (~)
#define COLOR_AGUA_FG 9		// Azul Claro para el simbolo del agua (~)

#define COLOR_TIERRA_BG 2	// Verde Oscuro para el fondo de la tierra (.)
#define COLOR_TIERRA_FG 10	// Verde Brillante para el simbolo de la tierra (.)

#define COLOR_MADERA 14		// Amarillo Brillante (Texto 'M' sobre tierra BG)
#define COLOR_PIEDRA 15		// Blanco Brillante (Texto 'R' sobre tierra BG)
#define COLOR_COMIDA 14		// Rojo Brillante (Texto 'C' sobre tierra BG)

#define COLOR_ORO 14		// Amarillo/Marron (Texto '$' sobre tierra BG)
#define COLOR_ENEMIGO 4		// Rojo Oscuro (Texto 'E' sobre fondo Rojo Brillante)

// Variables globales para el offset de la vista (scrolling)
int offset_f = 0;
int offset_c = 0;

/* =============================== */
/* Utilidades de consola          */
/* =============================== */
/* Se asume que setColor(int fondo, int texto) y otras utilidades
   como moverCursor y ocultarCursor estan definidas en menu.h/otro. */

/* =============================== */
/* Generador de islas fijas       */
/* =============================== */
void crearIsla(char mapa[MAPA_F][MAPA_C], int cx, int cy, int rx, int ry) {
    int i, j;
    int dx, dy, dist, deformacion, nx, ny;
    int maxX = rx + 4;
    int maxY = ry + 4;

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
/* Inicializar mapa               */
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
    crearIsla(mapa, 12, 12, 10, 8);	// Esquina superior izquierda
    crearIsla(mapa, 85, 12, 10, 8);	// Esquina superior derecha
    crearIsla(mapa, 12, 85, 10, 8);	// Esquina inferior izquierda
    crearIsla(mapa, 85, 85, 10, 8);	// Esquina inferior derecha
    crearIsla(mapa, 40, 50, 10, 8);	// Isla central ajustada (no se corta)

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
    placed = 0;
    while (placed < NUM_COMIDA) {
        ex = rand() % MAPA_F;
        ey = rand() % MAPA_C;
        if (mapa[ex][ey] == '.') {
            mapa[ex][ey] = 'C';
            placed++;
        }
    }
    placed = 0;
    while (placed < NUM_MADERA) {
        ex = rand() % MAPA_F;
        ey = rand() % MAPA_C;
        if (mapa[ex][ey] == '.') {
            mapa[ex][ey] = 'M';
            placed++;
        }
    }
    placed = 0;
    while (placed < NUM_PIEDRA) {
        ex = rand() % MAPA_F;
        ey = rand() % MAPA_C;
        if (mapa[ex][ey] == '.') {
            mapa[ex][ey] = 'R';
            placed++;
        }
    }
}

/* =============================== */
/* Dibujar marco del mapa         */
/* =============================== */
void dibujarMarcoMapa() {
    int i;
    setColor(0, 8); // Color gris oscuro para el marco

    // Esquinas y lineas horizontales
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

    // Lineas verticales
    for (i = 1; i < FILAS + 1; i++) {
        moverCursor(0, i);
        printf("%c", 186); // ║
        moverCursor(COLUMNAS * 2 + 2, i); // Ajustada la posicion
        printf("%c", 186); // ║
    }
    setColor(0, 15); // Restaurar color por defecto
}

/* =============================== */
/* Mostrar mapa                   */
/* =============================== */
// void mostrarMapa(char mapa[MAPA_F][MAPA_C]) {
//     int i, j;
//      // <-- Para clipping
//     ocultarCursor();
//     dibujarMarcoMapa();
    
//     for (i = 0; i < FILAS; i++) {
//         moverCursor(2, i + 1); // Posicionar cursor al inicio de la fila
        
//         for (j = 0; j < COLUMNAS; j++) {
            
            

//             char c = mapa[offset_f + i][offset_c + j];
//             if (c == '~') {
//                 setColor(COLOR_AGUA_BG, COLOR_AGUA_FG);
//                 printf("~ ");
//             } else if (c == '.') {
//                 setColor(COLOR_TIERRA_BG, COLOR_TIERRA_FG);
//                 printf(". ");
//             } else if (c == '$') {
//                 setColor(COLOR_TIERRA_BG, COLOR_ORO);
//                 printf("$ ");
//             } else if (c == 'C') {
//                 setColor(COLOR_TIERRA_BG, COLOR_COMIDA);
//                 printf("C ");
//             } else if (c == 'R') {
//                 setColor(COLOR_TIERRA_BG, COLOR_PIEDRA);
//                 printf("R ");
//             } else if (c == 'M') {
//                 setColor(COLOR_TIERRA_BG, COLOR_MADERA);
//                 printf("M ");
//             } else if (c == 'E') {
//                 setColor(12, COLOR_ENEMIGO);
//                 printf("E ");
//             } else {
//                 setColor(0, 15);
//                 printf("%c ", c);
//             }
//         }
//     }
//     setColor(0, 15);
// }


void mostrarMapa(char mapa[MAPA_F][MAPA_C]) {
    // Configuración para escritura en bloque (elimina el parpadeo)
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    
    // El ancho es COLUMNAS * 2 porque cada tile visualmente son 2 caracteres ("~ ", ". ", etc.)
    int bufferWidth = COLUMNAS * 2;
    int bufferHeight = FILAS;

    COORD bufferSize = { (short)bufferWidth, (short)bufferHeight };
    COORD bufferCoord = { 0, 0 };
    
    // Definimos la región de la pantalla donde vamos a "pegar" el buffer
    // Empieza en Columna 2, Fila 1 (según tu lógica original de moverCursor(2, i+1))
    SMALL_RECT writeRegion = { 
        2, 
        1, 
        (short)(2 + bufferWidth - 1), 
        (short)(1 + bufferHeight - 1) 
    };

    // Creamos un array de CHAR_INFO. Esto actúa como nuestro "mapa en memoria"
    // Usamos VLA (Variable Length Array) o un tamaño fijo grande si el compilador es antiguo.
    // Dado que FILAS y COLUMNAS son defines, esto es seguro.
    CHAR_INFO buffer[FILAS * COLUMNAS * 2];

    int i, j;

    // Dibujamos el marco primero (esto es rápido y estático)
    ocultarCursor();
    dibujarMarcoMapa();
    
    // Llenamos el buffer en memoria en lugar de imprimir en pantalla
    for (i = 0; i < FILAS; i++) {
        for (j = 0; j < COLUMNAS; j++) {
            char c = mapa[offset_f + i][offset_c + j];
            WORD attr = 0;
            char symbol = ' ';

            // Lógica de colores traducida a atributos de Windows
            // (Fondo << 4) | Texto
            if (c == '~') {
                attr = (COLOR_AGUA_BG << 4) | COLOR_AGUA_FG;
                symbol = '~';
            } else if (c == '.') {
                attr = (COLOR_TIERRA_BG << 4) | COLOR_TIERRA_FG;
                symbol = '.';
            } else if (c == '$') {
                attr = (COLOR_TIERRA_BG << 4) | COLOR_ORO;
                symbol = '$';
            } else if (c == 'C') {
                attr = (COLOR_TIERRA_BG << 4) | COLOR_COMIDA;
                symbol = 'C';
            } else if (c == 'R') {
                attr = (COLOR_TIERRA_BG << 4) | COLOR_PIEDRA;
                symbol = 'R';
            } else if (c == 'M') {
                attr = (COLOR_TIERRA_BG << 4) | COLOR_MADERA;
                symbol = 'M';
            } else if (c == 'E') {
                // Asumiendo que tu setColor(12, COLOR_ENEMIGO) usa 12 como fondo
                attr = (12 << 4) | COLOR_ENEMIGO;
                symbol = 'E';
            } else {
                attr = (0 << 4) | 15; // Blanco sobre negro por defecto
                symbol = c;
            }

            // Calculamos la posición lineal en el array unidimensional
            int index = (i * bufferWidth) + (j * 2);

            // Asignamos el primer caracter del tile (ej: '~')
            buffer[index].Char.AsciiChar = symbol;
            buffer[index].Attributes = attr;

            // Asignamos el segundo caracter del tile (espacio ' ')
            buffer[index + 1].Char.AsciiChar = ' ';
            buffer[index + 1].Attributes = attr;
        }
    }

    // ¡MAGIA! Escribimos todo el bloque de memoria a la consola en una sola operación.
    // Esto es instantáneo y elimina el parpadeo durante el scroll.
    WriteConsoleOutput(hConsole, buffer, bufferSize, bufferCoord, &writeRegion);

    // Restauramos colores para el resto de la interfaz
    setColor(0, 15);
}

/* =============================== */
/* Mover jugador                  */
/* =============================== */
/* =============================== */
/* Mover jugador                  */
/* =============================== */
// ¡¡Firma de la función modificada!!
int moverJugador(struct Jugador *j, char mapa[MAPA_F][MAPA_C], int *x, int *y, char direccion) {
    int nx = *x;
    int ny = *y;
    char destino;
    char msg[60];
    int i;
    char actual;
    int offset_changed = 0;
    int recurso_recolectado = 0;

    if (direccion >= 'a' && direccion <= 'z')
        direccion -= 32;

    if (direccion == 'W') nx--;
    else if (direccion == 'S') nx++;
    else if (direccion == 'A') ny--;
    else if (direccion == 'D') ny++;
    else return 0;

    if (nx < 0 || nx >= MAPA_F || ny < 0 || ny >= MAPA_C) {
        moverCursor(0, FILAS + 2);
        setColor(0, 14);
        printf("No puedes salir del mapa!          ");
        setColor(0, 15);
        return 0;
    }

    destino = mapa[nx][ny];
    msg[0] = '\0';

    if (destino == '~') {
        moverCursor(0, FILAS + 2);
        setColor(0, 14);
        printf("No puedes nadar!                   ");
        setColor(0, 15);
        return 0;
    }

    // --- LÓGICA DE RECOLECCIÓN DE RECURSOS ---
    if (destino == '$') {
        sprintf(msg, "Has encontrado 10 de oro!");
        j->Oro += 10; 
        recurso_recolectado = 1;
        mapa[nx][ny] = '.';
    } else if (destino == 'E') {
        sprintf(msg, "Has encontrado un enemigo!");
        mapa[nx][ny] = '.';
    } else if (destino == 'M') {
        sprintf(msg, "Has encontrado 15 de madera!");
        j->Madera += 15; 
        recurso_recolectado = 1;
        mapa[nx][ny] = '.';
    } else if (destino == 'R') {
        sprintf(msg, "Has encontrado 15 de piedra!");
        j->Piedra += 15; 
        recurso_recolectado = 1;
        mapa[nx][ny] = '.';
    } else if (destino == 'C') {
        sprintf(msg, "Has encontrado 20 de comida!");
        j->Comida += 20; 
        recurso_recolectado = 1;
        mapa[nx][ny] = '.';
    }
    // ---------------------------------------------

    // Calcular nuevo offset para centrar la vista en el jugador
    int new_offset_f = offset_f;
    int new_offset_c = offset_c;
    new_offset_f = nx - FILAS / 2;
    new_offset_c = ny - COLUMNAS / 2; // COLUMNAS es 41, así que esto funciona
    if (new_offset_f < 0) new_offset_f = 0;
    if (new_offset_f > MAPA_F - FILAS) new_offset_f = MAPA_F - FILAS;
    if (new_offset_c < 0) new_offset_c = 0;
    if (new_offset_c > MAPA_C - COLUMNAS) new_offset_c = MAPA_C - COLUMNAS;

    if (new_offset_f != offset_f || new_offset_c != offset_c) {
        offset_f = new_offset_f;
        offset_c = new_offset_c;
        offset_changed = 1;
    }

    if (!offset_changed) {
        // Borrar 'P' de la posición anterior
        // Ya no necesitamos la comprobación 'if (old_physical_x < STATS_X)'
        moverCursor((short)((*y - offset_c) * 2 + 2), (short)(*x - offset_f + 1));
        actual = mapa[*x][*y];
        if (actual == '~') {
            setColor(COLOR_AGUA_BG, COLOR_AGUA_FG);
        } else if (actual == '.') {
            setColor(COLOR_TIERRA_BG, COLOR_TIERRA_FG);
        } else if (actual == '$') {
            setColor(COLOR_TIERRA_BG, COLOR_ORO);
        } else if (actual == 'E') {
            setColor(12, COLOR_ENEMIGO);
        } else if (actual == 'M') {
            setColor(COLOR_TIERRA_BG, COLOR_MADERA);
        } else if (actual == 'R') {
            setColor(COLOR_TIERRA_BG, COLOR_PIEDRA);
        } else if (actual == 'C') {
            setColor(COLOR_TIERRA_BG, COLOR_COMIDA);
        } else {
            setColor(0, 15);
        }
        printf("%c ", actual);
    }


    *x = nx;
    *y = ny;

    if (offset_changed) {
        mostrarMapa(mapa);
    }

    // Dibujar 'P' en la nueva posición
    // Ya no necesitamos la comprobación 'if (new_physical_x < STATS_X)'
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

    return recurso_recolectado;
}


/* =============================== */
/* Animar agua con flujo (~ y ' ') */
/* =============================== */
void animarAgua(char mapa[MAPA_F][MAPA_C]) {
    int i, j;
    static int frame = 0;
    frame++;

    // El bucle 'j < COLUMNAS' ahora se detiene automáticamente antes del panel (en j = 40),
    // porque COLUMNAS se definió como 41 en mapa.h.
    for (i = 0; i < FILAS; i++) {
        for (j = 0; j < COLUMNAS; j++) { 
            
            // Ya no necesitamos la variable 'physical_x' ni la comprobación 'if'.

            // Solo dibujamos si es agua
            if (mapa[offset_f + i][offset_c + j] == '~') {

                moverCursor((j * 2) + 2, i + 1);
                if ((i + j + frame) % 3 == 0) {
                    setColor(COLOR_AGUA_BG, COLOR_AGUA_FG); printf("~ ");
                } else {
                    setColor(COLOR_AGUA_BG, COLOR_AGUA_FG); printf("  ");
                }
            }
        }
    }
    setColor(0, 15);
    moverCursor(0, FILAS + 3); 
}

