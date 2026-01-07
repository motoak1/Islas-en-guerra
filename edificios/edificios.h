// edificios/edificios.h
#ifndef EDIFICIOS_H
#define EDIFICIOS_H

#include <stdbool.h>
#include <windows.h>

// --- TIPOS DE EDIFICIOS ---
typedef enum {
  EDIFICIO_AYUNTAMIENTO,
  EDIFICIO_CUARTEL,
  EDIFICIO_GRANJA,
  EDIFICIO_MINA
} TipoEdificio;

// --- ESTRUCTURA DE EDIFICIO ---
typedef struct {
  TipoEdificio tipo;
  float x, y;      // Posición en píxeles del mundo (esquina superior izquierda)
  int ancho, alto; // Dimensiones del sprite (128x128 para ayuntamiento)
  bool construido; // Si está activo en el juego
  HBITMAP sprite;  // Sprite cargado del edificio

  // --- Lógica de Recursos (Mina) ---
  int oroAcumulado;
  int piedraAcumulada;
  int hierroAcumulado;
  DWORD ultimoTickGeneracion;

  // --- Sistema de agotamiento de recursos ---
  int oroRestante;    // Oro total que queda por extraer
  int piedraRestante; // Piedra total que queda por extraer
  int hierroRestante; // Hierro total que queda por extraer
  bool agotada;       // true si la mina ya no tiene recursos
} Edificio;

// --- FUNCIONES DE EDIFICIOS ---

// Inicializa un edificio del tipo especificado en una posición
void edificioInicializar(Edificio *e, TipoEdificio tipo, float x, float y);

// Actualiza la lógica del edificio (generación de recursos, etc)
void edificioActualizar(Edificio *e);

// Detecta si un punto (mundoX, mundoY) está dentro del edificio
bool edificioContienePunto(const Edificio *e, float mundoX, float mundoY);

// Carga los sprites de edificios desde archivos
void edificiosCargarSprites();

// Dibuja un edificio en pantalla con la cámara especificada
void edificioDibujar(HDC hdcBuffer, const Edificio *e, int camX, int camY,
                     float zoom, int anchoP, int altoP);

// Libera recursos gráficos de edificios
void edificiosLiberarSprites();

// Sprites globales de edificios (se cargan una vez)
extern HBITMAP g_spriteAyuntamiento;
extern HBITMAP g_spriteMina;
extern HBITMAP g_spriteCuartel;

#endif
