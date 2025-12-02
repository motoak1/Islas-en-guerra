#include "mapa.h"
#include "menu.h"
#include <stdlib.h>
#include <time.h>
#include <conio.h>
#include "../recursos/recursos.h"  // ✓ CORRECTO - mismo directorio 
#include <stdbool.h>

// --- CONFIGURACIÓN DE GENERACIÓN ---
#define NUM_RECURSOS 60
#define NUM_COMIDA 40
#define NUM_ARBOLES 150     // Cantidad de árboles decorativos/obstáculos

// --- COLORES ---
#define COLOR_AGUA_BG 1     // Azul Oscuro
#define COLOR_AGUA_FG 9     // Azul Claro
#define COLOR_TIERRA_BG 2   // Verde Oscuro
#define COLOR_TIERRA_FG 10  // Verde Brillante
#define COLOR_ARBOL 6       // Marrón/Ocre para árboles
#define COLOR_MINA_BG 8     // Gris Oscuro (Fondo Mina)
#define COLOR_MINA_FG 15    // Blanco (Texto Mina)
#define COLOR_COMIDA 14     // Rojo/Naranja
#define COLOR_ORO 14        // Dorado
#define COLOR_CAMP_BG 4     // Rojo Oscuro (Fondo Campamento)
#define COLOR_CAMP_FG 15    // Blanco (Texto Campamento)
#define COLOR_CASA_BG 5      // ✅ AÑADIR: Morado para Casa Central
#define COLOR_CASA_FG 15

// Variables globales para el offset (vista de cámara)
int offset_f = 0;
int offset_c = 0;

/* =============================== */
/* Generador de islas             */
/* =============================== */
void crearIsla(char mapa[MAPA_F][MAPA_C], int cx, int cy, int rx, int ry) {
    int i, j, dx, dy, dist, deformacion, nx, ny;
    int maxX = rx + 10;
    int maxY = ry + 10;

    for (i = -maxY; i <= maxY; i++) {
        for (j = -maxX; j <= maxX; j++) {
            dx = j; dy = i;
            dist = dx * dx + dy * dy;
            deformacion = rand() % 5;
            if (dist <= (rx * ry) + deformacion) {
                nx = cy + i;
                ny = cx + j; // Nota: cx es fila, cy es columna en tu lógica original invertida
                // Ajuste para coordenadas estándar (Fila, Columna)
                // Si cx es Fila y cy es Columna:
                nx = cx + i; 
                ny = cy + j;
                
                if (nx >= 0 && nx < MAPA_F && ny >= 0 && ny < MAPA_C)
                    mapa[nx][ny] = '.';
            }
        }
    }
}

/* =============================== */
/* Crear puentes                  */
/* =============================== */
void crearPuente(char mapa[MAPA_F][MAPA_C], int x1, int y1, int x2, int y2) {
    int dx = abs(x2 - x1), dy = abs(y2 - y1);
    int sx = x1 < x2 ? 1 : -1, sy = y1 < y2 ? 1 : -1;
    int err = dx - dy;

    while (true) {
        if (x1 >= 0 && x1 < MAPA_F && y1 >= 0 && y1 < MAPA_C)
            mapa[x1][y1] = '.';
        if (x1 == x2 && y1 == y2) break;
        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x1 += sx; }
        if (e2 < dx) { err += dx; y1 += sy; }
    }
}

/* =============================== */
/* Inicializar Mapa (Esquinas)    */
/* =============================== */
int esPuente(int x, int y) {
    // Puentes horizontales
    if (x == 25 && y >= 35 && y <= 265) return 1;  // Superior
    if (x == 125 && y >= 35 && y <= 265) return 1; // Inferior
    
    // Puentes verticales
    if (y == 35 && x >= 25 && x <= 125) return 1;  // Izquierdo
    if (y == 265 && x >= 25 && x <= 125) return 1; // Derecho
    if (y == 150 && x >= 25 && x <= 125) return 1; // Central vertical
    if (x == 75 && y >= 35 && y <= 265) return 1;  // Central horizontal
    
    return 0; // No es puente
}

void inicializarMapa(char mapa[MAPA_F][MAPA_C]) {
    int i, j, placed, rx, ry, ex, ey;
    srand((unsigned int)time(NULL));

    // 1. Llenar de agua
    for (i = 0; i < MAPA_F; i++) {
        for (j = 0; j < MAPA_C; j++) {
            mapa[i][j] = '~';
        }
    }

    // Definición de Islas para colocación posterior
    struct IslaInfo { int f, c, rf, rc; };
    struct IslaInfo islas[] = {
        {25, 35, 22, 20},   // Sup. Izq
        {25, 150, 22, 20},  // Sup. Der
        {125, 35, 22, 20},  // Inf. Izq
        {125, 150, 22, 20}, // Inf. Der
        {75, 75, 15, 25}    // Central
    };

    // 2. Crear Islas
    for (int k = 0; k < 5; k++) {
        crearIsla(mapa, islas[k].f, islas[k].c, islas[k].rf, islas[k].rc);
    }

    // 3. Conectar Islas (Puentes)
    crearPuente(mapa, 25, 35, 25, 265);   // Horizontal Superior
    crearPuente(mapa, 125, 35, 125, 265); // Horizontal Inferior
    crearPuente(mapa, 25, 35, 125, 35);   // Vertical Izquierdo
    crearPuente(mapa, 25, 265, 125, 265); // Vertical Derecho

    // Conexiones a Central
    crearPuente(mapa, 75, 150, 25, 150);
    crearPuente(mapa, 75, 150, 125, 150);
    crearPuente(mapa, 75, 150, 75, 35);
    crearPuente(mapa, 75, 150, 75, 265);

    // 4. Colocar Elementos Específicos por Isla (Minas, Campamentos y Casa Central)
for (int k = 0; k < 5; k++) {
    // Colocar 1 Casa Central por isla ('H') - en el centro aproximado
    placed = 0;
    while (placed < 1) {
        // Intentar colocar cerca del centro de la isla
        rx = islas[k].f + (rand() % 6) - 3; // Muy cerca del centro
        ry = islas[k].c + (rand() % 6) - 3;
        if (rx >= 0 && rx < MAPA_F && ry >= 0 && ry < MAPA_C && mapa[rx][ry] == '.') {
            mapa[rx][ry] = 'H'; // Casa Central
            placed++;
        }
    }
    
    // Colocar 1 Mina por isla ('M')
    placed = 0;
    while (placed < 1) {
        rx = islas[k].f + (rand() % (islas[k].rf * 2)) - islas[k].rf;
        ry = islas[k].c + (rand() % (islas[k].rc * 2)) - islas[k].rc;
        if (rx >= 0 && rx < MAPA_F && ry >= 0 && ry < MAPA_C && mapa[rx][ry] == '.') {
            mapa[rx][ry] = 'M';
            placed++;
        }
    }

    // Colocar 4 Campamentos por isla ('E')
    placed = 0;
    while (placed < 4) {
        ex = islas[k].f + (rand() % (islas[k].rf * 2)) - islas[k].rf;
        ey = islas[k].c + (rand() % (islas[k].rc * 2)) - islas[k].rc;
        if (ex >= 0 && ex < MAPA_F && ey >= 0 && ey < MAPA_C && mapa[ex][ey] == '.') {
            mapa[ex][ey] = 'E';
            placed++;
        }
    }
}

    // 5. Colocar Árboles y Recursos Aleatorios
   placed = 0; 
while (placed < NUM_ARBOLES) { 
    rx = rand() % MAPA_F; 
    ry = rand() % MAPA_C; 
    // Verificar que sea tierra Y que NO sea un puente
    if (mapa[rx][ry] == '.' && !esPuente(rx, ry)) { 
        mapa[rx][ry] = 'A'; 
        placed++; 
    } 
} // Árboles
    placed = 0; 
	while (placed < NUM_RECURSOS) { rx = rand() % MAPA_F; ry = rand() % MAPA_C; if (mapa[rx][ry] == '.') { mapa[rx][ry] = '$'; placed++; } 
	} // Oro
    placed = 0;
	 while (placed < NUM_COMIDA) { ex = rand() % MAPA_F; ey = rand() % MAPA_C; if (mapa[ex][ey] == '.') { mapa[ex][ey] = 'F'; placed++; } } // Cultivos (Farm)
}

/* =============================== */
/* Dibujar Marco                  */
/* =============================== */
void dibujarMarcoMapa() {
    int i;
    setColor(0, 8);
    moverCursor(0, 0); printf("%c", 201);
    for (i = 1; i < COLUMNAS * 2 + 2; i++) printf("%c", 205);
    printf("%c", 187);

    moverCursor(0, FILAS + 1); printf("%c", 200);
    for (i = 1; i < COLUMNAS * 2 + 2; i++) printf("%c", 205);
    printf("%c", 188);

    for (i = 1; i < FILAS + 1; i++) {
        moverCursor(0, i); printf("%c", 186);
        moverCursor(COLUMNAS * 2 + 2, i); printf("%c", 186);
    }
    setColor(0, 15);
}

/* =============================== */
/* Mostrar Mapa (Doble Buffer)    */
/* =============================== */
void mostrarMapa(char mapa[MAPA_F][MAPA_C], int px, int py) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    int bufferWidth = COLUMNAS * 2;
    int bufferHeight = FILAS;
    COORD bufferSize = { (short)bufferWidth, (short)bufferHeight };
    COORD bufferCoord = { 0, 0 };
    SMALL_RECT writeRegion = { 2, 1, (short)(2 + bufferWidth - 1), (short)(1 + bufferHeight - 1) };
    CHAR_INFO buffer[FILAS * COLUMNAS * 2];
    int i, j;

    static int marcoDibujado = 0;
    if (!marcoDibujado) {
        ocultarCursor();
        dibujarMarcoMapa();
        marcoDibujado = 1;
    }
    
    // Llenar buffer con el mapa
    for (i = 0; i < FILAS; i++) {
        for (j = 0; j < COLUMNAS; j++) {
            int absX = offset_f + i;
            int absY = offset_c + j;
            char c = mapa[absX][absY];
            WORD attr = 0;
            char symbol = ' ';

            if (c == '~') { attr = (COLOR_AGUA_BG << 4) | COLOR_AGUA_FG; symbol = '~'; }
            else if (c == '.') { attr = (COLOR_TIERRA_BG << 4) | COLOR_TIERRA_FG; symbol = '.'; }
            else if (c == '$') { attr = (COLOR_TIERRA_BG << 4) | COLOR_ORO; symbol = '$'; }
            else if (c == 'F') { attr = (COLOR_TIERRA_BG << 4) | COLOR_COMIDA; symbol = 'F'; }
            else if (c == 'M') { attr = (COLOR_MINA_BG << 4) | COLOR_MINA_FG; symbol = 'M'; }
            else if (c == 'A') { attr = (COLOR_TIERRA_BG << 4) | COLOR_ARBOL; symbol = 'A'; }
            else if (c == 'E') { attr = (COLOR_CAMP_BG << 4) | COLOR_CAMP_FG; symbol = 'E'; }
            else if (c == 'H') { attr = (5 << 4) | 15; symbol = 'H'; } // Casa Central (fondo morado, texto blanco)
            else { attr = (0 << 4) | 15; symbol = c; }

            // Dibujar Jugador en el buffer
            if (absX == px && absY == py) {
                symbol = 'P';
                attr = (0 << 4) | 10;
            }

            int index = (i * bufferWidth) + (j * 2);
            buffer[index].Char.AsciiChar = symbol;
            buffer[index].Attributes = attr;
            buffer[index + 1].Char.AsciiChar = ' ';
            buffer[index + 1].Attributes = attr;
        }
    }
    
    WriteConsoleOutput(hConsole, buffer, bufferSize, bufferCoord, &writeRegion);
    setColor(0, 15);
}

/* =============================== */
/* Mover Jugador (MEJORADO)       */
/* =============================== */
int explorarMapa(struct Jugador *j, char mapa[MAPA_F][MAPA_C], int *x, int *y, char direccion) {
    int nx = *x, ny = *y;
    char destino, msg[60] = "";
    int i, offset_changed = 0, recurso_recolectado = 0;
    int rx, ry; // Para recursos adyacentes

    // Convertir minuscula a mayuscula
    if (direccion >= 'a' && direccion <= 'z') direccion -= 32;
    
    // Detectar tecla ENTER para interacciones especiales
    if (direccion == 13) { // ENTER presionado
        // Verificar si hay un arbol adyacente
        if (hayRecursoAdyacente(mapa, *x, *y, 'A', &rx, &ry)) {
    animarTalado(rx, ry);
    j->Madera += 30;
    mapa[rx][ry] = '.';
    mostrarMapa(mapa, *x, *y);
    forzarRedibujoPanelEnMapa(*j); // ✅ AÑADIR ESTA LÍNEA
    return 1;
}
        // Verificar si hay un cultivo adyacente
        else if (hayRecursoAdyacente(mapa, *x, *y, 'F', &rx, &ry)) {
    j->Comida += 25;
    mapa[rx][ry] = '.';
    
    moverCursor(0, FILAS + 2);
    for (i = 0; i < 60; i++) printf(" ");
    moverCursor(0, FILAS + 2);
    setColor(0, 10);
    printf("Has cosechado el cultivo! +25 Comida");
    setColor(0, 15);
    Sleep(800);
    
    mostrarMapa(mapa, *x, *y);
    forzarRedibujoPanelEnMapa(*j); // ✅ AÑADIR ESTA LÍNEA
    return 1;
}
        // Verificar si hay un campamento adyacente
        else if (hayRecursoAdyacente(mapa, *x, *y, 'E', &rx, &ry)) {
    mostrarMenuCampamento(j, mapa, *x, *y); // Mostrar menu (incluye redibujo interno)
    return 0; // No se recolecto recurso
}
else if (hayRecursoAdyacente(mapa, *x, *y, 'H', &rx, &ry)) {
    mostrarMenuCasaCentral(j, mapa, *x, *y, rx, ry);
    return 0;
}
        // Verificar si hay una mina adyacente
        else if (hayRecursoAdyacente(mapa, *x, *y, 'M', &rx, &ry)) {
            // Mensaje especial para minas
            moverCursor(0, FILAS + 2);
            for (i = 0; i < 60; i++) printf(" ");
            moverCursor(0, FILAS + 2);
            setColor(0, 14);
            printf("Mina detectada. (Proximamente: sistema de mineria)");
            setColor(0, 15);
            Sleep(1000);
            return 0;
        }
        else {
            // No hay nada interactivo cerca
            moverCursor(0, FILAS + 2);
            for (i = 0; i < 60; i++) printf(" ");
            moverCursor(0, FILAS + 2);
            setColor(0, 8);
            printf("No hay nada con que interactuar aqui.");
            setColor(0, 15);
            Sleep(500);
            return 0;
        }
    }
    
    // Movimiento normal (WASD)
    if (direccion == 'W') nx--;
    else if (direccion == 'S') nx++;
    else if (direccion == 'A') ny--;
    else if (direccion == 'D') ny++;
    else return 0;

    // Verificar limites del mapa
    if (nx < 0 || nx >= MAPA_F || ny < 0 || ny >= MAPA_C) {
        moverCursor(0, FILAS + 2);
        setColor(0, 14);
        printf("Limite del mapa!               ");
        setColor(0, 15);
        return 0;
    }

    destino = mapa[nx][ny];
    
    // Verificar agua
    if (destino == '~') {
        moverCursor(0, FILAS + 2);
        setColor(0, 14);
        printf("No puedes nadar!               ");
        setColor(0, 15);
        return 0;
    }
    
    // Obstaculos (Arboles, Minas, Campamentos) - NO se pueden atravesar
    if (destino == 'A') {
        moverCursor(0, FILAS + 2);
        setColor(0, 6);
        printf("Un arbol! Presiona ENTER para talarlo");
        setColor(0, 15);
        return 0;
    }
    if (destino == 'M') {
        moverCursor(0, FILAS + 2);
        setColor(0, 8);
        printf("Una mina! Presiona ENTER para interactuar");
        setColor(0, 15);
        return 0;
    }
    if (destino == 'E') {
        moverCursor(0, FILAS + 2);
        setColor(0, 12);
        printf("Campamento! Presiona ENTER para entrar");
        setColor(0, 15);
        return 0;
    }
    if (destino == 'H') {
    moverCursor(0, FILAS + 2);
    setColor(0, 13);
    printf("Casa Central! Presiona ENTER para gestionar cultivos");
    setColor(0, 15);
    return 0;
}
    if (destino == 'F') {
        moverCursor(0, FILAS + 2);
        setColor(0, 10);
        printf("Cultivo! Presiona ENTER para cosechar");
        setColor(0, 15);
        return 0;
    }

    // Recoleccion automatica (oro solamente)
    if (destino == '$') {
        sprintf(msg, "Oro +10!");
        j->Oro += 10;
        recurso_recolectado = 1;
        mapa[nx][ny] = '.';
    }

    // Actualizar offset de camara
    int new_offset_f = nx - FILAS / 2;
    int new_offset_c = ny - COLUMNAS / 2;
    if (new_offset_f < 0) new_offset_f = 0;
    else if (new_offset_f > MAPA_F - FILAS) new_offset_f = MAPA_F - FILAS;
    if (new_offset_c < 0) new_offset_c = 0;
    else if (new_offset_c > MAPA_C - COLUMNAS) new_offset_c = MAPA_C - COLUMNAS;

    if (new_offset_f != offset_f || new_offset_c != offset_c) {
        offset_f = new_offset_f;
        offset_c = new_offset_c;
        offset_changed = 1;
    }

    // Optimizacion: Si la camara NO se mueve, solo redibujamos la celda abandonada
    if (!offset_changed) {
        moverCursor((short)((*y - offset_c) * 2 + 2), (short)(*x - offset_f + 1));
        char actual = mapa[*x][*y];
        if (actual == '~') setColor(COLOR_AGUA_BG, COLOR_AGUA_FG);
        else if (actual == '.') setColor(COLOR_TIERRA_BG, COLOR_TIERRA_FG);
        else if (actual == '$') setColor(COLOR_TIERRA_BG, COLOR_ORO);
        else if (actual == 'F') setColor(COLOR_TIERRA_BG, COLOR_COMIDA);
        else if (actual == 'M') setColor(COLOR_MINA_BG, COLOR_MINA_FG);
        else if (actual == 'A') setColor(COLOR_TIERRA_BG, COLOR_ARBOL);
        else if (actual == 'E') setColor(COLOR_CAMP_BG, COLOR_CAMP_FG);
        else setColor(0, 15);
        printf("%c ", actual);
    }

    *x = nx;
    *y = ny;

    // Redibujar si es necesario
   if (offset_changed) {
    mostrarMapa(mapa, *x, *y); // Redibujar todo (sin parpadeo)
    // IMPORTANTE: Redibujar panel cuando cambia el offset (forzado)
    if (j != NULL) forzarRedibujoPanelEnMapa(*j);
} else {
    moverCursor((short)((ny - offset_c) * 2 + 2), (short)(nx - offset_f + 1));
    setColor(0, 10);
    printf("P ");
    setColor(0, 15);
}

   ocultarCursor();

// Limpiar solo la linea de mensaje principal
moverCursor(0, FILAS + 2);
setColor(0, 0);
for (i = 0; i < 120; i++) printf(" ");

// Mostrar mensaje apropiado
moverCursor(0, FILAS + 2);
if (msg[0] != '\0') {
    setColor(0, 11);
    printf("%s", msg);
} else {
    setColor(0, 15);
    printf("Usa WASD para moverte. ENTER para interactuar. ESC para salir.");
}
setColor(0, 15);

    return recurso_recolectado;
}

/* =============================== */
/* Animar Agua                    */
/* =============================== */
void animarAgua(char mapa[MAPA_F][MAPA_C]) {
    int i, j;
    static int frame = 0;
    
    // Área del panel que debemos evitar (en coordenadas de pantalla)
    int panel_screen_x_start = 63;
    int panel_screen_x_end = 82;
    int panel_screen_y_start = 1;
    int panel_screen_y_end = 9;
    
    frame++;
    
    for (i = 0; i < FILAS; i++) {
        for (j = 0; j < COLUMNAS; j++) { 
            if (mapa[offset_f + i][offset_c + j] == '~') {
                int screen_x = (j * 2) + 2;
                int screen_y = i + 1;
                
                // Verificar si esta posicion esta dentro del area del panel
                if (screen_x >= panel_screen_x_start && screen_x <= panel_screen_x_end &&
                    screen_y >= panel_screen_y_start && screen_y <= panel_screen_y_end) {
                    continue; // Saltar esta celda
                }
                
                moverCursor(screen_x, screen_y);
                if ((i + j + frame) % 3 == 0) { 
                    setColor(COLOR_AGUA_BG, COLOR_AGUA_FG); 
                    printf("~ "); 
                } else { 
                    setColor(COLOR_AGUA_BG, COLOR_AGUA_FG); 
                    printf("  "); 
                }
            }
        }
    }
    ocultarCursor();
    setColor(0, 15);
}
/* =============================== */
/* Verificar Recurso Adyacente    */
/* =============================== */
// Verifica si hay un recurso especifico (A, F, E, M) en las 8 casillas adyacentes
// Devuelve 1 si lo encuentra, y guarda su posicion en rx, ry

int hayRecursoAdyacente(char mapa[MAPA_F][MAPA_C], int px, int py, char recurso, int *rx, int *ry) {
    int i;
    // Verificar las 8 direcciones alrededor del jugador
    int dx[] = {-1, -1, -1,  0,  0,  1,  1,  1};
    int dy[] = {-1,  0,  1, -1,  1, -1,  0,  1};
    
    for (i = 0; i < 8; i++) {
        int nx = px + dx[i];
        int ny = py + dy[i];
        
        // Verificar limites
        if (nx >= 0 && nx < MAPA_F && ny >= 0 && ny < MAPA_C) {
            if (mapa[nx][ny] == recurso) {
                *rx = nx;
                *ry = ny;
                return 1;
            }
        }
    }
    return 0;
}

/* =============================== */
/* Animacion de Talado de Arbol   */
/* =============================== */
void animarTalado(int x, int y) {
    int i, j;
    // Frames de la animacion (3 frames)
    char *frames[] = {
        "  A  ",  // Frame 1: Arbol completo
        " /A\\ ", // Frame 2: Arbol siendo cortado
        " ___ "  // Frame 3: Arbol cortado (tronco)
    };
    
    // Calcular posicion en pantalla
    int screen_x = (y - offset_c) * 2 + 2;
    int screen_y = x - offset_f + 1;
    
    // Animar 3 frames
    for (i = 0; i < 3; i++) {
        moverCursor(screen_x, screen_y);
        setColor(COLOR_TIERRA_BG, COLOR_ARBOL);
        printf("%s", frames[i]);
        Sleep(300); // Pausa de 300ms entre frames
    }
    
    // Mensaje de recoleccion
    // Mensaje de recoleccion
moverCursor(0, FILAS + 2);
for (j = 0; j < 60; j++) printf(" ");
moverCursor(0, FILAS + 2);
setColor(0, 6);
printf("Has talado el arbol! +30 Madera");
setColor(0, 15);
Sleep(800);
}

void mostrarMenuCampamento(struct Jugador *j, char mapa[MAPA_F][MAPA_C], int px, int py) {
    int tecla, i;
    int seleccion = 0; // 0=Espadachin, 1=Arquero, 2=Piquero, 3=Caballeria
    int actualizado = 1; // Flag para redibujar
    
    // Datos de las tropas: {Oro, Comida, Madera, Piedra, Vida, Ataque, VelAtq, Alcance}
    int costos[4][4] = {
        {50, 30, 20, 10},  // Espadachin
        {40, 25, 30, 5},   // Arquero
        {35, 20, 25, 15},  // Piquero
        {80, 50, 40, 20}   // Caballeria
    };
    
    char* nombres[4] = {"Espadachin", "Arquero", "Piquero", "Caballeria"};
    int stats[4][4] = {
        {100, 15, 2, 1},  // Espadachin: Vida, Ataque, VelAtq, Alcance
        {80, 12, 3, 5},   // Arquero
        {90, 10, 2, 2},   // Piquero
        {120, 20, 1, 1}   // Caballeria
    };
    
    while (1) {
        if (actualizado) {
            // Limpiar pantalla
            system("cls");
            
            // Dibujar marco del menu
            setColor(0, 15);
            printf("\n");
            printf("  ================================================================================\n");
            printf("  ||                     CUARTEL DE ENTRENAMIENTO - %s                  ||\n", j->Nombre);
            printf("  ================================================================================\n\n");
            
            // Mostrar tropas actuales del jugador
            setColor(0, 11);
            printf("  TROPAS ACTUALES:\n");
            setColor(0, 15);
            printf("  -------------------------------------------------------------------------------\n");
            printf("  Total: %d/%d | Espadas: %d | Arqueros: %d | Picas: %d | Caballeria: %d\n",
                   j->NumeroTropas, j->Capacidad > 0 ? j->Capacidad : 100,
                   j->CantidadEspadas, j->CantidadArqueros, j->CantidadPicas, j->CantidadCaballeria);
            printf("  -------------------------------------------------------------------------------\n\n");
            
            // Mostrar recursos disponibles
            setColor(0, 14);
            printf("  RECURSOS DISPONIBLES:\n");
            setColor(0, 15);
            printf("  -------------------------------------------------------------------------------\n");
            printf("  ");
            setColor(0, 14); printf("Oro: %d   ", j->Oro);
            setColor(0, 12); printf("Comida: %d   ", j->Comida);
            setColor(0, 6); printf("Madera: %d   ", j->Madera);
            setColor(0, 7); printf("Piedra: %d\n", j->Piedra);
            setColor(0, 15);
            printf("  -------------------------------------------------------------------------------\n\n");
            
            // Tabla de tropas disponibles
            setColor(0, 10);
            printf("  TROPAS DISPONIBLES PARA ENTRENAR:\n");
            setColor(0, 15);
            printf("  ===============================================================================\n");
            printf("  | # | Tipo        | Vida | Ataque | Vel.Atq | Alcance | Costo (O/C/M/P)     |\n");
            printf("  |---|-------------|------|--------|---------|---------|---------------------|\n");
            
            // Dibujar cada tropa con resaltado si está seleccionada
            for (i = 0; i < 4; i++) {
                // Si está seleccionada, cambiar fondo
                if (i == seleccion) {
                    setColor(2, 15); // Fondo verde, texto blanco
                    printf("  |>>>");
                } else {
                    setColor(0, 15);
                    printf("  | %d |", i + 1);
                }
                
                if (i == seleccion) setColor(2, 11); else setColor(0, 11);
                printf(" %-11s", nombres[i]);
                
                if (i == seleccion) setColor(2, 15); else setColor(0, 15);
                printf(" | %3d  |  %3d   |   %2d    |   %2d    | ",
                       stats[i][0], stats[i][1], stats[i][2], stats[i][3]);
                
                // Mostrar costos con colores
                if (i == seleccion) setColor(2, 14); else setColor(0, 14);
                printf("%d", costos[i][0]);
                if (i == seleccion) setColor(2, 15); else setColor(0, 15);
                printf("/");
                if (i == seleccion) setColor(2, 12); else setColor(0, 12);
                printf("%d", costos[i][1]);
                if (i == seleccion) setColor(2, 15); else setColor(0, 15);
                printf("/");
                if (i == seleccion) setColor(2, 6); else setColor(0, 6);
                printf("%d", costos[i][2]);
                if (i == seleccion) setColor(2, 15); else setColor(0, 15);
                printf("/");
                if (i == seleccion) setColor(2, 7); else setColor(0, 7);
                printf("%d", costos[i][3]);
                if (i == seleccion) setColor(2, 15); else setColor(0, 15);
                
                if (i == seleccion) {
                    printf("   <<<|\n");
                } else {
                    printf("       |\n");
                }
            }
            
            setColor(0, 15);
            printf("  ===============================================================================\n\n");
            
            // Descripción de las estadísticas
            setColor(0, 8);
            printf("  Vida: Puntos de vida | Ataque: Dano por golpe | Vel.Atq: Velocidad de ataque\n");
            printf("  Alcance: Distancia de ataque | Costo: Oro/Comida/Madera/Piedra\n\n");
            
            // Instrucciones
            setColor(0, 14);
            printf("  ===============================================================================\n");
            printf("  ||  W/S: Mover  ||  ENTER: Entrenar tropa  ||  ESC: Volver al mapa         ||\n");
            printf("  ===============================================================================\n");
            setColor(0, 15);
            
            actualizado = 0; // Ya redibujamos
        }
        
        // Esperar entrada
        if (_kbhit()) {
            tecla = _getch();
            
            // ESC para salir
            if (tecla == 27) {
                break;
            }
            
            // WASD para navegar
            if (tecla == 'w' || tecla == 'W') {
                seleccion--;
                if (seleccion < 0) seleccion = 3;
                actualizado = 1;
            }
            else if (tecla == 's' || tecla == 'S') {
                seleccion++;
                if (seleccion > 3) seleccion = 0;
                actualizado = 1;
            }
            
            // ENTER para entrenar
            else if (tecla == 13) {
                // Verificar si tiene suficientes recursos
                if (j->Oro >= costos[seleccion][0] &&
                    j->Comida >= costos[seleccion][1] &&
                    j->Madera >= costos[seleccion][2] &&
                    j->Piedra >= costos[seleccion][3]) {
                    
                    // Restar recursos
                    j->Oro -= costos[seleccion][0];
                    j->Comida -= costos[seleccion][1];
                    j->Madera -= costos[seleccion][2];
                    j->Piedra -= costos[seleccion][3];
                    
                    // Incrementar tropas
                    j->NumeroTropas++;
                    if (seleccion == 0) j->CantidadEspadas++;
                    else if (seleccion == 1) j->CantidadArqueros++;
                    else if (seleccion == 2) j->CantidadPicas++;
                    else if (seleccion == 3) j->CantidadCaballeria++;
                    
                    actualizado = 1; // Redibujar con nuevos valores
                    
                    // Mensaje de éxito (temporal)
                    setColor(0, 10);
                    printf("\n  >> %s entrenado exitosamente! <<\n", nombres[seleccion]);
                    setColor(0, 15);
                    Sleep(500);
                }
                else {
                    // Mensaje de error (temporal)
                    setColor(0, 12);
                    printf("\n  >> Recursos insuficientes para entrenar %s! <<\n", nombres[seleccion]);
                    setColor(0, 15);
                    Sleep(1000);
                    actualizado = 1;
                }
            }
            
            // Detectar teclas especiales (flechas)
            if (tecla == 224 || tecla == 0) {
                _getch(); // Consumir segundo byte
            }
        }
        
        Sleep(50);
    }
    
    // Restaurar pantalla del juego
    system("cls");
    
    // Redibujar el marco
    setColor(0, 8);
    moverCursor(0, 0); 
    printf("%c", 201);
    for (i = 1; i < COLUMNAS * 2 + 2; i++) printf("%c", 205);
    printf("%c", 187);

    moverCursor(0, FILAS + 1); 
    printf("%c", 200);
    for (i = 1; i < COLUMNAS * 2 + 2; i++) printf("%c", 205);
    printf("%c", 188);

    for (i = 1; i < FILAS + 1; i++) {
        moverCursor(0, i); 
        printf("%c", 186);
        moverCursor(COLUMNAS * 2 + 2, i); 
        printf("%c", 186);
    }
    setColor(0, 15);
    
    // Redibujar mapa
    mostrarMapa(mapa, px, py);
    
    // Redibujar panel (forzado para actualizar tropas)
    forzarRedibujoPanelEnMapa(*j);
    
    // Restaurar mensaje
    moverCursor(0, FILAS + 2);
    setColor(0, 15);
    printf("Usa WASD para moverte. ENTER para interactuar. ESC para salir.");
    
    ocultarCursor();
}
int generarParcelaCultivos(char mapa[MAPA_F][MAPA_C], int casa_x, int casa_y, int ancho, int alto) {
    // ✅ AQUÍ VA EL CÓDIGO DEL PASO 4
    int intentos = 0;
    int max_intentos = 100;
    int px, py, i, j;
    int valido;
    
    while (intentos < max_intentos) {
        px = casa_x + (rand() % 40) - 20;
        py = casa_y + (rand() % 40) - 20;
        
        valido = 1;
        for (i = 0; i < alto && valido; i++) {
            for (j = 0; j < ancho && valido; j++) {
                int nx = px + i;
                int ny = py + j;
                
                if (nx < 0 || nx >= MAPA_F || ny < 0 || ny >= MAPA_C) {
                    valido = 0;
                } else if (mapa[nx][ny] != '.' && !esPuente(nx, ny)) {
                    valido = 0;
                }
            }
        }
        
        if (valido) {
            for (i = 0; i < alto; i++) {
                for (j = 0; j < ancho; j++) {
                    mapa[px + i][py + j] = 'F';
                }
            }
            return 1;
        }
        
        intentos++;
    }
    
    return 0;
}
/* =============================== */
/* Menu de Casa Central           */
/* =============================== */
void mostrarMenuCasaCentral(struct Jugador *j, char mapa[MAPA_F][MAPA_C], int px, int py, int casa_x, int casa_y) {
    int tecla, i;
    int seleccion = 0; // 0=Pequeña, 1=Mediana, 2=Grande
    int actualizado = 1;
    
    // Costos y tamaños de parcelas: {Oro, Comida, Madera, Piedra, Ancho, Alto}
    int parcelas[3][6] = {
        {20, 10, 30, 5,  3, 3},   // Pequeña (3x3)
        {40, 20, 60, 10, 5, 5},   // Mediana (5x5)
        {80, 40, 120, 20, 7, 7}   // Grande (7x7)
    };
    
    char* nombres[3] = {"Pequena", "Mediana", "Grande"};
    
    while (1) {
        if (actualizado) {
            system("cls");
            
            // Marco del menú
            setColor(0, 15);
            printf("\n");
            printf("  ================================================================================\n");
            printf("  ||                       CASA CENTRAL - %s                           ||\n", j->Nombre);
            printf("  ================================================================================\n\n");
            
            // Recursos disponibles
            setColor(0, 14);
            printf("  RECURSOS DISPONIBLES:\n");
            setColor(0, 15);
            printf("  -------------------------------------------------------------------------------\n");
            printf("  ");
            setColor(0, 14); printf("Oro: %d   ", j->Oro);
            setColor(0, 12); printf("Comida: %d   ", j->Comida);
            setColor(0, 6); printf("Madera: %d   ", j->Madera);
            setColor(0, 7); printf("Piedra: %d\n", j->Piedra);
            setColor(0, 15);
            printf("  -------------------------------------------------------------------------------\n\n");
            
            // Tabla de parcelas
            setColor(0, 10);
            printf("  PARCELAS DE CULTIVO DISPONIBLES:\n");
            setColor(0, 15);
            printf("  ===============================================================================\n");
            printf("  | # | Tamano  | Dimension | Cultivos | Costo (O/C/M/P)                      |\n");
            printf("  |---|---------|-----------|----------|--------------------------------------|\n");
            
            // Dibujar cada parcela
            for (i = 0; i < 3; i++) {
                if (i == seleccion) {
                    setColor(2, 15);
                    printf("  |>>>");
                } else {
                    setColor(0, 15);
                    printf("  | %d |", i + 1);
                }
                
                if (i == seleccion) setColor(2, 11); else setColor(0, 11);
                printf(" %-7s", nombres[i]);
                
                if (i == seleccion) setColor(2, 15); else setColor(0, 15);
                printf(" |   %dx%-2d    |   %2d     | ",
                       parcelas[i][4], parcelas[i][5], 
                       parcelas[i][4] * parcelas[i][5]);
                
                // Costos
                if (i == seleccion) setColor(2, 14); else setColor(0, 14);
                printf("%d", parcelas[i][0]);
                if (i == seleccion) setColor(2, 15); else setColor(0, 15);
                printf("/");
                if (i == seleccion) setColor(2, 12); else setColor(0, 12);
                printf("%d", parcelas[i][1]);
                if (i == seleccion) setColor(2, 15); else setColor(0, 15);
                printf("/");
                if (i == seleccion) setColor(2, 6); else setColor(0, 6);
                printf("%d", parcelas[i][2]);
                if (i == seleccion) setColor(2, 15); else setColor(0, 15);
                printf("/");
                if (i == seleccion) setColor(2, 7); else setColor(0, 7);
                printf("%d", parcelas[i][3]);
                if (i == seleccion) setColor(2, 15); else setColor(0, 15);
                
                if (i == seleccion) {
                    printf("                   <<<|\n");
                } else {
                    printf("                      |\n");
                }
            }
            
            setColor(0, 15);
            printf("  ===============================================================================\n\n");
            
            // Descripción
            setColor(0, 8);
            printf("  Las parcelas generan cultivos automaticamente en la isla.\n");
            printf("  A mayor tamano, mas cultivos tendras disponibles para recolectar.\n");
            printf("  Los cultivos aparecen en rectangulos cerca de la Casa Central.\n\n");
            
            // Instrucciones
            setColor(0, 14);
            printf("  ===============================================================================\n");
            printf("  ||  W/S: Mover  ||  ENTER: Comprar parcela  ||  ESC: Volver al mapa        ||\n");
            printf("  ===============================================================================\n");
            setColor(0, 15);
            
            actualizado = 0;
        }
        
        // Esperar entrada
        if (_kbhit()) {
            tecla = _getch();
            
            if (tecla == 27) break; // ESC
            
            // Navegación
            if (tecla == 'w' || tecla == 'W') {
                seleccion--;
                if (seleccion < 0) seleccion = 2;
                actualizado = 1;
            }
            else if (tecla == 's' || tecla == 'S') {
                seleccion++;
                if (seleccion > 2) seleccion = 0;
                actualizado = 1;
            }
            
            // Comprar parcela
            else if (tecla == 13) {
                // Verificar recursos
                if (j->Oro >= parcelas[seleccion][0] &&
                    j->Comida >= parcelas[seleccion][1] &&
                    j->Madera >= parcelas[seleccion][2] &&
                    j->Piedra >= parcelas[seleccion][3]) {
                    
                    // Intentar generar parcela
                    if (generarParcelaCultivos(mapa, casa_x, casa_y, 
                                               parcelas[seleccion][4], 
                                               parcelas[seleccion][5])) {
                        // Restar recursos
                        j->Oro -= parcelas[seleccion][0];
                        j->Comida -= parcelas[seleccion][1];
                        j->Madera -= parcelas[seleccion][2];
                        j->Piedra -= parcelas[seleccion][3];
                        
                        setColor(0, 10);
                        printf("\n  >> Parcela %s creada exitosamente! <<\n", nombres[seleccion]);
                        setColor(0, 15);
                        Sleep(800);
                        actualizado = 1;
                    } else {
                        setColor(0, 12);
                        printf("\n  >> No hay espacio disponible en la isla para esta parcela! <<\n");
                        setColor(0, 15);
                        Sleep(1200);
                        actualizado = 1;
                    }
                }
                else {
                    setColor(0, 12);
                    printf("\n  >> Recursos insuficientes para crear parcela %s! <<\n", nombres[seleccion]);
                    setColor(0, 15);
                    Sleep(1000);
                    actualizado = 1;
                }
            }
            
            if (tecla == 224 || tecla == 0) {
                _getch();
            }
        }
        
        Sleep(50);
    }
    
    // Restaurar pantalla
    system("cls");
    
    // Redibujar marco
    setColor(0, 8);
    moverCursor(0, 0); 
    printf("%c", 201);
    for (i = 1; i < COLUMNAS * 2 + 2; i++) printf("%c", 205);
    printf("%c", 187);

    moverCursor(0, FILAS + 1); 
    printf("%c", 200);
    for (i = 1; i < COLUMNAS * 2 + 2; i++) printf("%c", 205);
    printf("%c", 188);

    for (i = 1; i < FILAS + 1; i++) {
        moverCursor(0, i); 
        printf("%c", 186);
        moverCursor(COLUMNAS * 2 + 2, i); 
        printf("%c", 186);
    }
    setColor(0, 15);
    
    mostrarMapa(mapa, px, py);
    forzarRedibujoPanelEnMapa(*j);
    
    moverCursor(0, FILAS + 2);
    setColor(0, 15);
    printf("Usa WASD para moverte. ENTER para interactuar. ESC para salir.");
    
    ocultarCursor();
}

void dibujarPanelEnMapa(struct Jugador j) {
    static int last_oro = -1;
    static int last_comida = -1;
    static int last_madera = -1;
    static int last_piedra = -1;
    static int last_espadas = -1;
    static int last_arqueros = -1;
    static int last_picas = -1;
    static int last_caballeria = -1;
    
    // Verificar cambios
    int cambios = (j.Oro != last_oro || 
                   j.Comida != last_comida || 
                   j.Madera != last_madera || 
                   j.Piedra != last_piedra ||
                   j.CantidadEspadas != last_espadas ||
                   j.CantidadArqueros != last_arqueros ||
                   j.CantidadPicas != last_picas ||
                   j.CantidadCaballeria != last_caballeria);
    
    if (!cambios) return;
    
    // Actualizar cache
    last_oro = j.Oro;
    last_comida = j.Comida;
    last_madera = j.Madera;
    last_piedra = j.Piedra;
    last_espadas = j.CantidadEspadas;
    last_arqueros = j.CantidadArqueros;
    last_picas = j.CantidadPicas;
    last_caballeria = j.CantidadCaballeria;
    
    // Redibujar usando la función forzada
    forzarRedibujoPanelEnMapa(j);
}
/* =============================== */
/* Limpiar Area de Mensajes       */
/* =============================== */
void limpiarMensajes() {
    int i, j;
    // Limpiar desde FILAS+2 hasta FILAS+10
    for (i = FILAS + 2; i <= FILAS + 10; i++) {
        moverCursor(0, i);
        setColor(0, 0); // Fondo negro
        for (j = 0; j < 120; j++) {
            printf(" ");
        }
    }
    setColor(0, 15); // Restaurar color
}
/* =============================== */
/* Resetear Cache del Panel       */
/* =============================== */
void resetearCachePanelEnMapa() {
    // Forzar a que la proxima llamada a dibujarPanelEnMapa redibuje
    extern void dibujarPanelEnMapa(struct Jugador j);
    
    // Simplemente llamamos a forzarRedibujoPanelEnMapa la próxima vez
    // Esta función resetea las variables estáticas dentro de dibujarPanelEnMapa
}

/* =============================== */
/* Forzar Redibujo del Panel      */
/* =============================== */
void forzarRedibujoPanelEnMapa(struct Jugador j) {
    int panel_x = 64;
    int panel_y = 2;
    int panel_width = 18;
    int i;
    
    // Dibujar fondo negro
    setColor(0, 0);
    for (i = panel_y - 1; i <= panel_y + 11; i++) { // Aumentado para más líneas
        moverCursor(panel_x - 1, i);
        for (int k = 0; k < panel_width + 2; k++) {
            printf(" ");
        }
    }
    
    // Dibujar marco
    setColor(8, 15);
    
    moverCursor(panel_x, panel_y);
    printf("%c", 218);
    for (i = 0; i < panel_width - 2; i++) printf("%c", 196);
    printf("%c", 191);
    
    // Nombre del jugador
    moverCursor(panel_x, panel_y + 1);
    printf("%c%-16s%c", 179, j.Nombre, 179);
    
    // Recursos
    moverCursor(panel_x, panel_y + 2);
    setColor(8, 14);
    printf("%cOro   : %-7d%c", 179, j.Oro, 179);
    
    moverCursor(panel_x, panel_y + 3);
    setColor(8, 12);
    printf("%cComida: %-7d%c", 179, j.Comida, 179);
    
    moverCursor(panel_x, panel_y + 4);
    setColor(8, 6);
    printf("%cMadera: %-7d%c", 179, j.Madera, 179);
    
    moverCursor(panel_x, panel_y + 5);
    setColor(8, 7);
    printf("%cPiedra: %-7d%c", 179, j.Piedra, 179);
    
    // Separador
    moverCursor(panel_x, panel_y + 6);
    setColor(8, 8);
    printf("%c", 195);
    for (i = 0; i < panel_width - 2; i++) printf("%c", 196);
    printf("%c", 180);
    
    // Tropas detalladas
    moverCursor(panel_x, panel_y + 7);
    setColor(8, 11);
    printf("%cEspadas : %-5d%c", 179, j.CantidadEspadas, 179);
    
    moverCursor(panel_x, panel_y + 8);
    printf("%cArqueros: %-5d%c", 179, j.CantidadArqueros, 179);
    
    moverCursor(panel_x, panel_y + 9);
    printf("%cPicas   : %-5d%c", 179, j.CantidadPicas, 179);
    
    moverCursor(panel_x, panel_y + 10);
    printf("%cCaball. : %-5d%c", 179, j.CantidadCaballeria, 179);
    
    // Línea inferior
    moverCursor(panel_x, panel_y + 11);
    setColor(8, 15);
    printf("%c", 192);
    for (i = 0; i < panel_width - 2; i++) printf("%c", 196);
    printf("%c", 217);
    
    setColor(0, 15);
}
