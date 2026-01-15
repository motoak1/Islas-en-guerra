// Definir version de Windows para habilitar GetTickCount64 en MinGW/GCC
#define _WIN32_WINNT 0x0600
#include "mapa.h"
#include "../edificios/edificios.h"
#include "../guardado/guardado.h"
#include "../recursos/navegacion.h"
#include "../recursos/recursos.h"
#include "../recursos/ui_compra.h"
#include "../recursos/ui_embarque.h"
#include "../recursos/ui_entrena.h"
#include "menu.h"
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

#define caballero_front "../assets/caballero/caballero_shield_front.bmp"
#define caballero_back "../assets/caballero/caballero_shield_back.bmp"
#define caballero_left "../assets/caballero/caballero_shield_left.bmp"
#define caballero_right "../assets/caballero/caballero_shield_right.bmp"

#define caballeroSinEscudo_front "../assets/caballero/caballero_front.bmp"
#define caballeroSinEscudo_back "../assets/caballero/caballero_back.bmp"
#define caballeroSinEscudo_left "../assets/caballero/caballero_left.bmp"
#define caballeroSinEscudo_right "../assets/caballero/caballero_right.bmp"

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

#define TIEMPO_DESAPARICION_CUERPO_MS 5000ULL

static HBITMAP hObreroBmp[4] = {NULL};    // Front, Back, Left, Right
static HBITMAP hCaballeroBmp[4] = {NULL}; // Front, Back, Left, Right (NUEVO)
static HBITMAP hCaballeroSinEscudoBmp[4] = {
    NULL};                            // Front, Back, Left, Right (NUEVO)
static HBITMAP hBarcoBmp[4] = {NULL}; // Front, Back, Left, Right (192x192)
static HBITMAP hBarcoDestruidoBmp = NULL; // Barco destruido (192x192)

static HBITMAP hGuerreroBmp[4] = {NULL};       // Front, Back, Left, Right
static HBITMAP hCaballeroAtk[2][3] = {{NULL}}; // dir (0=left,1=right) x frames
static HBITMAP hCaballeroSEAtk[2][3] = {{NULL}}; // Caballero sin escudo attack frames
static HBITMAP hGuerreroAtk[2][2] = {{NULL}};

// Sprites adicionales para combates (quieto, caminar, defensa, muerte)
static HBITMAP hCaballeroStand[2] = {NULL};   // [0=left,1=right]
static HBITMAP hCaballeroDefense[2] = {NULL}; // [0=left,1=right]
static HBITMAP hCaballeroDie[2] = {NULL};     // [0=left,1=right] die_1
static HBITMAP hCaballeroDie2[2] = {NULL};    // [0=left,1=right] die_2
static HBITMAP hCaballeroSEStand[2] = {NULL}; // [0=left,1=right] Caballero sin escudo (stand)

static HBITMAP hGuerreroStand[2] = {NULL}; // [0=left,1=right]
static HBITMAP hGuerreroWalk[2] = {NULL};  // [0=left,1=right]
static HBITMAP hGuerreroDie[2] = {NULL};   // [0=left,1=right] die_1
static HBITMAP hGuerreroDie2[2] = {NULL};  // [0=left,1=right] die_2

static bool gEsIslaPrincipalActual = false; // controla stand de guerreros en isla principal

static HBITMAP hVacaBmp[4] = {NULL};
static bool gGenerarRecursos = true;

static bool unidadCuerpoDesaparecido(Unidad *u, ULONGLONG ahora,
                                     ULONGLONG *outDt) {
  if (!u)
    return true;
  if (u->vida > 0) {
    if (outDt)
      *outDt = 0;
    return false;
  }
  if (u->tiempoMuerteMs == 0) {
    u->tiempoMuerteMs = ahora;
  }
  ULONGLONG dt = ahora - u->tiempoMuerteMs;
  if (outDt)
    *outDt = dt;
  return dt >= TIEMPO_DESAPARICION_CUERPO_MS;
}

static bool unidadBarraVisible(Unidad *u) {
  if (!u)
    return false;
  if (u->vida > 0)
    return true;
  return !unidadCuerpoDesaparecido(u, GetTickCount64(), NULL);
}

// Definiciones para obrerro fallback
#define OBRERO_F_ALT "assets/obrero/obrero_front.bmp"
#define OBRERO_B_ALT "assets/obrero/obrero_back.bmp"
#define OBRERO_L_ALT "assets/obrero/obrero_left.bmp"
#define OBRERO_R_ALT "assets/obrero/obrero_right.bmp"

#define BARCO_F "../assets/barco/barco_front.bmp"
#define BARCO_B "../assets/barco/barco_back.bmp"
#define BARCO_L "../assets/barco/barco_left.bmp"
#define BARCO_R "../assets/barco/barco_right.bmp"

#define BARCO_F_ALT "assets/barco/barco_front.bmp"
#define BARCO_B_ALT "assets/barco/barco_back.bmp"
#define BARCO_L_ALT "assets/barco/barco_left.bmp"
#define BARCO_R_ALT "assets/barco/barco_right.bmp"

#define BARCO_DESTRUIDO "../assets/barco/barco_destruido.bmp"
#define BARCO_DESTRUIDO_ALT "assets/barco/barco_destruido.bmp"

static HBITMAP hMapaBmp =
    NULL; // Mapa de isla individual (isla1, isla2, o isla3)
static HBITMAP hMapaGlobalBmp =
    NULL; // NUEVO: Mapa global con las 3 islas (mapaDemo2.bmp)
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

// ---------------------------------------------------------------------------
// PERSISTENCIA EN MEMORIA POR ISLA (1..3)
// ---------------------------------------------------------------------------
static bool gIslaGuardada[6] = {false};
static char gMapaObjetosIsla[6][GRID_SIZE][GRID_SIZE];
static int gCollisionIsla[6][GRID_SIZE][GRID_SIZE];
static Vaca gVacasIsla[6][10];
static int gNumVacasIsla[6] = {0};

// --- VIDA DE ARBOLES (NUEVO) ---
// Matriz paralela para almacenar la vida de cada árbol (3 = full, 2, 1, 0)
// Inicialmente 0. Se pone a 3 cuando se genera un árbol.
static int gArbolesVida[GRID_SIZE][GRID_SIZE] = {0};
static int gArbolesVidaIsla[6][GRID_SIZE][GRID_SIZE]; // Persistencia por isla


static void detectarAguaEnMapa(void);
void mapaMarcarArea(int f_inicio, int c_inicio, int ancho_celdas,
                    int alto_celdas, int valor);

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

// Limpia completamente mapaObjetos y collisionMap (deja todo en 0/vacío)
void mapaLimpiarObjetosYColision(void) {
  collisionMapAllocIfNeeded();
  for (int f = 0; f < GRID_SIZE; f++) {
    for (int c = 0; c < GRID_SIZE; c++) {
      mapaObjetos[f][c] = 0;
      if (gCollisionMap) {
        gCollisionMap[f][c] = 0;
      }
    }
  }
}

void mapaSetGenerarRecursos(bool habilitar) { gGenerarRecursos = habilitar; }

int **mapaObtenerCollisionMap(void) {
  collisionMapAllocIfNeeded();
  return gCollisionMap;
}

// Retorna el array de vacas y la cantidad actual
Vaca *mapaObtenerVacas(int *cantidad) {
  if (cantidad) {
    *cantidad = gNumVacas;
  }
  return gVacas;
}

// ============================================================================
// ELIMINAR VACA POR ÍNDICE (SOLUCIÓN A BUG DE SINCRONIZACIÓN)
// ============================================================================
// Esta función elimina una vaca usando su ÍNDICE en el array, no su posición
// en el mapa. Esto evita el bug donde la vaca se mueve mientras el usuario
// decide en el MessageBox de confirmación.
// ============================================================================
bool mapaEliminarVacaPorIndice(int indice) {
  // Validar índice
  if (indice < 0 || indice >= gNumVacas) {
    printf("[ERROR] mapaEliminarVacaPorIndice: índice %d fuera de rango (0-%d)\n",
           indice, gNumVacas - 1);
    return false;
  }

  // Obtener la posición ACTUAL de la vaca (puede haber cambiado)
  Vaca *pVaca = gVacas + indice;  // Aritmética de punteros
  int vacaFila = (int)(pVaca->y / TILE_SIZE);
  int vacaCol = (int)(pVaca->x / TILE_SIZE);

  printf("[DEBUG] Eliminando vaca #%d en posición ACTUAL: Celda[%d][%d]\n",
         indice, vacaFila, vacaCol);

  // 1. Limpiar la celda en mapaObjetos (posición ACTUAL, no la original)
  if (vacaFila >= 0 && vacaFila < GRID_SIZE && 
      vacaCol >= 0 && vacaCol < GRID_SIZE) {
    // Usar aritmética de punteros
    char *ptrCelda = *(mapaObjetos + vacaFila) + vacaCol;
    if (*ptrCelda == SIMBOLO_VACA) {
      *ptrCelda = SIMBOLO_VACIO;
    }

    // 2. Limpiar también en gCollisionMap
    if (gCollisionMap) {
      int *ptrColision = *(gCollisionMap + vacaFila) + vacaCol;
      if (*ptrColision == 3) {  // 3 = ocupado por unidad/vaca
        *ptrColision = 0;  // Liberar
      }
    }
  }

  // 3. Desplazar el resto del array para llenar el hueco
  // Usamos aritmética de punteros como requiere el proyecto
  Vaca *ptrSrc = gVacas + indice + 1;  // Siguiente vaca
  Vaca *ptrDst = gVacas + indice;       // Hueco a llenar
  for (int k = indice; k < gNumVacas - 1; k++, ptrSrc++, ptrDst++) {
    *ptrDst = *ptrSrc;
  }

  gNumVacas--;
  printf("[DEBUG] Vaca eliminada por índice. Restan: %d vacas\n", gNumVacas);

  return true;
}

// Guardado/carga: reconstruye las vacas y su ocupación cuando los datos
// provienen de un archivo de guardado.
void mapaRestaurarVacasExternas(const Vaca *vacas, int cantidad) {
  collisionMapAllocIfNeeded();

  for (int f = 0; f < GRID_SIZE; f++) {
    for (int c = 0; c < GRID_SIZE; c++) {
      if (mapaObjetos[f][c] == SIMBOLO_VACA) {
        mapaObjetos[f][c] = SIMBOLO_VACIO;
        if (gCollisionMap)
          gCollisionMap[f][c] = 0;
      }
    }
  }

  gNumVacas = 0;
  if (!vacas || cantidad <= 0)
    return;

  if (cantidad > 10)
    cantidad = 10;

  for (int i = 0; i < cantidad; i++) {
    Vaca vaca = vacas[i];
    if (vaca.x < 0)
      vaca.x = 0;
    if (vaca.y < 0)
      vaca.y = 0;
    if (vaca.x > (float)(MAPA_SIZE - TILE_SIZE))
      vaca.x = (float)(MAPA_SIZE - TILE_SIZE);
    if (vaca.y > (float)(MAPA_SIZE - TILE_SIZE))
      vaca.y = (float)(MAPA_SIZE - TILE_SIZE);

    int celdaX = (int)(vaca.x / TILE_SIZE);
    int celdaY = (int)(vaca.y / TILE_SIZE);
    if (celdaX < 0 || celdaX >= GRID_SIZE || celdaY < 0 || celdaY >= GRID_SIZE)
      continue;

    gVacas[gNumVacas] = vaca;
    mapaObjetos[celdaY][celdaX] = SIMBOLO_VACA;
    if (gCollisionMap)
      gCollisionMap[celdaY][celdaX] = 3;
    gNumVacas++;
  }
}

void mapaSeleccionarIsla(int isla) {
  int seleccion = (isla >= 1 && isla <= 5) ? isla : 1;
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
      char simbolo = mapaObjetos[f][c];
      if (simbolo == SIMBOLO_ARBOL) {
        gCollisionMap[f][c] = 1; // Bloquear celda
      } else if (simbolo == SIMBOLO_VACA || simbolo == SIMBOLO_ENEMIGO ||
                 simbolo == SIMBOLO_OBRERO || simbolo == SIMBOLO_CABALLERO ||
                 simbolo == SIMBOLO_GUERRERO || simbolo == SIMBOLO_BARCO) {
        gCollisionMap[f][c] = 3; // Ocupado por unidad/objeto temporal
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

// Desmarca un edificio en el collision map (cuando se destruye o explota)
void mapaDesmarcarEdificio(float x, float y, int ancho, int alto) {
  if (gCollisionMap == NULL)
    return;

  // Calcular celdas que ocupa el edificio (TILE_SIZE=64)
  int celInicioFila = (int)(y / TILE_SIZE);
  int celInicioCol = (int)(x / TILE_SIZE);
  int celFinFila = (int)((y + alto) / TILE_SIZE);
  int celFinCol = (int)((x + ancho) / TILE_SIZE);

  for (int f = celInicioFila; f <= celFinFila && f < GRID_SIZE; f++) {
    for (int c = celInicioCol; c <= celFinCol && c < GRID_SIZE; c++) {
      if (f >= 0 && c >= 0) {
        gCollisionMap[f][c] = 0; // Desbloquear celda

        // También limpiar el símbolo en mapaObjetos si era un edificio
        char s = mapaObjetos[f][c];
        if (s == SIMBOLO_EDIFICIO || s == SIMBOLO_MINA ||
            s == SIMBOLO_CUARTEL) {
          mapaObjetos[f][c] = SIMBOLO_VACIO;
        }
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

// ---------------------------------------------------------------------------
// PERSISTENCIA DE ESTADO POR ISLA (MEMORIA)
// ---------------------------------------------------------------------------
static bool islaValida(int isla) { return isla >= 1 && isla <= 5; }

void mapaGuardarEstadoIsla(int isla) {
  if (!islaValida(isla))
    return;
  collisionMapAllocIfNeeded();

  // Copiar mapaObjetos
  for (int f = 0; f < GRID_SIZE; f++) {
    for (int c = 0; c < GRID_SIZE; c++) {
      gMapaObjetosIsla[isla][f][c] = mapaObjetos[f][c];
      gArbolesVidaIsla[isla][f][c] = gArbolesVida[f][c]; // Copiar vida arboles
    }
  }

  // Copiar collision map (si existe)
  if (gCollisionMap) {
    for (int f = 0; f < GRID_SIZE; f++) {
      for (int c = 0; c < GRID_SIZE; c++) {
        gCollisionIsla[isla][f][c] = *(*(gCollisionMap + f) + c);
      }
    }
  }

  // Copiar vacas
  gNumVacasIsla[isla] = gNumVacas;
  for (int i = 0; i < gNumVacas; i++) {
    gVacasIsla[isla][i] = gVacas[i];
  }

  gIslaGuardada[isla] = true;
  printf("[DEBUG] Estado de isla %d guardado en memoria\n", isla);
}

void mapaRestaurarEstadoIsla(int isla) {
  if (!islaValida(isla) || !gIslaGuardada[isla])
    return;
  collisionMapAllocIfNeeded();

  // Restaurar mapaObjetos
  for (int f = 0; f < GRID_SIZE; f++) {
    for (int c = 0; c < GRID_SIZE; c++) {
      mapaObjetos[f][c] = gMapaObjetosIsla[isla][f][c];
      gArbolesVida[f][c] = gArbolesVidaIsla[isla][f][c]; // Restaurar vida arboles
    }
  }

  // Restaurar collision map
  if (gCollisionMap) {
    for (int f = 0; f < GRID_SIZE; f++) {
      for (int c = 0; c < GRID_SIZE; c++) {
        *(*(gCollisionMap + f) + c) = gCollisionIsla[isla][f][c];
      }
    }
  }

  // Restaurar vacas
  gNumVacas = gNumVacasIsla[isla];
  for (int i = 0; i < gNumVacas; i++) {
    gVacas[i] = gVacasIsla[isla][i];
  }

  printf("[DEBUG] Estado de isla %d restaurado desde memoria\n", isla);
}

// Guardado: extrae un snapshot del mapa/collision/vacas de cada isla para
// escribirlo en el archivo de guardado.
void mapaExportarEstadosIsla(MapaEstadoSerializable estados[6]) {
  if (!estados)
    return;
  for (int isla = 0; isla < 6; isla++) {
    estados[isla].valido = gIslaGuardada[isla];
    estados[isla].numVacas = 0;
    if (!gIslaGuardada[isla])
      continue;

    memcpy(estados[isla].objetos, gMapaObjetosIsla[isla],
           sizeof(gMapaObjetosIsla[isla]));
    memcpy(estados[isla].colisiones, gCollisionIsla[isla],
           sizeof(gCollisionIsla[isla]));

    estados[isla].numVacas = gNumVacasIsla[isla];
    if (estados[isla].numVacas > 10)
      estados[isla].numVacas = 10;
    for (int v = 0; v < estados[isla].numVacas; v++) {
      estados[isla].vacas[v] = gVacasIsla[isla][v];
    }
  }
}

// Carga: hidrata los snapshots per-isla desde el archivo para que el juego
// no tenga que regenerar estados al reanudar una partida.
void mapaImportarEstadosIsla(const MapaEstadoSerializable estados[6]) {
  if (!estados)
    return;
  for (int isla = 0; isla < 6; isla++) {
    gIslaGuardada[isla] = estados[isla].valido;
    if (!estados[isla].valido)
      continue;

    memcpy(gMapaObjetosIsla[isla], estados[isla].objetos,
           sizeof(gMapaObjetosIsla[isla]));
    memcpy(gCollisionIsla[isla], estados[isla].colisiones,
           sizeof(gCollisionIsla[isla]));

    gNumVacasIsla[isla] = estados[isla].numVacas;
    if (gNumVacasIsla[isla] > 10)
      gNumVacasIsla[isla] = 10;
    for (int v = 0; v < gNumVacasIsla[isla]; v++) {
      gVacasIsla[isla][v] = estados[isla].vacas[v];
    }
  }
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
  bmi.bmiHeader.biHeight = -bm.bmHeight; // Negativo = top-down
  bmi.bmiHeader.biPlanes = 1;
  bmi.bmiHeader.biBitCount = 24; // 24-bit RGB
  bmi.bmiHeader.biCompression = BI_RGB;

  // ================================================================
  // LEER TODO EL BITMAP A MEMORIA
  // ================================================================
  int rowSize = ((bm.bmWidth * 3 + 3) & ~3); // Alineado a 4 bytes
  int imageSize = rowSize * bm.bmHeight;
  BYTE *pixelData = (BYTE *)malloc(imageSize);

  if (!pixelData) {
    printf("[DEBUG AGUA] ERROR: No se pudo asignar memoria\n");
    fflush(stdout);
    return;
  }

  HDC hdcScreen = GetDC(NULL);
  int result = GetDIBits(hdcScreen, hMapaBmp, 0, bm.bmHeight, pixelData, &bmi,
                         DIB_RGB_COLORS);
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
  int posicionesTest[][2] = {{5, 5}, {10, 10}, {15, 15}, {7, 7}, {14, 8}};

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
      bool beige = (r > 80 && g > 80 && b < 100 && abs(r - g) < 50);
      printf("[DEBUG] Celda[%2d][%2d] @ (%4d,%4d): RGB(%3d,%3d,%3d) -> %s\n", f,
             c, px, py, r, g, b, (verde || beige) ? "TIERRA" : "AGUA");
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
      if (*(fila + c) != 0)
        continue;

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

      bool esAguaAzulOscura =
          (b > r + 20 && b > g + 20 && b > 60); // Azul oscuro
      bool esAguaAzulClara =
          (b > r && b > g && b > 100); // Azul claro (celeste)

      bool esAgua = esAguaAzulOscura || esAguaAzulClara;

      if (esAgua) {
        // ES AGUA - marcar como impasable
        *(fila + c) = 1;
        mapaObjetos[f][c] =
            SIMBOLO_AGUA; // NUEVO: Marcar con '~' en mapaObjetos
        contadorAgua++;

        // Debug primeras 5 escrituras
        if (contadorAgua <= 5) {
          printf("[DEBUG] AGUA DETECTADA: gCollisionMap[%d][%d] = 1 "
                 "(RGB=%d,%d,%d)\n",
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
  printf("[DEBUG AGUA] Total procesado: %d\n",
         contadorAgua + contadorTierra + contadorFueraDeLimites);
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
    printf(
        "\n[DEBUG AGUA] WARNING: No se detecto agua, activando fallback...\n");
    // Marcar bordes como agua
    for (int i = 0; i < GRID_SIZE; i++) {
      if (*(*(gCollisionMap + 0) + i) == 0)
        *(*(gCollisionMap + 0) + i) = 1;
      if (*(*(gCollisionMap + GRID_SIZE - 1) + i) == 0)
        *(*(gCollisionMap + GRID_SIZE - 1) + i) = 1;
      if (*(*(gCollisionMap + i) + 0) == 0)
        *(*(gCollisionMap + i) + 0) = 1;
      if (*(*(gCollisionMap + i) + GRID_SIZE - 1) == 0)
        *(*(gCollisionMap + i) + GRID_SIZE - 1) = 1;
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
        if (f < 1 || f >= GRID_SIZE - BARCO_CELDAS - 1 || c < 1 ||
            c >= GRID_SIZE - BARCO_CELDAS - 1)
          continue;

        // ================================================================
        // CRÍTICO: La celda actual debe ser AGUA (valor 1)
        // ================================================================
        if (*(*(col + f) + c) != 1)
          continue;

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

        if (!bloqueAgua)
          continue;

        // Verificar que al menos el 90% del bloque sea agua
        if (celdasAgua < (BARCO_CELDAS * BARCO_CELDAS * 9 / 10))
          continue;

        // Buscar TIERRA adyacente en las 4 direcciones para confirmar que es
        // orilla
        for (int d = 0; d < 4; d++) {
          int nf = f + dF[d];
          int nc = c + dC[d];

          // Verificar límites
          if (nf < 0 || nf >= GRID_SIZE || nc < 0 || nc >= GRID_SIZE)
            continue;

          // Verificar si la celda adyacente es TIERRA (0)
          if (*(*(col + nf) + nc) == 0) {
            // ¡Encontramos una orilla válida! El barco está en agua, orientado
            // hacia tierra
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
            printf("[DEBUG BARCO] "
                   "============================================\n\n");
            fflush(stdout);
            return;
          }
        }
      }
    }
  }

  // FALLBACK: Si no se encuentra orilla ideal, buscar CUALQUIER celda de agua
  printf("[WARNING BARCO] No se encontró orilla con tierra adyacente, buscando "
         "agua...\n");
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
        printf("[DEBUG BARCO] Agua encontrada en posición: (%.1f, %.1f)\n",
               *outX, *outY);
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

  // Limpiar objetos y colisiones antes de regenerar (para nuevas islas)
  mapaLimpiarObjetosYColision();

  // Requisito: Aritmética de punteros para manejar la matriz
  char (*ptrMatriz)[GRID_SIZE] = mapaObjetos;
  srand((unsigned int)time(NULL));

  // ============================================================================
  // RESERVAR CELDAS DE EDIFICIOS ANTES DE GENERAR BOSQUE
  // ============================================================================
  // Los edificios se inicializan en WM_CREATE DESPUÉS de esta función,
  // pero las posiciones son fijas. Reservamos las celdas para evitar que
  // árboles o vacas se coloquen en las posiciones de los edificios.
  // NUEVO: Reservar un margen de 2 celdas alrededor para evitar confusión.
  // ============================================================================

  // Función local temporal para marcar zonas de exclusión
  int edificios_coords[3][2] = {
      {(int)((1024.0f - 64.0f) / TILE_SIZE),
       (int)((1024.0f - 64.0f) / TILE_SIZE)}, // Ayuntamiento
      {(int)(450.0f / TILE_SIZE), (int)((1024.0f - 64.0f) / TILE_SIZE)}, // Mina
      {(int)(1600.0f / TILE_SIZE),
       (int)((1024.0f - 64.0f) / TILE_SIZE)} // Cuartel
  };

  for (int e = 0; e < 3; e++) {
    int baseF = edificios_coords[e][0];
    int baseC = edificios_coords[e][1];
    // Marcar 2x2 del edificio + margen de 2 tiles en todas direcciones
    // Total: bloque de 6x6 celdas centrado en el edificio
    for (int df = -2; df <= 3; df++) {
      for (int dc = -2; dc <= 3; dc++) {
        int nf = baseF + df;
        int nc = baseC + dc;
        if (nf >= 0 && nf < GRID_SIZE && nc >= 0 && nc < GRID_SIZE) {
          *(*(ptrMatriz + nf) + nc) = SIMBOLO_EDIFICIO;
        }
      }
    }
  }

  printf("[DEBUG] Edificios y perímetros de seguridad reservados en "
         "mapaObjetos.\n");

  // Obtener ID de isla para adaptar la logica de generacion
  int islaActual = menuObtenerIsla(); 
  printf("[DEBUG] Generando recursos para Isla ID: %d\n", islaActual);

  // REQUISITO CRÍTICO: Colocar exactamente 15 árboles (no por probabilidad)
  const int NUM_ARBOLES_EXACTO = 15;
  int contador = 0;

  // Bucle while que continúa hasta colocar exactamente 40 árboles
  int intentos = 0;
  const int MAX_INTENTOS = 50000; // Limite de seguridad para evitar congelamiento
  while (contador < NUM_ARBOLES_EXACTO && intentos < MAX_INTENTOS) {
    intentos++;
    // Generar coordenadas aleatorias en la matriz 32x32
    int fila = rand() % GRID_SIZE;
    int col = rand() % GRID_SIZE;

    // Verificar que la celda esté vacía (no haya otro árbol)
    if (*(*(ptrMatriz + fila) + col) != SIMBOLO_VACIO &&
        *(*(ptrMatriz + fila) + col) != 0) {
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

    bool esAgua = (b > r + 20 && b > g + 20) ||
                  (*(*(ptrMatriz + fila) + col) == SIMBOLO_AGUA);

    bool esSueloValido = false;

    // Lógica adaptativa según el tipo de isla
    if (islaActual == 4) {
        // ISLA 4 (Hielo/Nieve): Check de BLANCO / GRIS CLARO / CIAN
        // Criterio: rgb altos y balanceados (blanco/gris) o cian claro
        bool esBlanco = (r > 180 && g > 180 && b > 180);
        bool esCianSolo = (b > 150 && g > 150 && r > 100); 
        if (!esAgua && (esBlanco || esCianSolo)) {
            esSueloValido = true;
        } 
    } else if (islaActual == 5) {
        // ISLA 5 (Fuego/Volcán): Check de ROJO / GRIS OSCURO / CENIZA
        // Criterio: rojo dominante, o gris oscuro (tierra quemada)
        bool esRojo = (r > g + 30 && r > b + 30);
        bool esGrisOscuro = (r < 100 && g < 100 && b < 100 && abs(r-g) < 30 && abs(r-b) < 30);
        if (!esAgua && (esRojo || esGrisOscuro)) {
            esSueloValido = true;
        }
    } else {
        // ISLAS 1, 2, 3 (Bosque - Default): Check estricto de VERDE
        // Criterio: verde debe dominar claramente y azul debe ser bajo
        if (!esAgua && g > r && g > b && g > 70 && b < 80 && r < 100) {
            esSueloValido = true;
        }
    }

    if (esSueloValido) {
      // Colocar árbol usando aritmética de punteros y símbolo de caracter
      *(*(ptrMatriz + fila) + col) = SIMBOLO_ARBOL;
      // Inicializar vida del árbol (3 golpes)
      gArbolesVida[fila][col] = 3;
      contador++;
    }
  }

  if (intentos >= MAX_INTENTOS) {
      printf("[WARNING] Timeout generando arboles. Colocados: %d/%d. Verifique que el mapa tenga color verde (R<100, G>70, B<80).\n", contador, NUM_ARBOLES_EXACTO);
  }

  printf("[DEBUG] Logica: %d arboles registrados en la matriz con punteros.\n",
         contador);

  // ============================================================================
  // GENERACIÓN DE VACAS DINÁMICAS (en lugar de estáticas en mapaObjetos)
  // ============================================================================
  const int NUM_VACAS_EXACTO = 10;
  gNumVacas = 0;

  while (gNumVacas < NUM_VACAS_EXACTO && intentos < MAX_INTENTOS) {
    intentos++; // Reutilizamos variable, asumimos que se resetea o continúa incrementando, mejor resetear si queremos full attempts
    // Pero como MAX_INTENTOS es 50000, es suficiente para ambos. O mejor resetear.
    
    int fila = rand() % GRID_SIZE;
    int col = rand() % GRID_SIZE;

    // No colocar vaca donde ya hay árbol u otra vaca
    if (*(*(ptrMatriz + fila) + col) != 0)
      continue;

    // Verificar que el suelo sea tierra (mismo criterio que árboles)
    int px = (col * TILE_SIZE) + (TILE_SIZE / 2);
    int py = (fila * TILE_SIZE) + (TILE_SIZE / 2);

    COLORREF color = GetPixel(hdcMem, px, py);
    if (color == CLR_INVALID)
      continue;

    BYTE r = GetRValue(color);
    BYTE g = GetGValue(color);
    BYTE b = GetBValue(color);

    bool esAgua = (b > r + 20 && b > g + 20) ||
                  (*(*(ptrMatriz + fila) + col) == SIMBOLO_AGUA);

    bool esSueloValido = false;

    // Lógica adaptativa según el tipo de isla (MISMA LÓGICA QUE ÁRBOLES)
    if (islaActual == 4) {
        // ISLA 4 (Hielo): Blanco/Cian
        bool esBlanco = (r > 180 && g > 180 && b > 180);
        bool esCianSolo = (b > 150 && g > 150 && r > 100);
        if (!esAgua && (esBlanco || esCianSolo)) esSueloValido = true;
    } else if (islaActual == 5) {
        // ISLA 5 (Fuego): Rojo/Gris
        bool esRojo = (r > g + 30 && r > b + 30);
        bool esGrisOscuro = (r < 100 && g < 100 && b < 100 && abs(r-g) < 30 && abs(r-b) < 30);
        if (!esAgua && (esRojo || esGrisOscuro)) esSueloValido = true;
    } else {
        // ISLAS 1, 2, 3 (Bosque): Verde
        if (!esAgua && g > r && g > b && g > 70 && b < 80 && r < 100) esSueloValido = true;
    }

    if (esSueloValido) {
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

  if (intentos >= MAX_INTENTOS) {
       printf("[WARNING] Timeout generando vacas. Colocadas: %d/%d.\n", gNumVacas, NUM_VACAS_EXACTO);
  }

  printf("[DEBUG] Logica: %d vacas dinámicas generadas.\n", gNumVacas);

  // Construir la grilla de colisión con árboles Y detectar agua
  // NOTA: mapaReconstruirCollisionMap() YA llama a detectarAguaEnMapa()
  // internamente
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
  hMapaGlobalBmp =
      (HBITMAP)LoadImageA(NULL, "../assets/mapaDemo2.bmp", IMAGE_BITMAP, 0, 0,
                          LR_LOADFROMFILE | LR_CREATEDIBSECTION);
  if (!hMapaGlobalBmp) {
    hMapaGlobalBmp =
        (HBITMAP)LoadImageA(NULL, "assets/mapaDemo2.bmp", IMAGE_BITMAP, 0, 0,
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

  for (int i = 0; i < 4; i++) {
    hCaballeroBmp[i] = (HBITMAP)LoadImageA(NULL, rutasCab[i], IMAGE_BITMAP, 64,
                                           64, LR_LOADFROMFILE);

    if (!hCaballeroBmp[i]) {
      printf("[ERROR] No se pudo cargar caballero[%d]\n", i);
    } else {
      printf("[OK] Caballero BMP %d cargado correctamente.\n", i);
    }
  }

  // Caballero sin escudo

  const char *rutasCabSinEscudo[] = {
      caballeroSinEscudo_front, caballeroSinEscudo_back,
      caballeroSinEscudo_left, caballeroSinEscudo_right};

  for (int i = 0; i < 4; i++) {
    hCaballeroSinEscudoBmp[i] = (HBITMAP)LoadImageA(
        NULL, rutasCabSinEscudo[i], IMAGE_BITMAP, 64, 64, LR_LOADFROMFILE);

    if (!hCaballeroSinEscudoBmp[i]) {
      printf("[ERROR] No se pudo cargar caballero[%d]\n", i);
    } else {
      printf("[OK] Caballero BMP %d cargado correctamente.\n", i);
    }
  }

  // --- CARGAR SPRITES DE GUERREROS ---
  const char *rutasGuerr[] = {guerrero_front, guerrero_back, guerrero_left,
                              guerrero_right};

  const char *rutasGuerrAlt[] = {GUERRERO_F_ALT, GUERRERO_B_ALT, GUERRERO_L_ALT,
                                 GUERRERO_R_ALT};

  for (int i = 0; i < 4; i++) {
    hGuerreroBmp[i] = (HBITMAP)LoadImageA(NULL, rutasGuerr[i], IMAGE_BITMAP, 64,
                                          64, LR_LOADFROMFILE);
    if (!hGuerreroBmp[i]) {
      hGuerreroBmp[i] = (HBITMAP)LoadImageA(
          NULL, rutasGuerrAlt[i], IMAGE_BITMAP, 64, 64, LR_LOADFROMFILE);
    }
    if (!hGuerreroBmp[i]) {
      printf("[ERROR] No se pudo cargar guerrero[%d]\\n", i);
    } else {
      printf("[OK] Guerrero BMP %d cargado correctamente.\\n", i);
    }
  }

  // --- COMBATE: CABALLERO (stand, defense, die) ---
  const char *cabStandLR[2] = {
      "../assets/caballero/caballero_war_stand_left.bmp",
      "../assets/caballero/caballero_war_stand_right.bmp"};
  const char *cabStandLRAlt[2] = {"assets/caballero/caballero_war_stand_left.bmp",
                                  "assets/caballero/caballero_war_stand_right.bmp"};
  const char *cabDefLR[2] = {"../assets/caballero/caballero_defense_left.bmp",
                             "../assets/caballero/caballero_defense_right.bmp"};
  const char *cabDefLRAlt[2] = {"assets/caballero/caballero_defense_left.bmp",
                                "assets/caballero/caballero_defense_right.bmp"};
  const char *cabDie1LR[2] = {"../assets/caballero/caballero_die_1_left.bmp",
                              "../assets/caballero/caballero_die_1_right.bmp"};
  const char *cabDie1LRAlt[2] = {"assets/caballero/caballero_die_1_left.bmp",
                                 "assets/caballero/caballero_die_1_right.bmp"};
  const char *cabDie2LR[2] = {"../assets/caballero/caballero_die_2_left.bmp",
                              "../assets/caballero/caballero_die_2_right.bmp"};
  const char *cabDie2LRAlt[2] = {"assets/caballero/caballero_die_2_left.bmp",
                                 "assets/caballero/caballero_die_2_right.bmp"};
  for (int i = 0; i < 2; i++) {
    hCaballeroStand[i] = (HBITMAP)LoadImageA(NULL, cabStandLR[i], IMAGE_BITMAP, 64, 64, LR_LOADFROMFILE);
    if (!hCaballeroStand[i])
      hCaballeroStand[i] = (HBITMAP)LoadImageA(NULL, cabStandLRAlt[i], IMAGE_BITMAP, 64, 64, LR_LOADFROMFILE);
    hCaballeroDefense[i] = (HBITMAP)LoadImageA(NULL, cabDefLR[i], IMAGE_BITMAP, 64, 64, LR_LOADFROMFILE);
    if (!hCaballeroDefense[i])
      hCaballeroDefense[i] = (HBITMAP)LoadImageA(NULL, cabDefLRAlt[i], IMAGE_BITMAP, 64, 64, LR_LOADFROMFILE);
    hCaballeroDie[i] = (HBITMAP)LoadImageA(NULL, cabDie1LR[i], IMAGE_BITMAP, 64, 64, LR_LOADFROMFILE);
    if (!hCaballeroDie[i])
      hCaballeroDie[i] = (HBITMAP)LoadImageA(NULL, cabDie1LRAlt[i], IMAGE_BITMAP, 64, 64, LR_LOADFROMFILE);
    hCaballeroDie2[i] = (HBITMAP)LoadImageA(NULL, cabDie2LR[i], IMAGE_BITMAP, 64, 64, LR_LOADFROMFILE);
    if (!hCaballeroDie2[i])
      hCaballeroDie2[i] = (HBITMAP)LoadImageA(NULL, cabDie2LRAlt[i], IMAGE_BITMAP, 64, 64, LR_LOADFROMFILE);
  }

  // --- COMBATE: GUERRERO (stand, walk, die) ---
  const char *gueStandLR[2] = {"../assets/guerrero/guerrero_war_stand_left.bmp",
                               "../assets/guerrero/guerrero_war_stand_right.bmp"};
  const char *gueStandLRAlt[2] = {"assets/guerrero/guerrero_war_stand_left.bmp",
                                  "assets/guerrero/guerrero_war_stand_right.bmp"};
  const char *gueWalkLR[2] = {"../assets/guerrero/guerrero_war_walk_left.bmp",
                              "../assets/guerrero/guerrero_war_walk_right.bmp"};
  const char *gueWalkLRAlt[2] = {"assets/guerrero/guerrero_war_walk_left.bmp",
                                 "assets/guerrero/guerrero_war_walk_right.bmp"};
  const char *gueDie1LR[2] = {"../assets/guerrero/guerrero_war_die_1_left.bmp",
                              "../assets/guerrero/guerrero_war_die_1_right.bmp"};
  const char *gueDie1LRAlt[2] = {"assets/guerrero/guerrero_war_die_1_left.bmp",
                                 "assets/guerrero/guerrero_war_die_1_right.bmp"};
  const char *gueDie2LR[2] = {"../assets/guerrero/guerrero_war_die_2_left.bmp",
                              "../assets/guerrero/guerrero_war_die_2_right.bmp"};
  const char *gueDie2LRAlt[2] = {"assets/guerrero/guerrero_war_die_2_left.bmp",
                                 "assets/guerrero/guerrero_war_die_2_right.bmp"};
  for (int i = 0; i < 2; i++) {
    hGuerreroStand[i] = (HBITMAP)LoadImageA(NULL, gueStandLR[i], IMAGE_BITMAP, 64, 64, LR_LOADFROMFILE);
    if (!hGuerreroStand[i])
      hGuerreroStand[i] = (HBITMAP)LoadImageA(NULL, gueStandLRAlt[i], IMAGE_BITMAP, 64, 64, LR_LOADFROMFILE);
    hGuerreroWalk[i] = (HBITMAP)LoadImageA(NULL, gueWalkLR[i], IMAGE_BITMAP, 64, 64, LR_LOADFROMFILE);
    if (!hGuerreroWalk[i])
      hGuerreroWalk[i] = (HBITMAP)LoadImageA(NULL, gueWalkLRAlt[i], IMAGE_BITMAP, 64, 64, LR_LOADFROMFILE);
    hGuerreroDie[i] = (HBITMAP)LoadImageA(NULL, gueDie1LR[i], IMAGE_BITMAP, 64, 64, LR_LOADFROMFILE);
    if (!hGuerreroDie[i])
      hGuerreroDie[i] = (HBITMAP)LoadImageA(NULL, gueDie1LRAlt[i], IMAGE_BITMAP, 64, 64, LR_LOADFROMFILE);
    hGuerreroDie2[i] = (HBITMAP)LoadImageA(NULL, gueDie2LR[i], IMAGE_BITMAP, 64, 64, LR_LOADFROMFILE);
    if (!hGuerreroDie2[i])
      hGuerreroDie2[i] = (HBITMAP)LoadImageA(NULL, gueDie2LRAlt[i], IMAGE_BITMAP, 64, 64, LR_LOADFROMFILE);
  }

  // --- COMBATE: CABALLERO SIN ESCUDO (stand) ---
  const char *cseStandLR[2] = {
      "../assets/caballero/caballero_war_NO_stand_left.bmp",
      "../assets/caballero/caballero_war_NO_stand_right.bmp"};
  const char *cseStandLRAlt[2] = {
      "assets/caballero/caballero_war_NO_stand_left.bmp",
      "assets/caballero/caballero_war_NO_stand_right.bmp"};
  for (int i = 0; i < 2; i++) {
    hCaballeroSEStand[i] = (HBITMAP)LoadImageA(NULL, cseStandLR[i], IMAGE_BITMAP, 64, 64, LR_LOADFROMFILE);
    if (!hCaballeroSEStand[i])
      hCaballeroSEStand[i] = (HBITMAP)LoadImageA(NULL, cseStandLRAlt[i], IMAGE_BITMAP, 64, 64, LR_LOADFROMFILE);
  }



  // Ataque caballero (left/right, 3 frames)
  const char *cabAtkL[3] = {
      "../assets/caballero/caballero_war_move_1_left.bmp",
      "../assets/caballero/caballero_war_move_2_left.bmp",
      "../assets/caballero/caballero_war_move_3_left.bmp"};
  const char *cabAtkR[3] = {
      "../assets/caballero/caballero_war_move_1_right.bmp",
      "../assets/caballero/caballero_war_move_2_right.bmp",
      "../assets/caballero/caballero_war_move_3_right.bmp"};
  const char *cabAtkLAlt[3] = {
      "assets/caballero/caballero_war_move_1_left.bmp",
      "assets/caballero/caballero_war_move_2_left.bmp",
      "assets/caballero/caballero_war_move_3_left.bmp"};
  const char *cabAtkRAlt[3] = {
      "assets/caballero/caballero_war_move_1_right.bmp",
      "assets/caballero/caballero_war_move_2_right.bmp",
      "assets/caballero/caballero_war_move_3_right.bmp"};
    // Caballero sin escudo: nuevos sprites de ataque (NO_move)
    const char *cseAtkL[3] = {
      "../assets/caballero/caballero_war_NO_move_1_left.bmp",
      "../assets/caballero/caballero_war_NO_move_2_left.bmp",
      "../assets/caballero/caballero_war_NO_move_3_left.bmp"};
    const char *cseAtkR[3] = {
      "../assets/caballero/caballero_war_NO_move_1_right.bmp",
      "../assets/caballero/caballero_war_NO_move_2_right.bmp",
      "../assets/caballero/caballero_war_NO_move_3_right.bmp"};
    const char *cseAtkLAlt[3] = {
      "assets/caballero/caballero_war_NO_move_1_left.bmp",
      "assets/caballero/caballero_war_NO_move_2_left.bmp",
      "assets/caballero/caballero_war_NO_move_3_left.bmp"};
    const char *cseAtkRAlt[3] = {
      "assets/caballero/caballero_war_NO_move_1_right.bmp",
      "assets/caballero/caballero_war_NO_move_2_right.bmp",
      "assets/caballero/caballero_war_NO_move_3_right.bmp"};

  for (int i = 0; i < 3; i++) {
    hCaballeroAtk[0][i] = (HBITMAP)LoadImageA(NULL, cabAtkL[i], IMAGE_BITMAP,
                                              64, 64, LR_LOADFROMFILE);
    if (!hCaballeroAtk[0][i])
      hCaballeroAtk[0][i] = (HBITMAP)LoadImageA(
          NULL, cabAtkLAlt[i], IMAGE_BITMAP, 64, 64, LR_LOADFROMFILE);
    hCaballeroAtk[1][i] = (HBITMAP)LoadImageA(NULL, cabAtkR[i], IMAGE_BITMAP,
                                              64, 64, LR_LOADFROMFILE);
    if (!hCaballeroAtk[1][i])
      hCaballeroAtk[1][i] = (HBITMAP)LoadImageA(
          NULL, cabAtkRAlt[i], IMAGE_BITMAP, 64, 64, LR_LOADFROMFILE);
  }

  // Cargar sprites de ataque del caballero sin escudo
  for (int i = 0; i < 3; i++) {
    hCaballeroSEAtk[0][i] = (HBITMAP)LoadImageA(NULL, cseAtkL[i], IMAGE_BITMAP,
                                                64, 64, LR_LOADFROMFILE);
    if (!hCaballeroSEAtk[0][i])
      hCaballeroSEAtk[0][i] = (HBITMAP)LoadImageA(
          NULL, cseAtkLAlt[i], IMAGE_BITMAP, 64, 64, LR_LOADFROMFILE);
    hCaballeroSEAtk[1][i] = (HBITMAP)LoadImageA(NULL, cseAtkR[i], IMAGE_BITMAP,
                                                64, 64, LR_LOADFROMFILE);
    if (!hCaballeroSEAtk[1][i])
      hCaballeroSEAtk[1][i] = (HBITMAP)LoadImageA(
          NULL, cseAtkRAlt[i], IMAGE_BITMAP, 64, 64, LR_LOADFROMFILE);
  }

  // Ataque guerrero (left/right, 2 frames)
  const char *gueAtkL[2] = {"../assets/guerrero/guerrero_war_move_1_left.bmp",
                            "../assets/guerrero/guerrero_war_move_2_left.bmp"};
  const char *gueAtkR[2] = {"../assets/guerrero/guerrero_war_move_1_right.bmp",
                            "../assets/guerrero/guerrero_war_move_2_right.bmp"};
  const char *gueAtkLAlt[2] = {"assets/guerrero/guerrero_war_move_1_left.bmp",
                               "assets/guerrero/guerrero_war_move_2_left.bmp"};
  const char *gueAtkRAlt[2] = {"assets/guerrero/guerrero_war_move_1_right.bmp",
                               "assets/guerrero/guerrero_war_move_2_right.bmp"};
  for (int i = 0; i < 2; i++) {
    hGuerreroAtk[0][i] = (HBITMAP)LoadImageA(NULL, gueAtkL[i], IMAGE_BITMAP, 64,
                                             64, LR_LOADFROMFILE);
    if (!hGuerreroAtk[0][i])
      hGuerreroAtk[0][i] = (HBITMAP)LoadImageA(
          NULL, gueAtkLAlt[i], IMAGE_BITMAP, 64, 64, LR_LOADFROMFILE);
    hGuerreroAtk[1][i] = (HBITMAP)LoadImageA(NULL, gueAtkR[i], IMAGE_BITMAP, 64,
                                             64, LR_LOADFROMFILE);
    if (!hGuerreroAtk[1][i])
      hGuerreroAtk[1][i] = (HBITMAP)LoadImageA(
          NULL, gueAtkRAlt[i], IMAGE_BITMAP, 64, 64, LR_LOADFROMFILE);
  }

  // --- CARGAR SPRITES DE BARCO (192x192) ---
  const char *rutasBarco[] = {BARCO_F, BARCO_B, BARCO_L, BARCO_R};
  const char *rutasBarcoAlt[] = {BARCO_F_ALT, BARCO_B_ALT, BARCO_L_ALT, BARCO_R_ALT};

  for (int i = 0; i < 4; i++) {
    // Intento 1: Ruta principal (../assets)
    hBarcoBmp[i] = (HBITMAP)LoadImageA(NULL, rutasBarco[i], IMAGE_BITMAP,
                                       192, 192, LR_LOADFROMFILE);
    
    // Intento 2: Ruta alterna (assets/)
    if (!hBarcoBmp[i]) {
        hBarcoBmp[i] = (HBITMAP)LoadImageA(NULL, rutasBarcoAlt[i], IMAGE_BITMAP,
                                       192, 192, LR_LOADFROMFILE);
    }

    if (!hBarcoBmp[i]) {
      printf("[ERROR] No se pudo cargar barco[%d] (ni '%s' ni '%s')\n", i, rutasBarco[i], rutasBarcoAlt[i]);
    } else {
      printf("[OK] Barco BMP %d cargado correctamente (192x192).\n", i);
    }
  }

  // Cargar sprite del barco destruido
  hBarcoDestruidoBmp = (HBITMAP)LoadImageA(NULL, BARCO_DESTRUIDO, IMAGE_BITMAP,
                                           192, 192, LR_LOADFROMFILE);
  if (!hBarcoDestruidoBmp) {
    hBarcoDestruidoBmp = (HBITMAP)LoadImageA(NULL, BARCO_DESTRUIDO_ALT, IMAGE_BITMAP,
                                             192, 192, LR_LOADFROMFILE);
  }
  if (hBarcoDestruidoBmp) {
    printf("[OK] Barco destruido cargado correctamente (192x192).\n");
  } else {
    printf("[ERROR] No se pudo cargar barco_destruido.bmp\n");
  }

  const char *rutasVaca[] = {VACA_F, VACA_B, VACA_L, VACA_R};
  const char *rutasVacaAlt[] = {VACA_F_ALT, VACA_B_ALT, VACA_L_ALT, VACA_R_ALT};
  for (int i = 0; i < 4; i++) {
    hVacaBmp[i] = (HBITMAP)LoadImageA(NULL, rutasVaca[i], IMAGE_BITMAP, 64, 64,
                                      LR_LOADFROMFILE);
    if (!hVacaBmp[i]) {
      hVacaBmp[i] = (HBITMAP)LoadImageA(NULL, rutasVacaAlt[i], IMAGE_BITMAP, 64,
                                        64, LR_LOADFROMFILE);
    }
    printf("[%s] Vaca BMP %d cargado.\\n", hVacaBmp[i] ? "OK" : "ERROR", i);
  }

  if (gGenerarRecursos) {
    generarBosqueAutomatico();
  } else {
    mapaReconstruirCollisionMap();
  }
}

// ============================================================================
// ACTUALIZACIÓN DE VACAS (MOVIMIENTO AUTOMÁTICO)
// ============================================================================
// Se ejecuta cada frame (60 FPS). Cada vaca tiene un timer que cuenta hasta 120
// frames (~2 segundos). Cuando el timer llega a 120, la vaca se mueve una celda
// en una dirección aleatoria, validando colisiones antes de moverse.
// ============================================================================
void mapaActualizarVacas(void) {
  if (!gCollisionMap)
    return;

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
        case DIR_FRONT: // Abajo
          nuevoY += TILE_SIZE;
          break;
        case DIR_BACK: // Arriba
          nuevoY -= TILE_SIZE;
          break;
        case DIR_LEFT: // Izquierda
          nuevoX -= TILE_SIZE;
          break;
        case DIR_RIGHT: // Derecha
          nuevoX += TILE_SIZE;
          break;
        }

        // Verificar límites del mapa
        if (nuevoX < 0 || nuevoX >= MAPA_SIZE - TILE_SIZE || nuevoY < 0 ||
            nuevoY >= MAPA_SIZE - TILE_SIZE) {
          continue; // Fuera de límites, probar otra dirección
        }

        // Convertir a coordenadas de celda para verificar colisión
        int celdaX = (int)(nuevoX / TILE_SIZE);
        int celdaY = (int)(nuevoY / TILE_SIZE);

        // Verificar que esté dentro de la grid
        if (celdaX < 0 || celdaX >= GRID_SIZE || celdaY < 0 ||
            celdaY >= GRID_SIZE) {
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
        bool celdaVacia =
            (simboloDestino == SIMBOLO_VACIO || simboloDestino == 0);

        if (colisionLibre && celdaVacia) {
          // Calcular celda actual de la vaca ANTES de moverse
          int viejaCeldaX = (int)(v->x / TILE_SIZE);
          int viejaCeldaY = (int)(v->y / TILE_SIZE);

          // Sincronizar movimiento con mapaObjetos
          mapaMoverObjeto(v->x, v->y, nuevoX, nuevoY, SIMBOLO_VACA);

          // NUEVO: Sincronizar gCollisionMap (limpiar vieja celda, marcar
          // nueva)
          if (viejaCeldaY >= 0 && viejaCeldaY < GRID_SIZE && viejaCeldaX >= 0 &&
              viejaCeldaX < GRID_SIZE) {
            *(*(gCollisionMap + viejaCeldaY) + viejaCeldaX) =
                0; // Liberar celda anterior
          }
          *(*(gCollisionMap + celdaY) + celdaX) =
              3; // Marcar nueva celda como ocupada

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

  // ================================================================
  // SINCRONIZACIÓN FORZADA: Limpiar y remarcar posiciones de vacas
  // ================================================================
  // Esto asegura que mapaObjetos siempre refleje la posición real
  // de cada vaca, eliminando cualquier celda 'V' huérfana.
  // ================================================================

  // Paso 1: Limpiar TODAS las celdas 'V' de la matriz
  for (int f = 0; f < GRID_SIZE; f++) {
    for (int c = 0; c < GRID_SIZE; c++) {
      if (*(*(mapaObjetos + f) + c) == SIMBOLO_VACA) {
        *(*(mapaObjetos + f) + c) = SIMBOLO_VACIO;
      }
    }
  }

  // Paso 2: Remarcar las posiciones reales de cada vaca
  Vaca *pVaca = gVacas;
  for (int i = 0; i < gNumVacas; i++, pVaca++) {
    int f = (int)(pVaca->y / TILE_SIZE);
    int c = (int)(pVaca->x / TILE_SIZE);

    // Validar límites y marcar en matriz
    if (f >= 0 && f < GRID_SIZE && c >= 0 && c < GRID_SIZE) {
      *(*(mapaObjetos + f) + c) = SIMBOLO_VACA;
    }
  }
}

static void dibujarUnidadCombat(HDC hdcBuffer, HDC hdcSprites, Unidad *u,
                                Camara cam, int anchoP, int altoP,
                                bool permitirStand,
                                bool esEnemigo, bool atacando, int atkFrame) {
  if (!u || u->x < 0 || u->y < 0)
    return;

  int pantX = (int)((u->x - cam.x) * cam.zoom);
  int pantY = (int)((u->y - cam.y) * cam.zoom);
  int tam = (int)(64 * cam.zoom);
  if (pantX + tam <= 0 || pantX >= anchoP || pantY + tam <= 0 || pantY >= altoP)
    return;

  HBITMAP sprite = NULL;
  int dirIdx = (u->dir == DIR_RIGHT) ? 1 : 0;
  bool muerto = (u->vida <= 0);
  ULONGLONG dtMuerte = 0;
  if (muerto) {
    ULONGLONG ahora = GetTickCount64();
    if (unidadCuerpoDesaparecido(u, ahora, &dtMuerte)) {
      return; // Ocultar sprite tras 5s del deceso
    }
  }
  if (u->tipo == TIPO_CABALLERO) {
    if (muerto) {
      // Animar muerte con dos frames: die_1 luego die_2
      if (dtMuerte < 350 && hCaballeroDie[dirIdx]) {
        sprite = hCaballeroDie[dirIdx];
      } else if (hCaballeroDie2[dirIdx]) {
        sprite = hCaballeroDie2[dirIdx];
      } else {
        sprite = hCaballeroDie[dirIdx];
      }
    } else if (u->recibiendoAtaque && hCaballeroDefense[dirIdx]) {
      sprite = hCaballeroDefense[dirIdx];
    } else if (atacando && hCaballeroAtk[dirIdx][atkFrame % 3]) {
      sprite = hCaballeroAtk[dirIdx][atkFrame % 3];
    } else if (!u->moviendose && permitirStand && hCaballeroStand[dirIdx]) {
      sprite = hCaballeroStand[dirIdx];
    } else {
      sprite = hCaballeroBmp[u->dir];
    }
  } else if (u->tipo == TIPO_CABALLERO_SIN_ESCUDO) {
    if (muerto) {
      if (dtMuerte < 350 && hCaballeroDie[dirIdx]) {
        sprite = hCaballeroDie[dirIdx];
      } else if (hCaballeroDie2[dirIdx]) {
        sprite = hCaballeroDie2[dirIdx];
      } else {
        sprite = hCaballeroDie[dirIdx];
      }
    } else if (atacando && (hCaballeroSEAtk[dirIdx][atkFrame % 3] || hCaballeroAtk[dirIdx][atkFrame % 3])) {
      // Preferir sprites propios de CSE, caer a los del caballero si faltan
      sprite = hCaballeroSEAtk[dirIdx][atkFrame % 3] ? hCaballeroSEAtk[dirIdx][atkFrame % 3]
                                                     : hCaballeroAtk[dirIdx][atkFrame % 3];
    } else if (!u->moviendose && permitirStand && hCaballeroSEStand[dirIdx]) {
      // Stand del caballero sin escudo solo en islas no conquistadas
      sprite = hCaballeroSEStand[dirIdx];
    } else {
      sprite = hCaballeroSinEscudoBmp[u->dir];
    }
  } else { // GUERRERO
    if (muerto) {
      if (dtMuerte < 350 && hGuerreroDie[dirIdx]) {
        sprite = hGuerreroDie[dirIdx];
      } else if (hGuerreroDie2[dirIdx]) {
        sprite = hGuerreroDie2[dirIdx];
      } else {
        sprite = hGuerreroDie[dirIdx];
      }
    } else if (atacando && hGuerreroAtk[dirIdx][atkFrame % 2]) {
      sprite = hGuerreroAtk[dirIdx][atkFrame % 2];
    } else if (u->moviendose && !gEsIslaPrincipalActual && hGuerreroWalk[dirIdx]) {
      // Walk del guerrero solo fuera de la isla inicial
      sprite = hGuerreroWalk[dirIdx];
    } else if (!u->moviendose && permitirStand && hGuerreroStand[dirIdx]) {
      sprite = hGuerreroStand[dirIdx];
    } else {
      // Fallback: en isla principal usar sprite base; fuera, si existe, walk
      sprite = (!gEsIslaPrincipalActual && hGuerreroWalk[dirIdx])
                  ? hGuerreroWalk[dirIdx]
                  : hGuerreroBmp[u->dir];
    }
  }

  if (sprite) {
    SelectObject(hdcSprites, sprite);
    TransparentBlt(hdcBuffer, pantX, pantY, tam, tam, hdcSprites, 0, 0, 64, 64,
                   RGB(255, 255, 255));
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

// ============================================================================
// DIBUJADO DE BARRAS DE VIDA
// ============================================================================
static void dibujarBarraVida(HDC hdc, int x, int y, int vida, int vidaMax,
                             float zoom) {
  if (vidaMax <= 0)
    return;

  // Ancho de la barra (proporcional al sprite 64px)
  int anchoBarra = (int)(50 * zoom);
  int altoBarra = (int)(5 * zoom);

  // Posición relativa a la unidad (arriba del sprite 64x64)
  int barraX = x + (int)((64 * zoom - anchoBarra) / 2);
  int barraY = y - (int)(8 * zoom);

  // 1. Fondo (Rojo oscuro/negro)
  HBRUSH hBrushFondo = CreateSolidBrush(RGB(50, 0, 0));
  RECT rFondo;
  rFondo.left = barraX;
  rFondo.top = barraY;
  rFondo.right = barraX + anchoBarra;
  rFondo.bottom = barraY + altoBarra;
  FillRect(hdc, &rFondo, hBrushFondo);
  DeleteObject(hBrushFondo);

  // 2. Vida (Verde brillante)
  float porcentaje = (float)vida / (float)vidaMax;
  if (porcentaje < 0)
    porcentaje = 0;
  if (porcentaje > 1)
    porcentaje = 1;

  int anchoVida = (int)(anchoBarra * porcentaje);
  if (anchoVida > 0) {
    HBRUSH hBrushVida = CreateSolidBrush(RGB(0, 255, 0));
    RECT rVida;
    rVida.left = barraX;
    rVida.top = barraY;
    rVida.right = barraX + anchoVida;
    rVida.bottom = barraY + altoBarra;
    FillRect(hdc, &rVida, hBrushVida);
    DeleteObject(hBrushVida);
  }

  // 3. Borde (Negro)
  HPEN hPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
  HPEN oldPen = (HPEN)SelectObject(hdc, hPen);
  HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
  Rectangle(hdc, barraX, barraY, barraX + anchoBarra, barraY + altoBarra);
  SelectObject(hdc, oldPen);
  SelectObject(hdc, oldBrush);
  DeleteObject(hPen);
}

// Variante con color configurable para la barra de vida (usado por enemigos)
static void dibujarBarraVidaColor(HDC hdc, int x, int y, int vida, int vidaMax,
                                  float zoom, COLORREF colorVida) {
  if (vidaMax <= 0)
    return;

  int anchoBarra = (int)(50 * zoom);
  int altoBarra = (int)(5 * zoom);

  int barraX = x + (int)((64 * zoom - anchoBarra) / 2);
  int barraY = y - (int)(8 * zoom);

  HBRUSH hBrushFondo = CreateSolidBrush(RGB(50, 0, 0));
  RECT rFondo = {barraX, barraY, barraX + anchoBarra, barraY + altoBarra};
  FillRect(hdc, &rFondo, hBrushFondo);
  DeleteObject(hBrushFondo);

  float porcentaje = (float)vida / (float)vidaMax;
  if (porcentaje < 0) porcentaje = 0;
  if (porcentaje > 1) porcentaje = 1;

  int anchoVida = (int)(anchoBarra * porcentaje);
  if (anchoVida > 0) {
    HBRUSH hBrushVida = CreateSolidBrush(colorVida);
    RECT rVida = {barraX, barraY, barraX + anchoVida, barraY + altoBarra};
    FillRect(hdc, &rVida, hBrushVida);
    DeleteObject(hBrushVida);
  }

  HPEN hPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
  HPEN oldPen = (HPEN)SelectObject(hdc, hPen);
  HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
  Rectangle(hdc, barraX, barraY, barraX + anchoBarra, barraY + altoBarra);
  SelectObject(hdc, oldPen);
  SelectObject(hdc, oldBrush);
  DeleteObject(hPen);
}

void dibujarMundo(HDC hdc, RECT rect, Camara cam, struct Jugador *pJugador,
                  struct MenuCompra *menu, MenuEmbarque *menuEmb,
                  int highlightFila, int highlightCol, void *menuPausaPtr) {
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

  // 1.5. DIBUJAR EDIFICIOS - antes del Y-sorting para que estén debajo de
  // unidades
  if (pJugador->ayuntamiento != NULL) {
    Edificio *edificio = (Edificio *)pJugador->ayuntamiento;
    edificioDibujar(hdcBuffer, edificio, cam.x, cam.y, cam.zoom, anchoP, altoP,
            pJugador->islaActual);
  }

  // (Sección única: dibujar infraestructura disponible)
  if (pJugador->mina != NULL) {
    Edificio *edificioMina = (Edificio *)pJugador->mina;
    edificioDibujar(hdcBuffer, edificioMina, cam.x, cam.y, cam.zoom, anchoP,
                    altoP, pJugador->islaActual);
  }

  if (pJugador->cuartel != NULL) {
    Edificio *edificioCuartel = (Edificio *)pJugador->cuartel;
    edificioDibujar(hdcBuffer, edificioCuartel, cam.x, cam.y, cam.zoom, anchoP,
                    altoP, pJugador->islaActual);
  }

  HDC hdcSprites = CreateCompatibleDC(hdc);

  // Nota: Dibujamos árboles por fila para preservar Y-sorting con unidades.

  int enemigosActivosCount = 0;
  Unidad *enemigosActivos =
      navegacionObtenerEnemigosActivos(&enemigosActivosCount);
  static int frameAtaque = 0;
  {
    static ULONGLONG sUltimoAtkMs = 0;
    ULONGLONG ahora = GetTickCount64();
    if (sUltimoAtkMs == 0 || (ahora > sUltimoAtkMs && (ahora - sUltimoAtkMs) >= 500ULL)) {
      frameAtaque = (frameAtaque + 1) % 6;
      sUltimoAtkMs = ahora;
    }
  }
  // Determinar si se permite mostrar sprites de stand en la isla actual
  bool permitirStand = navegacionIslaActualNoConquistada(pJugador);
  bool esIslaPrincipal = false;
  if (pJugador) {
    int islaInicial = navegacionObtenerIslaInicial();
    esIslaPrincipal = (pJugador->islaActual == islaInicial);
  }
  gEsIslaPrincipalActual = esIslaPrincipal;
  bool permitirStandGuerrero = permitirStand && !esIslaPrincipal;

  Unidad *aliadosLista[12];
  int numAliadosLista = 0;
  for (int i = 0; i < 4; i++)
    aliadosLista[numAliadosLista++] = &pJugador->caballeros[i];
  for (int i = 0; i < 4; i++)
    aliadosLista[numAliadosLista++] = &pJugador->caballerosSinEscudo[i];
  for (int i = 0; i < 4; i++)
    aliadosLista[numAliadosLista++] = &pJugador->guerreros[i];

  bool ataqueAliados[12] = {false};
  bool ataqueEnemigos[8] = {false};
  const float rangoAtaque = RANGO_GOLPE_MELEE; // compartido con batallas
  const float rangoAtaque2 = rangoAtaque * rangoAtaque;
  if (enemigosActivos && enemigosActivosCount > 0) {
    for (int a = 0; a < numAliadosLista; a++) {
      Unidad *al = aliadosLista[a];
      if (!al || al->vida <= 0 || al->x < 0)
        continue;
      for (int e = 0; e < enemigosActivosCount; e++) {
        Unidad *en = &enemigosActivos[e];
        if (en->vida <= 0 || en->x < 0)
          continue;
        float dx = al->x - en->x;
        float dy = al->y - en->y;
        float d2 = dx * dx + dy * dy;
        if (d2 <= rangoAtaque2) {
          ataqueAliados[a] = true;
          ataqueEnemigos[e] = true;
          break;
        }
      }
    }
  }

  // Puntero a la matriz de objetos para aritmética de punteros
  char (*ptrMatriz)[GRID_SIZE] = mapaObjetos;

  // Recorrer cada fila del mapa (0 a GRID_SIZE-1)
  for (int f = 0; f < GRID_SIZE; f++) {
    // A) Árboles se dibujan al final de la fila para quedar encima.

    // ================================================================
    // B) DIBUJAR OBREROS CUYA BASE (y + 64) COINCIDE CON ESTA FILA
    // ================================================================
    // La "base de los pies" del obrero está en y + TILE_SIZE (64)
    // Calculamos la fila del mapa correspondiente a esa coordenada Y
    int yMinFila = f * TILE_SIZE;
    int yMaxFila = (f + 1) * TILE_SIZE;

    // Puntero base al array de obreros
    Unidad *baseObreros = pJugador->obreros;

    for (Unidad *o = baseObreros; o < baseObreros + MAX_OBREROS; o++) {
      // La base del obrero (pies) está en o->y + TILE_SIZE
      float basePies = o->y + (float)TILE_SIZE;

      // Si la base del obrero cae en esta fila, dibujarlo
      // Si la base del obrero cae en esta fila, dibujarlo
      if (basePies >= (float)yMinFila && basePies < (float)yMaxFila) {
        int pantX = (int)((o->x - cam.x) * cam.zoom);
        int pantY = (int)((o->y - cam.y) * cam.zoom);
        int tam = (int)(64 * cam.zoom); // Definir tam (asumiendo 64px)

        if (pantX + tam > 0 && pantX < anchoP && pantY + tam > 0 &&
            pantY < altoP) {
          // SELECCIONAR SPRITE SEGUN DIRECCION
          // Suponemos que hObreroBmp es un array de 4 bitmaps [FRONT, BACK,
          // LEFT, RIGHT] o algo similar. La estructura Unidad tiene 'dir'.
          // Buscamos hObreroBmp global.

          // NOTA: Como no vi la declaración exacta, asumo el patrón usado en
          // caballeros (hCaballeroBmp[c->dir]) Si falla, el compilador avisará
          // y corregiremos el nombre exacto.
          SelectObject(hdcSprites, hObreroBmp[o->dir]);

          // Dibujar Frame actual (si hay animación por frames en el bitmap,
          // ajustar srcX) Por ahora dibujamos el frame 0 (0,0) o usamos
          // o->frame si el bitmap es una tira. Viendo el código original
          // (truncado), asumiremos dibujo simple de 64x64.
          TransparentBlt(hdcBuffer, pantX, pantY, tam, tam, hdcSprites, 0, 0,
                         64, 64, RGB(255, 255, 255));

          // Dibujar barra de vida
          if (unidadBarraVisible(o)) {
            dibujarBarraVida(hdcBuffer, pantX, pantY, o->vida, o->vidaMax,
                             cam.zoom);
          }
        }

        // Círculo de selección
        if (o->seleccionado && unidadBarraVisible(o)) {
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

    // ================================================================
    // Z) DIBUJAR ÁRBOLES DE ESTA FILA (AL FINAL PARA ESTAR ENCIMA)
    // ================================================================
    for (int c = 0; c < GRID_SIZE; c++) {
      char celdaContenido = *(*(ptrMatriz + f) + c);
      if (celdaContenido == SIMBOLO_ARBOL) {
        int mundoX = c * TILE_SIZE;
        int mundoY = f * TILE_SIZE;
        int pantX = (int)((mundoX - cam.x) * cam.zoom);
        int pantY = (int)((mundoY - cam.y) * cam.zoom);
        int tamZoom = (int)(SPRITE_ARBOL * cam.zoom);
        if (pantX + tamZoom > 0 && pantX < anchoP && pantY + tamZoom > 0 &&
            pantY < altoP) {
          SelectObject(hdcSprites, hArboles[0]);
          TransparentBlt(hdcBuffer, pantX, pantY, tamZoom, tamZoom, hdcSprites,
                         0, 0, SPRITE_ARBOL, SPRITE_ARBOL, RGB(255, 255, 255));
          
          // DIBUJAR BARRA DE VIDA DEL ARBOL
          // Vida maxima 3, actual gArbolesVida[f][c]
          int vidaActual = gArbolesVida[f][c];
          if (vidaActual > 0 && vidaActual < 3) { // Solo mostrar si esta dañado (opcional, o siempre)
             // El usuario pidio "encima de cada arbol este una barra de vida"
             // Asi que la mostramos siempre o solo al dañarse?
             // "encima de cada arbol este una barra de vida" -> Siempre o casi siempre.
             // Mostremosla siempre para cumplir literal, o mejor:
             // Estilo simple: verde/rojo. 
             // Ajuste de centrado: (128 - 64) / 2 = 32px de offset
             int offsetX = (int)(32 * cam.zoom);
             dibujarBarraVida(hdcBuffer, pantX + offsetX, pantY - 10, vidaActual, 3, cam.zoom);
          } else if (vidaActual == 3) {
             // Mostrar tambien full vida si se pide "encima de cada arbol"
             int offsetX = (int)(32 * cam.zoom);
             dibujarBarraVida(hdcBuffer, pantX + offsetX, pantY - 10, 3, 3, cam.zoom);
          }
        }
      }
    }

    // C) DIBUJAR CABALLEROS (NUEVO)
    Unidad *baseCaballeros = pJugador->caballeros;
    for (Unidad *c = baseCaballeros; c < baseCaballeros + MAX_CABALLEROS; c++) {
      float basePies = c->y + (float)TILE_SIZE;

      if (basePies >= (float)yMinFila && basePies < (float)yMaxFila) {
        int pantX = (int)((c->x - cam.x) * cam.zoom);
        int pantY = (int)((c->y - cam.y) * cam.zoom);
        int tam = (int)(64 * cam.zoom);

        if (pantX + tam > 0 && pantX < anchoP && pantY + tam > 0 &&
            pantY < altoP) {
          int idxCab = (int)(c - baseCaballeros);
          bool atacando =
              (idxCab >= 0 && idxCab < 12) ? ataqueAliados[idxCab] : false;
          dibujarUnidadCombat(hdcBuffer, hdcSprites, c, cam, anchoP, altoP,
                              permitirStand, false, atacando, frameAtaque);

          // Dibujar barra de vida
          if (unidadBarraVisible(c)) {
            dibujarBarraVida(hdcBuffer, pantX, pantY, c->vida, c->vidaMax,
                             cam.zoom);
          }

          // Círculo de selección
          if (c->seleccionado && unidadBarraVisible(c)) {
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

    // C.5) DIBUJAR CABALLEROS SIN ESCUDO (NUEVO)
    Unidad *baseCSE = pJugador->caballerosSinEscudo;
    for (Unidad *c = baseCSE; c < baseCSE + MAX_CABALLEROS_SIN_ESCUDO; c++) {
      float basePies = c->y + (float)TILE_SIZE;

      if (basePies >= (float)yMinFila && basePies < (float)yMaxFila) {
        int pantX = (int)((c->x - cam.x) * cam.zoom);
        int pantY = (int)((c->y - cam.y) * cam.zoom);
        int tam = (int)(64 * cam.zoom);

        if (pantX + tam > 0 && pantX < anchoP && pantY + tam > 0 &&
            pantY < altoP) {
          SelectObject(hdcSprites, hCaballeroSinEscudoBmp[c->dir]);

          // Dibujar barra de vida
          if (unidadBarraVisible(c)) {
            dibujarBarraVida(hdcBuffer, pantX, pantY, c->vida, c->vidaMax,
                             cam.zoom);
          }

          // Círculo de selección (verde, igual que aliados)
          int idxCse = 4 + (int)(c - baseCSE);
          bool atacando =
              (idxCse >= 0 && idxCse < 12) ? ataqueAliados[idxCse] : false;
          dibujarUnidadCombat(hdcBuffer, hdcSprites, c, cam, anchoP, altoP,
                              permitirStand, false, atacando, frameAtaque);
          if (c->seleccionado && unidadBarraVisible(c)) {
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
    for (Unidad *g = baseGuerreros; g < baseGuerreros + MAX_GUERREROS; g++) {
      float basePies = g->y + (float)TILE_SIZE;

      if (basePies >= (float)yMinFila && basePies < (float)yMaxFila) {
        int pantX = (int)((g->x - cam.x) * cam.zoom);
        int pantY = (int)((g->y - cam.y) * cam.zoom);
        int tam = (int)(64 * cam.zoom);
        if (pantX + tam > 0 && pantX < anchoP && pantY + tam > 0 &&
            pantY < altoP) {
          int idxG = 8 + (int)(g - baseGuerreros);
          bool atacando =
              (idxG >= 0 && idxG < 12) ? ataqueAliados[idxG] : false;
          dibujarUnidadCombat(hdcBuffer, hdcSprites, g, cam, anchoP, altoP,
                              permitirStandGuerrero, false, atacando, frameAtaque);

          // Dibujar barra de vida
          if (unidadBarraVisible(g)) {
            dibujarBarraVida(hdcBuffer, pantX, pantY, g->vida, g->vidaMax,
                             cam.zoom);
          }
          // Círculo de selección (verde, unificado con aliados)
          if (g->seleccionado && unidadBarraVisible(g)) {
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

    // D.5) DIBUJAR ENEMIGOS PASIVOS
    if (enemigosActivos && enemigosActivosCount > 0) {
      for (int idx = 0; idx < enemigosActivosCount; idx++) {
        Unidad *e = &enemigosActivos[idx];
        float basePies = e->y + (float)TILE_SIZE;

        if (basePies >= (float)yMinFila && basePies < (float)yMaxFila) {
          int pantX = (int)((e->x - cam.x) * cam.zoom);
          int pantY = (int)((e->y - cam.y) * cam.zoom);
          int tam = (int)(64 * cam.zoom);

          if (pantX + tam > 0 && pantX < anchoP && pantY + tam > 0 &&
              pantY < altoP) {
                bool atacando = (idx >= 0 && idx < 8) ? ataqueEnemigos[idx] : false;
                bool permitirStandEnemigo = permitirStand;
                if (e->tipo == TIPO_GUERRERO) {
                  permitirStandEnemigo = permitirStandGuerrero;
                }
                dibujarUnidadCombat(hdcBuffer, hdcSprites, e, cam, anchoP, altoP,
                  permitirStandEnemigo, true, atacando, frameAtaque);
                bool hudVisible = unidadBarraVisible(e);
                // Barra de vida enemiga (rojo)
                if (hudVisible) {
                  dibujarBarraVidaColor(hdcBuffer, pantX, pantY, e->vida, e->vidaMax, cam.zoom, RGB(200, 60, 60));
                  HBRUSH nullBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
                  HPEN rojo = CreatePen(PS_SOLID, 2, RGB(200, 60, 60));
                  SelectObject(hdcBuffer, nullBrush);
                  SelectObject(hdcBuffer, rojo);
                  Ellipse(hdcBuffer, pantX, pantY + tam - 10, pantX + tam,
                    pantY + tam + 5);
                  DeleteObject(rojo);
                }
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
        if (pantX + tam > 0 && pantX < anchoP && pantY + tam > 0 &&
            pantY < altoP) {
          // Seleccionar sprite según dirección de la vaca
          SelectObject(hdcSprites, hVacaBmp[vaca->dir]);
          TransparentBlt(hdcBuffer, pantX, pantY, tam, tam, hdcSprites, 0, 0,
                         64, 64, RGB(255, 255, 255));
        }
      }
    }

    // F) DIBUJAR BARCO SI ESTÁ ACTIVO (192x192)
    if (pJugador->barco.activo) {
      Barco *b = &pJugador->barco;
      // El barco es 192px de alto, por lo que su base está en y + 192
      float basePies = b->y + 192.0f;

      // Debug temporal cada 60 frames aprox para no spamear
      static int debugFrame = 0;
      debugFrame++;
      if (debugFrame % 300 == 0 && b->construido) {
           printf("[RENDER BARCO] Pos(%.1f, %.1f) BasePies: %.1f FilaActual: %d-%d\n", 
                  b->x, b->y, basePies, yMinFila, yMaxFila);
      }

      if (basePies >= (float)yMinFila && basePies < (float)yMaxFila) {
        int pantX = (int)((b->x - cam.x) * cam.zoom);
        int pantY = (int)((b->y - cam.y) * cam.zoom);
        int tam = (int)(192 * cam.zoom);

        if (pantX + tam > 0 && pantX < anchoP && pantY + tam > 0 &&
            pantY < altoP) {
          // Seleccionar sprite: destruido o construido según estado
          if (b->construido) {
            HBITMAP sprite = hBarcoBmp[b->dir];
            if (!sprite) {
                 printf("[ERROR RENDER] Sprite de barco NULL para dir %d\n", b->dir);
                 sprite = hBarcoBmp[0]; // Fallback
            }
            SelectObject(hdcSprites, sprite);
          } else {
            SelectObject(hdcSprites, hBarcoDestruidoBmp);
          }
          BOOL res = TransparentBlt(hdcBuffer, pantX, pantY, tam, tam, hdcSprites, 0, 0,
                         192, 192, RGB(255, 255, 255));
          if (!res) printf("[ERROR RENDER] Fallo TransparentBlt barco\n");
        }
      }
    }
  }

  // 3. DIBUJAR INTERFAZ DE USUARIO (MENÚ) - AQUÍ EVITAMOS FLICKERING
  // Dibujamos el menú sobre el buffer ANTES de volcarlo a pantalla
  if (menu != NULL) {
    menuCompraDibujar(hdcBuffer, menu, pJugador);
  }

  // DIBUJAR MENÚ DE ENTRENAMIENTO (si está activo)
  extern MenuEntrenamiento menuEntrenamiento;
  if (menuEntrenamiento.abierto) {
    menuEntrenamientoDibujar(hdcBuffer, &menuEntrenamiento, pJugador);
  }

  // NUEVO: Dibujar menú de embarque si está activo (DENTRO DEL BUFFER)
  if (menuEmb != NULL && menuEmb->activo) {
    menuEmbarqueDibujar(hdcBuffer, menuEmb, pJugador);
  }

  // PANEL HUD DE RECURSOS (esquina superior derecha)
  // Se dibuja siempre que el jugador esté en vista local
  panelRecursosDibujar(hdcBuffer, pJugador, anchoP);

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
    HPEN hPen = CreatePen(PS_SOLID, 2, RGB(255, 255, 0)); // Amarillo brillante
    HPEN hOldPen = (HPEN)SelectObject(hdcBuffer, hPen);
    HBRUSH hBrush = CreateSolidBrush(RGB(255, 255, 0));
    HBRUSH hOldBrush = (HBRUSH)SelectObject(hdcBuffer, hBrush);

    // Dibujar con transparencia (solo borde)
    SelectObject(hdcBuffer, GetStockObject(NULL_BRUSH));
    Rectangle(hdcBuffer, pantallaX, pantallaY, pantallaX + tamZoom,
              pantallaY + tamZoom);

    // Dibujar información de la celda
    char info[128];
    sprintf(info, "Celda [%d][%d] | %dx%dpx", highlightFila, highlightCol,
            TILE_SIZE, TILE_SIZE);

    SetTextColor(hdcBuffer, RGB(255, 255, 0));
    SetBkMode(hdcBuffer, TRANSPARENT);
    TextOutA(hdcBuffer, pantallaX + 5, pantallaY + 5, info, strlen(info));

    // Restaurar
    SelectObject(hdcBuffer, hOldPen);
    SelectObject(hdcBuffer, hOldBrush);
    DeleteObject(hPen);
    DeleteObject(hBrush);
  }

  // Dibujar menú de pausa DENTRO del buffer (evita parpadeo)
  MenuPausa *menuPausa = (MenuPausa *)menuPausaPtr;
  if (menuPausa != NULL && menuPausa->activo) {
    menuPausaDibujar(hdcBuffer, rect, menuPausa);
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
// FUNCIONES DE GESTIÓN DE OBJETOS DEL MAPA (ACCESO EXTERNO)
// ============================================================================

int mapaObtenerTipoObjeto(int f, int c) {
  if (f >= 0 && f < GRID_SIZE && c >= 0 && c < GRID_SIZE) {
    // Aritmética de punteros para acceso a matriz
    return *(*(mapaObjetos + f) + c);
  }
  return 0; // 0 = Nada
}

void mapaEliminarObjeto(int f, int c) {
  if (f >= 0 && f < GRID_SIZE && c >= 0 && c < GRID_SIZE) {
    char tipo = mapaObjetos[f][c];

    // 1. Eliminar de la matriz lógica
    *(*(mapaObjetos + f) + c) = 0;

    // 2. Si es una VACA, eliminarla también del array dinámico
    if (tipo == SIMBOLO_VACA) {
      for (int i = 0; i < gNumVacas; i++) {
        // La posición lógica se basa en el top-left (igual que
        // mapaRegistrarObjeto)
        int vF = (int)(gVacas[i].y / TILE_SIZE);
        int vC = (int)(gVacas[i].x / TILE_SIZE);

        // También verificamos vecindad por si acaso, pero la celda debería
        // coincidir
        if (vF == f && vC == c) {
          // Desplazar el resto del array para llenar el hueco
          for (int k = i; k < gNumVacas - 1; k++) {
            gVacas[k] = gVacas[k + 1];
          }
          gNumVacas--;
          printf("[DEBUG] Vaca dinamica eliminada de gVacas. Restan: %d\n",
                 gNumVacas);
          break; // Asumimos una vaca por celda
        }
      }
    }

    // 3. Actualizar mapa de colisiones (el árbol ya no bloquea)
    mapaReconstruirCollisionMap();
  }
}

// ============================================================================
// SERIALIZACIÓN DEL MAPA
// ============================================================================
void mapaGuardar(FILE *f) {
  if (!f)
    return;
  // Guardar la matriz entera de objetos (64x64 ints)
  // mapaObjetos es int[GRID_SIZE][GRID_SIZE]
  fwrite(mapaObjetos, sizeof(int), GRID_SIZE * GRID_SIZE, f);
}

void mapaCargar(FILE *f) {
  if (!f)
    return;
  // Cargar la matriz entera
  fread(mapaObjetos, sizeof(int), GRID_SIZE * GRID_SIZE, f);

  // IMPORTANTE: Reconstruir colisiones tras cargar
  mapaReconstruirCollisionMap();
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
  printf("====================================================================="
         "=====\n");
  printf("                    MAPA LOGICO %dx%d (cada celda = %dpx)\n",
         GRID_SIZE, GRID_SIZE, TILE_SIZE);
  printf("====================================================================="
         "=====\n");
  printf("Leyenda: . =Vacio  A=Arbol  O=Obrero  C=Caballero  G=Guerrero\n");
  printf("         V=Vaca    B=Barco  E=Edificio  M=Mina  Q=Cuartel\n");
  printf("====================================================================="
         "=====\n\n");

  char (*ptrFila)[GRID_SIZE] = mapa;

  // Encabezado de columnas (solo números 0-9 repetidos)
  printf("      ");
  for (int c = 0; c < GRID_SIZE; c++) {
    printf("%d ", c % 10);
  }
  printf("\n");

  printf("     +");
  for (int c = 0; c < GRID_SIZE; c++)
    printf("--");
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
  for (int c = 0; c < GRID_SIZE; c++)
    printf("--");
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
  switch (contenido) {
  case SIMBOLO_ARBOL:
    printf("ARBOL\n");
    break;
  case SIMBOLO_VACIO:
    printf("Vacio\n");
    break;
  default:
    printf("%c\n", contenido);
  }
}

char obtenerContenidoCelda(char *celda) {
  if (celda == NULL)
    return '\0';
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

void mapaMoverObjeto(float viejoX, float viejoY, float nuevoX, float nuevoY,
                     char simbolo) {
  int viejaFila = (int)(viejoY / TILE_SIZE);
  int viejaCol = (int)(viejoX / TILE_SIZE);
  int nuevaFila = (int)(nuevoY / TILE_SIZE);
  int nuevaCol = (int)(nuevoX / TILE_SIZE);

  if (viejaFila != nuevaFila || viejaCol != nuevaCol) {
    if (viejaFila >= 0 && viejaFila < GRID_SIZE && viejaCol >= 0 &&
        viejaCol < GRID_SIZE) {
      // Solo limpiar si el contenido coincide con el simbolo móvil
      if (mapaObjetos[viejaFila][viejaCol] == simbolo) {
        mapaObjetos[viejaFila][viejaCol] = SIMBOLO_VACIO;
      }
    }

    if (nuevaFila >= 0 && nuevaFila < GRID_SIZE && nuevaCol >= 0 &&
        nuevaCol < GRID_SIZE) {
      // Evitar sobreescribir contenido estático (ej: árboles)
      if (mapaObjetos[nuevaFila][nuevaCol] == SIMBOLO_VACIO) {
        mapaObjetos[nuevaFila][nuevaCol] = simbolo;
      }
    }
  }
}

// Retorna true si la celda es tierra transitable; false si es agua u obstáculo.
bool mapaCeldaEsTierra(int fila, int col) {
  if (fila < 0 || col < 0 || fila >= GRID_SIZE || col >= GRID_SIZE)
    return false;

  collisionMapAllocIfNeeded();

  int valor = 0;
  if (gCollisionMap) {
    valor = *(*(gCollisionMap + fila) + col);
  }

  char simb = mapaObjetos[fila][col];

  // Bloquear cualquier cosa marcada como colisión u océano conocido
  if (valor != 0)
    return false;
  if (simb == SIMBOLO_AGUA)
    return false;

  // Validación adicional por color directo del mapa (por si el agua no fue
  // marcada)
  if (hMapaBmp) {
    BITMAP bm;
    if (GetObject(hMapaBmp, sizeof(BITMAP), &bm)) {
      int px = (col * TILE_SIZE) + (TILE_SIZE / 2);
      int py = (fila * TILE_SIZE) + (TILE_SIZE / 2);
      if (px >= 0 && px < bm.bmWidth && py >= 0 && py < bm.bmHeight) {
        HDC memDC = CreateCompatibleDC(NULL);
        HBITMAP oldBmp = (HBITMAP)SelectObject(memDC, hMapaBmp);
        COLORREF color = GetPixel(memDC, px, py);
        SelectObject(memDC, oldBmp);
        DeleteDC(memDC);
        if (color != CLR_INVALID) {
          BYTE r = GetRValue(color);
          BYTE g = GetGValue(color);
          BYTE b = GetBValue(color);
          bool agua = (b > r + 20 && b > g + 20 && b > 60) ||
                      (b > r && b > g && b > 100);
          if (agua)
            return false;
        }
      }
    }
  }

  return true;
}

int mapaContarArboles(void) {
  int contador = 0;
  for (int f = 0; f < GRID_SIZE; f++) {
    for (int c = 0; c < GRID_SIZE; c++) {
      if (mapaObjetos[f][c] == SIMBOLO_ARBOL) {
        contador++;
      }
    }
  }
  return contador;
}

void mapaActualizarArboles(void) {
  static int frameCounter = 0;
  frameCounter++;

  // Verificar cada ~5.0 segundos (300 frames) para regeneracion lenta
  if (frameCounter < 300)
    return;
  frameCounter = 0;

  int totalArboles = mapaContarArboles();
  // REQUISITO: Eliminar regeneración de árboles. Son recursos finitos.
  // Si se acaban, no vuelven a aparecer.
  return;

  /* Lógica anterior de regeneración deshabilitada
  if (totalArboles >= 20)
    return;
  */

  if (!hMapaBmp)
      return;

  printf("[DEBUG] Regenerando arboles (Faltan %d)...\n", 20 - totalArboles);

  HDC hdcPantalla = GetDC(NULL);
  HDC hdcMem = CreateCompatibleDC(hdcPantalla);
  HBITMAP hOldBmp = (HBITMAP)SelectObject(hdcMem, hMapaBmp);

  int **colMat = mapaObtenerCollisionMap();
  bool colocado = false;
  int intentos = 0;
  
  // Intentar agresivamente encontrar un lugar (200 intentos)
  while (!colocado && intentos < 200) {
    intentos++;
    int fila = rand() % GRID_SIZE;
    int col = rand() % GRID_SIZE;

    // 1. La celda debe estar totalmente vacía en el mapa de objetos
    if (mapaObjetos[fila][col] != SIMBOLO_VACIO)
      continue;

    // 2. No debe haber colisiones (edificios, agua, piedras)
    if (colMat && colMat[fila][col] != 0)
      continue;

    // 3. VERIFICACIÓN ESTRICTA DE PASTO
    // Usamos GetPixel para asegurar que SUELO SEA VERDE (Pasto)
    // Esto evita agua, arena, rocas, nieve, etc.
    int px = (col * TILE_SIZE) + (TILE_SIZE / 2);
    int py = (fila * TILE_SIZE) + (TILE_SIZE / 2);

    COLORREF color = GetPixel(hdcMem, px, py);
    if (color != CLR_INVALID) {
         BYTE r = GetRValue(color);
         BYTE g = GetGValue(color);
         BYTE b = GetBValue(color);

         // Criterio mas estricto para evitar ARENA (Rojo y Verde altos)
         // La arena suele tener componente Rojo alta (mezcla amarillo). El pasto tiene Rojo bajo.
         // Exigimos que el Verde supere al Rojo por un margen amplio (40) y limitamos el brillo Rojo maximo.
         bool esPasto = (g > r + 40 && g > b + 30 && r < 160 && g > 50);

         if (esPasto) {
             // Colocar árbol
             mapaObjetos[fila][col] = SIMBOLO_ARBOL;
             // Marcar colisión
             if (colMat) colMat[fila][col] = 1;
             
             colocado = true;
             printf("[DEBUG] Arbol regenerado en [%d][%d] (RGB: %d,%d,%d)\n", fila, col, r, g, b);
         }
    }
  }

  // Limpieza GDI
  SelectObject(hdcMem, hOldBmp);
  DeleteDC(hdcMem);
  ReleaseDC(NULL, hdcPantalla);
}

// Retorna true si el arbol fue destruido
bool mapaGolpearArbol(int f, int c) {
    if (f < 0 || f >= GRID_SIZE || c < 0 || c >= GRID_SIZE) return false;
    
    if (mapaObjetos[f][c] == SIMBOLO_ARBOL) {
        if (gArbolesVida[f][c] > 0) {
            gArbolesVida[f][c]--;
        }
        
        if (gArbolesVida[f][c] <= 0) {
            mapaEliminarObjeto(f, c);
            gArbolesVida[f][c] = 0;
            return true; // Destruido
        }
    }
    return false; // No destruido aun
}
