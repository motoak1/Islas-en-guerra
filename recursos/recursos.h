#ifndef RECURSOS_H
#define RECURSOS_H
#include <stdbool.h>
#include <windows.h> // Necesario para COORD en gotoxy

// --- CONSTANTES DE STATS ---
#define CABALLERO_VIDA 150
#define CABALLERO_DANO 35
#define CABALLERO_CRITICO 15
#define CABALLERO_DEFENSA 25

#define CABALLERO_SIN_ESCUDO_VIDA 135
#define CABALLERO_SIN_ESCUDO_DANO 35
#define CABALLERO_SIN_ESCUDO_CRITICO 20
#define CABALLERO_SIN_ESCUDO_DEFENSA 20

#define GUERRERO_VIDA 120
#define GUERRERO_DANO 50
#define GUERRERO_CRITICO 30
#define GUERRERO_DEFENSA 15

#define COSTO_MEJORA_BARCO_2_ORO 100
#define COSTO_MEJORA_BARCO_2_MADERA 250
#define COSTO_MEJORA_BARCO_2_PIEDRA 150
#define COSTO_MEJORA_BARCO_2_HIERRO 300

#define COSTO_MEJORA_BARCO_3_ORO 250
#define COSTO_MEJORA_BARCO_3_MADERA 550
#define COSTO_MEJORA_BARCO_3_PIEDRA 350
#define COSTO_MEJORA_BARCO_3_HIERRO 600

#define COSTO_MEJORA_BARCO_4_ORO 500
#define COSTO_MEJORA_BARCO_4_MADERA 1000
#define COSTO_MEJORA_BARCO_4_PIEDRA 600
#define COSTO_MEJORA_BARCO_4_HIERRO 1200

#define COSTO_CURACION 100
#define CANTIDAD_CURACION 25
#define OBRERO_VIDA_MAX 100
#define RANGO_GOLPE_MELEE 64.0f

// --- ESTRUCTURAS ---

struct Tropa {
  char Nombre[30];
  int CostoOro;
  int CostoComida;
  int CostoMadera;
  int CostoPiedra;
  int Vida;
  int Fuerza;
  int VelocidadAtaque;
  int DistanciaAtaque;
};
// Tipo de unidad
typedef enum {
  TIPO_OBRERO,
  TIPO_CABALLERO,
  TIPO_CABALLERO_SIN_ESCUDO,
  TIPO_GUERRERO,
  TIPO_BARCO
} TipoUnidad;

// Estados de animación/dirección
typedef enum { DIR_FRONT, DIR_BACK, DIR_LEFT, DIR_RIGHT } Direccion;

// Sistema mínimo de animación (lógica). El render actual usa BMP por dirección;
// aquí mantenemos el estado como punteros a estructuras, como pide la
// especificación.
typedef struct Animation {
  Direccion dir;     // Dirección asociada a la animación
  int frameCount;    // Cantidad de frames (lógico)
  int ticksPerFrame; // Cuántos ticks dura cada frame
} Animation;

typedef struct {
  float x, y;               // Posición exacta en el mapa (píxeles)
  float destinoX, destinoY; // A donde debe ir
  bool moviendose;
  bool seleccionado;
  Direccion dir;
  int frame; // Para animar el caminado

  // --- Movimiento RTS con pathfinding (memoria dinámica) ---
  int objetivoFila; // Celda destino en grid (0..GRID_SIZE-1)
  int objetivoCol;
  int *rutaCeldas; // Ruta como lista de celdas a visitar (fila*GRID_SIZE+col)
  int rutaLen;
  int rutaIdx;

  // --- Sincronización Matriz <-> Mundo ---
  // Celda actualmente ocupada (se actualiza solo cuando cambia de
  // celda por completo).
  int celdaFila;
  int celdaCol;

  // --- Animación por puntero a estado ---
  const Animation *animActual;
  int animTick;

  // --- Tipo de unidad ---
  TipoUnidad tipo; // TIPO_OBRERO o TIPO_CABALLERO

  // --- Atributos de unidad ---
  int vida;
  int vidaMax;
  int damage;
  int critico; // Probabilidad de crítico (0-100)
  int defensa;
  int alcance;
  // --- Estado de combate ---
  bool recibiendoAtaque; // Flag para mostrar animación de defensa/impacto
  // --- Animación de muerte ---
  ULONGLONG tiempoMuerteMs; // Momento en que murió (0 si vivo)
  int frameMuerte;          // Frame de muerte actual (0/1)
} Unidad;

// Estado de vista del juego
typedef enum {
  VISTA_LOCAL, // Vista normal de la isla con zoom
  VISTA_GLOBAL // Mapa completo con todas las islas
} EstadoVista;

// Estructura para el barco (192x192 píxeles)
typedef struct {
  float x, y;    // Posición en el mapa
  Direccion dir; // Orientación del barco
  bool activo;   // Si el barco está colocado

  // === Sistema de tropas ===
  Unidad *tropas[15]; // Punteros a las tropas embarcadas (máximo 15)
  int numTropas;      // Cantidad actual de tropas en el barco

  // === Sistema de mejoras ===
  int nivelMejora;     // Nivel de mejora del barco (1-4)
  int capacidadMaxima; // Capacidad máxima según nivel (6, 9, 12, 15)

  // === Sistema de navegación ===
  bool navegando;           // Si está en ruta a otra isla
  float destinoX, destinoY; // Coordenadas del destino
  float velocidad;          // Velocidad de navegación (px/frame)
} Barco;

// --- CONSTANTES DE LIMITES ---
#define MAX_OBREROS 100
#define MAX_CABALLEROS 100
#define MAX_CABALLEROS_SIN_ESCUDO 100
#define MAX_GUERREROS 100

struct Jugador {
  char Nombre[30];
  int Comida;
  int Oro;
  int Madera;
  int Piedra;
  int Hierro;
  struct Tropa *Ejercito;
  Unidad obreros[MAX_OBREROS];             // Trabajadores
  Unidad caballeros[MAX_CABALLEROS];          // Caballeros con escudo
  Unidad caballerosSinEscudo[MAX_CABALLEROS_SIN_ESCUDO]; // Caballeros sin escudo
  Unidad guerreros[MAX_GUERREROS];           // Guerreros
  Barco barco;                   // Barco en la orilla (192x192px)
  int CantidadEspadas;
  int CantidadArqueros;
  int CantidadPicas;
  int CantidadCaballeria;
  int NumeroTropas;
  int Capacidad;

  // Edificios del jugador
  void *ayuntamiento; // Puntero a Edificio (void* para evitar dependencia
                      // circular)
  // Estado de vista
  EstadoVista vistaActual; // Vista actual (local o global)
  int islaActual;          // Isla donde está el jugador (1, 2, o 3)
  void *mina;              // Puntero a Edificio de la mina
  void *cuartel;           // Puntero a Edificio del cuartel
};

void actualizarPersonajes(struct Jugador *j);
// Movimiento estilo RTS: planifica ruta evitando colisiones y límites del mapa.
void rtsComandarMovimiento(struct Jugador *j, float mundoX, float mundoY);
// Libera memoria dinámica asociada al movimiento (rutas por unidad, etc).
void rtsLiberarMovimientoJugador(struct Jugador *j);
void IniciacionRecursos(struct Jugador *j, const char *Nombre);
void IniciacionTropa(struct Tropa *t, const char *Nombre, int Oro, int Comida,
                     int Madera, int Piedra, int Vida, int Fuerza,
                     int VelocidadAtaque, int DistanciaAtaque);
void gotoxy(int x, int y);
void mostrarStats(struct Jugador j, int x, int y);

// Nueva función lógica para talar
bool recursosIntentarTalar(struct Jugador *j, float mundoX, float mundoY);
// Nueva función para recoger de la mina
bool recursosIntentarRecogerMina(struct Jugador *j, float mundoX, float mundoY);
// Nueva función para cazar vacas
bool recursosIntentarCazar(struct Jugador *j, float mundoX, float mundoY);

// Funciones de entrenamiento de tropas
bool entrenarObrero(struct Jugador *j, float x, float y);
bool entrenarCaballero(struct Jugador *j, float x, float y);
bool entrenarCaballeroSinEscudo(struct Jugador *j, float x, float y);
bool entrenarGuerrero(struct Jugador *j, float x, float y);

// Función para mejorar la capacidad del barco
bool mejorarBarco(struct Jugador *j);

// Verifica si hay algun obrero seleccionado cerca de un punto
bool recursosObreroCercaDePunto(struct Jugador *j, float x, float y,
                                float distMax);
// Verifica si hay CUALQUIER tropa seleccionada cerca de un punto
bool recursosCualquierTropaCercaDePunto(struct Jugador *j, float x, float y,
                                        float distMax);

// ============================================================================
// PANEL HUD DE RECURSOS (Esquina superior derecha)
// ============================================================================
// Dibuja un panel con los recursos actuales del jugador (Oro, Madera, Piedra,
// Comida) y el conteo de unidades. Se renderiza directamente sobre el buffer
// para evitar parpadeo.
// ============================================================================
void panelRecursosDibujar(HDC hdcBuffer, struct Jugador *j, int anchoPantalla);

#endif
