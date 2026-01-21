// edificios/edificios.c
#include "edificios.h"
#include "../../mapa/mapa.h"
#include "../navegacion.h"
#include <stdio.h>
#include <string.h>
#include <windows.h>


#define imgMina "../assets/mina.bmp"
#define imgMinaFuego "../assets/mina-fuego.bmp"
#define imgCastilloAliado "../assets/castillo_aliado.bmp"
#define imgCastilloEnemigo "../assets/castillo_enemigo.bmp"
#define imgCastilloFuego "../assets/castillo_fuego.bmp"
#define imgCuartelAliado "../assets/cuartel_aliado.bmp"
#define imgCuartelEnemigo "../assets/cuartel_enemigo.bmp"
#define imgCuartelFuego "../assets/cuartel_fuego.bmp"
#define imgCastilloHielo "../assets/castillo_hielo.bmp"
#define imgCuartelHielo "../assets/cuartel_hielo.bmp"
#define imgMinaHielo "../assets/iglu.bmp"

// Tamaño del castillo y cuartel (4x4 celdas de 64px = 256px)
#define CASTILLO_SIZE 256
#define CUARTEL_SIZE 256

// Sprites globales de edificios
HBITMAP g_sprites[12] = {0}; // 0-2: Basics, 3-6: Castillos, 7-10: Cuarteles, 11: MinaHielo 

HBITMAP g_spriteAyuntamiento = NULL; 
HBITMAP g_spriteMina = NULL;
HBITMAP g_spriteCuartel = NULL;
HBITMAP g_spriteMinaFuego = NULL;
HBITMAP g_spriteMinaHielo = NULL;

HBITMAP g_spriteCastilloAliado = NULL;
HBITMAP g_spriteCastilloEnemigo = NULL;
HBITMAP g_spriteCastilloFuego = NULL;
HBITMAP g_spriteCastilloHielo = NULL;

HBITMAP g_spriteCuartelAliado = NULL;
HBITMAP g_spriteCuartelEnemigo = NULL;
HBITMAP g_spriteCuartelFuego = NULL;
HBITMAP g_spriteCuartelHielo = NULL;

static HBITMAP cargarSprite(const char* path, int size) {
    return (HBITMAP)LoadImageA(NULL, path, IMAGE_BITMAP, size, size, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
}

void edificioInicializar(Edificio *e, TipoEdificio tipo, float x, float y) {
  e->tipo = tipo;
  e->x = x;
  e->y = y;
  e->construido = true;

  // Configurar dimensiones según el tipo
  switch (tipo) {
  case EDIFICIO_AYUNTAMIENTO:
    // Castillos de 256x256 (4x4 celdas de 64px)
    e->ancho = CASTILLO_SIZE;
    e->alto = CASTILLO_SIZE;
    // Sprite se asigna dinámicamente según estado de conquista en edificioDibujar
    e->sprite = NULL;
    break;
  case EDIFICIO_MINA:
    e->ancho = 128;
    e->alto = 128;
    e->sprite = g_spriteMina;
    break;
  case EDIFICIO_CUARTEL:
    // Cuarteles de 256x256 (4x4 celdas de 64px)
    e->ancho = CUARTEL_SIZE;
    e->alto = CUARTEL_SIZE;
    // Sprite se asigna dinámicamente según estado de conquista en edificioDibujar
    e->sprite = NULL;
    break;
  case EDIFICIO_GRANJA:
    e->ancho = 128;
    e->alto = 128;
    e->sprite = NULL; // Por ahora no implementado
    break;
  }

  // Inicializar acumuladores de recursos
  e->oroAcumulado = 0;
  e->piedraAcumulada = 0;
  e->hierroAcumulado = 0;
  e->ultimoTickGeneracion = GetTickCount();

  // Inicializar recursos totales de la mina (solo para minas)
  if (tipo == EDIFICIO_MINA) {
    e->oroRestante = 800;     // Intercambiado: Era 1500 (Ahora escaso)
    e->piedraRestante = 1500; 
    e->hierroRestante = 1500; // Intercambiado: Era 800 (Ahora abundante)
    e->agotada = false;

  } else {
    e->oroRestante = 0;
    e->piedraRestante = 0;
    e->hierroRestante = 0;
    e->agotada = false;
  }
}

bool edificioContienePunto(const Edificio *e, float mundoX, float mundoY) {
  if (!e->construido)
    return false;

  // Verificar si el punto está dentro del rectángulo del edificio
  return (mundoX >= e->x && mundoX < e->x + e->ancho && mundoY >= e->y &&
          mundoY < e->y + e->alto);
}

void edificioActualizar(Edificio *e) {
  if (!e->construido || e->tipo != EDIFICIO_MINA)
    return;

  // Si la mina está agotada, no generar más recursos
  if (e->agotada) {
    return;
  }

  DWORD tickActual = GetTickCount();
  // Generar cada 15 segundos (15000 ms)
  if (tickActual - e->ultimoTickGeneracion >= 15000) {
    // Generar recursos solo si hay reservas disponibles
    int oroGenerado = 0, piedraGenerada = 0, hierroGenerado = 0;

    // Generar +20 de oro (Escaso, valioso)
    if (e->oroAcumulado < 300 && e->oroRestante > 0) {
      int cantidad = (e->oroRestante >= 20) ? 20 : e->oroRestante;
      e->oroAcumulado += cantidad;
      e->oroRestante -= cantidad;
      oroGenerado = cantidad;
    }

    // Generar +35 de piedra
    if (e->piedraAcumulada < 300 && e->piedraRestante > 0) {
      int cantidad = (e->piedraRestante >= 35) ? 35 : e->piedraRestante;
      e->piedraAcumulada += cantidad;
      e->piedraRestante -= cantidad;
      piedraGenerada = cantidad;
    }

    // Generar +35 de hierro (Abundante, comun)
    if (e->hierroAcumulado < 300 && e->hierroRestante > 0) {
      int cantidad = (e->hierroRestante >= 35) ? 35 : e->hierroRestante;
      e->hierroAcumulado += cantidad;
      e->hierroRestante -= cantidad;
      hierroGenerado = cantidad;
    }

    e->ultimoTickGeneracion = tickActual;

    // Verificar si la mina se agotó completamente
    if (e->oroRestante <= 0 && e->piedraRestante <= 0 &&
        e->hierroRestante <= 0) {
      e->agotada = true;
      e->construido = false; // "EXPLOTÓ": Desaparece del mapa

      // Liberar el espacio en el mapa de colisiones y matriz de objetos
      mapaDesmarcarEdificio(e->x, e->y, e->ancho, e->alto);

      // Mostrar mensaje de evento
      MessageBoxA(GetActiveWindow(),
                  "¡UNA MINA HA EXPLOTADO!\n\nSe han agotado todas sus vetas y "
                  "la estructura ha colapsado.",
                  "Evento: Mina Agotada", MB_OK | MB_ICONWARNING);


    }
  }
}

#define MAX_PATH_LEN 512

void edificiosCargarSprites() {
  g_spriteMina = cargarSprite(imgMina, 128);
  g_spriteMinaFuego = cargarSprite(imgMinaFuego, 128);
  g_spriteMinaHielo = cargarSprite(imgMinaHielo, 128);

  g_spriteCastilloAliado = cargarSprite(imgCastilloAliado, CASTILLO_SIZE);
  g_spriteCastilloEnemigo = cargarSprite(imgCastilloEnemigo, CASTILLO_SIZE);
  g_spriteCastilloFuego = cargarSprite(imgCastilloFuego, CASTILLO_SIZE);
  g_spriteCastilloHielo = cargarSprite(imgCastilloHielo, CASTILLO_SIZE);

  g_spriteCuartelAliado = cargarSprite(imgCuartelAliado, CUARTEL_SIZE);
  g_spriteCuartelEnemigo = cargarSprite(imgCuartelEnemigo, CUARTEL_SIZE);
  g_spriteCuartelFuego = cargarSprite(imgCuartelFuego, CUARTEL_SIZE);
  g_spriteCuartelHielo = cargarSprite(imgCuartelHielo, CUARTEL_SIZE);
}

void edificioDibujar(HDC hdcBuffer, const Edificio *e, int camX, int camY,
                     float zoom, int anchoP, int altoP, int islaActual) {
  if (!e->construido)
    return;

  bool esIslaFuego = mapaTemaActualEsFuego();
  bool esIslaHielo = mapaTemaActualEsHielo();
  
  // Debug temporal para verificar tema en islas 4 y 5
  static int debugContador = 0;
  if (debugContador < 10 && e->tipo == EDIFICIO_MINA && (esIslaFuego || esIslaHielo)) {

    debugContador++;
  }

  // Convertir coordenadas del mundo a pantalla
  int pantallaX = (int)((e->x - camX) * zoom);
  int pantallaY = (int)((e->y - camY) * zoom);
  int anchoZoom = (int)(e->ancho * zoom);
  int altoZoom = (int)(e->alto * zoom);

  // Culling: no dibujar si está fuera de la pantalla
  if (pantallaX + anchoZoom < 0 || pantallaX > anchoP ||
      pantallaY + altoZoom < 0 || pantallaY > altoP) {
    return;
  }

  // SELECCIÓN DE SPRITE SEGÚN TEMA DE ISLA
  HBITMAP spriteADibujar = NULL;
  
  if (e->tipo == EDIFICIO_AYUNTAMIENTO || e->tipo == EDIFICIO_CUARTEL) {
      bool esAyu = (e->tipo == EDIFICIO_AYUNTAMIENTO);
      HBITMAP sFuego = esAyu ? g_spriteCastilloFuego : g_spriteCuartelFuego;
      HBITMAP sHielo = esAyu ? g_spriteCastilloHielo : g_spriteCuartelHielo;
      HBITMAP sAliado = esAyu ? g_spriteCastilloAliado : g_spriteCuartelAliado;
      HBITMAP sEnemigo = esAyu ? g_spriteCastilloEnemigo : g_spriteCuartelEnemigo;
      HBITMAP sDefault = esAyu ? g_spriteAyuntamiento : g_spriteCuartel;

      if (esIslaFuego && sFuego) spriteADibujar = sFuego;
      else if (esIslaHielo && sHielo) spriteADibujar = sHielo;
      else {
           bool esConquistada = navegacionIsIslaConquistada(islaActual);
           if (esConquistada && sAliado) spriteADibujar = sAliado;
           else if (!esConquistada && sEnemigo) spriteADibujar = sEnemigo;
           else spriteADibujar = sDefault;
      }
  } else if (e->tipo == EDIFICIO_MINA) {
    if (esIslaFuego && g_spriteMinaFuego) spriteADibujar = g_spriteMinaFuego;
    else if (esIslaHielo && g_spriteMinaHielo) spriteADibujar = g_spriteMinaHielo;
    else spriteADibujar = g_spriteMina;
  } else {
    spriteADibujar = e->sprite;
  }

  // DIBUJAR SPRITE
  if (spriteADibujar) {
    HDC hdcSprite = CreateCompatibleDC(hdcBuffer);
    HBITMAP oldBmp = (HBITMAP)SelectObject(hdcSprite, spriteADibujar);
    BITMAP bm;
    GetObject(spriteADibujar, sizeof(bm), &bm);

    TransparentBlt(hdcBuffer, pantallaX, pantallaY, anchoZoom, altoZoom,
                   hdcSprite, 0, 0, bm.bmWidth, bm.bmHeight,
                   RGB(255, 255, 255));

    SelectObject(hdcSprite, oldBmp);
    DeleteDC(hdcSprite);
  } else {
    // Cuadro cafe si falla todo
    HBRUSH brushEdificio = CreateSolidBrush(RGB(139, 69, 19));
    HBRUSH oldBrush = (HBRUSH)SelectObject(hdcBuffer, brushEdificio);
    Rectangle(hdcBuffer, pantallaX, pantallaY, pantallaX + anchoZoom,
              pantallaY + altoZoom);
    SelectObject(hdcBuffer, oldBrush);
    DeleteObject(brushEdificio);
  }

  // --- SEÑALIZACIÓN DE RECURSOS (Mina) ---
  if (e->tipo == EDIFICIO_MINA &&
      (e->oroAcumulado > 0 || e->piedraAcumulada > 0)) {
    // Dibujar un pequeño círculo indicador (Amarillo para Oro, Gris para
    // Piedra)
    int offsetSignal = (int)(10 * zoom);

    if (e->oroAcumulado > 0) {
      HBRUSH hBrush = CreateSolidBrush(RGB(255, 215, 0)); // Oro
      HBRUSH oldB = (HBRUSH)SelectObject(hdcBuffer, hBrush);
      Ellipse(hdcBuffer, pantallaX + offsetSignal, pantallaY - offsetSignal,
              pantallaX + offsetSignal + (int)(15 * zoom), pantallaY);
      SelectObject(hdcBuffer, oldB);
      DeleteObject(hBrush);
    }

    if (e->piedraAcumulada > 0) {
      HBRUSH hBrush = CreateSolidBrush(RGB(169, 169, 169)); // Piedra
      HBRUSH oldB = (HBRUSH)SelectObject(hdcBuffer, hBrush);
      int offsetPiedra = (int)(25 * zoom);
      Ellipse(hdcBuffer, pantallaX + offsetPiedra, pantallaY - offsetSignal,
              pantallaX + offsetPiedra + (int)(15 * zoom), pantallaY);
      SelectObject(hdcBuffer, oldB);
      DeleteObject(hBrush);
    }

    // Texto flotante opcional
    SetBkMode(hdcBuffer, TRANSPARENT);
    SetTextColor(hdcBuffer, RGB(255, 255, 255));
    char txt[32];
    sprintf(txt, "[!] RECURSOS");
    TextOutA(hdcBuffer, pantallaX + (int)(40 * zoom),
             pantallaY - (int)(20 * zoom), txt, strlen(txt));
  }
}

static void safeDelete(HBITMAP *bmp) {
    if (*bmp) { DeleteObject(*bmp); *bmp = NULL; }
}

void edificiosLiberarSprites() {
    safeDelete(&g_spriteAyuntamiento);
    safeDelete(&g_spriteMina);
    safeDelete(&g_spriteMinaFuego);
    safeDelete(&g_spriteMinaHielo);
    safeDelete(&g_spriteCuartel);
    safeDelete(&g_spriteCastilloAliado);
    safeDelete(&g_spriteCastilloEnemigo);
    safeDelete(&g_spriteCastilloFuego);
    safeDelete(&g_spriteCastilloHielo);
    safeDelete(&g_spriteCuartelAliado);
    safeDelete(&g_spriteCuartelEnemigo);
    safeDelete(&g_spriteCuartelFuego);
    safeDelete(&g_spriteCuartelHielo);
}
