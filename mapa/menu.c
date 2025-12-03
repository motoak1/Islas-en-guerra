#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <windows.h> // Necesario para HANDLE, COORD, SetConsoleTextAttribute, Sleep, etc.
#include <string.h>

// Prototipos para evitar declaraciones implícitas cuando funciones se usan antes
void moverCursor(int x, int y);
void setColor(int fondo, int texto);

// --- CONSTANTES DE DIMENSIONES Y POSICIONAMIENTO ---
#define ANCHO_CONSOLA 80 // Ancho estándar de la consola
#define ALTO_CONSOLA 25  // Alto estándar de la consola

#define INICIO_TITULO_X 24
#define INICIO_MENU_X 31
#define INICIO_Y 12

// --- CONSTANTES DE COLOR (Basadas en la API de Windows) ---
#define COLOR_NEGRO 0
#define COLOR_AZUL_MARINO 1  // Color del Océano
#define COLOR_VERDE_OSCURO 2 // Color de las Islas
#define COLOR_ROJO 4

#define COLOR_AMARILLO 14
#define COLOR_VERDE_CLARO 10 // Color de la selección
#define COLOR_MARRON 6       // Color de la arena
#define COLOR_AMARILLO_CLARO 14
#define COLOR_GRIS_OSCURO 8   // Color gris oscuro

#define COLOR_BLANCO        7 
#define COLOR_GRIS_CLARO    8  // Gris brillante

// Colores del Cielo y Océano
#define COLOR_AZUL_CIELO    11 // CIAN BRILLANTE (Funciona mejor como un azul claro/cielo)
#define COLOR_AZUL_OCEANO   1  // AZUL (El azul oscuro principal del agua)
#define COLOR_AZUL_CLARO    9  // AZUL BRILLANTE (Para agua más clara/transparencia)

// Colores de la Arena
#define COLOR_AMARILLO_ARENA 4  // AMARILLO OSCURO / MARRÓN (Generalmente un marrón rojizo en la paleta ANSI. Más adecuado para arena)
#define COLOR_MARRON_GRANOS  6 


#define COLOR_PURPURA_CIELO 13
#define COLOR_AZUL_PROFUNDO_MAR 9
#define COLOR_ARENA_CLARA 14
#define COLOR_AMARILLO_ARENA_OSCURO 6
#define COLOR_GRIS_OSCURO_ROCA 8
#define COLOR_GRIS_CLARO_MOAI 7
#define COLOR_VERDE_OSCURO 2
#define COLOR_MARRON_TRONCO 6
#define COLOR_VERDE_PALMERA 10


// --- CONSTANTES DE TECLA ---
#define TECLA_ENTER 13
#define TECLA_ESC 27
#define TECLA_ESPECIAL 224 // Prefijo para flechas
#define FLECHA_ARRIBA 72
#define FLECHA_ABAJO 80
#define TECLA_W 'w'
#define TECLA_S 's'

// --- FUNCIONES DE CONSOLA (Implementaciones) ---

/**
 * @brief Oculta el cursor de la consola.
 */
void ocultarCursor() {
    CONSOLE_CURSOR_INFO cursorInfo;
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    cursorInfo.dwSize = 1;
    cursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(hConsole, &cursorInfo);
}

// Tamaño de la consola en tiempo de ejecución
int g_ancho_consola = ANCHO_CONSOLA;
int g_alto_consola = ALTO_CONSOLA;

void actualizarTamConsola() {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) return;
    if (GetConsoleScreenBufferInfo(hOut, &csbi)) {
        int ancho = csbi.srWindow.Right - csbi.srWindow.Left + 1;
        int alto = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
        if (ancho > 0) g_ancho_consola = ancho;
        if (alto > 0) g_alto_consola = alto;
    }
}

// Habilitar procesamiento de secuencias ANSI (Virtual Terminal)
void habilitarProcesamientoANSI() {
#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) return;
    DWORD mode = 0;
    if (!GetConsoleMode(hOut, &mode)) return;
    mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, mode);
}

/**
 * Dibuja un marco de doble línea en el rectángulo dado (x,y,w,h).
 */
void dibujarMarcoArea(int x, int y, int w, int h) {
    int i;
    setColor(COLOR_NEGRO, COLOR_AMARILLO);
    moverCursor(x, y);
    printf("%c", 201); // ╔
    for (i = 1; i < w - 1; ++i) printf("%c", 205); // ═
    printf("%c", 187); // ╗
    // inferior
    moverCursor(x, y + h - 1);
    printf("%c", 200); // ╚
    for (i = 1; i < w - 1; ++i) printf("%c", 205); // ═
    printf("%c", 188); // ╝
    // laterales
    for (i = 1; i < h - 1; ++i) {
        moverCursor(x, y + i);
        printf("%c", 186); // ║
        moverCursor(x + w - 1, y + i);
        printf("%c", 186); // ║
    }
}

/**
 * Limpia (pinta espacios) el área del menú definida por (bx,by,bw,bh).
 * Usaremos esto temporalmente en lugar de renderizar un fondo ANSI.
 */
void limpiarAreaMenu(int bx, int by, int bw, int bh) {
    int r, c;
    // Fondo negro en el área
    setColor(COLOR_NEGRO, COLOR_NEGRO);
    for (r = 0; r < bh; ++r) {
        moverCursor(bx, by + r);
        for (c = 0; c < bw; ++c) {
            printf(" ");
        }
    }
    // Restaurar color de texto por defecto para que las opciones se vean
    setColor(COLOR_NEGRO, COLOR_BLANCO);
}

/**
 * @brief Mueve el cursor a la posición (x, y).
 */
void moverCursor(int x, int y) {
    COORD coord;
    coord.X = x;
    coord.Y = y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

/**
 * @brief Establece los colores de fondo y texto.
 * @param fondo Código de color del fondo.
 * @param texto Código de color del texto.
 */
void setColor(int fondo, int texto) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    // fondo * 16 + texto (Background color << 4 | Foreground color)
    SetConsoleTextAttribute(hConsole, fondo * 16 + texto);
}

// --- NUEVAS FUNCIONES PARA AMBIENTACIÓN ---

/**
 * @brief Dibuja un marco alrededor de toda la consola (80x25).
 * Usa caracteres de doble línea ASCII extendido.
 */
void dibujarMarco() {
    int i;
    // Actualizar al tamaño actual de la consola
    actualizarTamConsola();
    setColor(COLOR_NEGRO, COLOR_AMARILLO); // Marco amarillo sobre fondo negro (o el color que haya en el fondo)

    // Esquinas y líneas horizontales
    moverCursor(0, 0);
    printf("%c", 201); // ╔
    for (i = 1; i < g_ancho_consola - 1; i++) {
        printf("%c", 205); // ═
    }
    printf("%c", 187); // ╗

    moverCursor(0, g_alto_consola - 1);
    printf("%c", 200); // ╚
    for (i = 1; i < g_ancho_consola - 1; i++) {
        printf("%c", 205); // ═
    }
    printf("%c", 188); // ╝

    // Líneas verticales
    for (i = 1; i < g_alto_consola - 1; i++) {
        moverCursor(0, i);
        printf("%c", 186); // ║
        moverCursor(g_ancho_consola - 1, i);
        printf("%c", 186); // ║
    }
}

/**
 * @brief Dibuja el fondo temático de océano e islas.
 * Nota: Esto también funciona como "limpiar pantalla" al pintar todo el fondo.
 */

void dibujarFondo() {
    int x, y;
    // 1. PINTAR TODA LA CONSOLA DE NEGRO
    // Establecemos el color de texto y el color de fondo a COLOR_NEGRO.
    actualizarTamConsola();
    setColor(COLOR_NEGRO, COLOR_NEGRO);

    // Iteramos sobre todas las filas y columnas de la consola actual.
    for (y = 0; y < g_alto_consola; y++) {
        moverCursor(0, y);
        for (x = 0; x < g_ancho_consola; x++) {
            // Imprimimos un espacio (" "), que se colorea con los colores
            // de texto y fondo actualmente establecidos (ambos negros).
            printf(" ");
        }
    }

    // El resto del código que dibujaba el cielo, el océano, la arena, la isla,
    // el Moai y la palmera ha sido ELIMINADO para dejar el fondo totalmente negro
    // y sin elementos.

    // 2. RESTAURAR: Devolver el color del texto a la configuración predeterminada.
    // Usamos COLOR_BLANCO para el texto sobre COLOR_NEGRO para el fondo
    // para que el texto de la aplicación (como el menú) sea visible.
    setColor(COLOR_BLANCO, COLOR_NEGRO);
    moverCursor(0, g_alto_consola - 1); // Mover el cursor al final para no interferir con el texto del menú
}

/**
 * @brief Muestra la pantalla de Instrucciones y espera la entrada del usuario para volver.
 */
void mostrarInstrucciones() {
    // Asegurarnos de usar el tamaño actual y limpiar la pantalla completa
    actualizarTamConsola();
    dibujarFondo(); // limpia toda la consola
    ocultarCursor();

    // Calcular un área centrada igual al área del menú principal
    int menuW = g_ancho_consola - 12;
    int menuH = g_alto_consola - 6;
    if (menuW < 20) menuW = g_ancho_consola;
    if (menuH < 10) menuH = g_alto_consola;
    int menuX = (g_ancho_consola - menuW) / 2;
    int menuY = (g_alto_consola - menuH) / 2;

    // Dibujar marco del mismo tamaño que el menú principal
    dibujarMarcoArea(menuX, menuY, menuW, menuH);

    // Contenido de Instrucciones (centrado dentro del área)
    const char *titulo = "--- INSTRUCCIONES ---";
    const char *lineas[] = {
        "OBJETIVO: Conquistar las islas enemigas.",
        ">>> CONTROLES DE NAVEGACION <<<",
        " - W / Flecha Arriba: Mover seleccion arriba",
        " - S / Flecha Abajo: Mover seleccion abajo",
        " - ENTER: Seleccionar opcion"
    };
    const int numLineas = sizeof(lineas) / sizeof(lineas[0]);

    int titulo_len = (int)strlen(titulo);
    int titulo_print_len = titulo_len + 2; // because we print with surrounding spaces: " %s "
    int innerW = menuW - 2; // espacio interior al marco
    int innerX = menuX + 1;

    // Calcular altura del bloque: título + líneas + prompt (+1 separación)
    int blockHeight = 1 + numLineas + 1 + 1; // title + lines + prompt + extra gap
    int startY = menuY + (menuH - blockHeight) / 2;
    if (startY < menuY + 1) startY = menuY + 1;

    // Título en la primera fila del bloque
    int tituloY = startY;
    int tituloX = innerX + (innerW - titulo_print_len) / 2;
    moverCursor(tituloX, tituloY);
    setColor(COLOR_NEGRO, COLOR_AMARILLO);
    printf(" %s ", titulo);

    // Imprimir líneas centradas dentro del bloque
    setColor(COLOR_NEGRO, COLOR_BLANCO);
    for (int i = 0; i < numLineas; ++i) {
        int len = (int)strlen(lineas[i]);
        int maxInnerWidth = innerW - 2; // dejar un pequeño margen dentro del interior
        int displayLen = len;
        char tmp[1024];
        if (len > maxInnerWidth) {
            int allowed = maxInnerWidth;
            strncpy(tmp, lineas[i], allowed);
            tmp[allowed] = '\0';
            displayLen = allowed;
        } else {
            strcpy(tmp, lineas[i]);
        }
        int x = innerX + (innerW - displayLen) / 2; // centrar dentro del interior real
        int y = startY + 1 + i;
        if (y >= menuY + menuH - 2) break; // no salirse del marco
        moverCursor(x, y);
        printf("%s", tmp);
    }

    // Prompt centrado dentro del marco, justo debajo de las líneas
    const char *prompt = "PRESIONA CUALQUIER TECLA PARA VOLVER";
    int prompt_len = (int)strlen(prompt);
    int promptY = startY + 1 + numLineas + 1; // leave one-line gap
    if (promptY >= menuY + menuH - 1) promptY = menuY + menuH - 2;
    int promptX = innerX + (innerW - prompt_len) / 2;
    moverCursor(promptX, promptY);
    setColor(COLOR_VERDE_OSCURO, COLOR_BLANCO);
    printf("%s", prompt);

    // Esperar entrada y luego volver
    while (!_kbhit()) {
        Sleep(50);
    }
    (void)_getch();
}

/**
 * @brief Desarrolla la interfaz de menú interactiva y visual para el inicio del juego.
 * Implementa navegación por flechas/W/S y selección con ENTER.
 */
void mostrarMenu() {
    // Array de opciones del menú
    const char *opciones[] = {
        "1.\tJUGAR (START NEW GAME)",
        "2.\tINSTRUCCIONES (HELP)",
        "3.\tSALIR (EXIT GAME)"
    };
    const int numOpciones = sizeof(opciones) / sizeof(opciones[0]);
    int opcionSeleccionada = 0; // Índice actual (0, 1, 2)
    char tecla = 0;
    int menuW, menuH, menuX, menuY;
    int optionX[10];
    int optionY[10];
    char optionBuf[10][256];
    int spacing = 2;
    
    // ==========================================================
    // === PASO 1: Dibujar Elementos ESTÁTICOS (FUERA del bucle)
    // ==========================================================
    dibujarFondo(); // Se dibuja una sola vez.
        // ==========================================================
        // === PASO 1: Dibujar Elementos ESTÁTICOS (FUERA del bucle)
        // ==========================================================
    // Actualizar tamaño y calcular área del menú centrada
    actualizarTamConsola();
    // Área del menú: dejar margen de 6 cols/rows alrededor
    menuW = g_ancho_consola - 12;
    menuH = g_alto_consola - 6;
    if (menuW < 20) menuW = g_ancho_consola; // fallback
    if (menuH < 10) menuH = g_alto_consola; // fallback
    menuX = (g_ancho_consola - menuW) / 2;
    menuY = (g_alto_consola - menuH) / 2;

        // Limpiar pantalla y dibujar marco del menú
        dibujarFondo();
        dibujarMarcoArea(menuX, menuY, menuW, menuH);
        ocultarCursor();

    // (Temporal) limpiar el área interior del menú en lugar de cargar fondo ANSI
    limpiarAreaMenu(menuX + 1, menuY + 1, menuW - 2, menuH - 2);

        // --- Dibujar Título centrado dentro del área del menú ---
        const char *titulo = "ISLAS EN GUERRA";
        int titulo_len = (int)strlen(titulo);
        int tituloX = menuX + (menuW - titulo_len) / 2;
        int tituloY = menuY + 1;
        moverCursor(tituloX, tituloY);
        setColor(COLOR_NEGRO, COLOR_BLANCO);
        printf("%s", titulo);
    
    // ------------------------------
    
    // ==========================================================
    // === PASO 1.5: Dibujar TODAS las opciones una vez (Inicial)
    // ==========================================================
    // Calcular posiciones de las opciones dentro del área del menú
    actualizarTamConsola();
    menuW = g_ancho_consola - 12;
    menuH = g_alto_consola - 6;
    if (menuW < 20) menuW = g_ancho_consola;
    if (menuH < 10) menuH = g_alto_consola;
    menuX = (g_ancho_consola - menuW) / 2;
    menuY = (g_alto_consola - menuH) / 2;

    int totalH = (numOpciones - 1) * spacing + 1;
    int opcionesStartY = menuY + (menuH - totalH) / 2;
    setColor(COLOR_NEGRO, COLOR_BLANCO);
    for (int i = 0; i < numOpciones; i++) {
        snprintf(optionBuf[i], sizeof(optionBuf[i]), "   %s   ", opciones[i]);
        int len = (int)strlen(optionBuf[i]);
        optionX[i] = menuX + (menuW - len) / 2;
        optionY[i] = opcionesStartY + i * spacing;
        moverCursor(optionX[i], optionY[i]);
        printf("%s", optionBuf[i]);
    }
    
    // Variable para saber qué opción estaba seleccionada antes del cambio
    int opcionAnterior = -1; // Mantener para la lógica de redibujo dinámico

    // ==========================================================
    // === PASO 2: Bucle Principal (Solo Actualiza Dinámicos)
    // ==========================================================
    while (1) {
        // --- Redibujar SOLO SI la opción seleccionada ha cambiado ---
        // Este bloque ahora solo se encarga de cambiar el formato (resaltado/normal)
        // Si la consola cambió de tamaño, redibujar todo el área del menú
        static int prevW = 0, prevH = 0;
        actualizarTamConsola();
        if (g_ancho_consola != prevW || g_alto_consola != prevH) {
            prevW = g_ancho_consola; prevH = g_alto_consola;
            // recalcular área y volver a pintar fondo, marco, imagen y opciones
            menuW = g_ancho_consola - 12; if (menuW < 20) menuW = g_ancho_consola;
            menuH = g_alto_consola - 6; if (menuH < 10) menuH = g_alto_consola;
            menuX = (g_ancho_consola - menuW) / 2;
            menuY = (g_alto_consola - menuH) / 2;
            dibujarFondo();
            dibujarMarcoArea(menuX, menuY, menuW, menuH);
            // (Temporal) limpiar el área interior del menú en lugar de cargar fondo ANSI
            limpiarAreaMenu(menuX + 1, menuY + 1, menuW - 2, menuH - 2);
            // redraw title
            moverCursor(menuX + (menuW - (int)strlen("ISLAS EN GUERRA")) / 2, menuY + 1);
            setColor(COLOR_NEGRO, COLOR_BLANCO);
            printf("%s", "ISLAS EN GUERRA");
            // recalc options positions
            totalH = (numOpciones - 1) * spacing + 1;
            opcionesStartY = menuY + (menuH - totalH) / 2;
            setColor(COLOR_NEGRO, COLOR_BLANCO);
            for (int i = 0; i < numOpciones; i++) {
                int len = (int)strlen(optionBuf[i]);
                optionX[i] = menuX + (menuW - len) / 2;
                optionY[i] = opcionesStartY + i * spacing;
                moverCursor(optionX[i], optionY[i]);
                printf("%s", optionBuf[i]);
            }
        }

        if (opcionSeleccionada != opcionAnterior) {
            
            // 1. Dibujar la opción ANTERIOR con formato normal (si es que existe)
            if (opcionAnterior != -1) {
                moverCursor(optionX[opcionAnterior], optionY[opcionAnterior]);
                // Restaurar colores para opciones no seleccionadas
                setColor(COLOR_NEGRO, COLOR_BLANCO);
                printf("%s", optionBuf[opcionAnterior]);
            }

            // 2. Dibujar la opción ACTUAL con formato de selección
            moverCursor(optionX[opcionSeleccionada], optionY[opcionSeleccionada]);
            // Colores para la opción seleccionada
            setColor(COLOR_VERDE_CLARO, COLOR_NEGRO);
            char selBuf[256];
            snprintf(selBuf, sizeof(selBuf), " >>>> %s <<<< ", opciones[opcionSeleccionada]);
            int selX = menuX + (menuW - (int)strlen(selBuf)) / 2;
            moverCursor(selX, optionY[opcionSeleccionada]);
            printf("%s", selBuf);
            
            // Actualizar la opción anterior para la siguiente iteración
            opcionAnterior = opcionSeleccionada;
        }
        // ---------------------------------

        // --- Procesar Entrada del Usuario ---
        while (!_kbhit()) {
            Sleep(50); // Pausa para reducir el uso de CPU
        }

        tecla = _getch();

        // 1. Manejo de Flechas (Requiere doble _getch)
        if (tecla == 0 || tecla == TECLA_ESPECIAL) {
            tecla = _getch(); // Captura la segunda parte del código

            if (tecla == FLECHA_ARRIBA) {
                opcionSeleccionada = (opcionSeleccionada - 1 + numOpciones) % numOpciones;
            } else if (tecla == FLECHA_ABAJO) {
                opcionSeleccionada = (opcionSeleccionada + 1) % numOpciones;
            }
            continue; // Vuelve al inicio del bucle para redibujar
        }

        // 2. Manejo de W/S y ENTER
        switch (tecla) {
            case TECLA_W: // 'w'
            case 'W':
                opcionSeleccionada = (opcionSeleccionada - 1 + numOpciones) % numOpciones;
                break;

            case TECLA_S: // 's'
            case 'S':
                opcionSeleccionada = (opcionSeleccionada + 1) % numOpciones;
                break;

            case TECLA_ENTER: // [ENTER]
                // ... (La lógica de ENTER sigue igual) ...
                // 1. Jugar
                if (opcionSeleccionada == 0) {
                    system("cls"); // Limpia la pantalla para empezar el juego
                    setColor(COLOR_NEGRO, COLOR_BLANCO);
                    return; // Sale de la función para continuar con el juego.
                }
                // 2. Instrucciones
                else if (opcionSeleccionada == 1) {
                    mostrarInstrucciones();
                    // Importante: después de Instrucciones, forzar redibujo del menú (dinámico)
                    actualizarTamConsola();
                    menuW = g_ancho_consola - 12; if (menuW < 20) menuW = g_ancho_consola;
                    menuH = g_alto_consola - 6; if (menuH < 10) menuH = g_alto_consola;
                    menuX = (g_ancho_consola - menuW) / 2;
                    menuY = (g_alto_consola - menuH) / 2;
                    dibujarFondo();
                    dibujarMarcoArea(menuX, menuY, menuW, menuH);
                    // (Temporal) limpiar el área interior del menú en lugar de cargar fondo ANSI
                    limpiarAreaMenu(menuX + 1, menuY + 1, menuW - 2, menuH - 2);
                    // redibujar titulo
                    moverCursor(menuX + (menuW - (int)strlen("ISLAS EN GUERRA")) / 2, menuY + 1);
                    setColor(COLOR_NEGRO, COLOR_BLANCO);
                    printf("%s", "ISLAS EN GUERRA");
                    // redibujar opciones
                    totalH = (numOpciones - 1) * spacing + 1;
                    opcionesStartY = menuY + (menuH - totalH) / 2;
                    setColor(COLOR_NEGRO, COLOR_BLANCO);
                    for (int i = 0; i < numOpciones; i++) {
                        int len = (int)strlen(optionBuf[i]);
                        optionX[i] = menuX + (menuW - len) / 2;
                        optionY[i] = opcionesStartY + i * spacing;
                        moverCursor(optionX[i], optionY[i]);
                        printf("%s", optionBuf[i]);
                    }
                    opcionAnterior = -1; // Fuerza que la opción seleccionada se redibuje con el resalte.
                }
                // 3. Salir
                else if (opcionSeleccionada == 2) {
                    system("cls");
                    setColor(COLOR_NEGRO, COLOR_BLANCO);
                    printf("Gracias por jugar a Islas en Guerra!\n");
                    exit(0); // Sale del programa inmediatamente.
                }
                break;
        }
    }
}
