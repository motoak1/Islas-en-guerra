#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <windows.h> // Necesario para HANDLE, COORD, SetConsoleTextAttribute, Sleep, etc.

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
    setColor(COLOR_NEGRO, COLOR_AMARILLO); // Marco amarillo sobre fondo negro (o el color que haya en el fondo)

    // Esquinas y líneas horizontales
    moverCursor(0, 0);
    printf("%c", 201); // ╔
    for (i = 1; i < ANCHO_CONSOLA - 1; i++) {
        printf("%c", 205); // ═
    }
    printf("%c", 187); // ╗

    moverCursor(0, ALTO_CONSOLA - 1);
    printf("%c", 200); // ╚
    for (i = 1; i < ANCHO_CONSOLA - 1; i++) {
        printf("%c", 205); // ═
    }
    printf("%c", 188); // ╝

    // Líneas verticales
    for (i = 1; i < ALTO_CONSOLA - 1; i++) {
        moverCursor(0, i);
        printf("%c", 186); // ║
        moverCursor(ANCHO_CONSOLA - 1, i);
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
    setColor(COLOR_NEGRO, COLOR_NEGRO);

    // Iteramos sobre todas las filas y columnas de la consola.
    for (y = 0; y < ALTO_CONSOLA; y++) {
        moverCursor(0, y);
        for (x = 0; x < ANCHO_CONSOLA; x++) {
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
    moverCursor(0, ALTO_CONSOLA - 1); // Mover el cursor al final para no interferir con el texto del menú
}

/**
 * @brief Muestra la pantalla de Instrucciones y espera la entrada del usuario para volver.
 */
void mostrarInstrucciones() {
    dibujarFondo(); // Dibuja el fondo
    dibujarMarco(); // Dibuja el marco
    ocultarCursor();

    moverCursor(INICIO_TITULO_X, 6);
    printf("||       --- INSTRUCCIONES ---   ||");

    // Contenido de Instrucciones
    setColor(COLOR_NEGRO, COLOR_BLANCO); // Texto Blanco sobre fondo (transparente)
    moverCursor(INICIO_MENU_X - 10, 10);
    printf("OBJETIVO: Conquistar las islas enemigas. ");

    moverCursor(INICIO_MENU_X - 10, 12);
    printf(">>> CONTROLES DE NAVEGACION <<<");
    moverCursor(INICIO_MENU_X - 10, 13);
    printf(" - W / Flecha Arriba: Mover seleccion arriba");
    moverCursor(INICIO_MENU_X - 10, 14);
    printf(" - S / Flecha Abajo: Mover seleccion abajo");
    moverCursor(INICIO_MENU_X - 10, 15);
    printf(" - ENTER: Seleccionar opcion");


    // Mensaje de retorno
    moverCursor(INICIO_MENU_X - 12, 19);
    setColor(COLOR_VERDE_OSCURO, COLOR_BLANCO); // Texto Blanco sobre fondo Verde Oscuro
    printf(" PRESIONA CUALQUIER TECLA PARA VOLVER ");

    // Esperar entrada
    while (!_kbhit()) {
        Sleep(50); // Pequeña pausa
    }
    _getch(); // Captura la tecla
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
    
    // ==========================================================
    // === PASO 1: Dibujar Elementos ESTÁTICOS (FUERA del bucle)
    // ==========================================================
    dibujarFondo(); // Se dibuja una sola vez.
    dibujarMarco(); // Se dibuja una sola vez.
    ocultarCursor();
    
    // --- Dibujar Título y Marco con Arte ASCII Simple ---
    // Colores para el título: Blanco sobre el color del Océano (Azul Marino)

    moverCursor(INICIO_TITULO_X, INICIO_Y - 7);
    printf("       ISLAS EN GUERRA       ");
    
    // ------------------------------
    
    // ==========================================================
    // === PASO 1.5: Dibujar TODAS las opciones una vez (Inicial)
    // ==========================================================
    // Dibujar todas las opciones inicialmente con el formato NO seleccionado.
    // La opción '0' se redibujará inmediatamente después con el formato de selección.
    setColor(COLOR_NEGRO, COLOR_BLANCO);
    for (int i = 0; i < numOpciones; i++) {
        moverCursor(INICIO_MENU_X-10, INICIO_Y + i * 2);
        printf("       %s       ", opciones[i]);
    }
    
    // Variable para saber qué opción estaba seleccionada antes del cambio
    int opcionAnterior = -1; // Mantener para la lógica de redibujo dinámico

    // ==========================================================
    // === PASO 2: Bucle Principal (Solo Actualiza Dinámicos)
    // ==========================================================
    while (1) {
        // --- Redibujar SOLO SI la opción seleccionada ha cambiado ---
        // Este bloque ahora solo se encarga de cambiar el formato (resaltado/normal)
        if (opcionSeleccionada != opcionAnterior) {
            
            // 1. Dibujar la opción ANTERIOR con formato normal (si es que existe)
            if (opcionAnterior != -1) {
                moverCursor(INICIO_MENU_X-10, INICIO_Y + opcionAnterior * 2);
                // Restaurar colores para opciones no seleccionadas
                setColor(COLOR_NEGRO, COLOR_BLANCO); 
                printf("       %s       ", opciones[opcionAnterior]); // Quitamos los >>>> y <<<<
            }

            // 2. Dibujar la opción ACTUAL con formato de selección
            moverCursor(INICIO_MENU_X-10, INICIO_Y + opcionSeleccionada * 2);
            // Colores para la opción seleccionada
            setColor(COLOR_VERDE_CLARO, COLOR_NEGRO); 
            printf(" >>>> %s <<<< ", opciones[opcionSeleccionada]);
            
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
                    // Importante: después de Instrucciones, forzar redibujo del menú
                    dibujarFondo(); // Redibuja estáticos que pudo haber borrado Instrucciones
                    dibujarMarco();
                    // Redibujar el título (ya que los colores se cambiaron para el título)
                    // ... (Copiar/pegar el código de dibujo del título aquí) ...
                    // Colores para el título: Blanco sobre el color del Océano (Azul Marino)
                    moverCursor(INICIO_TITULO_X, INICIO_Y - 7);
                    printf("       ISLAS EN GUERRA       ");
                    
                    // ------------------------------
                    
                    // ==========================================================
                    // === PASO 1.5: Dibujar TODAS las opciones una vez (Inicial)
                    // ==========================================================
                    // Dibujar todas las opciones inicialmente con el formato NO seleccionado.
                    // La opción '0' se redibujará inmediatamente después con el formato de selección.
                    setColor(COLOR_NEGRO, COLOR_BLANCO);
                    for (int i = 0; i < numOpciones; i++) {
                        moverCursor(INICIO_MENU_X-10, INICIO_Y + i * 2);
                        printf("       %s       ", opciones[i]);
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
// /**
//  * @brief Función principal del programa.
//  */
// int main() {
//     // Establecer la ventana de la consola si es necesario (ej. para forzar 80x25)
//     // Se comenta ya que podría interferir en algunos entornos.
//     /*
//     char command[50];
//     sprintf(command, "mode con: cols=%d lines=%d", ANCHO_CONSOLA, ALTO_CONSOLA);
//     system(command);
//     */

//     mostrarMenu();

//     // Simulación del inicio del juego
//     setColor(COLOR_NEGRO, COLOR_BLANCO);
//     moverCursor(ANCHO_CONSOLA / 2 - 15, ALTO_CONSOLA / 2);
//     printf("JUEGO INICIADO! (Presiona ESC para salir)");

//     // Espera hasta que el usuario presione ESC
//     while (_getch() != TECLA_ESC) {
//         // Bucle de juego
//     }

//     // Limpieza final
//     system("cls");
//     setColor(COLOR_NEGRO, COLOR_BLANCO);
//     return 0;
// }