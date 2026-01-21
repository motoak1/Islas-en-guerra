#include "mapa.h"
#include "../recursos/edificios/edificios.h"
#include "../batallas/guardado/guardado.h"
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

// Definiciones de rutas de assets (relativas a output/)
#define RUTA_MAPA "../assets/islas/isla1.bmp"

#define ARBOL1 "../assets/arboles/arbol1.bmp"
#define ARBOL2 "../assets/arboles/arbol2.bmp"
#define ARBOL3 "../assets/arboles/arbol3.bmp"
#define ARBOL4 "../assets/arboles/arbol4.bmp"
#define ARBOL_FUEGO "../assets/arboles/arbol-fuego.bmp"
#define ARBOL_HIELO "../assets/arboles/arbol-hielo.bmp"

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

#define VACA_F "../assets/vaca/vaca_front.bmp"
#define VACA_B "../assets/vaca/vaca_back.bmp"
#define VACA_L "../assets/vaca/vaca_left.bmp"
#define VACA_R "../assets/vaca/vaca_right.bmp"

#define BARCO_F "../assets/barco/barco_front.bmp"
#define BARCO_B "../assets/barco/barco_back.bmp"
#define BARCO_L "../assets/barco/barco_left.bmp"
#define BARCO_R "../assets/barco/barco_right.bmp"
#define BARCO_DESTRUIDO "../assets/barco/barco_destruido.bmp"


#define VACA_FUEGO_F "../assets/vaca/vaca-fuego-front.bmp"
#define VACA_FUEGO_B "../assets/vaca/vaca-fuego-back.bmp"
#define VACA_FUEGO_L "../assets/vaca/vaca-fuego-left.bmp"
#define VACA_FUEGO_R "../assets/vaca/vaca-fuego-right.bmp"
#define VACA_HIELO_F "../assets/vaca/vaca-hielo-front.bmp"
#define VACA_HIELO_B "../assets/vaca/vaca-hielo-back.bmp"
#define VACA_HIELO_L "../assets/vaca/vaca-hielo-left.bmp"
#define VACA_HIELO_R "../assets/vaca/vaca-hielo-right.bmp"

#define CAB_FUEGO_FRONT "../assets/caballero-fuego/caballero-fuego-front.bmp"
#define CAB_FUEGO_BACK "../assets/caballero-fuego/caballero-fuego-back.bmp"
#define CAB_FUEGO_LEFT "../assets/caballero-fuego/caballero-fuego-left.bmp"
#define CAB_FUEGO_RIGHT "../assets/caballero-fuego/caballero-fuego-right.bmp"
#define CAB_FUEGO_STAND_L "../assets/caballero-fuego/caballero-fuego-war-stand-left.bmp"
#define CAB_FUEGO_STAND_R "../assets/caballero-fuego/caballero-fuego-war-stand-right.bmp"
#define CAB_FUEGO_DIE1_L "../assets/caballero-fuego/caballero-fuego-die-1-left.bmp"
#define CAB_FUEGO_DIE1_R "../assets/caballero-fuego/caballero-fuego-die-1-right.bmp"
#define CAB_FUEGO_DIE2_L "../assets/caballero-fuego/caballero-fuego-die-2-left.bmp"
#define CAB_FUEGO_DIE2_R "../assets/caballero-fuego/caballero-fuego-die-2-right.bmp"
#define CAB_FUEGO_MOVE1_L "../assets/caballero-fuego/caballero-fuego-war-move-1-left.bmp"
#define CAB_FUEGO_MOVE1_R "../assets/caballero-fuego/caballero-fuego-war-move-1-right.bmp"
#define CAB_FUEGO_MOVE2_L "../assets/caballero-fuego/caballero-fuego-war-move-2-left.bmp"
#define CAB_FUEGO_MOVE2_R "../assets/caballero-fuego/caballero-fuego-war-move-2-right.bmp"

#define CAB_HIELO_FRONT "../assets/caballero-hielo/caballero-hielo-front.bmp"
#define CAB_HIELO_BACK "../assets/caballero-hielo/caballero-hielo-back.bmp"
#define CAB_HIELO_LEFT "../assets/caballero-hielo/caballero-hielo-left.bmp"
#define CAB_HIELO_RIGHT "../assets/caballero-hielo/caballero-hielo-right.bmp"
#define CAB_HIELO_STAND_L "../assets/caballero-hielo/caballero-hielo-stand-left.bmp"
#define CAB_HIELO_STAND_R "../assets/caballero-hielo/caballero-hielo-stand-right.bmp"
#define CAB_HIELO_DIE1_L "../assets/caballero-hielo/caballero-hielo-die-1-left.bmp"
#define CAB_HIELO_DIE1_R "../assets/caballero-hielo/caballero-hielo-die-1-right.bmp"
#define CAB_HIELO_DIE2_L "../assets/caballero-hielo/caballero-hielo-die-2-left.bmp"
#define CAB_HIELO_DIE2_R "../assets/caballero-hielo/caballero-hielo-die-2-right.bmp"
#define CAB_HIELO_MOVE1_L "../assets/caballero-hielo/caballero-hielo-war-move-1-left.bmp"
#define CAB_HIELO_MOVE2_L "../assets/caballero-hielo/caballero-hielo-war-move-2-left.bmp"
#define CAB_HIELO_MOVE3_L "../assets/caballero-hielo/caballero-hielo-war-move-3-left.bmp"
#define CAB_HIELO_MOVE1_R "../assets/caballero-hielo/caballero-hielo-war-move-1-right.bmp"
#define CAB_HIELO_MOVE2_R "../assets/caballero-hielo/caballero-hielo-war-move-2-right.bmp"
#define CAB_HIELO_MOVE3_R "../assets/caballero-hielo/caballero-hielo-war-move-3-right.bmp"

#define GUERRERO_FUEGO_FRONT "../assets/guerrero-fuego/guerrero-fuego-front.bmp"
#define GUERRERO_FUEGO_BACK "../assets/guerrero-fuego/guerrero-fuego-back.bmp"
#define GUERRERO_FUEGO_LEFT "../assets/guerrero-fuego/guerrero-fuego-left.bmp"
#define GUERRERO_FUEGO_RIGHT "../assets/guerrero-fuego/guerrero-fuego-right.bmp"
#define GUERRERO_FUEGO_STAND_L "../assets/guerrero-fuego/guerrero-fuego-war-stand-left.bmp"
#define GUERRERO_FUEGO_STAND_R "../assets/guerrero-fuego/guerrero-fuego-war-stand-right.bmp"
#define GUERRERO_FUEGO_DIE1_L "../assets/guerrero-fuego/guerrero-fuego-die-1-left.bmp"
#define GUERRERO_FUEGO_DIE1_R "../assets/guerrero-fuego/guerrero-fuego-die-1-right.bmp"
#define GUERRERO_FUEGO_DIE2_L "../assets/guerrero-fuego/guerrero-fuego-die-2-left.bmp"
#define GUERRERO_FUEGO_DIE2_R "../assets/guerrero-fuego/guerrero-fuego-die-2-right.bmp"
#define GUERRERO_FUEGO_MOVE1_L "../assets/guerrero-fuego/guerrero-fuego-war-move-1-left.bmp"
#define GUERRERO_FUEGO_MOVE1_R "../assets/guerrero-fuego/guerrero-fuego-war-move-1-right.bmp"
#define GUERRERO_FUEGO_MOVE2_L "../assets/guerrero-fuego/guerrero-fuego-war-move-2-left.bmp"
#define GUERRERO_FUEGO_MOVE2_R "../assets/guerrero-fuego/guerrero-fuego-war-move-2-right.bmp"

#define GUERRERO_HIELO_FRONT "../assets/guerrero-hielo/guerrero-hielo-front.bmp"
#define GUERRERO_HIELO_BACK "../assets/guerrero-hielo/guerrero-hielo-back.bmp"
#define GUERRERO_HIELO_LEFT "../assets/guerrero-hielo/guerrero-hielo-left.bmp"
#define GUERRERO_HIELO_RIGHT "../assets/guerrero-hielo/guerrero-hielo-right.bmp"
#define GUERRERO_HIELO_STAND_L "../assets/guerrero-hielo/guerrero-hielo-war-stand-left.bmp"
#define GUERRERO_HIELO_STAND_R "../assets/guerrero-hielo/guerrero-hielo-war-stand-right.bmp"
#define GUERRERO_HIELO_DIE1_L "../assets/guerrero-hielo/guerrero-hielo-die-1-left.bmp"
#define GUERRERO_HIELO_DIE1_R "../assets/guerrero-hielo/guerrero-hielo-die-1-right.bmp"
#define GUERRERO_HIELO_DIE2_L "../assets/guerrero-hielo/guerrero-hielo-die-2-left.bmp"
#define GUERRERO_HIELO_DIE2_R "../assets/guerrero-hielo/guerrero-hielo-die-2-right.bmp"
#define GUERRERO_HIELO_MOVE1_L "../assets/guerrero-hielo/guerrero-hielo-war-move-1-left.bmp"
#define GUERRERO_HIELO_MOVE1_R "../assets/guerrero-hielo/guerrero-hielo-war-move-1-right.bmp"
#define GUERRERO_HIELO_MOVE2_L "../assets/guerrero-hielo/guerrero-hielo-war-move-2-left.bmp"
#define GUERRERO_HIELO_MOVE2_R "../assets/guerrero-hielo/guerrero-hielo-war-move-2-right.bmp"

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
static HBITMAP hCaballeroFuegoBmp[4] = {NULL};
static HBITMAP hCaballeroFuegoStand[2] = {NULL};
static HBITMAP hCaballeroFuegoDie[2] = {NULL};
static HBITMAP hCaballeroFuegoDie2[2] = {NULL};
static HBITMAP hCaballeroFuegoAtk[2][3] = {{NULL}};
static HBITMAP hCaballeroHieloBmp[4] = {NULL};
static HBITMAP hCaballeroHieloStand[2] = {NULL};
static HBITMAP hCaballeroHieloDie[2] = {NULL};
static HBITMAP hCaballeroHieloDie2[2] = {NULL};
static HBITMAP hCaballeroHieloAtk[2][3] = {{NULL}};
static HBITMAP hGuerreroFuegoBmp[4] = {NULL};
static HBITMAP hGuerreroFuegoStand[2] = {NULL};
static HBITMAP hGuerreroFuegoWalk[2] = {NULL};
static HBITMAP hGuerreroFuegoDie[2] = {NULL};
static HBITMAP hGuerreroFuegoDie2[2] = {NULL};
static HBITMAP hGuerreroFuegoAtk[2][2] = {{NULL}};
static HBITMAP hGuerreroHieloBmp[4] = {NULL};
static HBITMAP hGuerreroHieloStand[2] = {NULL};
static HBITMAP hGuerreroHieloWalk[2] = {NULL};
static HBITMAP hGuerreroHieloDie[2] = {NULL};
static HBITMAP hGuerreroHieloDie2[2] = {NULL};
static HBITMAP hGuerreroHieloAtk[2][2] = {{NULL}};

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

typedef enum {
  MAPA_TEMA_CLASICO = 0,
  MAPA_TEMA_HIELO = 1,
  MAPA_TEMA_FUEGO = 2
} TemaIsla;

static TemaIsla gTemaIslaActual = MAPA_TEMA_CLASICO;
static int gIslaSeleccionadaActual = 1;
static bool gSpritesFuegoCargados = false;
static bool gSpritesHieloCargados = false;

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
  return !unidadCuerpoDesaparecido(u, (ULONGLONG)GetTickCount(), NULL);
}

static bool mapaEsTemaFuego(void) { return gTemaIslaActual == MAPA_TEMA_FUEGO; }
static bool mapaEsTemaHielo(void) { return gTemaIslaActual == MAPA_TEMA_HIELO; }

static HBITMAP loadBmp(const char *path, int w, int h) {
  HBITMAP bmp = (HBITMAP)LoadImageA(NULL, path, IMAGE_BITMAP, w, h, LR_LOADFROMFILE);
  return bmp ? bmp : (HBITMAP)LoadImageA(NULL, path, IMAGE_BITMAP, w, h, LR_LOADFROMFILE);
}

static void cargarSpritesTema(int tema) {
    if ((tema == 1 && gSpritesHieloCargados) || (tema == 2 && gSpritesFuegoCargados)) return;
    
    // Arrays de punteros a globals y rutas
    // Carga recursiva de recursos gráficos según el tema seleccionado.
    HBITMAP *targets[] = {
        hCaballeroFuegoBmp, hCaballeroHieloBmp, // Front/Back/Left/Right
        hGuerreroFuegoBmp, hGuerreroHieloBmp
    };
    const char *paths[][4] = {
        {CAB_FUEGO_FRONT, CAB_FUEGO_BACK, CAB_FUEGO_LEFT, CAB_FUEGO_RIGHT},
        {CAB_HIELO_FRONT, CAB_HIELO_BACK, CAB_HIELO_LEFT, CAB_HIELO_RIGHT},
        {GUERRERO_FUEGO_FRONT, GUERRERO_FUEGO_BACK, GUERRERO_FUEGO_LEFT, GUERRERO_FUEGO_RIGHT},
        {GUERRERO_HIELO_FRONT, GUERRERO_HIELO_BACK, GUERRERO_HIELO_LEFT, GUERRERO_HIELO_RIGHT}
    };
    
    int tOffset = (tema == 2) ? 0 : 1; // 0=Fuego, 1=Hielo
    for(int i=0; i<4; i++) targets[tOffset*2][i] = loadBmp(paths[tOffset*2][i], 64, 64);
    for(int i=0; i<4; i++) targets[tOffset*2+1][i] = loadBmp(paths[tOffset*2+1][i], 64, 64);

    // Animaciones (Stand, Die, Atk) - Simplificado: Carga directa a punteros especificos
    struct { HBITMAP *arr; const char **files; int count; } anims[] = {
        {hCaballeroFuegoStand, (const char*[]){CAB_FUEGO_STAND_L, CAB_FUEGO_STAND_R}, 2},
        {hCaballeroFuegoDie, (const char*[]){CAB_FUEGO_DIE1_L, CAB_FUEGO_DIE1_R}, 2},
        {hCaballeroFuegoDie2, (const char*[]){CAB_FUEGO_DIE2_L, CAB_FUEGO_DIE2_R}, 2},
        {hGuerreroFuegoStand, (const char*[]){GUERRERO_FUEGO_STAND_L, GUERRERO_FUEGO_STAND_R}, 2},
        {hGuerreroFuegoDie, (const char*[]){GUERRERO_FUEGO_DIE1_L, GUERRERO_FUEGO_DIE1_R}, 2},
        {hGuerreroFuegoDie2, (const char*[]){GUERRERO_FUEGO_DIE2_L, GUERRERO_FUEGO_DIE2_R}, 2},
        {hGuerreroFuegoWalk, (const char*[]){GUERRERO_FUEGO_MOVE1_L, GUERRERO_FUEGO_MOVE1_R}, 2},
        
        {hCaballeroHieloStand, (const char*[]){CAB_HIELO_STAND_L, CAB_HIELO_STAND_R}, 2},
        {hCaballeroHieloDie, (const char*[]){CAB_HIELO_DIE1_L, CAB_HIELO_DIE1_R}, 2},
        {hCaballeroHieloDie2, (const char*[]){CAB_HIELO_DIE2_L, CAB_HIELO_DIE2_R}, 2},
        {hGuerreroHieloStand, (const char*[]){GUERRERO_HIELO_STAND_L, GUERRERO_HIELO_STAND_R}, 2},
        {hGuerreroHieloDie, (const char*[]){GUERRERO_HIELO_DIE1_L, GUERRERO_HIELO_DIE1_R}, 2},
        {hGuerreroHieloDie2, (const char*[]){GUERRERO_HIELO_DIE2_L, GUERRERO_HIELO_DIE2_R}, 2},
        {hGuerreroHieloWalk, (const char*[]){GUERRERO_HIELO_MOVE1_L, GUERRERO_HIELO_MOVE1_R}, 2}
    };
    
    int start = (tema==2) ? 0 : 7;
    int end = (tema==2) ? 7 : 14;
    for(int k=start; k<end; k++) 
        for(int j=0; j<anims[k].count; j++) 
            anims[k].arr[j] = loadBmp(anims[k].files[j], 64, 64);

    // Ataques (Logica especial para arrays multidimensionales)
    if (tema == 2) {
        const char *cfAtk[] = {CAB_FUEGO_MOVE1_L, CAB_FUEGO_MOVE2_L, CAB_FUEGO_MOVE2_L, CAB_FUEGO_MOVE1_R, CAB_FUEGO_MOVE2_R, CAB_FUEGO_MOVE2_R};
        for(int i=0; i<6; i++) hCaballeroFuegoAtk[i/3][i%3] = loadBmp(cfAtk[i], 64, 64);
        
        const char *gfAtk[] = {GUERRERO_FUEGO_MOVE1_L, GUERRERO_FUEGO_MOVE2_L, GUERRERO_FUEGO_MOVE1_R, GUERRERO_FUEGO_MOVE2_R};
        for(int i=0; i<4; i++) hGuerreroFuegoAtk[i/2][i%2] = loadBmp(gfAtk[i], 64, 64);
        gSpritesFuegoCargados = true;
    } else {
        const char *chAtk[] = {CAB_HIELO_MOVE1_L, CAB_HIELO_MOVE2_L, CAB_HIELO_MOVE3_L, CAB_HIELO_MOVE1_R, CAB_HIELO_MOVE2_R, CAB_HIELO_MOVE3_R};
         for(int i=0; i<6; i++) hCaballeroHieloAtk[i/3][i%3] = loadBmp(chAtk[i], 64, 64);
         
         const char *ghAtk[] = {GUERRERO_HIELO_MOVE1_L, GUERRERO_HIELO_MOVE2_L, GUERRERO_HIELO_MOVE1_R, GUERRERO_HIELO_MOVE2_R};
         for(int i=0; i<4; i++) hGuerreroHieloAtk[i/2][i%2] = loadBmp(ghAtk[i], 64, 64);
         gSpritesHieloCargados = true;
    }
}

static void cargarSpritesFuego(void) { cargarSpritesTema(2); }
static void cargarSpritesHielo(void) { cargarSpritesTema(1); }



static HBITMAP hMapaBmp =
    NULL; // Mapa de isla individual (isla1, isla2, o isla3)
static HBITMAP hMapaGlobalBmp =
    NULL; // NUEVO: Mapa global con las 3 islas (mapaDemo2.bmp)
static HBITMAP hArboles[4] = {NULL};

static char gRutaMapaPrincipal[MAX_PATH] = RUTA_MAPA;

// Matriz lógica de objetos - ahora usa CARACTERES (32x32 con celdas de 64px)
char mapaObjetos[GRID_SIZE][GRID_SIZE] = {0};

// --- COLISIONES (matriz dinámica int**)
// 0 = libre, 1 = ocupado (árboles u obstáculos)
// 2 = Ocupada por unidad (temporalmente bloqueada)
// Se utiliza puntero a puntero (int**) para gestión dinámica de memoria.
static int **gCollisionMap = NULL;

// --- SISTEMA DE VACAS DINÁMICAS ---
// Array de vacas con movimiento automático (10 vacas)
static Vaca gVacas[10];
static int gNumVacas = 0; // Cantidad de vacas activas


// PERSISTENCIA EN MEMORIA POR ISLA (1..3)
static bool gIslaGuardada[6] = {false};
static char gMapaObjetosIsla[6][GRID_SIZE][GRID_SIZE];
static int gCollisionIsla[6][GRID_SIZE][GRID_SIZE];
static Vaca gVacasIsla[6][10];
static int gNumVacasIsla[6] = {0};

// --- VIDA DE ARBOLES ---
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

// Elimina una vaca usando su ÍNDICE en el array.
// Evita el bug donde la vaca se mueve mientra el usuario confirma.
bool mapaEliminarVacaPorIndice(int indice) {
  // Validar índice
  if (indice < 0 || indice >= gNumVacas) {

    return false;
  }

  // Obtener la posición ACTUAL de la vaca (puede haber cambiado)
  Vaca *pVaca = gVacas + indice;  // Aritmética de punteros
  int vacaFila = (int)(pVaca->y / TILE_SIZE);
  int vacaCol = (int)(pVaca->x / TILE_SIZE);

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
    gIslaSeleccionadaActual = seleccion;
    if (seleccion == 5) {
      gTemaIslaActual = MAPA_TEMA_FUEGO;
    } else if (seleccion == 4) {
      gTemaIslaActual = MAPA_TEMA_HIELO;
    } else {
      gTemaIslaActual = MAPA_TEMA_CLASICO;
    }
}

int mapaObtenerIslaSeleccionada(void) { return gIslaSeleccionadaActual; }
bool mapaTemaActualEsFuego(void) { return mapaEsTemaFuego(); }
bool mapaTemaActualEsHielo(void) { return mapaEsTemaHielo(); }

void mapaReconstruirCollisionMap(void) {
  collisionMapAllocIfNeeded();
    collisionMapClear(0);

  // Reconstrucción del mapa de colisiones:
  // Sincroniza la matriz visual (mapaObjetos) con la matriz lógica (gCollisionMap).
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

// PERSISTENCIA DE ESTADO POR ISLA (MEMORIA)
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

  // Para islas 4 y 5, REGENERAR el collision map en lugar de restaurarlo
  // Esto asegura que se use la nueva lógica de detección de agua
  if (isla == 4 || isla == 5) {
    mapaReconstruirCollisionMap();
  } else {
    // Restaurar collision map normalmente para islas 1-3
    if (gCollisionMap) {
      for (int f = 0; f < GRID_SIZE; f++) {
        for (int c = 0; c < GRID_SIZE; c++) {
          *(*(gCollisionMap + f) + c) = gCollisionIsla[isla][f][c];
        }
      }
    }
  }

  // Restaurar vacas
  gNumVacas = gNumVacasIsla[isla];
  for (int i = 0; i < gNumVacas; i++) {
    gVacas[i] = gVacasIsla[isla][i];
  }
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

// DETECCIÓN DE AGUA Y RECONSTRUCCIÓN DE COLISIONES
// 
// Valores en collisionMap:
//   0 = Celda libre (transitable)
//   1 = Obstáculo (árbol o agua - impasable)
//   2 = Ocupada por unidad (temporalmente bloqueada)

// Helper para verificar color de agua segun tema
// Detecta si un color RGB corresponde a agua basándose en el tema actual.

static inline bool esColorAgua(BYTE r, BYTE g, BYTE b, int tema) {
    if (tema == 4) return (r < 20 && g < 80 && b > 100);
    if (tema == 5) return (r < 50 && b > 80 && b > g + 40);
    return ((b > r + 20 && b > g + 20 && b > 60) || (b > r && b > g && b > 100));
}

static void detectarAguaEnMapa(void) {
  if (!hMapaBmp || !gCollisionMap) return;

  BITMAP bm;
  if (!GetObject(hMapaBmp, sizeof(BITMAP), &bm)) return;

  BITMAPINFO bmi = {0};
  bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  bmi.bmiHeader.biWidth = bm.bmWidth;
  bmi.bmiHeader.biHeight = -bm.bmHeight;
  bmi.bmiHeader.biPlanes = 1;
  bmi.bmiHeader.biBitCount = 24;
  bmi.bmiHeader.biCompression = BI_RGB;

  int rowSize = ((bm.bmWidth * 3 + 3) & ~3);
  int imageSize = rowSize * bm.bmHeight;
  BYTE *pixelData = (BYTE *)malloc(imageSize);
  if (!pixelData) return;

  HDC hdcScreen = GetDC(NULL);
  if (!GetDIBits(hdcScreen, hMapaBmp, 0, bm.bmHeight, pixelData, &bmi, DIB_RGB_COLORS)) {
    ReleaseDC(NULL, hdcScreen); free(pixelData); return;
  }
  ReleaseDC(NULL, hdcScreen);

  int contadorAgua = 0, contadorTierra = 0;
  int tema = gIslaSeleccionadaActual;

  for (int f = 0; f < GRID_SIZE; f++) {
    int *fila = *(gCollisionMap + f);
    for (int c = 0; c < GRID_SIZE; c++) {
      if (*(fila + c) != 0) continue;

      int px = (c * TILE_SIZE) + 16, py = (f * TILE_SIZE) + 16;
      if (px < 0 || px >= bm.bmWidth || py < 0 || py >= bm.bmHeight) continue;

      int offset = py * rowSize + px * 3;
      // Pixel format: BGR
      if (esColorAgua(pixelData[offset+2], pixelData[offset+1], pixelData[offset], tema)) {
        *(fila + c) = 1;
        mapaObjetos[f][c] = SIMBOLO_AGUA;
        contadorAgua++;
      } else {
        contadorTierra++;
      }
    }
  }
  free(pixelData);

  if (contadorAgua == 0) { // Fallback borders
    for (int i = 0; i < GRID_SIZE; i++) {
        gCollisionMap[0][i] = 1; gCollisionMap[GRID_SIZE-1][i] = 1;
        gCollisionMap[i][0] = 1; gCollisionMap[i][GRID_SIZE-1] = 1;
    }
  }
}

// DETECCIÓN AUTOMÁTICA DE ORILLA PARA BARCO (192x192px)

// Encuentra una posición válida en la orilla del mapa donde:
// - El barco está EN EL AGUA (valor 1 = azul)
// - Hay tierra adyacente (para confirmar que es orilla, no mar abierto)
// - Hay espacio suficiente para el barco (6x6 celdas = 192px de agua)
// Retorna la posición en píxeles y la dirección hacia la tierra

void mapaDetectarOrilla(float *outX, float *outY, int *outDir) {
  int **col = mapaObtenerCollisionMap();
  if (!col) {
    *outX = 512.0f;
    *outY = 512.0f;
    *outDir = DIR_FRONT;
    return;
  }

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
            c >= GRID_SIZE - BARCO_CELDAS - 1) continue;

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
            //  El barco está en agua, orientado
            // hacia tierra
            *outX = (float)(c * TILE_SIZE);
            *outY = (float)(f * TILE_SIZE);
            *outDir = direcciones[d];

            return;
          }
        }
      }
    }
  }
  // FALLBACK: Si no se encuentra orilla ideal, buscar CUALQUIER celda de agua
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
        return;
      }
    }
  }

  // Último recurso: centro del mapa (probablemente error en el mapa)
  *outX = 1000.0f;
  *outY = 1000.0f;
  *outDir = DIR_FRONT;
}

// Helper para validar suelo para vegetacion/animales
static inline bool esSueloValido(BYTE r, BYTE g, BYTE b, int tema, bool esAgua) {
    if (esAgua) return false;
    if (tema == 4) return (r > 180 && g > 180 && b > 180) || (b > 150 && g > 150 && r > 100);
    if (tema == 5) return (r > g + 30 && r > b + 30) || (r < 100 && g < 100 && b < 100 && abs(r - g) < 30 && abs(r - b) < 30);
    return g > r && g > b && g > 70 && b < 80 && r < 100;
}

static void colocarObjetosAleatorios(int cantidad, char simbolo, HDC hdcMem, int tema) {
   int cnt = 0, tries = 0;
   while (cnt < cantidad && tries < 50000) {
       tries++;
       int f = rand() % GRID_SIZE, c = rand() % GRID_SIZE;
       if (mapaObjetos[f][c] != 0) continue;
       
       int px = (c * TILE_SIZE) + 16, py = (f * TILE_SIZE) + 16;
       COLORREF cRef = GetPixel(hdcMem, px, py);
       if (cRef == CLR_INVALID) continue;
       BYTE r = GetRValue(cRef), g = GetGValue(cRef), b = GetBValue(cRef);
       
       bool agua = esColorAgua(r, g, b, tema) || (mapaObjetos[f][c] == SIMBOLO_AGUA);
       
       if (esSueloValido(r, g, b, tema, agua)) {
           mapaObjetos[f][c] = simbolo;
           if (simbolo == SIMBOLO_ARBOL) gArbolesVida[f][c] = 3;
           else if (simbolo == SIMBOLO_VACA && gNumVacas < MAX_VACAS) {
               gVacas[gNumVacas++] = (Vaca){ (float)(c*TILE_SIZE), (float)(f*TILE_SIZE), rand()%4, rand()%120 };
           }
           cnt++;
       }
   }
}

void generarBosqueAutomatico() {
  if (!hMapaBmp) return;
  HDC hdcSc = GetDC(NULL); HDC hdcMem = CreateCompatibleDC(hdcSc);
  HBITMAP hOld = (HBITMAP)SelectObject(hdcMem, hMapaBmp);
  mapaLimpiarObjetosYColision();
  
  // Reservar edificios
  int edif[3][2] = {
      {(int)((1024-64)/32), (int)((1024-64)/32)}, 
      {(int)(450/32), (int)((1024-64)/32)}, 
      {(int)(1600/32), (int)((1024-64)/32)}
  };
  for(int i=0; i<3; i++) {
      for(int df=-2; df<=3; df++) {
          for(int dc=-2; dc<=3; dc++) {
              int nf = edif[i][0]+df, nc = edif[i][1]+dc;
              if (nf>=0 && nf<GRID_SIZE && nc>=0 && nc<GRID_SIZE) mapaObjetos[nf][nc] = SIMBOLO_EDIFICIO;
          }
      }
  }

  srand((unsigned int)time(NULL));
  colocarObjetosAleatorios(15, SIMBOLO_ARBOL, hdcMem, gIslaSeleccionadaActual);
  gNumVacas = 0;
  colocarObjetosAleatorios(10, SIMBOLO_VACA, hdcMem, gIslaSeleccionadaActual);

  mapaReconstruirCollisionMap();
  SelectObject(hdcMem, hOld); DeleteDC(hdcMem); ReleaseDC(NULL, hdcSc);
}

void cargarRecursosGraficos() {
  hMapaBmp = (HBITMAP)LoadImageA(NULL, gRutaMapaPrincipal, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
  if (mapaEsTemaFuego()) cargarSpritesFuego();
  else if (mapaEsTemaHielo()) cargarSpritesHielo();

  hMapaGlobalBmp = (HBITMAP)LoadImageA(NULL, "../assets/mapaDemo2.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);

  const char *arboles[][4] = {
      {ARBOL_FUEGO, ARBOL_FUEGO, ARBOL_FUEGO, ARBOL_FUEGO}, // Tema Fuego
      {ARBOL_HIELO, ARBOL_HIELO, ARBOL_HIELO, ARBOL_HIELO}, // Tema Hielo
      {ARBOL1, ARBOL2, ARBOL3, ARBOL4} // Default
  };
  int tIdx = mapaEsTemaFuego() ? 0 : (mapaEsTemaHielo() ? 1 : 2);
  for (int i=0; i<4; i++) hArboles[i] = loadBmp(arboles[tIdx][i], SPRITE_ARBOL, SPRITE_ARBOL);

  // Carga de unidades (Obreros, Caballeros, SinEscudo, Guerreros)
  HBITMAP *dest[] = {hObreroBmp, hCaballeroBmp, hCaballeroSinEscudoBmp, hGuerreroBmp};
  const char **src[] = {
      (const char*[]){obrero_front, obrero_back, obrero_left, obrero_right},
      (const char*[]){caballero_front, caballero_back, caballero_left, caballero_right},
      (const char*[]){caballeroSinEscudo_front, caballeroSinEscudo_back, caballeroSinEscudo_left, caballeroSinEscudo_right},
      (const char*[]){guerrero_front, guerrero_back, guerrero_left, guerrero_right}
  };
  for(int k=0; k<4; k++) for(int i=0; i<4; i++) dest[k][i] = loadBmp(src[k][i], 64, 64);

  // Combate
  struct { HBITMAP *arr; const char *files[2]; } combat[] = {
      { hCaballeroStand, {"../assets/caballero/caballero_war_stand_left.bmp", "../assets/caballero/caballero_war_stand_right.bmp"} },
      { hCaballeroDefense, {"../assets/caballero/caballero_defense_left.bmp", "../assets/caballero/caballero_defense_right.bmp"} },
      { hCaballeroDie, {"../assets/caballero/caballero_die_1_left.bmp", "../assets/caballero/caballero_die_1_right.bmp"} },
      { hCaballeroDie2, {"../assets/caballero/caballero_die_2_left.bmp", "../assets/caballero/caballero_die_2_right.bmp"} },
      { hCaballeroSEStand, {"../assets/caballero/caballero_war_NO_stand_left.bmp", "../assets/caballero/caballero_war_NO_stand_right.bmp"} },
      { hGuerreroStand, {"../assets/guerrero/guerrero_war_stand_left.bmp", "../assets/guerrero/guerrero_war_stand_right.bmp"} },
      { hGuerreroWalk, {"../assets/guerrero/guerrero_war_walk_left.bmp", "../assets/guerrero/guerrero_war_walk_right.bmp"} },
      { hGuerreroDie, {"../assets/guerrero/guerrero_war_die_1_left.bmp", "../assets/guerrero/guerrero_war_die_1_right.bmp"} },
      { hGuerreroDie2, {"../assets/guerrero/guerrero_war_die_2_left.bmp", "../assets/guerrero/guerrero_war_die_2_right.bmp"} }
  };
  for(int i=0; i<9; i++) for(int d=0; d<2; d++) combat[i].arr[d] = loadBmp(combat[i].files[d], 64, 64);

  // Ataques
  const char *atkFiles[][6] = {
      {"../assets/caballero/caballero_war_move_1_left.bmp", "../assets/caballero/caballero_war_move_2_left.bmp", "../assets/caballero/caballero_war_move_3_left.bmp", "../assets/caballero/caballero_war_move_1_right.bmp", "../assets/caballero/caballero_war_move_2_right.bmp", "../assets/caballero/caballero_war_move_3_right.bmp"},
      {"../assets/caballero/caballero_war_NO_move_1_left.bmp", "../assets/caballero/caballero_war_NO_move_2_left.bmp", "../assets/caballero/caballero_war_NO_move_3_left.bmp", "../assets/caballero/caballero_war_NO_move_1_right.bmp", "../assets/caballero/caballero_war_NO_move_2_right.bmp", "../assets/caballero/caballero_war_NO_move_3_right.bmp"},
      {"../assets/guerrero/guerrero_war_move_1_left.bmp", "../assets/guerrero/guerrero_war_move_2_left.bmp", NULL, "../assets/guerrero/guerrero_war_move_1_right.bmp", "../assets/guerrero/guerrero_war_move_2_right.bmp", NULL}
  };
  for(int i=0; i<6; i++) { hCaballeroAtk[i/3][i%3] = loadBmp(atkFiles[0][i], 64, 64); hCaballeroSEAtk[i/3][i%3] = loadBmp(atkFiles[1][i], 64, 64); hCaballeroAtk[2 + i/3][i%3] = loadBmp(atkFiles[1][i], 64, 64); }
  for(int i=0; i<4; i++) { int f = (i < 2) ? i : i+1; hGuerreroAtk[i/2][i%2] = loadBmp(atkFiles[2][f], 64, 64); }

  const char *barcos[] = {BARCO_F, BARCO_B, BARCO_L, BARCO_R};
  for(int i=0; i<4; i++) hBarcoBmp[i] = loadBmp(barcos[i], 192, 192);
  hBarcoDestruidoBmp = loadBmp(BARCO_DESTRUIDO, 192, 192);

  const char *vacas[][4] = {{VACA_FUEGO_F, VACA_FUEGO_B, VACA_FUEGO_L, VACA_FUEGO_R}, {VACA_HIELO_F, VACA_HIELO_B, VACA_HIELO_L, VACA_HIELO_R}, {VACA_F, VACA_B, VACA_L, VACA_R}};
  for(int i=0; i<4; i++) hVacaBmp[i] = loadBmp(vacas[tIdx][i], 64, 64);

  if (gGenerarRecursos) generarBosqueAutomatico(); else mapaReconstruirCollisionMap();
}

// ACTUALIZACIÓN DE VACAS (MOVIMIENTO AUTOMÁTICO)

// en una dirección aleatoria, validando colisiones antes de moverse.

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

  // SINCRONIZACIÓN FORZADA: Limpiar y remarcar posiciones de vacas

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
    ULONGLONG ahora = (ULONGLONG)GetTickCount();
    if (unidadCuerpoDesaparecido(u, ahora, &dtMuerte)) {
      return; // Ocultar sprite tras 5s del deceso
    }
  }
  bool usarSpritesFuego = esEnemigo && mapaEsTemaFuego();
  bool usarSpritesHielo = esEnemigo && mapaEsTemaHielo();
  if (u->tipo == TIPO_CABALLERO) {
    HBITMAP muerte1 = hCaballeroDie[dirIdx];
    HBITMAP muerte2 = hCaballeroDie2[dirIdx];
    HBITMAP standSprite = hCaballeroStand[dirIdx];
    HBITMAP (*atkSet)[3] = hCaballeroAtk;
    HBITMAP *dirsBase = hCaballeroBmp;

    if (usarSpritesFuego) {
      if (hCaballeroFuegoDie[dirIdx])
        muerte1 = hCaballeroFuegoDie[dirIdx];
      if (hCaballeroFuegoDie2[dirIdx])
        muerte2 = hCaballeroFuegoDie2[dirIdx];
      if (hCaballeroFuegoStand[dirIdx])
        standSprite = hCaballeroFuegoStand[dirIdx];
      atkSet = hCaballeroFuegoAtk;
      dirsBase = hCaballeroFuegoBmp;
    } else if (usarSpritesHielo) {
      if (hCaballeroHieloDie[dirIdx])
        muerte1 = hCaballeroHieloDie[dirIdx];
      if (hCaballeroHieloDie2[dirIdx])
        muerte2 = hCaballeroHieloDie2[dirIdx];
      if (hCaballeroHieloStand[dirIdx])
        standSprite = hCaballeroHieloStand[dirIdx];
      atkSet = hCaballeroHieloAtk;
      dirsBase = hCaballeroHieloBmp;
    }
    HBITMAP ataqueFrame = atkSet[dirIdx][atkFrame % 3];
    HBITMAP marcha = (dirsBase && dirsBase[u->dir]) ? dirsBase[u->dir]
                                                    : hCaballeroBmp[u->dir];

    if (muerto) {
      if (dtMuerte < 350 && muerte1) {
        sprite = muerte1;
      } else if (muerte2) {
        sprite = muerte2;
      } else {
        sprite = muerte1;
      }
    } else if (u->recibiendoAtaque && hCaballeroDefense[dirIdx]) {
      sprite = hCaballeroDefense[dirIdx];
    } else if (atacando && ataqueFrame) {
      sprite = ataqueFrame;
    } else if (!u->moviendose && permitirStand && standSprite) {
      sprite = standSprite;
    } else {
      sprite = marcha;
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
    HBITMAP muerte1 = hGuerreroDie[dirIdx];
    HBITMAP muerte2 = hGuerreroDie2[dirIdx];
    HBITMAP standSprite = hGuerreroStand[dirIdx];
    HBITMAP (*atkSetG)[2] = hGuerreroAtk;
    HBITMAP *dirsGuerr = hGuerreroBmp;

    if (usarSpritesFuego) {
      if (hGuerreroFuegoDie[dirIdx])
        muerte1 = hGuerreroFuegoDie[dirIdx];
      if (hGuerreroFuegoDie2[dirIdx])
        muerte2 = hGuerreroFuegoDie2[dirIdx];
      if (hGuerreroFuegoStand[dirIdx])
        standSprite = hGuerreroFuegoStand[dirIdx];
      atkSetG = hGuerreroFuegoAtk;
      dirsGuerr = hGuerreroFuegoBmp;
    } else if (usarSpritesHielo) {
      if (hGuerreroHieloDie[dirIdx])
        muerte1 = hGuerreroHieloDie[dirIdx];
      if (hGuerreroHieloDie2[dirIdx])
        muerte2 = hGuerreroHieloDie2[dirIdx];
      if (hGuerreroHieloStand[dirIdx])
        standSprite = hGuerreroHieloStand[dirIdx];
      atkSetG = hGuerreroHieloAtk;
      dirsGuerr = hGuerreroHieloBmp;
    }
    HBITMAP ataqueFrame = atkSetG[dirIdx][atkFrame % 2];
    HBITMAP movimiento = NULL;
    if (!gEsIslaPrincipalActual) {
      if (usarSpritesFuego && hGuerreroFuegoWalk[dirIdx]) {
        movimiento = hGuerreroFuegoWalk[dirIdx];
      } else if (usarSpritesHielo && hGuerreroHieloWalk[dirIdx]) {
        movimiento = hGuerreroHieloWalk[dirIdx];
      } else {
        movimiento = hGuerreroWalk[dirIdx];
      }
    }
    HBITMAP baseDir = (dirsGuerr && dirsGuerr[u->dir]) ? dirsGuerr[u->dir]
                                                       : hGuerreroBmp[u->dir];

    if (muerto) {
      if (dtMuerte < 350 && muerte1) {
        sprite = muerte1;
      } else if (muerte2) {
        sprite = muerte2;
      } else {
        sprite = muerte1;
      }
    } else if (atacando && ataqueFrame) {
      sprite = ataqueFrame;
    } else if (u->moviendose && movimiento) {
      sprite = movimiento;
    } else if (!u->moviendose && permitirStand && standSprite) {
      sprite = standSprite;
    } else {
      sprite = (!gEsIslaPrincipalActual && movimiento) ? movimiento : baseDir;
    }
  }

  if (sprite) {
    SelectObject(hdcSprites, sprite);
    TransparentBlt(hdcBuffer, pantX, pantY, tam, tam, hdcSprites, 0, 0, 64, 64,
                   RGB(255, 255, 255));
  }
}

// En lugar de dibujar todos los árboles y luego todos los obreros,
// dibujamos el mapa fila por fila (de arriba hacia abajo).
// Por cada fila Y:
//   1. Dibujamos los árboles cuya fila coincide
//   2. Dibujamos los obreros cuya posición Y coincide con esa fila
//
// Esto crea un efecto de profundidad natural: los objetos "más abajo"
// en la pantalla se dibujan después (encima) de los objetos "más arriba).


// DIBUJADO DE BARRAS DE VIDA

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
    ULONGLONG ahora = (ULONGLONG)GetTickCount();
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
    // Rango Y de la fila actual
    float yMin = (float)(f * TILE_SIZE);
    float yMax = (float)((f + 1) * TILE_SIZE);

    // --- 1. OBREROS ---
    Unidad *baseObreros = pJugador->obreros;
    for (Unidad *o = baseObreros; o < baseObreros + MAX_OBREROS; o++) {
      if (o->y + TILE_SIZE >= yMin && o->y + TILE_SIZE < yMax) {
        int px = (int)((o->x - cam.x) * cam.zoom), py = (int)((o->y - cam.y) * cam.zoom), sz = (int)(64 * cam.zoom);
        if (px + sz > 0 && px < anchoP && py + sz > 0 && py < altoP) {
           SelectObject(hdcSprites, hObreroBmp[o->dir]);
           TransparentBlt(hdcBuffer, px, py, sz, sz, hdcSprites, 0, 0, 64, 64, RGB(255, 255, 255));
           if (unidadBarraVisible(o)) dibujarBarraVida(hdcBuffer, px, py, o->vida, o->vidaMax, cam.zoom);
           if (o->seleccionado && unidadBarraVisible(o)) {
               HBRUSH nb = (HBRUSH)GetStockObject(NULL_BRUSH); HPEN p = CreatePen(PS_SOLID, 2, RGB(0, 255, 0));
               SelectObject(hdcBuffer, nb); SelectObject(hdcBuffer, p);
               Ellipse(hdcBuffer, px, py + sz - 10, px + sz, py + sz + 5);
               DeleteObject(p);
           }
        }
      }
    }

    // --- 2. ÁRBOLES ---
    for (int c = 0; c < GRID_SIZE; c++) {
      if (*(*(ptrMatriz + f) + c) == SIMBOLO_ARBOL) {
        int px = (int)((c * TILE_SIZE - cam.x) * cam.zoom), py = (int)((f * TILE_SIZE - cam.y) * cam.zoom), sz = (int)(SPRITE_ARBOL * cam.zoom);
        if (px + sz > 0 && px < anchoP && py + sz > 0 && py < altoP) {
          SelectObject(hdcSprites, hArboles[0]);
          TransparentBlt(hdcBuffer, px, py, sz, sz, hdcSprites, 0, 0, SPRITE_ARBOL, SPRITE_ARBOL, RGB(255, 255, 255));
          int v = gArbolesVida[f][c];
          if (v > 0) dibujarBarraVida(hdcBuffer, px + (int)(32 * cam.zoom), py - 10, (v==3?3:v), 3, cam.zoom);
        }
      }
    }

    // --- 3. UNIDADES DE COMBATE (ALIADOS) ---
    struct { Unidad *base; int cap; int type; int offsetIdx; } groups[] = {
        {pJugador->caballeros, MAX_CABALLEROS, TIPO_CABALLERO, 0},
        {pJugador->caballerosSinEscudo, MAX_CABALLEROS_SIN_ESCUDO, TIPO_CABALLERO_SIN_ESCUDO, 4},
        {pJugador->guerreros, MAX_GUERREROS, TIPO_GUERRERO, 8}
    };

    for(int g=0; g<3; g++) {
        Unidad *u = groups[g].base, *end = u + groups[g].cap;
        for(; u < end; u++) {
            if (u->y + TILE_SIZE >= yMin && u->y + TILE_SIZE < yMax) {
                int px = (int)((u->x - cam.x) * cam.zoom), py = (int)((u->y - cam.y) * cam.zoom), sz = (int)(64 * cam.zoom);
                if (px + sz > 0 && px < anchoP && py + sz > 0 && py < altoP) {
                    int idx = groups[g].offsetIdx + (int)(u - groups[g].base);
                    bool atk = (idx >= 0 && idx < 12) ? ataqueAliados[idx] : false;
                    bool stand = (g==2) ? permitirStandGuerrero : permitirStand;
                    
                    if (groups[g].type == TIPO_CABALLERO_SIN_ESCUDO) SelectObject(hdcSprites, hCaballeroSinEscudoBmp[u->dir]);
                    dibujarUnidadCombat(hdcBuffer, hdcSprites, u, cam, anchoP, altoP, stand, false, atk, frameAtaque);
                    
                    if (unidadBarraVisible(u)) dibujarBarraVida(hdcBuffer, px, py, u->vida, u->vidaMax, cam.zoom);
                    if (u->seleccionado && unidadBarraVisible(u)) {
                         HBRUSH nb = (HBRUSH)GetStockObject(NULL_BRUSH); HPEN p = CreatePen(PS_SOLID, 2, RGB(0, 255, 0));
                         SelectObject(hdcBuffer, nb); SelectObject(hdcBuffer, p);
                         Ellipse(hdcBuffer, px, py + sz - 10, px + sz, py + sz + 5); DeleteObject(p);
                    }
                }
            }
        }
    }

    // --- 4. ENEMIGOS ---
    if (enemigosActivos && enemigosActivosCount > 0) {
         for (int i = 0; i < enemigosActivosCount; i++) {
             Unidad *e = &enemigosActivos[i];
             if (e->y + TILE_SIZE >= yMin && e->y + TILE_SIZE < yMax) {
                 int px = (int)((e->x - cam.x) * cam.zoom), py = (int)((e->y - cam.y) * cam.zoom), sz = (int)(64 * cam.zoom);
                 if (px + sz > 0 && px < anchoP && py + sz > 0 && py < altoP) {
                     dibujarUnidadCombat(hdcBuffer, hdcSprites, e, cam, anchoP, altoP, (e->tipo == TIPO_GUERRERO ? permitirStandGuerrero : permitirStand), true, (i<8?ataqueEnemigos[i]:false), frameAtaque);
                     if (unidadBarraVisible(e)) {
                         dibujarBarraVidaColor(hdcBuffer, px, py, e->vida, e->vidaMax, cam.zoom, RGB(200, 60, 60));
                         HBRUSH nb = (HBRUSH)GetStockObject(NULL_BRUSH); HPEN p = CreatePen(PS_SOLID, 2, RGB(200, 60, 60));
                         SelectObject(hdcBuffer, nb); SelectObject(hdcBuffer, p);
                         Ellipse(hdcBuffer, px, py + sz - 10, px + sz, py + sz + 5); DeleteObject(p);
                     }
                 }
             }
         }
    }

    // --- 5. VACAS ---
    Vaca *baseVacas = gVacas;
    for (Vaca *v = baseVacas; v < baseVacas + gNumVacas; v++) {
        if (v->y + TILE_SIZE >= yMin && v->y + TILE_SIZE < yMax) {
            int px = (int)((v->x - cam.x) * cam.zoom), py = (int)((v->y - cam.y) * cam.zoom), sz = (int)(64 * cam.zoom);
            if (px + sz > 0 && px < anchoP && py + sz > 0 && py < altoP) {
                SelectObject(hdcSprites, hVacaBmp[v->dir]);
                TransparentBlt(hdcBuffer, px, py, sz, sz, hdcSprites, 0, 0, 64, 64, RGB(255, 255, 255));
            }
        }
    }

    // --- 6. BARCO ---
    if (pJugador->barco.activo) {
        Barco *b = &pJugador->barco;
        if (b->y + 192.0f >= yMin && b->y + 192.0f < yMax) {
             int px = (int)((b->x - cam.x) * cam.zoom), py = (int)((b->y - cam.y) * cam.zoom), sz = (int)(192 * cam.zoom);
             if (px + sz > 0 && px < anchoP && py + sz > 0 && py < altoP) {
                SelectObject(hdcSprites, b->construido ? (hBarcoBmp[b->dir] ? hBarcoBmp[b->dir] : hBarcoBmp[0]) : hBarcoDestruidoBmp);
                TransparentBlt(hdcBuffer, px, py, sz, sz, hdcSprites, 0, 0, 192, 192, RGB(255, 255, 255));
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

  // RESALTAR CELDA BAJO EL CURSOR (DEBUG/VISUAL AID)
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

// FUNCIONES DE GESTIÓN DE OBJETOS DEL MAPA (ACCESO EXTERNO)

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
          break; // Asumimos una vaca por celda
        }
      }
    }

    // 3. Actualizar mapa de colisiones (el árbol ya no bloquea)
    mapaReconstruirCollisionMap();
  }
}

// SERIALIZACIÓN DEL MAPA

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

// FUNCIONES REQUERIDAS POR ESPECIFICACI\u00d3N ACAD\u00c9MICA

void inicializarMapa(char mapa[GRID_SIZE][GRID_SIZE]) {
  char (*ptrFila)[GRID_SIZE] = mapa;
  printf("[DEBUG] Inicializando mapa logico %dx%d...\n", GRID_SIZE, GRID_SIZE);
  for (int f = 0; f < GRID_SIZE; f++) {
    char *ptrCelda = *(ptrFila + f);
    for (int c = 0; c < GRID_SIZE; c++) {
      *(ptrCelda + c) = SIMBOLO_VACIO;
    }
  }
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

// FUNCIONES DE SINCRONIZACIÓN: mapaObjetos <-> Estado del Juego

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
  // DEBUG: Contador estático para limitar mensajes
  static int debugCount = 0;
  
  if (fila < 0 || col < 0 || fila >= GRID_SIZE || col >= GRID_SIZE)
    return false;

  collisionMapAllocIfNeeded();

  int valor = 0;
  if (gCollisionMap) {
    valor = *(*(gCollisionMap + fila) + col);
  }

  char simb = mapaObjetos[fila][col];

  // Bloquear cualquier cosa marcada como colisión u océano conocido
  if (valor != 0) {
    if (debugCount < 30 && gIslaSeleccionadaActual == 4) {

      fflush(stdout);
      debugCount++;
    }
    return false;
  }
  if (simb == SIMBOLO_AGUA) {
    if (debugCount < 30 && gIslaSeleccionadaActual == 4) {

      fflush(stdout);
      debugCount++;
    }
    return false;
  }

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
          
          // DETECCIÓN DE AGUA SEGÚN TIPO DE ISLA
          // Solo el AZUL OSCURO oceánico bloquea el paso
          bool agua = false;
          
          if (gIslaSeleccionadaActual == 4) {
            // ISLA DE HIELO: Solo azul oceánico MUY oscuro es agua
            // El agua oceánica del mapa tiene aprox RGB(4,59,124)
            // Los blancos, cianes, grises claros son suelo transitable
            // Criterio MUY estricto: R muy bajo (<20), G bajo (<80), B alto (>100)
            agua = (r < 20 && g < 80 && b > 100);
            
            // DEBUG: Mostrar por qué se bloquea o no
            if (debugCount < 30) {
              fflush(stdout);
              debugCount++;
            }
          } else if (gIslaSeleccionadaActual == 5) {
            // ISLA DE FUEGO: Solo azul oceánico es agua
            // Los rojos, naranjas, grises oscuros son suelo transitable
            agua = (r < 50 && b > 80 && b > g + 40);
          } else {
            // ISLAS CLÁSICAS (1, 2, 3): Lógica original
            agua = (b > r + 20 && b > g + 20 && b > 60) ||
                   (b > r && b > g && b > 100);
          }
          
          if (agua) {
            if (debugCount < 30 && gIslaSeleccionadaActual == 4) {
              fflush(stdout);
              debugCount++;
            }
            return false;
          }
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

  if (!hMapaBmp)
      return;


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