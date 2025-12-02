#ifndef MENU_H
#define MENU_H

// --- CONSTANTES DE COLOR (Basadas en la API de Windows) ---
// Estos valores corresponden a los códigos estándar de color de consola.
#define COLOR_NEGRO 0
#define COLOR_AZUL_MARINO 1
#define COLOR_VERDE_OSCURO 2
#define COLOR_ROJO 4
#define COLOR_AZUL_CLARO 3 // Nuevo color para el agua
#define COLOR_BLANCO 15
#define COLOR_AMARILLO 14
#define COLOR_VERDE_CLARO 10
#define COLOR_GRIS 8 // Color para el borde

// --- CONSTANTES DE TECLA ---
// Códigos ASCII y extendidos para la gestión de entrada con _getch()
#define TECLA_ENTER 13
#define TECLA_ESC 27
#define TECLA_ESPECIAL 224 // Prefijo para teclas de función/flecha
#define FLECHA_ARRIBA 72
#define FLECHA_ABAJO 80
#define TECLA_W 'w'
#define TECLA_S 's'

// --- COORDENADAS PARA CENTRADO DE LA VENTANA (Asumiendo 80x25) ---
// Dimensiones del marco del menú: Ancho 38, Alto 14
#define ANCHO_CONSOLA 80
#define ALTO_CONSOLA 25

#define INICIO_BORDE_X 21 // Columna donde empieza el marco (centrado en 80)
#define INICIO_BORDE_Y 6  // Fila donde empieza el marco

#define INICIO_TITULO_X 26 // Columna de inicio del título dentro del marco
#define INICIO_MENU_X 27   // Columna de inicio de las opciones

// --- PROTOTIPOS DE FUNCIONES DE UTILIDAD DE CONSOLA ---

/**
 * @brief Oculta el cursor de la consola.
 */
void ocultarCursor();

/**
 * @brief Mueve el cursor a la posición (x, y).
 * @param x Coordenada X (columna).
 * @param y Coordenada Y (fila).
 */
void moverCursor(int x, int y);

/**
 * @brief Establece los colores de fondo y texto de la consola.
 * @param fondo Código de color del fondo.
 * @param texto Código de color del texto.
 */
void setColor(int fondo, int texto);

// --- PROTOTIPOS DE FUNCIONES DEL JUEGO ---

/**
 * @brief Dibuja un fondo de mapa estilizado con colores y caracteres ASCII.
 */
void dibujarFondo();

/**
 * @brief Muestra la pantalla de Instrucciones del juego.
 */
void mostrarInstrucciones();

/**
 * @brief Muestra el menú principal interactivo del juego.
 * Permite la navegación y selección de opciones (Jugar, Instrucciones, Salir).
 */
void mostrarMenu();

#endif // MENU_H
