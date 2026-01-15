// guardado/guardado.h
// Sistema de guardado y carga de partidas en archivos binarios
// Guardado único por nombre de jugador
#ifndef GUARDADO_H
#define GUARDADO_H

#include "../edificios/edificios.h"
#include "../mapa/mapa.h"
#include "../recursos/navegacion.h"
#include "../recursos/recursos.h"
#include <stdbool.h>
#include <windows.h>


// ============================================================================
// CONSTANTES DEL SISTEMA DE GUARDADO
// ============================================================================
#define SAVE_MAGIC 0x49534C41          // "ISLA" en hex (identificador de archivo)
#define SAVE_VERSION 2                 // Versión del formato de guardado
#define SAVE_FOLDER "../saves"            // Carpeta donde se guardan las partidas
#define SAVE_EXTENSION ".isla"         // Extensión de archivos de guardado
#define MAX_PARTIDAS 10                // Máximo de partidas guardadas a mostrar
#define MAX_NOMBRE_JUGADOR 30          // Longitud máxima del nombre

// ============================================================================
// ESTRUCTURA DE CABECERA DEL ARCHIVO DE GUARDADO
// ============================================================================
typedef struct {
  unsigned int magic;                     // Identificador mágico (SAVE_MAGIC)
  unsigned int version;                   // Versión del formato
  char timestamp[32];                     // Fecha y hora del guardado
  char nombreJugador[MAX_NOMBRE_JUGADOR]; // Nombre del jugador
  int islaActual;                         // Isla donde está el jugador
} SaveHeader;

// ============================================================================
// ESTRUCTURA PARA GUARDAR UNIDADES (SIN PUNTEROS)
// ============================================================================
typedef struct {
  float x, y;
  float destinoX, destinoY;
  bool moviendose;
  bool seleccionado;
  int dir; // Direccion (guardada como int)
  int frame;
  int objetivoFila;
  int objetivoCol;
  int celdaFila;
  int celdaCol;
  int tipo; // TipoUnidad (guardado como int)
  int vida;
  int vidaMax;
  int damage;
  int critico;
  int defensa;
  int alcance;
} UnidadGuardada;

// ============================================================================
// ESTRUCTURA PARA GUARDAR EDIFICIOS (SIN PUNTEROS)
// ============================================================================
typedef struct {
  int tipo; // TipoEdificio (guardado como int)
  float x, y;
  int ancho, alto;
  bool construido;
  int oroAcumulado;
  int piedraAcumulada;
  int hierroAcumulado;

  // Agregado para sistema de agotamiento
  int oroRestante;
  int piedraRestante;
  int hierroRestante;
  bool agotada;
} EdificioGuardado;

// ============================================================================
// ESTRUCTURA PARA GUARDAR EL BARCO (SIN PUNTEROS)
// ============================================================================
typedef struct {
  float x, y;
  int dir; // Direccion (guardada como int)
  bool activo;
  bool construido; // Si el barco ha sido construido (false = destruido)
  int numTropas;
  int indiceTropas[15]; // Índices de las tropas embarcadas (-1 si vacío)
  int tipoTropas[15];   // Tipo de cada tropa embarcada

  // Agregado para sistema de mejoras
  int nivelMejora;
  int capacidadMaxima;

  // Agregado para sistema de navegación
  bool navegando;
  float destinoX;
  float destinoY;
  float velocidad;
} BarcoGuardado;

// ============================================================================
// ESTRUCTURA PARA GUARDAR VACAS
// ============================================================================
typedef struct {
  float x, y;
  int dir;
  int timerMovimiento;
} VacaGuardada;

// ============================================================================
// ESTRUCTURA PRINCIPAL DE DATOS DE GUARDADO
// ============================================================================
typedef struct {
  SaveHeader header;

  // --- Recursos del jugador ---
  int Comida;
  int Oro;
  int Madera;
  int Piedra;
  int Hierro;

  // --- Unidades ---
  // --- Unidades ---
  UnidadGuardada obreros[MAX_OBREROS];
  UnidadGuardada caballeros[MAX_CABALLEROS];
  UnidadGuardada caballerosSinEscudo[MAX_CABALLEROS_SIN_ESCUDO];
  UnidadGuardada guerreros[MAX_GUERREROS];

  // --- Barco ---
  BarcoGuardado barco;

  // --- Edificios ---
  EdificioGuardado ayuntamiento;
  EdificioGuardado mina;
  EdificioGuardado cuartel;
  bool tieneAyuntamiento;
  bool tieneMina;
  bool tieneCuartel;

  // --- Estado de vista ---
  int vistaActual; // EstadoVista (guardado como int)
  int islaActual;

  // --- Mapa de objetos ---
  char mapaObjetosGuardado[GRID_SIZE][GRID_SIZE];

  // --- Vacas ---
  int numVacas;
  VacaGuardada vacas[10];

  // --- Cámara ---
  int camaraX;
  int camaraY;
  float camaraZoom;

  // --- Persistencia extendida ---
  MapaEstadoSerializable estadosMapa[6];
  EstadoIslaSerializable estadosIsla[6];
  int islaInicial;
  bool islaInicialDefinida;
  int islasConquistadas[6]; // Guardado como int (0/1)

} DatosGuardado;

// ============================================================================
// ESTRUCTURA PARA INFORMACIÓN DE PARTIDAS GUARDADAS
// ============================================================================
typedef struct {
  bool existe;                            // Si la partida existe
  char nombreJugador[MAX_NOMBRE_JUGADOR]; // Nombre del jugador
  char timestamp[32];                     // Fecha del guardado
  int islaActual;                         // Isla donde estaba
  char rutaArchivo[256];                  // Ruta completa del archivo
} InfoPartida;

// ============================================================================
// FUNCIONES PÚBLICAS DE GUARDADO/CARGA
// ============================================================================

// Guarda la partida con el nombre del jugador especificado
bool guardarPartidaPorNombre(const char *nombreJugador, struct Jugador *j,
                             Camara *cam);

// Carga una partida dado el nombre del jugador
bool cargarPartidaPorNombre(const char *nombreJugador, struct Jugador *j,
                            Camara *cam, Edificio *ayuntamiento, Edificio *mina,
                            Edificio *cuartel);

// Obtiene lista de todas las partidas guardadas
int obtenerPartidasGuardadas(InfoPartida partidas[MAX_PARTIDAS]);

// Verifica si existe una partida con ese nombre
bool existePartida(const char *nombreJugador);

// Elimina una partida por nombre
bool eliminarPartida(const char *nombreJugador);

// Obtiene la ruta del archivo de guardado para un nombre
void obtenerRutaGuardado(const char *nombreJugador, char *ruta, int maxLen);

// ============================================================================
// MENÚ DE PAUSA EN PANTALLA (GDI) - Sin parpadeo
// ============================================================================

// Modos del menú de pausa
typedef enum {
  MODO_PRINCIPAL = 0,      // Menú principal de pausa
  MODO_GUARDAR = 1,        // Pantalla de guardar (pide nombre)
  MODO_CARGAR = 2,         // Pantalla de cargar (lista partidas)
  MODO_CONFIRMAR_SALIR = 3, // Confirmación antes de salir
  MODO_NUEVA_PARTIDA = 10   // Ingreso de nombre para nueva partida
} ModoPausa;

// Estado del menú de pausa
typedef struct {
  bool activo;    // Si el menú está visible
  int seleccion;  // Opción seleccionada
  ModoPausa modo; // Modo actual del menú

  // Para entrada de nombre al guardar
  char nombreInput[MAX_NOMBRE_JUGADOR]; // Nombre que está escribiendo
  int cursorPos;                        // Posición del cursor en el nombre
  bool nombreExiste;                    // Si el nombre ya existe

  // Lista de partidas para cargar
  InfoPartida partidas[MAX_PARTIDAS];
  int numPartidas;         // Cantidad de partidas encontradas
  int partidaSeleccionada; // Partida seleccionada para cargar

  // Mensajes de estado
  char mensaje[128];      // Mensaje de estado
  int timerMensaje;       // Frames restantes para mostrar mensaje
  char rutaGuardado[256]; // Ruta donde se guardó (para mostrar)

  // Flag para indicar que debe volver al menú principal
  bool volverAlMenu;

  // Flag para indicar que la partida fue guardada en esta sesión
  bool partidaGuardada;
} MenuPausa;

// Inicializa el menú de pausa
void menuPausaInicializar(MenuPausa *menu);

// Dibuja el menú de pausa en un buffer para evitar parpadeo
void menuPausaDibujar(HDC hdcBuffer, RECT rect, MenuPausa *menu);

// Procesa teclas del menú de pausa (retorna true si consumió la tecla)
bool menuPausaProcesarTecla(MenuPausa *menu, WPARAM tecla, struct Jugador *j,
                            Camara *cam, Edificio *ayuntamiento, Edificio *mina,
                            Edificio *cuartel);

// Procesa caracteres para entrada de texto (WM_CHAR)
bool menuPausaProcesarCaracter(MenuPausa *menu, WPARAM caracter);

// Actualiza el menú de pausa (timers de mensajes, etc.)
void menuPausaActualizar(MenuPausa *menu);

// Abre el menú de pausa
void menuPausaAbrir(MenuPausa *menu);

// Cierra el menú de pausa
void menuPausaCerrar(MenuPausa *menu);

#endif // GUARDADO_H
