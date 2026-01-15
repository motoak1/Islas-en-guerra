// edificios/edificios.c
#include "edificios.h"
#include "../mapa/mapa.h"
#include "../recursos/navegacion.h"
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

// Tamaño del castillo y cuartel (4x4 celdas de 64px = 256px)
#define CASTILLO_SIZE 256
#define CUARTEL_SIZE 256

// Sprites globales de edificios
HBITMAP g_spriteAyuntamiento = NULL;
HBITMAP g_spriteMina = NULL;
HBITMAP g_spriteCuartel = NULL;
HBITMAP g_spriteMinaFuego = NULL;
HBITMAP g_spriteMinaHielo = NULL;

// Sprites de castillos para islas (256x256)
HBITMAP g_spriteCastilloAliado = NULL;
HBITMAP g_spriteCastilloEnemigo = NULL;
HBITMAP g_spriteCastilloFuego = NULL;
HBITMAP g_spriteCastilloHielo = NULL;

// Sprites de cuarteles para islas (256x256)
HBITMAP g_spriteCuartelAliado = NULL;
HBITMAP g_spriteCuartelEnemigo = NULL;
HBITMAP g_spriteCuartelFuego = NULL;
HBITMAP g_spriteCuartelHielo = NULL;

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
    printf("[MINA] Inicializada con: %d Oro, %d Piedra, %d Hierro\n",
           e->oroRestante, e->piedraRestante, e->hierroRestante);
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

      // NUEVO: Liberar el espacio en el mapa de colisiones y matriz de objetos
      mapaDesmarcarEdificio(e->x, e->y, e->ancho, e->alto);

      // Mostrar mensaje de evento
      MessageBoxA(GetActiveWindow(),
                  "¡UNA MINA HA EXPLOTADO!\n\nSe han agotado todas sus vetas y "
                  "la estructura ha colapsado.",
                  "Evento: Mina Agotada", MB_OK | MB_ICONWARNING);

      printf("[MINA] ¡EXPLOTÓ! Ya no quedan recursos y la estructura ha "
             "colapsado.\n");
    } else {
      // Debug para consola
      printf("[MINA] Generado: +%d Oro, +%d Piedra, +%d Hierro | Restante: %d "
             "Oro, %d Piedra, %d Hierro\n",
             oroGenerado, piedraGenerada, hierroGenerado, e->oroRestante,
             e->piedraRestante, e->hierroRestante);
    }
  }
}

#define MAX_PATH_LEN 512

void edificiosCargarSprites() {
  char pathExe[MAX_PATH_LEN];
  GetModuleFileNameA(NULL, pathExe, MAX_PATH_LEN);
  char *last = strrchr(pathExe, '\\');
  if (last)
    *last = '\0';

  char fullPath[MAX_PATH_LEN];

  // Lista de intentos para la Mina
  const char *attemptsMina[] = {"\\assets\\mina.bmp", "\\..\\assets\\mina.bmp",
                                "\\mina.bmp"};

  g_spriteMina = NULL;
  for (int i = 0; i < 3; i++) {
    sprintf(fullPath, "%s%s", pathExe, attemptsMina[i]);
    g_spriteMina = (HBITMAP)LoadImageA(NULL, fullPath, IMAGE_BITMAP, 128, 128,
                                       LR_LOADFROMFILE | LR_CREATEDIBSECTION);
    if (g_spriteMina)
      break;
  }

  const char *attemptsMinaFuego[] = {"\\assets\\mina-fuego.bmp",
                                     "\\..\\assets\\mina-fuego.bmp",
                                     "\\mina-fuego.bmp"};
  g_spriteMinaFuego = NULL;
  for (int i = 0; i < 3; i++) {
    sprintf(fullPath, "%s%s", pathExe, attemptsMinaFuego[i]);
    g_spriteMinaFuego = (HBITMAP)LoadImageA(
        NULL, fullPath, IMAGE_BITMAP, 128, 128,
        LR_LOADFROMFILE | LR_CREATEDIBSECTION);
    if (g_spriteMinaFuego)
      break;
  }

  const char *attemptsMinaHielo[] = {"\\assets\\iglu.bmp",
                                     "\\..\\assets\\iglu.bmp",
                                     "\\iglu.bmp"};
  g_spriteMinaHielo = NULL;
  for (int i = 0; i < 3; i++) {
    sprintf(fullPath, "%s%s", pathExe, attemptsMinaHielo[i]);
    g_spriteMinaHielo = (HBITMAP)LoadImageA(
        NULL, fullPath, IMAGE_BITMAP, 128, 128,
        LR_LOADFROMFILE | LR_CREATEDIBSECTION);
    if (g_spriteMinaHielo)
      break;
  }

  if ( g_spriteMina) {
    printf("[SISTEMA] Recursos de edificios cargados con exito.\n");
  } else {
    printf("[ERROR] Fallo en carga de sprites:\n");
    if (!g_spriteMina)
      printf(" - Fallo Mina\n");

  }

  // ============================================================================
  // CARGAR SPRITES DE CASTILLOS ALIADO Y ENEMIGO (256x256, 4x4 celdas)
  // ============================================================================
  const char *attemptsCastilloAliado[] = {
      "\\assets\\castillo_aliado.bmp", "\\..\\assets\\castillo_aliado.bmp",
      "\\castillo_aliado.bmp"};

  g_spriteCastilloAliado = NULL;
  for (int i = 0; i < 3; i++) {
    sprintf(fullPath, "%s%s", pathExe, attemptsCastilloAliado[i]);
    g_spriteCastilloAliado =
        (HBITMAP)LoadImageA(NULL, fullPath, IMAGE_BITMAP, CASTILLO_SIZE, CASTILLO_SIZE,
                            LR_LOADFROMFILE | LR_CREATEDIBSECTION);
    if (g_spriteCastilloAliado) {
      printf("[SISTEMA] Imagen castillo_aliado.bmp cargada con exito (256x256).\n");
      break;
    }
  }

  const char *attemptsCastilloEnemigo[] = {
      "\\assets\\castillo_enemigo.bmp", "\\..\\assets\\castillo_enemigo.bmp",
      "\\castillo_enemigo.bmp"};

  g_spriteCastilloEnemigo = NULL;
  for (int i = 0; i < 3; i++) {
    sprintf(fullPath, "%s%s", pathExe, attemptsCastilloEnemigo[i]);
    g_spriteCastilloEnemigo =
        (HBITMAP)LoadImageA(NULL, fullPath, IMAGE_BITMAP, CASTILLO_SIZE, CASTILLO_SIZE,
                            LR_LOADFROMFILE | LR_CREATEDIBSECTION);
    if (g_spriteCastilloEnemigo) {
      printf("[SISTEMA] Imagen castillo_enemigo.bmp cargada con exito (256x256).\n");
      break;
    }
  }

  // Verificar si se cargaron los castillos
  if (!g_spriteCastilloAliado) {
    printf("[ERROR] No se pudo cargar castillo_aliado.bmp\n");
  }
  if (!g_spriteCastilloEnemigo) {
    printf("[ERROR] No se pudo cargar castillo_enemigo.bmp\n");
  }

  const char *attemptsCastilloFuego[] = {
      "\\assets\\castillo_fuego.bmp", "\\..\\assets\\castillo_fuego.bmp",
      "\\castillo_fuego.bmp"};
  g_spriteCastilloFuego = NULL;
  for (int i = 0; i < 3; i++) {
    sprintf(fullPath, "%s%s", pathExe, attemptsCastilloFuego[i]);
    g_spriteCastilloFuego = (HBITMAP)LoadImageA(
        NULL, fullPath, IMAGE_BITMAP, CASTILLO_SIZE, CASTILLO_SIZE,
        LR_LOADFROMFILE | LR_CREATEDIBSECTION);
    if (g_spriteCastilloFuego) {
      break;
    }
  }

  const char *attemptsCastilloHielo[] = {
      "\\assets\\castillo_hielo.bmp", "\\..\\assets\\castillo_hielo.bmp",
      "\\castillo_hielo.bmp"};
  g_spriteCastilloHielo = NULL;
  for (int i = 0; i < 3; i++) {
    sprintf(fullPath, "%s%s", pathExe, attemptsCastilloHielo[i]);
    g_spriteCastilloHielo = (HBITMAP)LoadImageA(
        NULL, fullPath, IMAGE_BITMAP, CASTILLO_SIZE, CASTILLO_SIZE,
        LR_LOADFROMFILE | LR_CREATEDIBSECTION);
    if (g_spriteCastilloHielo) {
      break;
    }
  }

  // ============================================================================
  // CARGAR SPRITES DE CUARTELES ALIADO Y ENEMIGO (256x256, 4x4 celdas)
  // ============================================================================
  const char *attemptsCuartelAliado[] = {
      "\\assets\\cuartel_aliado.bmp", "\\..\\assets\\cuartel_aliado.bmp",
      "\\cuartel_aliado.bmp"};

  g_spriteCuartelAliado = NULL;
  for (int i = 0; i < 3; i++) {
    sprintf(fullPath, "%s%s", pathExe, attemptsCuartelAliado[i]);
    g_spriteCuartelAliado =
        (HBITMAP)LoadImageA(NULL, fullPath, IMAGE_BITMAP, CUARTEL_SIZE, CUARTEL_SIZE,
                            LR_LOADFROMFILE | LR_CREATEDIBSECTION);
    if (g_spriteCuartelAliado) {
      printf("[SISTEMA] Imagen cuartel_aliado.bmp cargada con exito (256x256).\n");
      break;
    }
  }

  const char *attemptsCuartelEnemigo[] = {
      "\\assets\\cuartel_enemigo.bmp", "\\..\\assets\\cuartel_enemigo.bmp",
      "\\cuartel_enemigo.bmp"};

  g_spriteCuartelEnemigo = NULL;
  for (int i = 0; i < 3; i++) {
    sprintf(fullPath, "%s%s", pathExe, attemptsCuartelEnemigo[i]);
    g_spriteCuartelEnemigo =
        (HBITMAP)LoadImageA(NULL, fullPath, IMAGE_BITMAP, CUARTEL_SIZE, CUARTEL_SIZE,
                            LR_LOADFROMFILE | LR_CREATEDIBSECTION);
    if (g_spriteCuartelEnemigo) {
      printf("[SISTEMA] Imagen cuartel_enemigo.bmp cargada con exito (256x256).\n");
      break;
    }
  }

  // Verificar si se cargaron los cuarteles
  if (!g_spriteCuartelAliado) {
    printf("[ERROR] No se pudo cargar cuartel_aliado.bmp\n");
  }
  if (!g_spriteCuartelEnemigo) {
    printf("[ERROR] No se pudo cargar cuartel_enemigo.bmp\n");
  }

  const char *attemptsCuartelFuego[] = {
      "\\assets\\cuartel_fuego.bmp", "\\..\\assets\\cuartel_fuego.bmp",
      "\\cuartel_fuego.bmp"};
  g_spriteCuartelFuego = NULL;
  for (int i = 0; i < 3; i++) {
    sprintf(fullPath, "%s%s", pathExe, attemptsCuartelFuego[i]);
    g_spriteCuartelFuego = (HBITMAP)LoadImageA(
        NULL, fullPath, IMAGE_BITMAP, CUARTEL_SIZE, CUARTEL_SIZE,
        LR_LOADFROMFILE | LR_CREATEDIBSECTION);
    if (g_spriteCuartelFuego) {
      break;
    }
  }

  const char *attemptsCuartelHielo[] = {
      "\\assets\\cuartel_hielo.bmp", "\\..\\assets\\cuartel_hielo.bmp",
      "\\cuartel_hielo.bmp"};
  g_spriteCuartelHielo = NULL;
  for (int i = 0; i < 3; i++) {
    sprintf(fullPath, "%s%s", pathExe, attemptsCuartelHielo[i]);
    g_spriteCuartelHielo = (HBITMAP)LoadImageA(
        NULL, fullPath, IMAGE_BITMAP, CUARTEL_SIZE, CUARTEL_SIZE,
        LR_LOADFROMFILE | LR_CREATEDIBSECTION);
    if (g_spriteCuartelHielo) {
      break;
    }
  }
}

void edificioDibujar(HDC hdcBuffer, const Edificio *e, int camX, int camY,
                     float zoom, int anchoP, int altoP, int islaActual) {
  if (!e->construido)
    return;

  bool esIslaFuego = mapaTemaActualEsFuego();
  bool esIslaHielo = mapaTemaActualEsHielo();

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

  // SEGURIDAD: Si no tiene sprite asignado, intentar usar el global de carga
  HBITMAP spriteADibujar = e->sprite;
  if (!spriteADibujar) {
    if (e->tipo == EDIFICIO_AYUNTAMIENTO) {
      if (esIslaFuego && g_spriteCastilloFuego) {
        spriteADibujar = g_spriteCastilloFuego;
      } else if (esIslaHielo && g_spriteCastilloHielo) {
        spriteADibujar = g_spriteCastilloHielo;
      } else {
        bool esConquistada = navegacionIsIslaConquistada(islaActual);
        if (esConquistada && g_spriteCastilloAliado) {
          spriteADibujar = g_spriteCastilloAliado;
        } else if (!esConquistada && g_spriteCastilloEnemigo) {
          spriteADibujar = g_spriteCastilloEnemigo;
        } else {
          spriteADibujar = g_spriteAyuntamiento;
        }
      }
    }
    else if (e->tipo == EDIFICIO_MINA) {
      if (esIslaFuego && g_spriteMinaFuego) {
        spriteADibujar = g_spriteMinaFuego;
      } else if (esIslaHielo && g_spriteMinaHielo) {
        spriteADibujar = g_spriteMinaHielo;
      } else {
        spriteADibujar = g_spriteMina;
      }
    }
    else if (e->tipo == EDIFICIO_CUARTEL) {
      if (esIslaFuego && g_spriteCuartelFuego) {
        spriteADibujar = g_spriteCuartelFuego;
      } else if (esIslaHielo && g_spriteCuartelHielo) {
        spriteADibujar = g_spriteCuartelHielo;
      } else {
        bool esConquistada = navegacionIsIslaConquistada(islaActual);
        if (esConquistada && g_spriteCuartelAliado) {
          spriteADibujar = g_spriteCuartelAliado;
        } else if (!esConquistada && g_spriteCuartelEnemigo) {
          spriteADibujar = g_spriteCuartelEnemigo;
        } else {
          spriteADibujar = g_spriteCuartel;
        }
      }
    }
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

void edificiosLiberarSprites() {
  if (g_spriteAyuntamiento) {
    DeleteObject(g_spriteAyuntamiento);
    g_spriteAyuntamiento = NULL;
  }
  if (g_spriteMina) {
    DeleteObject(g_spriteMina);
    g_spriteMina = NULL;
  }
  if (g_spriteMinaFuego) {
    DeleteObject(g_spriteMinaFuego);
    g_spriteMinaFuego = NULL;
  }
  if (g_spriteMinaHielo) {
    DeleteObject(g_spriteMinaHielo);
    g_spriteMinaHielo = NULL;
  }
  if (g_spriteCuartel) {
    DeleteObject(g_spriteCuartel);
    g_spriteCuartel = NULL;
  }
  // Liberar sprites de castillos
  if (g_spriteCastilloAliado) {
    DeleteObject(g_spriteCastilloAliado);
    g_spriteCastilloAliado = NULL;
  }
  if (g_spriteCastilloEnemigo) {
    DeleteObject(g_spriteCastilloEnemigo);
    g_spriteCastilloEnemigo = NULL;
  }
  if (g_spriteCastilloFuego) {
    DeleteObject(g_spriteCastilloFuego);
    g_spriteCastilloFuego = NULL;
  }
  if (g_spriteCastilloHielo) {
    DeleteObject(g_spriteCastilloHielo);
    g_spriteCastilloHielo = NULL;
  }
  // Liberar sprites de cuarteles
  if (g_spriteCuartelAliado) {
    DeleteObject(g_spriteCuartelAliado);
    g_spriteCuartelAliado = NULL;
  }
  if (g_spriteCuartelEnemigo) {
    DeleteObject(g_spriteCuartelEnemigo);
    g_spriteCuartelEnemigo = NULL;
  }
  if (g_spriteCuartelFuego) {
    DeleteObject(g_spriteCuartelFuego);
    g_spriteCuartelFuego = NULL;
  }
  if (g_spriteCuartelHielo) {
    DeleteObject(g_spriteCuartelHielo);
    g_spriteCuartelHielo = NULL;
  }
}
