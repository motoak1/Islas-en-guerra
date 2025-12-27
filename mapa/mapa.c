#include "mapa.h"
#include "../edificios/edificios.h"
#include "../recursos/recursos.h"
#include "../recursos/ui_compra.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>

// Definiciones manuales de rutas
#define RUTA_MAPA "../assets/islas/isla1.bmp"
#define RUTA_MAPA_ALT "assets/islas/isla1.bmp"

#define ARBOL1 "../assets/arboles/arbol1.bmp"
#define ARBOL1_ALT "assets/arboles/arbol1.bmp"

#define ARBOL2 "../assets/arboles/arbol2.bmp"
#define ARBOL2_ALT "assets/arboles/arbol2.bmp"

#define ARBOL3 "../assets/arboles/arbol3.bmp"
#define ARBOL3_ALT "assets/arboles/arbol3.bmp"

#define ARBOL4 "../assets/arboles/arbol4.bmp"
#define ARBOL4_ALT "assets/arboles/arbol4.bmp"

#define obrero_front "../assets/obrero/obrero_front.bmp"
#define obrero_back "../assets/obrero/obrero_back.bmp"
#define obrero_left "../assets/obrero/obrero_left.bmp"
#define obrero_right "../assets/obrero/obrero_right.bmp"

static HBITMAP hObreroBmp[4] = {NULL}; // Front, Back, Left, Right

// Definiciones para obrerro fallback
#define OBRERO_F_ALT "assets/obrero/obrero_front.bmp"
#define OBRERO_B_ALT "assets/obrero/obrero_back.bmp"
#define OBRERO_L_ALT "assets/obrero/obrero_left.bmp"
#define OBRERO_R_ALT "assets/obrero/obrero_right.bmp"

static HBITMAP hMapaBmp = NULL;
static HBITMAP hArboles[4] = {NULL};

// Matriz lógica 32x32
int mapaObjetos[GRID_SIZE][GRID_SIZE] = {0};

// --- COLISIONES (matriz dinámica int**)
// 0 = libre, 1 = ocupado (árboles u obstáculos)
static int **gCollisionMap = NULL;

static void collisionMapAllocIfNeeded(void) {
  if (gCollisionMap)
    return;

  gCollisionMap = (int **)malloc(GRID_SIZE * sizeof(int *));
  if (!gCollisionMap)
    return;

  for (int i = 0; i < GRID_SIZE; i++) {
    // Use calloc to zero-initialize memory (clean map)
    *(gCollisionMap + i) = (int *)calloc(GRID_SIZE, sizeof(int));
    if (!*(gCollisionMap + i)) {
      for (int k = 0; k < i; k++)
        free(*(gCollisionMap + k));
      free(gCollisionMap);
      gCollisionMap = NULL;
      return;
    }
  }
}

static void collisionMapClear(int value) {
  if (!gCollisionMap)
    return;
  for (int i = 0; i < GRID_SIZE; i++) {
    int *row = *(gCollisionMap + i);
    for (int j = 0; j < GRID_SIZE; j++) {
      *(row + j) = value;
    }
  }
}

int **mapaObtenerCollisionMap(void) {
  collisionMapAllocIfNeeded();
  return gCollisionMap;
}

void mapaReconstruirCollisionMap(void) {
  collisionMapAllocIfNeeded();
  collisionMapClear(0); // Todo libre al inicio
  for (int f = 0; f < GRID_SIZE; f++) {
    for (int c = 0; c < GRID_SIZE; c++) {
      if (mapaObjetos[f][c] > 0) { // Hay un árbol
        for (int df = 0; df < 2; df++) {
          for (int dc = 0; dc < 2; dc++) {
            if (f + df < GRID_SIZE && c + dc < GRID_SIZE)
              gCollisionMap[f + df][c + dc] = 1; // 1 = ÁRBOL (Impasable)
          }
        }
      }
    }
  }
}

// Función auxiliar para marcar edificios en el collision map
void mapaMarcarEdificio(float x, float y, int ancho, int alto) {
  collisionMapAllocIfNeeded();

  // Calcular celdas que ocupa el edificio (TILE_SIZE=64)
  int celInicioFila = (int)(y / TILE_SIZE);
  int celInicioCol = (int)(x / TILE_SIZE);
  int celFinFila = (int)((y + alto) / TILE_SIZE);
  int celFinCol = (int)((x + ancho) / TILE_SIZE);

  // Marcar celdas como bloqueadas Y eliminar árboles
  // Expandir 2 celdas de margen para evitar que las copas de los árboles tapen
  // el edificio
  for (int f = celInicioFila - 2; f <= celFinFila + 2 && f < GRID_SIZE; f++) {
    for (int c = celInicioCol - 2; c <= celFinCol + 2 && c < GRID_SIZE; c++) {
      if (f >= 0 && c >= 0) {
        // Bloquear solo las celdas del edificio exacto
        if (f >= celInicioFila && f <= celFinFila && c >= celInicioCol &&
            c <= celFinCol) {
          gCollisionMap[f][c] = 1; // Bloquear celda
        }

        // Eliminar árboles en área expandida
        mapaObjetos[f][c] = 0;
      }
    }
  }
}

void mapaLiberarCollisionMap(void) {
  if (!gCollisionMap)
    return;
  for (int i = 0; i < GRID_SIZE; i++) {
    free(*(gCollisionMap + i));
  }
  free(gCollisionMap);
  gCollisionMap = NULL;
}

// ============================================================================
// DETECCIÓN DE AGUA Y RECONSTRUCCIÓN DE COLISIONES
// ============================================================================
// Valores en collisionMap:
//   0 = Celda libre (transitable)
//   1 = Obstáculo (árbol o agua - impasable)
//   2 = Ocupada por unidad (temporalmente bloqueada)
// ============================================================================

// Detecta agua analizando los píxeles del mapa base
// Utiliza aritmética de punteros para recorrer la matriz
static void detectarAguaEnMapa(void) {
  if (!hMapaBmp || !gCollisionMap)
    return;

  HDC hdcPantalla = GetDC(NULL);
  HDC hdcMem = CreateCompatibleDC(hdcPantalla);
  HBITMAP hOldBmp = (HBITMAP)SelectObject(hdcMem, hMapaBmp);

  // Puntero a la matriz de colisiones para aritmética de punteros
  int **ptrCol = gCollisionMap;
  int contadorAgua = 0;

  // Recorremos cada celda de la matriz 32x32
  for (int f = 0; f < GRID_SIZE; f++) {
    // Aritmética de punteros: acceso a la fila f
    int *fila = *(ptrCol + f);

    for (int c = 0; c < GRID_SIZE; c++) {
      // Si ya está marcado como obstáculo (árbol), saltar
      if (*(fila + c) == 1)
        continue;

      // Analizar el píxel central de la celda 64x64
      int px = (c * TILE_SIZE) + (TILE_SIZE / 2);
      int py = (f * TILE_SIZE) + (TILE_SIZE / 2);

      COLORREF color = GetPixel(hdcMem, px, py);
      if (color == CLR_INVALID)
        continue;

      BYTE r = GetRValue(color);
      BYTE g = GetGValue(color);
      BYTE b = GetBValue(color);

      // DETECCIÓN DE AGUA: el azul domina sobre rojo y verde
      // Criterio: B > R y B > G y B > umbral (agua azul)
      if (b > r && b > g && b > 80) {
        // Marcar como agua (impasable, valor 1)
        *(fila + c) = 1;
        contadorAgua++;
      }
    }
  }

  printf("[DEBUG] Agua detectada: %d celdas marcadas como impasables.\n",
         contadorAgua);

  SelectObject(hdcMem, hOldBmp);
  DeleteDC(hdcMem);
  ReleaseDC(NULL, hdcPantalla);
}

void generarBosqueAutomatico() {
  if (!hMapaBmp)
    return;

  HDC hdcPantalla = GetDC(NULL);
  HDC hdcMem = CreateCompatibleDC(hdcPantalla);
  HBITMAP hOldBmp = (HBITMAP)SelectObject(hdcMem, hMapaBmp);

  // Requisito: Aritmética de punteros para manejar la matriz
  int (*ptrMatriz)[GRID_SIZE] = mapaObjetos;
  srand((unsigned int)time(NULL));
  int contador = 0;

  for (int i = 0; i < GRID_SIZE; i++) {
    for (int j = 0; j < GRID_SIZE; j++) {
      // Analizamos el píxel central de cada celda de 64x64
      int px = (j * TILE_SIZE) + (TILE_SIZE / 2);
      int py = (i * TILE_SIZE) + (TILE_SIZE / 2);

      COLORREF color = GetPixel(hdcMem, px, py);
      if (color == CLR_INVALID)
        continue;

      BYTE r = GetRValue(color);
      BYTE g = GetGValue(color);
      BYTE b = GetBValue(color);

      // Detección de color verde para colocar árboles
      // Solo en tierra (verde domina), no en agua (azul domina)
      if (g > r && g > b && g > 45 && b < 100) {
        if ((rand() % 100) < 25) {
          // Acceso por punteros: *(base + desplazamiento)
          *(*(ptrMatriz + i) + j) = (rand() % 4) + 1;
          contador++;
        }
      }
    }
  }
  printf("[DEBUG] Logica: %d arboles registrados en la matriz con punteros.\n",
         contador);

  // 1. Construir la grilla de colisión con árboles
  mapaReconstruirCollisionMap();

  // 2. Detectar agua y marcarla como impasable
  detectarAguaEnMapa();

  SelectObject(hdcMem, hOldBmp);
  DeleteDC(hdcMem);
  ReleaseDC(NULL, hdcPantalla);
}

void cargarRecursosGraficos() {
  // 1. Cargar Mapa Base (Intento doble)
  hMapaBmp = (HBITMAP)LoadImageA(NULL, RUTA_MAPA, IMAGE_BITMAP, 0, 0,
                                 LR_LOADFROMFILE | LR_CREATEDIBSECTION);
  if (!hMapaBmp)
    hMapaBmp = (HBITMAP)LoadImageA(NULL, RUTA_MAPA_ALT, IMAGE_BITMAP, 0, 0,
                                   LR_LOADFROMFILE | LR_CREATEDIBSECTION);

  // 2. Cargar cada árbol individualmente con sistema de respaldo (Fallback)
  const char *rutasPrincipales[] = {ARBOL1, ARBOL2, ARBOL3, ARBOL4};
  const char *rutasAlternas[] = {ARBOL1_ALT, ARBOL2_ALT, ARBOL3_ALT,
                                 ARBOL4_ALT};

  int cargados = 0;
  for (int i = 0; i < 4; i++) {
    // Intento 1: Con "../"
    hArboles[i] =
        (HBITMAP)LoadImageA(NULL, rutasPrincipales[i], IMAGE_BITMAP,
                            SPRITE_ARBOL, SPRITE_ARBOL, LR_LOADFROMFILE);

    // Intento 2: Sin "../" (si el primer intento falló)
    if (!hArboles[i]) {
      hArboles[i] =
          (HBITMAP)LoadImageA(NULL, rutasAlternas[i], IMAGE_BITMAP,
                              SPRITE_ARBOL, SPRITE_ARBOL, LR_LOADFROMFILE);
    }

    if (hArboles[i])
      cargados++;
  }

  // --- AGREGA ESTO PARA LOS OBREROS (CON FALLBACK) ---
  const char *rutasObr[] = {obrero_front, obrero_back, obrero_left,
                            obrero_right};
  const char *rutasObrAlt[] = {OBRERO_F_ALT, OBRERO_B_ALT, OBRERO_L_ALT,
                               OBRERO_R_ALT};

  for (int i = 0; i < 4; i++) {
    hObreroBmp[i] = (HBITMAP)LoadImageA(NULL, rutasObr[i], IMAGE_BITMAP, 64, 64,
                                        LR_LOADFROMFILE);
    if (!hObreroBmp[i]) {
      hObreroBmp[i] = (HBITMAP)LoadImageA(NULL, rutasObrAlt[i], IMAGE_BITMAP,
                                          64, 64, LR_LOADFROMFILE);
    }

    if (!hObreroBmp[i]) {
      printf("[ERROR] No se pudo cargar el BMP del obrero indice %d. Ruta 1: "
             "%s, Ruta 2: %s\n",
             i, rutasObr[i], rutasObrAlt[i]);
    } else {
      printf("[OK] Obrero BMP %d cargado correctamente.\n", i);
    }
  }

  printf("[DEBUG] Recursos: %d/4 arboles cargados fisicamente.\n", cargados);
  generarBosqueAutomatico();
}
// ============================================================================
// DIBUJADO CON Y-SORTING (PROFUNDIDAD POR FILA)
// ============================================================================
// En lugar de dibujar todos los árboles y luego todos los obreros,
// dibujamos el mapa fila por fila (de arriba hacia abajo).
// Por cada fila Y:
//   1. Dibujamos los árboles cuya fila coincide
//   2. Dibujamos los obreros cuya posición Y coincide con esa fila
//
// Esto crea un efecto de profundidad natural: los objetos "más abajo"
// en la pantalla se dibujan después (encima) de los objetos "más arriba).
// ============================================================================

void dibujarMundo(HDC hdc, RECT rect, Camara cam, struct Jugador *pJugador,
                  struct MenuCompra *menu) {
  if (!hMapaBmp)
    return;

  int anchoP = rect.right - rect.left;
  int altoP = rect.bottom - rect.top;

  // Crear buffer de doble renderizado
  HDC hdcBuffer = CreateCompatibleDC(hdc);
  HBITMAP hbmBuffer = CreateCompatibleBitmap(hdc, anchoP, altoP);
  HBITMAP hOldBuffer = (HBITMAP)SelectObject(hdcBuffer, hbmBuffer);

  HDC hdcMapa = CreateCompatibleDC(hdc);
  HBITMAP hOldMapa = (HBITMAP)SelectObject(hdcMapa, hMapaBmp);

  // 1. DIBUJAR SUELO (MAPA BASE)
  SetStretchBltMode(hdcBuffer, HALFTONE);
  StretchBlt(hdcBuffer, 0, 0, anchoP, altoP, hdcMapa, cam.x, cam.y,
             (int)(anchoP / cam.zoom), (int)(altoP / cam.zoom), SRCCOPY);

  // 1.5. DIBUJAR EDIFICIO (AYUNTAMIENTO) - antes del Y-sorting
  if (pJugador->ayuntamiento != NULL) {
    Edificio *edificio = (Edificio *)pJugador->ayuntamiento;
    edificioDibujar(hdcBuffer, edificio, cam.x, cam.y, cam.zoom, anchoP, altoP);
  }

  // 2. Y-SORTING: DIBUJAR FILA POR FILA (árboles + obreros mezclados)
  HDC hdcSprites = CreateCompatibleDC(hdc);

  // Puntero a la matriz de objetos para aritmética de punteros
  int (*ptrMatriz)[GRID_SIZE] = mapaObjetos;

  // Recorrer cada fila del mapa (0 a GRID_SIZE-1)
  for (int f = 0; f < GRID_SIZE; f++) {
    // Puntero a la fila actual
    int *filaActual = *(ptrMatriz + f);

    // ================================================================
    // A) DIBUJAR ÁRBOLES DE ESTA FILA
    // ================================================================
    for (int c = 0; c < GRID_SIZE; c++) {
      // Lectura mediante aritmética de punteros
      int tipo = *(filaActual + c);

      if (tipo >= 1 && tipo <= 4 && hArboles[tipo - 1] != NULL) {
        int mundoX = c * TILE_SIZE;
        int mundoY = f * TILE_SIZE;

        // Ajuste visual (centrar 128x128 sobre 64x64)
        int dibX = mundoX - (SPRITE_ARBOL - TILE_SIZE) / 2;
        int dibY = mundoY - (SPRITE_ARBOL - TILE_SIZE);

        int pantX = (int)((dibX - cam.x) * cam.zoom);
        int pantY = (int)((dibY - cam.y) * cam.zoom);
        int tamZoom = (int)(SPRITE_ARBOL * cam.zoom);

        if (pantX + tamZoom > 0 && pantX < anchoP && pantY + tamZoom > 0 &&
            pantY < altoP) {
          SelectObject(hdcSprites, hArboles[tipo - 1]);
          TransparentBlt(hdcBuffer, pantX, pantY, tamZoom, tamZoom, hdcSprites,
                         0, 0, SPRITE_ARBOL, SPRITE_ARBOL, RGB(255, 255, 255));
        }
      }
    }

    // ================================================================
    // B) DIBUJAR OBREROS CUYA BASE (y + 64) COINCIDE CON ESTA FILA
    // ================================================================
    // La "base de los pies" del obrero está en y + TILE_SIZE (64)
    // Calculamos la fila del mapa correspondiente a esa coordenada Y
    int yMinFila = f * TILE_SIZE;
    int yMaxFila = (f + 1) * TILE_SIZE;

    // Puntero base al array de obreros
    UnidadObrero *baseObreros = pJugador->obreros;

    for (UnidadObrero *o = baseObreros; o < baseObreros + 6; o++) {
      // La base del obrero (pies) está en o->y + TILE_SIZE
      float basePies = o->y + (float)TILE_SIZE;

      // Si la base del obrero cae en esta fila, dibujarlo
      if (basePies >= (float)yMinFila && basePies < (float)yMaxFila) {
        int pantX = (int)((o->x - cam.x) * cam.zoom);
        int pantY = (int)((o->y - cam.y) * cam.zoom);
        int tam = (int)(64 * cam.zoom);

        if (pantX + tam > 0 && pantX < anchoP && pantY + tam > 0 &&
            pantY < altoP) {
          SelectObject(hdcSprites, hObreroBmp[o->dir]);
          TransparentBlt(hdcBuffer, pantX, pantY, tam, tam, hdcSprites, 0, 0,
                         64, 64, RGB(255, 255, 255));

          // Círculo de selección
          if (o->seleccionado) {
            HBRUSH nullBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
            HPEN verde = CreatePen(PS_SOLID, 2, RGB(0, 255, 0));
            SelectObject(hdcBuffer, nullBrush);
            SelectObject(hdcBuffer, verde);
            Ellipse(hdcBuffer, pantX, pantY + tam - 10, pantX + tam,
                    pantY + tam + 5);
            DeleteObject(verde);
          }
        }
      }
    }
  }

  // 3. DIBUJAR INTERFAZ DE USUARIO (MENÚ) - AQUÍ EVITAMOS FLICKERING
  // Dibujamos el menú sobre el buffer ANTES de volcarlo a pantalla
  if (menu != NULL) {
    menuCompraDibujar(hdcBuffer, menu, pJugador);
  }

  // Copiar el buffer COMPLETO (Juego + UI) a la pantalla de una vez
  BitBlt(hdc, 0, 0, anchoP, altoP, hdcBuffer, 0, 0, SRCCOPY);

  // Limpieza
  DeleteDC(hdcSprites);
  SelectObject(hdcMapa, hOldMapa);
  DeleteDC(hdcMapa);
  SelectObject(hdcBuffer, hOldBuffer);
  DeleteObject(hbmBuffer);
  DeleteDC(hdcBuffer);
}

