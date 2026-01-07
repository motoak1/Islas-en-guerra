#ifndef MAPA_H
#define MAPA_H

#include "../recursos/recursos.h"
#include "../recursos/ui_compra.h"
#include <stdio.h>
#include <windows.h>

// Forward declaration de las estructuras
struct MenuCompra;
typedef struct MenuEmbarque MenuEmbarque;

// --- CONSTANTES DE DIMENSIÓN ---
#define MAPA_SIZE 2048
#define TILE_SIZE                                                              \
  64 // Tamaño lógico (celda de matriz) - coincide con sprites 64x64
#define GRID_SIZE (MAPA_SIZE / TILE_SIZE) // 2048/64 = 32x32 celdas
#define SPRITE_ARBOL 128                  // Tamaño visual del BMP de árbol

// ============================================================================
// SIMBOLOGÍA DE OBJETOS EN EL MAPA (REQUISITO ACADÉMICO)
// ============================================================================
#define SIMBOLO_AGUA '~'      // Agua (no transitable)
#define SIMBOLO_VACIO '.'     // Terreno vacío (tierra transitable)
#define SIMBOLO_ARBOL 'A'     // Árbol (obstáculo)
#define SIMBOLO_OBRERO 'O'    // Obrero (worker)
#define SIMBOLO_CABALLERO 'C' // Caballero (cavalry)
#define SIMBOLO_GUERRERO 'G'  // Guerrero (warrior)
#define SIMBOLO_VACA 'V'      // Vaca (recurso móvil)
#define SIMBOLO_BARCO 'B'     // Barco
#define SIMBOLO_EDIFICIO 'E'  // Edificio (ayuntamiento)
#define SIMBOLO_MINA 'M'      // Mina
#define SIMBOLO_CUARTEL 'Q'   // Cuartel (barracks)
#define SIMBOLO_RECURSO '$'   // Recurso (para futuro uso)
#define SIMBOLO_ENEMIGO 'X'   // Enemigo pasivo en isla
#define SIMBOLO_JUGADOR 'P'   // Posición base del jugador
// ============================================================================

// Matriz lógica de objetos (accesible desde otros módulos)
extern char mapaObjetos[GRID_SIZE][GRID_SIZE];

typedef struct {
  int x;      // Posicion X en el mapa 2048
  int y;      // Posicion Y en el mapa 2048
  float zoom; // Nivel de zoom
} Camara;

// --- SISTEMA DE VACAS DINÁMICAS ---
// Estructura para vacas con movimiento automático
typedef struct {
  float x, y;          // Posición en píxeles (0-2048)
  Direccion dir;       // DIR_FRONT, DIR_BACK, DIR_LEFT, DIR_RIGHT
  int timerMovimiento; // Contador de frames hasta próximo movimiento
} Vaca;

// --- COLISIONES / GRID (matriz dinámica con punteros) ---
// Retorna una matriz GRID_SIZE x GRID_SIZE (int**) donde 1 = ocupado.
int **mapaObtenerCollisionMap(void);
// Reconstruye la matriz en base a los árboles registrados en mapaObjetos.
void mapaReconstruirCollisionMap(void);
// Marca un edificio en el collision map como impasable
void mapaMarcarEdificio(float x, float y, int ancho, int alto);
// Desmarca un edificio en el collision map (cuando se destruye o explota)
void mapaDesmarcarEdificio(float x, float y, int ancho, int alto);
// Detecta automáticamente una posición válida en la orilla del mapa
void mapaDetectarOrilla(float *outX, float *outY, int *outDir);
// Detecta automáticamente una posición válida en la orilla del mapa
void mapaDetectarOrilla(float *outX, float *outY, int *outDir);
// Libera la memoria del collisionMap dinámico.
void mapaLiberarCollisionMap(void);

// --- FUNCIONES REQUERIDAS POR ESPECIFICACIÓN ACADÉMICA ---
// Inicializa la matriz de caracteres con terreno vacío
void inicializarMapa(char mapa[GRID_SIZE][GRID_SIZE]);
// Muestra el mapa en la consola (debug/demo)
void mostrarMapa(char mapa[GRID_SIZE][GRID_SIZE]);
// Permite explorar celdas del mapa (con validación de límites)
void explorarMapa(char mapa[GRID_SIZE][GRID_SIZE]);
// Obtiene el contenido de una celda usando punteros
char obtenerContenidoCelda(char *celda);

// --- FUNCIONES DE SINCRONIZACIÓN ---
// Registra un objeto en mapaObjetos (conversión píxeles -> celda)
void mapaRegistrarObjeto(float pixelX, float pixelY, char simbolo);
// Mueve un objeto en mapaObjetos (limpia celda vieja, marca celda nueva)
void mapaMoverObjeto(float viejoX, float viejoY, float nuevoX, float nuevoY,
                     char simbolo);
// Verifica si una celda está ocupada
bool mapaEstaOcupada(int fila, int columna);
// Limpia una celda (marca como vacía)
void mapaLimpiarCelda(int fila, int columna);
// Habilita o deshabilita la generación automática de árboles/vacas en
// cargarRecursosGraficos
void mapaSetGenerarRecursos(bool habilitar);

// --- FUNCIONES DE VACAS ---
// Actualiza la posición de las vacas (movimiento automático)
void mapaActualizarVacas(void);
// Obtiene el array de vacas para renderizado
Vaca *mapaObtenerVacas(int *cantidad);

// Dibuja el mundo (terreno, árboles, obreros) en el DC especificado
// Ahora acepta el menú para dibujarlo dentro del mismo buffer (evitar parpadeo)
// highlightFila/Col: celda a resaltar (-1 = ninguna)
void dibujarMundo(HDC hdc, RECT rect, Camara cam, struct Jugador *pJugador,
                  struct MenuCompra *menu, MenuEmbarque *menuEmb,
                  int highlightFila, int highlightCol);

// Dibuja vista de mapa global (solo mapa y barco, sin zoom)
void dibujarMapaGlobal(HDC hdc, RECT rect, struct Jugador *pJugador);

void cargarRecursosGraficos();
void dibujarObreros(HDC hdcBuffer, struct Jugador *j, Camara cam, int anchoP,
                    int altoP);
void mapaSeleccionarIsla(int isla);

// Nuevas funciones para interacción con recursos
int mapaObtenerTipoObjeto(int f, int c);
void mapaEliminarObjeto(int f, int c);
int mapaContarArboles(void);
void mapaActualizarArboles(void);

// Verifica si una celda es tierra transitable (no agua) usando colisiones y
// color del mapa
bool mapaCeldaEsTierra(int fila, int col);

// Persistencia de estado por isla (memoria)
void mapaGuardarEstadoIsla(
    int isla); // Almacena mapaObjetos, colisiones y vacas de la isla actual
void mapaRestaurarEstadoIsla(
    int isla); // Restaura el estado previamente guardado

// Serialización
void mapaGuardar(FILE *f);
void mapaCargar(FILE *f);

// Nuevas funciones para interacción con recursos
int mapaObtenerTipoObjeto(int f, int c);
void mapaEliminarObjeto(int f, int c);

// Serialización
void mapaGuardar(FILE *f);
void mapaCargar(FILE *f);

#endif