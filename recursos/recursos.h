// recursos/recursos.h
#ifndef RECURSOS_H
#define RECURSOS_H
#include <stdbool.h>
#include <windows.h> // Necesario para COORD en gotoxy

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
  // Celda actualmente ocupada por el obrero (se actualiza solo cuando cambia de
  // celda por completo).
  int celdaFila;
  int celdaCol;

  // --- Animación por puntero a estado ---
  const Animation *animActual;
  int animTick;
} UnidadObrero;

struct Jugador {
  char Nombre[30];
  int Comida;
  int Oro;
  int Madera;
  int Piedra;
  struct Tropa *Ejercito;
  UnidadObrero obreros[6];
  int CantidadEspadas;
  int CantidadArqueros;
  int CantidadPicas;
  int CantidadCaballeria;
  int NumeroTropas;
  int Capacidad;

  // Edificio del jugador (por ahora solo el ayuntamiento)
  void *ayuntamiento; // Puntero a Edificio (void* para evitar dependencia
                      // circular)
};

void actualizarObreros(struct Jugador *j);
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

#endif
