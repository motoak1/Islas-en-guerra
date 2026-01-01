#include "mapa.h"
#include "../edificios/edificios.h"
#include "../recursos/recursos.h"
#include "../recursos/ui_compra.h"
#include "../recursos/ui_embarque.h"
#include "../recursos/navegacion.h"
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

#define caballero_front "../assets/caballero/caballero_front.bmp"
#define caballero_back "../assets/caballero/caballero_back.bmp"
#define caballero_left "../assets/caballero/caballero_walkLeft3.bmp"
#define caballero_right "../assets/caballero/caballero_walkRight3.bmp"

#define CABALLO_F_ALT "assets/caballero/caballero_front.bmp"
#define CABALLO_B_ALT "assets/caballero/caballero_back.bmp"
#define CABALLO_L_ALT "assets/caballero/caballero_walkLeft3.bmp"
#define CABALLO_R_ALT "assets/caballero/caballero_walkRight3.bmp"

#define guerrero_front "../assets/guerrero/guerrero_front.bmp"
#define guerrero_back "../assets/guerrero/guerrero_back.bmp"
#define guerrero_left "../assets/guerrero/guerrero_left.bmp"
#define guerrero_right "../assets/guerrero/guerrero_right.bmp"

#define GUERRERO_F_ALT "assets/guerrero/guerrero_front.bmp"
#define GUERRERO_B_ALT "assets/guerrero/guerrero_back.bmp"
#define GUERRERO_L_ALT "assets/guerrero/guerrero_left.bmp"
#define GUERRERO_R_ALT "assets/guerrero/guerrero_right.bmp"

#define VACA_F "../assets/vaca/vaca_front.bmp"
#define VACA_B "../assets/vaca/vaca_back.bmp"
#define VACA_L "../assets/vaca/vaca_left.bmp"
#define VACA_R "../assets/vaca/vaca_right.bmp"

#define VACA_F_ALT "assets/vaca/vaca_front.bmp"
#define VACA_B_ALT "assets/vaca/vaca_back.bmp"
#define VACA_L_ALT "assets/vaca/vaca_left.bmp"
#define VACA_R_ALT "assets/vaca/vaca_right.bmp"

static HBITMAP hObreroBmp[4] = {NULL};    // Front, Back, Left, Right
static HBITMAP hCaballeroBmp[4] = {NULL}; // Front, Back, Left, Right (NUEVO)
static HBITMAP hBarcoBmp[4] = {NULL};     // Front, Back, Left, Right (192x192)

static HBITMAP hGuerreroBmp[4] = {NULL}; // Front, Back, Left, Right

static HBITMAP hVacaBmp[4] = {NULL};

// Definiciones para obrerro fallback
#define OBRERO_F_ALT "assets/obrero/obrero_front.bmp"
#define OBRERO_B_ALT "assets/obrero/obrero_back.bmp"
#define OBRERO_L_ALT "assets/obrero/obrero_left.bmp"
#define OBRERO_R_ALT "assets/obrero/obrero_right.bmp"

#define BARCO_F_ALT "../assets/barco/barco_front.bmp"
#define BARCO_B_ALT "../assets/barco/barco_back.bmp"
#define BARCO_L_ALT "../assets/barco/barco_left.bmp"
#define BARCO_R_ALT "../assets/barco/barco_right.bmp"




static HBITMAP hMapaBmp = NULL;        // Mapa de isla individual (isla1, isla2, o isla3)
static HBITMAP hMapaGlobalBmp = NULL;  // NUEVO: Mapa global con las 3 islas (mapaDemo2.bmp)
static HBITMAP hArboles[4] = {NULL};

static char gRutaMapaPrincipal[MAX_PATH] = RUTA_MAPA;
static char gRutaMapaAlterna[MAX_PATH] = RUTA_MAPA_ALT;

// Matriz lógica de objetos - ahora usa CARACTERES (32x32 con celdas de 64px)
char mapaObjetos[GRID_SIZE][GRID_SIZE] = {0};

// --- COLISIONES (matriz dinámica int**)
// 0 = libre, 1 = ocupado (árboles u obstáculos)
static int **gCollisionMap = NULL;

// --- SISTEMA DE VACAS DINÁMICAS ---
// Array de vacas con movimiento automático (10 vacas)
static Vaca gVacas[10];
static int gNumVacas = 0; // Cantidad de vacas activas

static void detectarAguaEnMapa(void);
void mapaMarcarArea(int f_inicio, int c_inicio, int ancho_celdas, int alto_celdas, int valor);

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

void mapaSeleccionarIsla(int isla) {
  int seleccion = (isla >= 1 && isla <= 3) ? isla : 1;
  snprintf(gRutaMapaPrincipal, sizeof(gRutaMapaPrincipal), "..\\assets\\islas\\isla%d.bmp", seleccion);
  snprintf(gRutaMapaAlterna, sizeof(gRutaMapaAlterna), "assets/islas/isla%d.bmp", seleccion);
}

void mapaReconstruirCollisionMap(void) {
    collisionMapAllocIfNeeded();
    collisionMapClear(0); 

    // mapaObjetos ahora es 32x32 chars, collision map también es 32x32
    // Mapeo 1:1 entre mapaObjetos y collisionMap (cada celda = 64px)
    for (int f = 0; f < GRID_SIZE; f++) {
        for (int c = 0; c < GRID_SIZE; c++) {
            // Si hay un arbol en esta celda, marcar como obstaculo en collision map
            if (mapaObjetos[f][c] == SIMBOLO_ARBOL) {
                gCollisionMap[f][c] = 1; // Bloquear celda
            }
            // NUEVO: Las vacas tambien bloquean el paso temporalmente
            if (mapaObjetos[f][c] == SIMBOLO_VACA) {
                gCollisionMap[f][c] = 3; // Mismo valor que unidades temporales
            }
        }
    }
    detectarAguaEnMapa(); 
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

  printf("\n[DEBUG AGUA] ============================================\n");
  printf("[DEBUG AGUA] Iniciando deteccion de agua con GetDIBits...\n");
  fflush(stdout);

  // ================================================================
  // OBTENER INFORMACIÓN DEL BITMAP
  // ================================================================
  BITMAP bm;
  if (!GetObject(hMapaBmp, sizeof(BITMAP), &bm)) {
    printf("[DEBUG AGUA] ERROR: No se pudo obtener info del bitmap\n");
    fflush(stdout);
    return;
  }

  printf("[DEBUG AGUA] Bitmap dimensions: %dx%d\n", bm.bmWidth, bm.bmHeight);
  printf("[DEBUG AGUA] Bits per pixel: %d\n", bm.bmBitsPixel);
  fflush(stdout);

  // ================================================================
  // CONFIGURAR BITMAPINFO PARA LEER PIXELS
  // ================================================================
  BITMAPINFO bmi;
  ZeroMemory(&bmi, sizeof(BITMAPINFO));
  bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  bmi.bmiHeader.biWidth = bm.bmWidth;
  bmi.bmiHeader.biHeight = -bm.bmHeight;  // Negativo = top-down
  bmi.bmiHeader.biPlanes = 1;
  bmi.bmiHeader.biBitCount = 24;  // 24-bit RGB
  bmi.bmiHeader.biCompression = BI_RGB;

  // ================================================================
  // LEER TODO EL BITMAP A MEMORIA
  // ================================================================
  int rowSize = ((bm.bmWidth * 3 + 3) & ~3);  // Alineado a 4 bytes
  int imageSize = rowSize * bm.bmHeight;
  BYTE *pixelData = (BYTE*)malloc(imageSize);
  
  if (!pixelData) {
    printf("[DEBUG AGUA] ERROR: No se pudo asignar memoria\n");
    fflush(stdout);
    return;
  }

  HDC hdcScreen = GetDC(NULL);
  int result = GetDIBits(hdcScreen, hMapaBmp, 0, bm.bmHeight, pixelData, &bmi, DIB_RGB_COLORS);
  ReleaseDC(NULL, hdcScreen);

  if (!result) {
    printf("[DEBUG AGUA] ERROR: GetDIBits fallo\n");
    free(pixelData);
    fflush(stdout);
    return;
  }

  printf("[DEBUG AGUA] Bitmap leido exitosamente a memoria\n");
  fflush(stdout);

  // ================================================================
  // MUESTREO: Verificar primeros píxeles
  // ================================================================
  printf("\n[DEBUG AGUA] === MUESTREO DE COLORES ===\n");
  int posicionesTest[][2] = {{5,5}, {10,10}, {15,15}, {7,7}, {14,8}};
  
  for (int i = 0; i < 5; i++) {
    int f = posicionesTest[i][0];
    int c = posicionesTest[i][1];
    int px = (c * TILE_SIZE) + 16;
    int py = (f * TILE_SIZE) + 16;
    
    if (px >= 0 && px < bm.bmWidth && py >= 0 && py < bm.bmHeight) {
      int offset = py * rowSize + px * 3;
      BYTE b = pixelData[offset + 0];
      BYTE g = pixelData[offset + 1];
      BYTE r = pixelData[offset + 2];
      
      bool verde = (g > r && g > b && g > 45);
      bool beige = (r > 80 && g > 80 && b < 100 && abs(r-g) < 50);
      printf("[DEBUG] Celda[%2d][%2d] @ (%4d,%4d): RGB(%3d,%3d,%3d) -> %s\n", 
             f, c, px, py, r, g, b, (verde || beige) ? "TIERRA" : "AGUA");
    }
  }
  fflush(stdout);

  // ================================================================
  // DETECCIÓN COMPLETA DE AGUA
  // ================================================================
  int contadorAgua = 0;
  int contadorTierra = 0;
  int contadorFueraDeLimites = 0;

  for (int f = 0; f < GRID_SIZE; f++) {
    int *fila = *(gCollisionMap + f);
    for (int c = 0; c < GRID_SIZE; c++) {
      // No sobreescribir árboles u obstáculos
      if (*(fila + c) != 0) continue;

      // Calcular pixel central de la celda
      int px = (c * TILE_SIZE) + 16;
      int py = (f * TILE_SIZE) + 16;

      // Verificar límites
      if (px < 0 || px >= bm.bmWidth || py < 0 || py >= bm.bmHeight) {
        contadorFueraDeLimites++;
        continue;
      }

      // Leer pixel desde buffer
      int offset = py * rowSize + px * 3;
      BYTE b = pixelData[offset + 0];
      BYTE g = pixelData[offset + 1];
      BYTE r = pixelData[offset + 2];

      // ================================================================
      // CRITERIO: DETECTAR SOLO AGUA AZUL
      // ================================================================
      // AGUA debe tener azul dominante sobre rojo y verde
      // Arena/beige/verde son transitables
      // ================================================================
      
      bool esAguaAzulOscura = (b > r + 20 && b > g + 20 && b > 60);  // Azul oscuro
      bool esAguaAzulClara = (b > r && b > g && b > 100);             // Azul claro (celeste)
      
      bool esAgua = esAguaAzulOscura || esAguaAzulClara;

      if (esAgua) {
        // ES AGUA - marcar como impasable
        *(fila + c) = 1;
        mapaObjetos[f][c] = SIMBOLO_AGUA;  // NUEVO: Marcar con '~' en mapaObjetos
        contadorAgua++;
        
        // Debug primeras 5 escrituras
        if (contadorAgua <= 5) {
          printf("[DEBUG] AGUA DETECTADA: gCollisionMap[%d][%d] = 1 (RGB=%d,%d,%d)\n", 
                 f, c, r, g, b);
          fflush(stdout);
        }
      } else {
        // NO es agua - es tierra transitable (verde, arena, beige, etc)
        contadorTierra++;
      }
    }
  }

  // Liberar memoria
  free(pixelData);

  // ================================================================
  // REPORTE FINAL
  // ================================================================
  printf("\n[DEBUG AGUA] === RESUMEN ===\n");
  printf("[DEBUG AGUA] Celdas de AGUA: %d\n", contadorAgua);
  printf("[DEBUG AGUA] Celdas de TIERRA: %d\n", contadorTierra);
  printf("[DEBUG AGUA] Fuera de limites: %d\n", contadorFueraDeLimites);
  printf("[DEBUG AGUA] Total procesado: %d\n", contadorAgua + contadorTierra + contadorFueraDeLimites);
  fflush(stdout);

  // Verificación post-escritura
  printf("\n[DEBUG AGUA] === VERIFICACION POST-ESCRITURA ===\n");
  for (int i = 0; i < 5; i++) {
    int f = posicionesTest[i][0];
    int c = posicionesTest[i][1];
    int valor = *(*(gCollisionMap + f) + c);
    printf("[DEBUG] gCollisionMap[%d][%d] = %d\n", f, c, valor);
  }

  // Snapshot de fila 10
  printf("\n[DEBUG] Fila 10: ");
  for (int c = 0; c < 20; c++) {
    printf("%d", *(*(gCollisionMap + 10) + c));
  }
  printf("...\n");

  if (contadorAgua == 0) {
    printf("\n[DEBUG AGUA] WARNING: No se detecto agua, activando fallback...\n");
    // Marcar bordes como agua
    for (int i = 0; i < GRID_SIZE; i++) {
      if (*(*(gCollisionMap + 0) + i) == 0) *(*(gCollisionMap + 0) + i) = 1;
      if (*(*(gCollisionMap + GRID_SIZE-1) + i) == 0) *(*(gCollisionMap + GRID_SIZE-1) + i) = 1;
      if (*(*(gCollisionMap + i) + 0) == 0) *(*(gCollisionMap + i) + 0) = 1;
      if (*(*(gCollisionMap + i) + GRID_SIZE-1) == 0) *(*(gCollisionMap + i) + GRID_SIZE-1) = 1;
    }
  }

  printf("[DEBUG AGUA] Deteccion completada exitosamente.\n");
  printf("[DEBUG AGUA] ============================================\n\n");
  fflush(stdout);
}

// ============================================================================
// DETECCIÓN AUTOMÁTICA DE ORILLA PARA BARCO (192x192px)
// ============================================================================
// Encuentra una posición válida en la orilla del mapa donde:
// - El barco está EN EL AGUA (valor 1 = azul)
// - Hay tierra adyacente (para confirmar que es orilla, no mar abierto)
// - Hay espacio suficiente para el barco (6x6 celdas = 192px de agua)
// Retorna la posición en píxeles y la dirección hacia la tierra
// ============================================================================
void mapaDetectarOrilla(float *outX, float *outY, int *outDir) {
  int **col = mapaObtenerCollisionMap();
  if (!col) {
    *outX = 512.0f;
    *outY = 512.0f;
    *outDir = DIR_FRONT;
    return;
  }
  
  printf("\n[DEBUG BARCO] ============================================\n");
  printf("[DEBUG BARCO] Buscando AGUA para colocar barco...\n");
  fflush(stdout);
  
  // El barco ocupa 6x6 celdas (192px / 32px = 6)
  const int BARCO_CELDAS = 6;
  
  // Direcciones: arriba, abajo, izquierda, derecha
  int dF[4] = {-1, 1, 0, 0};
  int dC[4] = {0, 0, -1, 1};
  // Orientación HACIA la tierra (inverso)
  Direccion direcciones[4] = {DIR_FRONT, DIR_BACK, DIR_RIGHT, DIR_LEFT};
  
  // Buscar desde el centro del mapa hacia afuera
  int centroF = GRID_SIZE / 2;
  int centroC = GRID_SIZE / 2;
  
  for (int radio = 10; radio < GRID_SIZE / 2; radio++) {
    for (int f = centroF - radio; f <= centroF + radio; f++) {
      for (int c = centroC - radio; c <= centroC + radio; c++) {
        // Verificar límites (el barco necesita espacio 6x6)
        if (f < 1 || f >= GRID_SIZE - BARCO_CELDAS - 1 || 
            c < 1 || c >= GRID_SIZE - BARCO_CELDAS - 1) continue;
        
        // ================================================================
        // CRÍTICO: La celda actual debe ser AGUA (valor 1)
        // ================================================================
        if (*(*(col + f) + c) != 1) continue;
        
        // Verificar que el bloque 6x6 completo sea AGUA (valor 1)
        bool bloqueAgua = true;
        int celdasAgua = 0;
        for (int df = 0; df < BARCO_CELDAS && bloqueAgua; df++) {
          int *fila_ptr = *(col + f + df);
          for (int dc = 0; dc < BARCO_CELDAS && bloqueAgua; dc++) {
            int valor = *(fila_ptr + c + dc);
            // Debe ser agua (1) para que el barco flote
            if (valor == 1) {
              celdasAgua++;
            } else {
              // Si encuentra tierra o árbol, este bloque no es válido
              bloqueAgua = false;
            }
          }
        }
        
        if (!bloqueAgua) continue;
        
        // Verificar que al menos el 90% del bloque sea agua
        if (celdasAgua < (BARCO_CELDAS * BARCO_CELDAS * 9 / 10)) continue;
        
        // Buscar TIERRA adyacente en las 4 direcciones para confirmar que es orilla
        for (int d = 0; d < 4; d++) {
          int nf = f + dF[d];
          int nc = c + dC[d];
          
          // Verificar límites
          if (nf < 0 || nf >= GRID_SIZE || nc < 0 || nc >= GRID_SIZE) continue;
          
          // Verificar si la celda adyacente es TIERRA (0)
          if (*(*(col + nf) + nc) == 0) {
            // ¡Encontramos una orilla válida! El barco está en agua, orientado hacia tierra
            *outX = (float)(c * TILE_SIZE);
            *outY = (float)(f * TILE_SIZE);
            *outDir = direcciones[d];
            
            printf("[DEBUG BARCO] Orilla encontrada!\n");
            printf("[DEBUG BARCO]   Celda AGUA: [%d][%d]\n", f, c);
            printf("[DEBUG BARCO]   Posicion: (%.1f, %.1f)\n", *outX, *outY);
            printf("[DEBUG BARCO]   Direccion hacia tierra: %d\n", *outDir);
            printf("[DEBUG BARCO]   Espacio: %dx%d celdas de AGUA\n", 
                   BARCO_CELDAS, BARCO_CELDAS);
            printf("[DEBUG BARCO]   Celdas de agua verificadas: %d/%d\n",
                   celdasAgua, BARCO_CELDAS * BARCO_CELDAS);
            printf("[DEBUG BARCO] ============================================\n\n");
            fflush(stdout);
            return;
          }
        }
      }
    }
  }
  
  // FALLBACK: Si no se encuentra orilla ideal, buscar CUALQUIER celda de agua
  printf("[WARNING BARCO] No se encontró orilla con tierra adyacente, buscando agua...\n");
  for (int f = 0; f < GRID_SIZE - BARCO_CELDAS; f++) {
    for (int c = 0; c < GRID_SIZE - BARCO_CELDAS; c++) {
      // Verificar si hay un bloque 6x6 de agua
      bool esAgua = true;
      for (int df = 0; df < BARCO_CELDAS && esAgua; df++) {
        for (int dc = 0; dc < BARCO_CELDAS && esAgua; dc++) {
          if (*(*(col + f + df) + c + dc) != 1) {
            esAgua = false;
          }
        }
      }
      
      if (esAgua) {
        *outX = (float)(c * TILE_SIZE);
        *outY = (float)(f * TILE_SIZE);
        *outDir = DIR_FRONT;
        printf("[DEBUG BARCO] Agua encontrada en posición: (%.1f, %.1f)\n", *outX, *outY);
        fflush(stdout);
        return;
      }
    }
  }
  
  // Último recurso: centro del mapa (probablemente error en el mapa)
  *outX = 1000.0f;
  *outY = 1000.0f;
  *outDir = DIR_FRONT;
  printf("[ERROR BARCO] ¡No se encontró NINGUNA agua en todo el mapa!\n");
  printf("[DEBUG BARCO] ============================================\n\n");
  fflush(stdout);
}

void generarBosqueAutomatico() {
  if (!hMapaBmp)
    return;

  HDC hdcPantalla = GetDC(NULL);
  HDC hdcMem = CreateCompatibleDC(hdcPantalla);
  HBITMAP hOldBmp = (HBITMAP)SelectObject(hdcMem, hMapaBmp);

  // Requisito: Aritmética de punteros para manejar la matriz
  char (*ptrMatriz)[GRID_SIZE] = mapaObjetos;
  srand((unsigned int)time(NULL));
  
  // REQUISITO CRÍTICO: Colocar exactamente 40 árboles (no por probabilidad)
  const int NUM_ARBOLES_EXACTO = 40;
  int contador = 0;
  
  // Bucle while que continúa hasta colocar exactamente 40 árboles
  while (contador < NUM_ARBOLES_EXACTO) {
    // Generar coordenadas aleatorias en la matriz 32x32
    int fila = rand() % GRID_SIZE;
    int col = rand() % GRID_SIZE;
    
    // Verificar que la celda esté vacía (no haya otro árbol)
    if (*(*(ptrMatriz + fila) + col) != SIMBOLO_VACIO && *(*(ptrMatriz + fila) + col) != 0) {
      continue; // Ya hay algo aquí, intentar otra posición
    }
    
    // Calcular píxel central de la celda para verificar el color del suelo
    int px = (col * TILE_SIZE) + (TILE_SIZE / 2);
    int py = (fila * TILE_SIZE) + (TILE_SIZE / 2);
    
    COLORREF color = GetPixel(hdcMem, px, py);
    if (color == CLR_INVALID)
      continue;
    
    BYTE r = GetRValue(color);
    BYTE g = GetGValue(color);
    BYTE b = GetBValue(color);
    
    // Verificar que el suelo sea verde (tierra válida para árbol)
    // Solo colocar en tierra (verde domina), no en agua (azul domina)
    // CRITERIO MÁS ESTRICTO: verde debe dominar claramente y azul debe ser bajo
    if (g > r && g > b && g > 70 && b < 80 && r < 100) {
      // Colocar árbol usando aritmética de punteros y símbolo de caracter
      *(*(ptrMatriz + fila) + col) = SIMBOLO_ARBOL;
      contador++;
    }
  }
  
  printf("[DEBUG] Logica: %d arboles registrados en la matriz con punteros.\n",
         contador);


  // ============================================================================
  // GENERACIÓN DE VACAS DINÁMICAS (en lugar de estáticas en mapaObjetos)
  // ============================================================================
  const int NUM_VACAS_EXACTO = 10;
  gNumVacas = 0;
  
  while (gNumVacas < NUM_VACAS_EXACTO) {
    int fila = rand() % GRID_SIZE;
    int col = rand() % GRID_SIZE;
    
    // No colocar vaca donde ya hay árbol u otra vaca
    if (*(*(ptrMatriz + fila) + col) != 0) continue;
    
    // Verificar que el suelo sea tierra (mismo criterio que árboles)
    int px = (col * TILE_SIZE) + (TILE_SIZE / 2);
    int py = (fila * TILE_SIZE) + (TILE_SIZE / 2);
    
    COLORREF color = GetPixel(hdcMem, px, py);
    if (color == CLR_INVALID) continue;
    
    BYTE r = GetRValue(color);
    BYTE g = GetGValue(color);
    BYTE b = GetBValue(color);
    
    // Solo en tierra verde (mismo criterio estricto que árboles)
    if (g > r && g > b && g > 70 && b < 80 && r < 100) {
      // Inicializar vaca en el array dinámico
      Vaca *v = &gVacas[gNumVacas];
      
      // Posición: centro de la celda en píxeles
      v->x = (float)(col * TILE_SIZE);
      v->y = (float)(fila * TILE_SIZE);
      
      // Dirección aleatoria (0-3 = DIR_FRONT, DIR_BACK, DIR_LEFT, DIR_RIGHT)
      v->dir = (Direccion)(rand() % 4);
      
      // Timer aleatorio para que no se muevan todas a la vez
      v->timerMovimiento = rand() % 120;
      
      // NUEVO: Registrar vaca en mapaObjetos para identificar su posicion
      *(*(ptrMatriz + fila) + col) = SIMBOLO_VACA;
      
      gNumVacas++;
    }
  }
  
  printf("[DEBUG] Logica: %d vacas dinámicas generadas.\n", gNumVacas);
  
  
  
  // Construir la grilla de colisión con árboles Y detectar agua
  // NOTA: mapaReconstruirCollisionMap() YA llama a detectarAguaEnMapa() internamente
  mapaReconstruirCollisionMap();

  SelectObject(hdcMem, hOldBmp);
  DeleteDC(hdcMem);
  ReleaseDC(NULL, hdcPantalla);
}

void cargarRecursosGraficos() {
  // 1. Cargar Mapa Base (Intento doble)
  hMapaBmp = (HBITMAP)LoadImageA(NULL, gRutaMapaPrincipal, IMAGE_BITMAP, 0, 0,
                                 LR_LOADFROMFILE | LR_CREATEDIBSECTION);
  if (!hMapaBmp)
    hMapaBmp = (HBITMAP)LoadImageA(NULL, gRutaMapaAlterna, IMAGE_BITMAP, 0, 0,
                                   LR_LOADFROMFILE | LR_CREATEDIBSECTION);

  // NUEVO: Cargar mapa global con las 3 islas (mapaDemo2.bmp)
  hMapaGlobalBmp = (HBITMAP)LoadImageA(NULL, "../assets/mapaDemo2.bmp", 
                                       IMAGE_BITMAP, 0, 0,
                                       LR_LOADFROMFILE | LR_CREATEDIBSECTION);
  if (!hMapaGlobalBmp) {
    hMapaGlobalBmp = (HBITMAP)LoadImageA(NULL, "assets/mapaDemo2.bmp", 
                                         IMAGE_BITMAP, 0, 0,
                                         LR_LOADFROMFILE | LR_CREATEDIBSECTION);
  }
  
  if (!hMapaGlobalBmp) {
    printf("[ERROR] No se pudo cargar mapaDemo2.bmp para vista global\n");
  } else {
    printf("[OK] Mapa global (mapaDemo2.bmp) cargado correctamente\n");
  }

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
      printf("[ERROR] No se pudo cargar obrero[%d]\n", i);
    } else {
      printf("[OK] Obrero BMP %d cargado correctamente.\n", i);
    }
  }

  // --- CARGAR SPRITES DE CABALLEROS ---


  const char *rutasCab[] = {caballero_front, caballero_back, caballero_left,
                            caballero_right};
  const char *rutasCabAlt[] = {CABALLO_F_ALT, CABALLO_B_ALT, CABALLO_L_ALT,
                               CABALLO_R_ALT};

  for (int i = 0; i < 4; i++) {
    hCaballeroBmp[i] = (HBITMAP)LoadImageA(NULL, rutasCab[i], IMAGE_BITMAP, 64, 64,
                                        LR_LOADFROMFILE);
    if (!hCaballeroBmp[i]) {
      hCaballeroBmp[i] = (HBITMAP)LoadImageA(NULL, rutasCabAlt[i], IMAGE_BITMAP,
                                          64, 64, LR_LOADFROMFILE);
    }

    if (!hCaballeroBmp[i]) {
      printf("[ERROR] No se pudo cargar caballero[%d]\n", i);
    } else {
      printf("[OK] Caballero BMP %d cargado correctamente.\n", i);
    }
  }

  // --- CARGAR SPRITES DE GUERREROS ---
  const char *rutasGuerr[] = {guerrero_front, guerrero_back, guerrero_left, guerrero_right};

  const char *rutasGuerrAlt[] = {GUERRERO_F_ALT, GUERRERO_B_ALT, GUERRERO_L_ALT, GUERRERO_R_ALT};

  for (int i = 0; i < 4; i++) {
    hGuerreroBmp[i] = (HBITMAP)LoadImageA(NULL, rutasGuerr[i], IMAGE_BITMAP, 64, 64, LR_LOADFROMFILE);
    if (!hGuerreroBmp[i]) {
      hGuerreroBmp[i] = (HBITMAP)LoadImageA(NULL, rutasGuerrAlt[i], IMAGE_BITMAP, 64, 64, LR_LOADFROMFILE);
    }
    if (!hGuerreroBmp[i]) {
      printf("[ERROR] No se pudo cargar guerrero[%d]\\n", i);
    } else {
      printf("[OK] Guerrero BMP %d cargado correctamente.\\n", i);
    }
  }

  // --- CARGAR SPRITES DE BARCO (192x192) ---
  const char *rutasBarcoAlt[] = {BARCO_F_ALT, BARCO_B_ALT, BARCO_L_ALT, BARCO_R_ALT};

  for (int i = 0; i < 4; i++) {
    hBarcoBmp[i] = (HBITMAP)LoadImageA(NULL, rutasBarcoAlt[i], IMAGE_BITMAP, 
                                        192, 192, LR_LOADFROMFILE);

    if (!hBarcoBmp[i]) {
      printf("[ERROR] No se pudo cargar barco[%d]\n", i);
    } else {
      printf("[OK] Barco BMP %d cargado correctamente (192x192).\n", i);
    }
  }

  const char *rutasVaca[] = {VACA_F, VACA_B, VACA_L, VACA_R};
  const char *rutasVacaAlt[] = {VACA_F_ALT, VACA_B_ALT, VACA_L_ALT, VACA_R_ALT};
  for (int i = 0; i < 4; i++) {
      hVacaBmp[i] = (HBITMAP)LoadImageA(NULL, rutasVaca[i], IMAGE_BITMAP, 
                                        64, 64, LR_LOADFROMFILE);
      if (!hVacaBmp[i]) {
          hVacaBmp[i] = (HBITMAP)LoadImageA(NULL, rutasVacaAlt[i], IMAGE_BITMAP,
                                            64, 64, LR_LOADFROMFILE);
      }
      printf("[%s] Vaca BMP %d cargado.\\n", hVacaBmp[i] ? "OK" : "ERROR", i);
  }

  generarBosqueAutomatico();
}

// ============================================================================
// ACTUALIZACIÓN DE VACAS (MOVIMIENTO AUTOMÁTICO)
// ============================================================================
// Se ejecuta cada frame (60 FPS). Cada vaca tiene un timer que cuenta hasta 120
// frames (~2 segundos). Cuando el timer llega a 120, la vaca se mueve una celda
// en una dirección aleatoria, validando colisiones antes de moverse.
// ============================================================================
void mapaActualizarVacas(void) {
  if (!gCollisionMap) return;
  
  // Iterar sobre todas las vacas usando punteros
  Vaca *v = gVacas;
  for (int i = 0; i < gNumVacas; i++, v++) {
    // Incrementar timer de movimiento
    v->timerMovimiento++;
    
    // Si el timer alcanza 120 frames (2 segundos a 60 FPS), mover la vaca
    if (v->timerMovimiento >= 120) {
      // Resetear timer
      v->timerMovimiento = 0;
      
      // Intentar mover en una dirección aleatoria
      // Probar hasta 4 direcciones diferentes si hay obstáculos
      int intentos = 4;
      int direccionBase = rand() % 4; // 0=front, 1=back, 2=left, 3=right
      
      for (int d = 0; d < intentos; d++) {
        int direccion = (direccionBase + d) % 4;
        
        // Calcular nueva posición según la dirección
        float nuevoX = v->x;
        float nuevoY = v->y;
        
        switch (direccion) {
          case DIR_FRONT:  // Abajo
            nuevoY += TILE_SIZE;
            break;
          case DIR_BACK:   // Arriba
            nuevoY -= TILE_SIZE;
            break;
          case DIR_LEFT:   // Izquierda
            nuevoX -= TILE_SIZE;
            break;
          case DIR_RIGHT:  // Derecha
            nuevoX += TILE_SIZE;
            break;
        }
        
        // Verificar límites del mapa
        if (nuevoX < 0 || nuevoX >= MAPA_SIZE - TILE_SIZE ||
            nuevoY < 0 || nuevoY >= MAPA_SIZE - TILE_SIZE) {
          continue; // Fuera de límites, probar otra dirección
        }
        
        // Convertir a coordenadas de celda para verificar colisión
        int celdaX = (int)(nuevoX / TILE_SIZE);
        int celdaY = (int)(nuevoY / TILE_SIZE);
        
        // Verificar que esté dentro de la grid
        if (celdaX < 0 || celdaX >= GRID_SIZE ||
            celdaY < 0 || celdaY >= GRID_SIZE) {
          continue;
        }
        
        // Verificar colision usando aritmetica de punteros
        int valorColision = *(*(gCollisionMap + celdaY) + celdaX);
        
        // Verificar tambien en mapaObjetos que no haya objetos
        char simboloDestino = mapaObjetos[celdaY][celdaX];
        
        // ================================================================
        // VERIFICACION DE BLOQUEO COMPLETA:
        // - collisionMap: 0=libre, 1=agua/arbol, 2=edificio, 3=unidad
        // - mapaObjetos: '.'=vacio, 'A'=arbol, 'E'=edificio, 'M'=mina, 
        //                'V'=vaca, 'O'/'C'/'G'=personajes
        // ================================================================
        // Solo moverse si AMBAS condiciones son true:
        // 1. collisionMap == 0 (no hay agua, arbol, edificio ni unidad)
        // 2. mapaObjetos esta vacio (no hay arbol, edificio, mina, vaca, etc)
        // ================================================================
        bool colisionLibre = (valorColision == 0);
        bool celdaVacia = (simboloDestino == SIMBOLO_VACIO || simboloDestino == 0);
        
        if (colisionLibre && celdaVacia) {
          // Calcular celda actual de la vaca ANTES de moverse
          int viejaCeldaX = (int)(v->x / TILE_SIZE);
          int viejaCeldaY = (int)(v->y / TILE_SIZE);
          
          // Sincronizar movimiento con mapaObjetos
          mapaMoverObjeto(v->x, v->y, nuevoX, nuevoY, SIMBOLO_VACA);
          
          // NUEVO: Sincronizar gCollisionMap (limpiar vieja celda, marcar nueva)
          if (viejaCeldaY >= 0 && viejaCeldaY < GRID_SIZE && 
              viejaCeldaX >= 0 && viejaCeldaX < GRID_SIZE) {
            *(*(gCollisionMap + viejaCeldaY) + viejaCeldaX) = 0; // Liberar celda anterior
          }
          *(*(gCollisionMap + celdaY) + celdaX) = 3; // Marcar nueva celda como ocupada
          
          // Actualizar posicion y direccion
          v->x = nuevoX;
          v->y = nuevoY;
          v->dir = (Direccion)direccion;
          break; // Salir del bucle de intentos
        }
        
        // Si llegamos aquí, esta dirección está bloqueada, probar la siguiente
      }
    }
  }
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
                  struct MenuCompra *menu, MenuEmbarque *menuEmb,
                  int highlightFila, int highlightCol) {
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

  // 1.5. DIBUJAR EDIFICIOS - antes del Y-sorting para que estén debajo de unidades
  if (pJugador->ayuntamiento != NULL) {
    Edificio *edificio = (Edificio *)pJugador->ayuntamiento;
    edificioDibujar(hdcBuffer, edificio, cam.x, cam.y, cam.zoom, anchoP, altoP);
  }

  // Dibujar mina
  if (pJugador->mina != NULL) {
    Edificio *edificioMina = (Edificio *)pJugador->mina;
    edificioDibujar(hdcBuffer, edificioMina, cam.x, cam.y, cam.zoom, anchoP, altoP);
  }

  HDC hdcSprites = CreateCompatibleDC(hdc);

  // Puntero a la matriz de objetos para aritmética de punteros
  char (*ptrMatriz)[GRID_SIZE] = mapaObjetos;

  // Recorrer cada fila del mapa (0 a GRID_SIZE-1)
  for (int f = 0; f < GRID_SIZE; f++) {
    // ================================================================
    // A) DIBUJAR ÁRBOLES DE ESTA FILA
    // ================================================================
    for (int c = 0; c < GRID_SIZE; c++) {
      // Obtener el contenido de la celda usando aritmética de punteros
      char celdaContenido = *(*(ptrMatriz + f) + c);
      
      // Si hay un árbol en esta celda, dibujarlo
      if (celdaContenido == SIMBOLO_ARBOL) {
        int mundoX = c * TILE_SIZE;
        int mundoY = f * TILE_SIZE;

        int pantX = (int)((mundoX - cam.x) * cam.zoom);
        int pantY = (int)((mundoY - cam.y) * cam.zoom);
        int tamZoom = (int)(SPRITE_ARBOL * cam.zoom);

        if (pantX + tamZoom > 0 && pantX < anchoP && pantY + tamZoom > 0 &&
            pantY < altoP) {
          // Usar el primer sprite de árbol (índice 0)
          SelectObject(hdcSprites, hArboles[0]);
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
    Unidad *baseObreros = pJugador->obreros;

    for (Unidad *o = baseObreros; o < baseObreros + 6; o++) {
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
    
    // C) DIBUJAR CABALLEROS (NUEVO)
    Unidad *baseCaballeros = pJugador->caballeros;
    for (Unidad *c = baseCaballeros; c < baseCaballeros + 4; c++) {
      float basePies = c->y + (float)TILE_SIZE;
      
      if (basePies >= (float)yMinFila && basePies < (float)yMaxFila) {
        int pantX = (int)((c->x - cam.x) * cam.zoom);
        int pantY = (int)((c->y - cam.y) * cam.zoom);
        int tam = (int)(64 * cam.zoom);

        if (pantX + tam > 0 && pantX < anchoP && pantY + tam > 0 && pantY < altoP) {
          SelectObject(hdcSprites, hCaballeroBmp[c->dir]);
          TransparentBlt(hdcBuffer, pantX, pantY, tam, tam, hdcSprites, 0, 0,
                         64, 64, RGB(255, 255, 255));

          // Círculo de selección
          if (c->seleccionado) {
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
    
    

    // D) DIBUJAR GUERREROS (NUEVO)
Unidad *baseGuerreros = pJugador->guerreros;
for (Unidad *g = baseGuerreros; g < baseGuerreros + 2; g++) {
  float basePies = g->y + (float)TILE_SIZE;
  
  if (basePies >= (float)yMinFila && basePies < (float)yMaxFila) {
    int pantX = (int)((g->x - cam.x) * cam.zoom);
    int pantY = (int)((g->y - cam.y) * cam.zoom);
    int tam = (int)(64 * cam.zoom);
    if (pantX + tam > 0 && pantX < anchoP && pantY + tam > 0 && pantY < altoP) {
      SelectObject(hdcSprites, hGuerreroBmp[g->dir]);
      TransparentBlt(hdcBuffer, pantX, pantY, tam, tam, hdcSprites, 0, 0, 64, 64, RGB(255, 255, 255));
      // Círculo de selección (color diferente para distinguirlos)
      if (g->seleccionado) {
        HBRUSH nullBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
        HPEN amarillo = CreatePen(PS_SOLID, 2, RGB(255, 255, 0)); // Amarillo para guerreros
        SelectObject(hdcBuffer, nullBrush);
        SelectObject(hdcBuffer, amarillo);
        Ellipse(hdcBuffer, pantX, pantY + tam - 10, pantX + tam, pantY + tam + 5);
        DeleteObject(amarillo);
      }
    }
  }
}

    // E) DIBUJAR VACAS DINÁMICAS DE ESTA FILA (64x64)
    // Usar el array dinámico gVacas en lugar de leer desde mapaObjetos
    Vaca *baseVacas = gVacas;
    for (Vaca *vaca = baseVacas; vaca < baseVacas + gNumVacas; vaca++) {
      // La base de la vaca (pies) está en vaca->y + TILE_SIZE
      float basePies = vaca->y + (float)TILE_SIZE;
      
      // Si la base de la vaca cae en esta fila, dibujarla
      if (basePies >= (float)yMinFila && basePies < (float)yMaxFila) {
        int pantX = (int)((vaca->x - cam.x) * cam.zoom);
        int pantY = (int)((vaca->y - cam.y) * cam.zoom);
        int tam = (int)(64 * cam.zoom);
        
        // Verificar que la vaca esté visible en pantalla
        if (pantX + tam > 0 && pantX < anchoP && pantY + tam > 0 && pantY < altoP) {
          // Seleccionar sprite según dirección de la vaca
          SelectObject(hdcSprites, hVacaBmp[vaca->dir]);
          TransparentBlt(hdcBuffer, pantX, pantY, tam, tam,
                        hdcSprites, 0, 0, 64, 64, RGB(255, 255, 255));
        }
      }
    }

    // F) DIBUJAR BARCO SI ESTÁ ACTIVO (192x192)
    if (pJugador->barco.activo) {
      Barco *b = &pJugador->barco;
      // El barco es 192px de alto, por lo que su base está en y + 192
      float basePies = b->y + 192.0f;
      
      if (basePies >= (float)yMinFila && basePies < (float)yMaxFila) {
        int pantX = (int)((b->x - cam.x) * cam.zoom);
        int pantY = (int)((b->y - cam.y) * cam.zoom);
        int tam = (int)(192 * cam.zoom);
        
        if (pantX + tam > 0 && pantX < anchoP && pantY + tam > 0 && pantY < altoP) {
          SelectObject(hdcSprites, hBarcoBmp[b->dir]);
          TransparentBlt(hdcBuffer, pantX, pantY, tam, tam, hdcSprites, 0, 0,
                         192, 192, RGB(255, 255, 255));
        }
      }
    }


  }

  // 3. DIBUJAR INTERFAZ DE USUARIO (MENÚ) - AQUÍ EVITAMOS FLICKERING
  // Dibujamos el menú sobre el buffer ANTES de volcarlo a pantalla
  if (menu != NULL) {
    menuCompraDibujar(hdcBuffer, menu, pJugador);
  }
  
  // NUEVO: Dibujar menú de embarque si está activo (DENTRO DEL BUFFER)
  if (menuEmb != NULL && menuEmb->activo) {
    menuEmbarqueDibujar(hdcBuffer, menuEmb, pJugador);
  }
  
  // ============================================================================
  // RESALTAR CELDA BAJO EL CURSOR (DEBUG/VISUAL AID)
  // ============================================================================
  if (highlightFila >= 0 && highlightCol >= 0) {
    // Calcular coordenadas en píxeles del mundo
    int mundoX = highlightCol * TILE_SIZE;
    int mundoY = highlightFila * TILE_SIZE;
    
    // Convertir a coordenadas de pantalla
    int pantallaX = (int)((mundoX - cam.x) * cam.zoom);
    int pantallaY = (int)((mundoY - cam.y) * cam.zoom);
    int tamZoom = (int)(TILE_SIZE * cam.zoom);
    
    // Dibujar rectángulo semi-transparente amarillo
    HPEN hPen = CreatePen(PS_SOLID, 2, RGB(255, 255, 0));  // Amarillo brillante
    HPEN hOldPen = (HPEN)SelectObject(hdcBuffer, hPen);
    HBRUSH hBrush = CreateSolidBrush(RGB(255, 255, 0));
    HBRUSH hOldBrush = (HBRUSH)SelectObject(hdcBuffer, hBrush);
    
    // Dibujar con transparencia (solo borde)
    SelectObject(hdcBuffer, GetStockObject(NULL_BRUSH));
    Rectangle(hdcBuffer, pantallaX, pantallaY, 
              pantallaX + tamZoom, pantallaY + tamZoom);
    
    // Dibujar información de la celda
    char info[128];
    sprintf(info, "Celda [%d][%d] | %dx%dpx", 
            highlightFila, highlightCol, TILE_SIZE, TILE_SIZE);
    
    SetTextColor(hdcBuffer, RGB(255, 255, 0));
    SetBkMode(hdcBuffer, TRANSPARENT);
    TextOutA(hdcBuffer, pantallaX + 5, pantallaY + 5, info, strlen(info));
    
    // Restaurar
    SelectObject(hdcBuffer, hOldPen);
    SelectObject(hdcBuffer, hOldBrush);
    DeleteObject(hPen);
    DeleteObject(hBrush);
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

// ============================================================================
// FUNCIONES REQUERIDAS POR ESPECIFICACI\u00d3N ACAD\u00c9MICA
// ============================================================================

void inicializarMapa(char mapa[GRID_SIZE][GRID_SIZE]) {
  char (*ptrFila)[GRID_SIZE] = mapa;
  printf("[DEBUG] Inicializando mapa logico %dx%d...\n", GRID_SIZE, GRID_SIZE);
  for (int f = 0; f < GRID_SIZE; f++) {
    char *ptrCelda = *(ptrFila + f);
    for (int c = 0; c < GRID_SIZE; c++) {
      *(ptrCelda + c) = SIMBOLO_VACIO;
    }
  }
  printf("[DEBUG] Mapa inicializado\n");
}

void mostrarMapa(char mapa[GRID_SIZE][GRID_SIZE]) {
  printf("\n");
  printf("==========================================================================\n");
  printf("                    MAPA LOGICO %dx%d (cada celda = %dpx)\n", 
         GRID_SIZE, GRID_SIZE, TILE_SIZE);
  printf("==========================================================================\n");
  printf("Leyenda: . =Vacio  A=Arbol  O=Obrero  C=Caballero  G=Guerrero\n");
  printf("         V=Vaca    B=Barco  E=Edificio  M=Mina\n");
  printf("==========================================================================\n\n");
  
  char (*ptrFila)[GRID_SIZE] = mapa;
  
  // Encabezado de columnas (solo números 0-9 repetidos)
  printf("      ");
  for (int c = 0; c < GRID_SIZE; c++) {
    printf("%d ", c % 10);
  }
  printf("\n");
  
  printf("     +");
  for (int c = 0; c < GRID_SIZE; c++) printf("--");
  printf("+\n");
  
  // Filas con contenido
  for (int f = 0; f < GRID_SIZE; f++) {
    printf(" %2d  |", f);
    
    char *ptrCelda = *(ptrFila + f);
    for (int c = 0; c < GRID_SIZE; c++) {
      char simbolo = *(ptrCelda + c);
      printf("%c ", (simbolo == 0) ? '.' : simbolo);
    }
    printf("|\n");
  }
  
  printf("     +");
  for (int c = 0; c < GRID_SIZE; c++) printf("--");
  printf("+\n\n");
}

void explorarMapa(char mapa[GRID_SIZE][GRID_SIZE]) {
  int fila, columna;
  printf("\n=== EXPLORAR MAPA ===\n");
  printf("Fila (0-%d): ", GRID_SIZE - 1);
  scanf("%d", &fila);
  printf("Columna (0-%d): ", GRID_SIZE - 1);
  scanf("%d", &columna);
  
  if (fila < 0 || fila >= GRID_SIZE || columna < 0 || columna >= GRID_SIZE) {
    printf("[ERROR] Coordenadas fuera del mapa\n");
    return;
  }
  
  char (*ptrFila)[GRID_SIZE] = mapa;
  char *celda = (*(ptrFila + fila)) + columna;
  char contenido = *celda;
  
  printf("[EXPLORACION] Celda [%d][%d]: ", fila, columna);
  switch(contenido) {
    case SIMBOLO_ARBOL: printf("ARBOL\n"); break;
    case SIMBOLO_VACIO: printf("Vacio\n"); break;
    default: printf("%c\n", contenido);
  }
}

char obtenerContenidoCelda(char *celda) {
  if (celda == NULL) return '\0';
  return *celda;
}
// ============================================================================
// FUNCIONES DE SINCRONIZACIÓN: mapaObjetos <-> Estado del Juego
// ============================================================================
// Mantienen mapaObjetos actualizado con las posiciones reales de los objetos

void mapaRegistrarObjeto(float pixelX, float pixelY, char simbolo) {
  int fila = (int)(pixelY / TILE_SIZE);
  int columna = (int)(pixelX / TILE_SIZE);
  
  if (fila >= 0 && fila < GRID_SIZE && columna >= 0 && columna < GRID_SIZE) {
    mapaObjetos[fila][columna] = simbolo;
  }
}

void mapaMoverObjeto(float viejoX, float viejoY, float nuevoX, float nuevoY, char simbolo) {
  int viejaFila = (int)(viejoY / TILE_SIZE);
  int viejaCol = (int)(viejoX / TILE_SIZE);
  int nuevaFila = (int)(nuevoY / TILE_SIZE);
  int nuevaCol = (int)(nuevoX / TILE_SIZE);
  
  if (viejaFila != nuevaFila || viejaCol != nuevaCol) {
    if (viejaFila >= 0 && viejaFila < GRID_SIZE && viejaCol >= 0 && viejaCol < GRID_SIZE) {
      mapaObjetos[viejaFila][viejaCol] = SIMBOLO_VACIO;
    }
    
    if (nuevaFila >= 0 && nuevaFila < GRID_SIZE && nuevaCol >= 0 && nuevaCol < GRID_SIZE) {
      mapaObjetos[nuevaFila][nuevaCol] = simbolo;
    }
  }
}