// Motor básico de batallas en islas
#include "batallas.h"
#include "../mapa/mapa.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define BATTLE_MAX_UNITS     14   // Máximo por bando
#define BATTLE_TILE          80   // Tamaño lógico de celda en la escena de batalla
#define BATTLE_COLS          10
#define BATTLE_ROWS          8
#define BATTLE_STEP_PIXELS   36.0f
#define BATTLE_RANGE_PIXELS  60.0f
#define BATTLE_ANIM_STEP_SEC 0.25f

// Rutas locales de sprites (reutilizamos los mismos BMP con carga propia)
static const char *kCaballeroRutas[4] = {
	"..\\assets\\caballero\\caballero_front.bmp",
	"..\\assets\\caballero\\caballero_back.bmp"
};

static const char *kCaballeroRutasAlt[4] = {
	"assets/caballero/caballero_front.bmp",
	"assets/caballero/caballero_back.bmp"
};

static const char *kGuerreroRutas[4] = {
    "..\\assets\\guerrero\\guerrero_front.bmp",
    "..\\assets\\guerrero\\guerrero_back.bmp",
    "..\\assets\\guerrero\\guerrero_left.bmp",
    "..\\assets\\guerrero\\guerrero_right.bmp"};

static const char *kGuerreroRutasAlt[4] = {
    "assets/guerrero/guerrero_front.bmp", "assets/guerrero/guerrero_back.bmp",
    "assets/guerrero/guerrero_left.bmp", "assets/guerrero/guerrero_right.bmp"};

static HBITMAP hCaballeroBmp[4] = {NULL};
static HBITMAP hGuerreroBmp[4] = {NULL};
static HBITMAP hFondoIsla = NULL; // BMP de la isla destino para el fondo

typedef enum { ANIM_IDLE, ANIM_MOVE, ANIM_ATTACK, ANIM_DEATH } AnimState;

typedef struct {
  HBITMAP idle[2];      // 0 left, 1 right
  HBITMAP walk[2];      // loop mientras se desplaza
  HBITMAP attack[2][3]; // 3 frames
  HBITMAP death[2][2];  // 2 frames
} SpriteSet;

typedef struct {
  Unidad *ref; // Unidad real (para sincronizar bajas)
  bool esEnemigo;
  bool vivo;
  float x, y; // Coordenadas de escena de batalla (px)
  float vel;  // Avance por tick (px)
  int hp;
  int hpMax;
  int danio;
  int defensa;
  int crit;
  Direccion dir;
  int animFrame;
  AnimState animState;
  float animTimer;
  int animFrameMax;
  TipoUnidad tipo;
} BattleUnit;

typedef struct {
  bool activa;
  int islaDestino;
  float tickAcumulado;
  float animAccum;
  BatallaResultado resultadoPendiente;
  struct Jugador *jugador; // Referencia para aplicar bajas
  BattleUnit aliados[BATTLE_MAX_UNITS];
  BattleUnit enemigos[BATTLE_MAX_UNITS];
  int numAliados;
  int numEnemigos;
} BattleState;

static BattleState gBatalla = {0};
static DWORD gInicioMs = 0;

// ---------------------------------------------------------------------------
// Utilidades internas
// ---------------------------------------------------------------------------
static int clampInt(int v, int lo, int hi) {
  return (v < lo) ? lo : (v > hi) ? hi : v;
}
static int dirIndex(Direccion d) { return (d == DIR_RIGHT) ? 1 : 0; }

static HBITMAP cargarBmp(const char *ruta1, const char *ruta2, int w, int h) {
  HBITMAP bmp = (HBITMAP)LoadImageA(NULL, ruta1, IMAGE_BITMAP, w, h,
                                    LR_LOADFROMFILE | LR_CREATEDIBSECTION);
  if (!bmp && ruta2) {
    bmp = (HBITMAP)LoadImageA(NULL, ruta2, IMAGE_BITMAP, w, h,
                              LR_LOADFROMFILE | LR_CREATEDIBSECTION);
  }
  return bmp;
}

static void asegurarSprites(void) {
  for (int i = 0; i < 4; i++) {
    if (!hCaballeroBmp[i]) {
      hCaballeroBmp[i] =
          cargarBmp(kCaballeroRutas[i], kCaballeroRutasAlt[i], 64, 64);
    }
    if (!hGuerreroBmp[i]) {
      hGuerreroBmp[i] =
          cargarBmp(kGuerreroRutas[i], kGuerreroRutasAlt[i], 64, 64);
    }
  }
}

// --------------------------------------------------------------------------------
// Carga detallada de sprites por estado/dirección
// --------------------------------------------------------------------------------
static SpriteSet gSprGuerrero = {0};
static SpriteSet gSprCaballero = {0};

static void cargarSpriteDir(HBITMAP *dst, const char *pathL,
                            const char *pathR) {
  if (!dst[0])
    dst[0] = cargarBmp(pathL, NULL, 64, 64);
  if (!dst[1])
    dst[1] = cargarBmp(pathR, NULL, 64, 64);
}

static void cargarGuerreroAnim(void) {
	// Idle
	const char *idleL1 = "..\\assets\\guerrero\\guerrero_war_stand_left.bmp";
	const char *idleR1 = "..\\assets\\guerrero\\guerrero_war_stand_right.bmp";
	const char *idleL2 = "assets/guerrero/guerrero_war_stand_left.bmp";
	const char *idleR2 = "assets/guerrero/guerrero_war_stand_right.bmp";
	if (!gSprGuerrero.idle[0]) gSprGuerrero.idle[0] = cargarBmp(idleL1, idleL2, 64, 64);
	if (!gSprGuerrero.idle[1]) gSprGuerrero.idle[1] = cargarBmp(idleR1, idleR2, 64, 64);

	// Walk
	const char *walkL1 = "..\\assets\\guerrero\\guerrero_war_walk_left.bmp";
	const char *walkR1 = "..\\assets\\guerrero\\guerrero_war_walk_right.bmp";
	const char *walkL2 = "assets/guerrero/guerrero_war_walk_left.bmp";
	const char *walkR2 = "assets/guerrero/guerrero_war_walk_right.bmp";
	if (!gSprGuerrero.walk[0]) gSprGuerrero.walk[0] = cargarBmp(walkL1, walkL2, 64, 64);
	if (!gSprGuerrero.walk[1]) gSprGuerrero.walk[1] = cargarBmp(walkR1, walkR2, 64, 64);

	// Attack (2 frames)
	const char *atkL1[2] = {
		"..\\assets\\guerrero\\guerrero_war_move_1_left.bmp",
		"..\\assets\\guerrero\\guerrero_war_move_2_left.bmp"};
	const char *atkR1[2] = {
		"..\\assets\\guerrero\\guerrero_war_move_1_right.bmp",
		"..\\assets\\guerrero\\guerrero_war_move_2_right.bmp"};
	const char *atkL2[2] = {
		"assets/guerrero/guerrero_war_move_1_left.bmp",
		"assets/guerrero/guerrero_war_move_2_left.bmp"};
	const char *atkR2[2] = {
		"assets/guerrero/guerrero_war_move_1_right.bmp",
		"assets/guerrero/guerrero_war_move_2_right.bmp"};
	for (int i = 0; i < 2; i++) {
		if (!gSprGuerrero.attack[0][i]) gSprGuerrero.attack[0][i] = cargarBmp(atkL1[i], atkL2[i], 64, 64);
		if (!gSprGuerrero.attack[1][i]) gSprGuerrero.attack[1][i] = cargarBmp(atkR1[i], atkR2[i], 64, 64);
	}

	// Death (2 frames)
	const char *dieL1[2] = {
		"..\\assets\\guerrero\\guerrero_war_die_1_left.bmp",
		"..\\assets\\guerrero\\guerrero_war_die_2_left.bmp"};
	const char *dieR1[2] = {
		"..\\assets\\guerrero\\guerrero_war_die_1_right.bmp",
		"..\\assets\\guerrero\\guerrero_war_die_2_right.bmp"};
	const char *dieL2[2] = {
		"assets/guerrero/guerrero_war_die_1_left.bmp",
		"assets/guerrero/guerrero_war_die_2_left.bmp"};
	const char *dieR2[2] = {
		"assets/guerrero/guerrero_war_die_1_right.bmp",
		"assets/guerrero/guerrero_war_die_2_right.bmp"};
	for (int i = 0; i < 2; i++) {
		if (!gSprGuerrero.death[0][i]) gSprGuerrero.death[0][i] = cargarBmp(dieL1[i], dieL2[i], 64, 64);
		if (!gSprGuerrero.death[1][i]) gSprGuerrero.death[1][i] = cargarBmp(dieR1[i], dieR2[i], 64, 64);
	}
}

static void cargarCaballeroAnim(void) {
	// Idle
	const char *idleL1 = "..\\assets\\caballero\\caballero_war_stand_left.bmp";
	const char *idleR1 = "..\\assets\\caballero\\caballero_war_stand_right.bmp";
	const char *idleL2 = "assets/caballero/caballero_war_stand_left.bmp";
	const char *idleR2 = "assets/caballero/caballero_war_stand_right.bmp";
	if (!gSprCaballero.idle[0]) gSprCaballero.idle[0] = cargarBmp(idleL1, idleL2, 64, 64);
	if (!gSprCaballero.idle[1]) gSprCaballero.idle[1] = cargarBmp(idleR1, idleR2, 64, 64);

	// Attack (3 frames)
	const char *atkL1[3] = {
		"..\\assets\\caballero\\caballero_war_move_1_left.bmp",
		"..\\assets\\caballero\\caballero_war_move_2_left.bmp",
		"..\\assets\\caballero\\caballero_war_move_3_left.bmp"};
	const char *atkR1[3] = {
		"..\\assets\\caballero\\caballero_war_move_1_right.bmp",
		"..\\assets\\caballero\\caballero_war_move_2_right.bmp",
		"..\\assets\\caballero\\caballero_war_move_3_right.bmp"};
	const char *atkL2[3] = {
		"assets/caballero/caballero_war_move_1_left.bmp",
		"assets/caballero/caballero_war_move_2_left.bmp",
		"assets/caballero/caballero_war_move_3_left.bmp"};
	const char *atkR2[3] = {
		"assets/caballero/caballero_war_move_1_right.bmp",
		"assets/caballero/caballero_war_move_2_right.bmp",
		"assets/caballero/caballero_war_move_3_right.bmp"};
	for (int i = 0; i < 3; i++) {
		if (!gSprCaballero.attack[0][i]) gSprCaballero.attack[0][i] = cargarBmp(atkL1[i], atkL2[i], 64, 64);
		if (!gSprCaballero.attack[1][i]) gSprCaballero.attack[1][i] = cargarBmp(atkR1[i], atkR2[i], 64, 64);
	}

	// Death (2 frames)
	const char *dieL1[2] = {
		"..\\assets\\caballero\\caballero_die_1_left.bmp",
		"..\\assets\\caballero\\caballero_die_2_left.bmp"};
	const char *dieR1[2] = {
		"..\\assets\\caballero\\caballero_die_1_right.bmp",
		"..\\assets\\caballero\\caballero_die_2_right.bmp"};
	const char *dieL2[2] = {
		"assets/caballero/caballero_die_1_left.bmp",
		"assets/caballero/caballero_die_2_left.bmp"};
	const char *dieR2[2] = {
		"assets/caballero/caballero_die_1_right.bmp",
		"assets/caballero/caballero_die_2_right.bmp"};
	for (int i = 0; i < 2; i++) {
		if (!gSprCaballero.death[0][i]) gSprCaballero.death[0][i] = cargarBmp(dieL1[i], dieL2[i], 64, 64);
		if (!gSprCaballero.death[1][i]) gSprCaballero.death[1][i] = cargarBmp(dieR1[i], dieR2[i], 64, 64);
	}
}

static void cargarFondoIsla(int islaDestino) {
  if (hFondoIsla)
    return;
  char ruta1[MAX_PATH];
  char ruta2[MAX_PATH];
  char ruta3[MAX_PATH];
  snprintf(ruta1, sizeof(ruta1), "..\\assets\\islas\\isla%d.bmp", islaDestino);
  snprintf(ruta2, sizeof(ruta2), "assets/islas/isla%d.bmp", islaDestino);
  snprintf(ruta3, sizeof(ruta3), ".\\assets\\islas\\isla%d.bmp", islaDestino);
  hFondoIsla = cargarBmp(ruta1, ruta2, 0, 0);
  if (!hFondoIsla)
    hFondoIsla = cargarBmp(ruta3, NULL, 0, 0);
  printf("[BATALLA] Carga fondo isla %d: %s %s %s -> %s\n", islaDestino, ruta1,
         ruta2, ruta3, hFondoIsla ? "OK" : "FAIL");
}

static void statsPorTipo(TipoUnidad t, int *hp, int *danio, int *def, int *crit) {
	switch (t) {
	case TIPO_CABALLERO:
		*hp = 150; *danio = 35; *def = 25; *crit = 15; return;
	case TIPO_CABALLERO_SIN_ESCUDO:
		// Valores alineados con recursos.h
		*hp = 100; *danio = 35; *def = 5; *crit = 20; return;
	case TIPO_GUERRERO:
		*hp = 120; *danio = 30; *def = 20; *crit = 10; return;
	case TIPO_OBRERO: // Obrero no es tropa de combate, pero lo soportamos con stats bajos
	default:
		*hp = 60; *danio = 8; *def = 2; *crit = 0; return;
	}
}

static SpriteSet *spriteSetPorTipo(TipoUnidad t) {
	switch (t) {
	case TIPO_GUERRERO:
		return &gSprGuerrero;
	case TIPO_CABALLERO:
		return &gSprCaballero;
	case TIPO_CABALLERO_SIN_ESCUDO:
		return &gSprCaballero;
	default:
		return &gSprCaballero; // caballero y variantes comparten animaciones
	}
}

static void initUnit(BattleUnit *u, Unidad *ref, bool esEnemigo,
										 int hp, int dmg, int def, int crit) {
	memset(u, 0, sizeof(BattleUnit));
	u->ref = ref;
	u->esEnemigo = esEnemigo;
	u->vivo = true;
	u->vel = BATTLE_STEP_PIXELS;
	u->hp = hp;
	u->hpMax = hp;
	u->danio = dmg;
	u->defensa = def;
	u->crit = crit;
	u->dir = esEnemigo ? DIR_LEFT : DIR_RIGHT;
	u->animFrame = 0;
	u->animState = ANIM_IDLE;
	u->animTimer = 0.0f;
	u->animFrameMax = 1;
	u->tipo = ref ? ref->tipo : TIPO_GUERRERO;
}

static float dist2(const BattleUnit *a, const BattleUnit *b) {
  float dx = a->x - b->x;
  float dy = a->y - b->y;
  return dx * dx + dy * dy;
}

static int cmpBattleUnitPtr(const void *lhs, const void *rhs) {
	const BattleUnit *a = *(const BattleUnit *const *)lhs;
	const BattleUnit *b = *(const BattleUnit *const *)rhs;
	if (a->y < b->y) return -1;
	if (a->y > b->y) return 1;
	return (a->x < b->x) ? -1 : 1;
}
static void marcarBajasAliadas(void) {
  if (!gBatalla.jugador)
    return;
  Barco *barco = &gBatalla.jugador->barco;
  for (int i = 0; i < gBatalla.numAliados; i++) {
    BattleUnit *u = &gBatalla.aliados[i];
    if (!u->ref)
      continue;
    if (!u->vivo) {
      u->ref->x = -1000.0f;
      u->ref->y = -1000.0f;
      u->ref->moviendose = false;
      u->ref->seleccionado = false;
      u->ref->celdaFila = -1;
      u->ref->celdaCol = -1;
    }
  }

  // Compactar tripulación del barco eliminando punteros caídos
  int writeIdx = 0;
  for (int i = 0; i < barco->numTropas; i++) {
    Unidad *ptr = barco->tropas[i];
    bool muerta = false;
    for (int j = 0; j < gBatalla.numAliados; j++) {
      if (gBatalla.aliados[j].ref == ptr && !gBatalla.aliados[j].vivo) {
        muerta = true;
        break;
      }
    }
    if (!muerta && ptr) {
      barco->tropas[writeIdx++] = ptr;
    }
  }
  for (int k = writeIdx; k < 6; k++)
    barco->tropas[k] = NULL;
  barco->numTropas = writeIdx;
}

static int calcularPresupuesto(int islasConquistadas) {
  DWORD ahora = GetTickCount();
  int minutos = 0;
  if (gInicioMs > 0 && ahora > gInicioMs) {
    minutos = (int)((ahora - gInicioMs) / 60000);
  }

  int presupuesto = 4;              // base sencilla
  presupuesto += minutos / 5;       // +1 cada 5 minutos
  presupuesto += islasConquistadas; // +1 por isla conquistada
  return clampInt(presupuesto, 3, 12);
}

static void poblarEnemigos(int islasConquistadas) {
  gBatalla.numEnemigos = 0;
  int puntos = calcularPresupuesto(islasConquistadas);

  // Caballero cuesta 14/10, guerrero 10/10 (aprox 1.4 vs 1.0)
  while (puntos >= 14 && gBatalla.numEnemigos < BATTLE_MAX_UNITS) {
    BattleUnit *u = &gBatalla.enemigos[gBatalla.numEnemigos++];
    int hp, dmg, def, crit;
    statsPorTipo(TIPO_CABALLERO, &hp, &dmg, &def, &crit);
    initUnit(u, NULL, true, hp, dmg, def, crit);
    u->tipo = TIPO_CABALLERO;
    puntos -= 14;
  }

  while (puntos >= 10 && gBatalla.numEnemigos < BATTLE_MAX_UNITS) {
    BattleUnit *u = &gBatalla.enemigos[gBatalla.numEnemigos++];
    int hp, dmg, def, crit;
    statsPorTipo(TIPO_GUERRERO, &hp, &dmg, &def, &crit);
    initUnit(u, NULL, true, hp, dmg, def, crit);
    u->tipo = TIPO_GUERRERO;
    puntos -= 10;
  }

  if (gBatalla.numEnemigos < 3) {
    int falta = 3 - gBatalla.numEnemigos;
    for (int i = 0; i < falta && gBatalla.numEnemigos < BATTLE_MAX_UNITS; i++) {
      BattleUnit *u = &gBatalla.enemigos[gBatalla.numEnemigos++];
      int hp, dmg, def, crit;
      statsPorTipo(TIPO_GUERRERO, &hp, &dmg, &def, &crit);
      initUnit(u, NULL, true, hp, dmg, def, crit);
      u->tipo = TIPO_GUERRERO;
    }
  }
}

static void posicionarUnidades(void) {
  // Aliados a la izquierda, enemigos a la derecha, con separación vertical
  // amplia
  const float offsetAliadosX = 200.0f;
  const float offsetEnemigosX = 420.0f;
  const float offsetY = 200.0f;
  const int maxPorColumna = 6;

  for (int i = 0; i < gBatalla.numAliados; i++) {
    int col = i / maxPorColumna;
    int row = i % maxPorColumna;
    gBatalla.aliados[i].x = offsetAliadosX + col * BATTLE_TILE;
    gBatalla.aliados[i].y = offsetY + row * (BATTLE_TILE + 12);
    gBatalla.aliados[i].dir = DIR_RIGHT;
  }

  for (int i = 0; i < gBatalla.numEnemigos; i++) {
    int col = i / maxPorColumna;
    int row = i % maxPorColumna;
    gBatalla.enemigos[i].x = offsetEnemigosX + col * BATTLE_TILE;
    gBatalla.enemigos[i].y = offsetY + row * (BATTLE_TILE + 12);
    gBatalla.enemigos[i].dir = DIR_LEFT;
  }
}

static void resolverTick(void) {
	// Seleccionar objetivos y actuar
	for (int a = 0; a < gBatalla.numAliados; a++) {
		BattleUnit *u = &gBatalla.aliados[a];
		if (!u->vivo) continue;
		AnimState prevAnim = u->animState;
		u->animState = ANIM_IDLE;
		u->animFrameMax = 1;

    // Buscar enemigo más cercano
    BattleUnit *target = NULL;
    float mejorD2 = 0.0f;
    for (int e = 0; e < gBatalla.numEnemigos; e++) {
      BattleUnit *v = &gBatalla.enemigos[e];
      if (!v->vivo)
        continue;
      float d2 = dist2(u, v);
      if (!target || d2 < mejorD2) {
        target = v;
        mejorD2 = d2;
      }
    }
    if (!target)
      break;

		float dx = target->x - u->x;
		float dy = target->y - u->y;
		float dist = sqrtf(dx * dx + dy * dy);
		if (dist > BATTLE_RANGE_PIXELS) {
			float inv = 1.0f / (dist + 0.0001f);
			u->x += dx * inv * u->vel;
			u->y += dy * inv * u->vel;
			u->dir = (dx >= 0) ? DIR_RIGHT : DIR_LEFT;
			u->animState = ANIM_MOVE;
			u->animFrameMax = 1; // se ajusta en render con walk (loop en dibujar)
		} else {
			u->animState = ANIM_ATTACK;
			if (prevAnim != ANIM_ATTACK) u->animFrame = 0; // solo reiniciar al entrar
			u->animFrameMax = 3;
			int raw = u->danio;
			if ((rand() % 100) < u->crit) raw *= 2;
			int real = raw - target->defensa;
			if (real < 1) real = 1;
			target->hp -= real;
			if (target->hp <= 0) { target->hp = 0; target->vivo = false; target->animState = ANIM_DEATH; target->animFrame = 0; target->animFrameMax = 2; }
		}
		if (u->animState == ANIM_MOVE) {
			u->animFrameMax = 1; // walk usa frame único, el loop se hace en dibujar
		}
	}

	for (int e = 0; e < gBatalla.numEnemigos; e++) {
		BattleUnit *u = &gBatalla.enemigos[e];
		if (!u->vivo) continue;
		AnimState prevAnim = u->animState;
		u->animState = ANIM_IDLE;
		u->animFrameMax = 1;

    BattleUnit *target = NULL;
    float mejorD2 = 0.0f;
    for (int a = 0; a < gBatalla.numAliados; a++) {
      BattleUnit *v = &gBatalla.aliados[a];
      if (!v->vivo)
        continue;
      float d2 = dist2(u, v);
      if (!target || d2 < mejorD2) {
        target = v;
        mejorD2 = d2;
      }
    }
    if (!target)
      break;

		float dx = target->x - u->x;
		float dy = target->y - u->y;
		float dist = sqrtf(dx * dx + dy * dy);
		if (dist > BATTLE_RANGE_PIXELS) {
			float inv = 1.0f / (dist + 0.0001f);
			u->x += dx * inv * u->vel;
			u->y += dy * inv * u->vel;
			u->dir = (dx >= 0) ? DIR_RIGHT : DIR_LEFT;
			u->animState = ANIM_MOVE;
			u->animFrameMax = 1;
		} else {
			u->animState = ANIM_ATTACK;
			if (prevAnim != ANIM_ATTACK) u->animFrame = 0;
			u->animFrameMax = 3;
			int raw = u->danio;
			if ((rand() % 100) < u->crit) raw *= 2;
			int real = raw - target->defensa;
			if (real < 1) real = 1;
			target->hp -= real;
			if (target->hp <= 0) { target->hp = 0; target->vivo = false; target->animState = ANIM_DEATH; target->animFrame = 0; target->animFrameMax = 2; }
		}
		if (u->animState == ANIM_MOVE) {
			u->animFrameMax = 1;
		}
	}

	// Evaluar fin de batalla

	// Ajustar posiciones para evitar solapamiento de celdas
	{
		BattleUnit *lista[BATTLE_MAX_UNITS * 2];
		int n = 0;
		for (int i = 0; i < gBatalla.numAliados; i++) {
			if (gBatalla.aliados[i].vivo) lista[n++] = &gBatalla.aliados[i];
		}
		for (int i = 0; i < gBatalla.numEnemigos; i++) {
			if (gBatalla.enemigos[i].vivo) lista[n++] = &gBatalla.enemigos[i];
		}

		const float minDist = BATTLE_TILE * 1.1f; // espacio completo de celda
		const float minDist2 = minDist * minDist;
		const float minX = 40.0f, minY = 40.0f;
		const float maxX = (BATTLE_COLS * BATTLE_TILE) - 40.0f;
		const float maxY = (BATTLE_ROWS * BATTLE_TILE) - 40.0f;

		// Varias iteraciones para resolver empujes acumulados
		for (int iter = 0; iter < 6; iter++) {
			for (int i = 0; i < n; i++) {
				for (int j = i + 1; j < n; j++) {
					float dx = lista[j]->x - lista[i]->x;
					float dy = lista[j]->y - lista[i]->y;
					float d2 = dx * dx + dy * dy;
					if (d2 < minDist2) {
						float dist = sqrtf(d2);
						if (dist < 0.001f) { dist = 0.001f; dx = 0.001f; dy = 0.0f; }
						float overlap = (minDist - dist) * 0.5f;
						float nx = dx / dist;
						float ny = dy / dist;
						lista[i]->x -= nx * overlap;
						lista[i]->y -= ny * overlap;
						lista[j]->x += nx * overlap;
						lista[j]->y += ny * overlap;
					}
				}
			}
			// Clamp al área de batalla
			for (int k = 0; k < n; k++) {
				if (lista[k]->x < minX) lista[k]->x = minX;
				if (lista[k]->x > maxX) lista[k]->x = maxX;
				if (lista[k]->y < minY) lista[k]->y = minY;
				if (lista[k]->y > maxY) lista[k]->y = maxY;
			}
		}
	}

	bool algunAliado = false, algunEnemigo = false;
	for (int a = 0; a < gBatalla.numAliados; a++) if (gBatalla.aliados[a].vivo) { algunAliado = true; break; }
	for (int e = 0; e < gBatalla.numEnemigos; e++) if (gBatalla.enemigos[e].vivo) { algunEnemigo = true; break; }

  if (!algunAliado && !algunEnemigo) {
    gBatalla.resultadoPendiente = BATALLA_RESULTADO_DERROTA;
    gBatalla.activa = false;
    marcarBajasAliadas();
  } else if (!algunAliado) {
    gBatalla.resultadoPendiente = BATALLA_RESULTADO_DERROTA;
    gBatalla.activa = false;
    marcarBajasAliadas();
  } else if (!algunEnemigo) {
    gBatalla.resultadoPendiente = BATALLA_RESULTADO_VICTORIA;
    gBatalla.activa = false;
    marcarBajasAliadas();
  }
}

// ---------------------------------------------------------------------------
// API pública
// ---------------------------------------------------------------------------
void batallasInicializar(void) {
  gInicioMs = GetTickCount();
  srand((unsigned int)time(NULL));
  asegurarSprites();
  cargarGuerreroAnim();
  cargarCaballeroAnim();
}

bool batallasPrepararDesdeViaje(struct Jugador *j, int islaDestino,
                                int islasConquistadas, bool esIslaInicial) {
  if (esIslaInicial)
    return false; // Isla inicial nunca tiene enemigos
  if (!j)
    return false;
  if (j->barco.numTropas <= 0)
    return false; // Sin tropas, no puede iniciar

  asegurarSprites();
  cargarGuerreroAnim();
  cargarCaballeroAnim();
  cargarFondoIsla(islaDestino);

  memset(&gBatalla, 0, sizeof(gBatalla));
  gBatalla.islaDestino = islaDestino;
  gBatalla.jugador = j;

  // Copiar tropas aliadas desde el barco
  gBatalla.numAliados = 0;
  for (int i = 0;
       i < j->barco.numTropas && gBatalla.numAliados < BATTLE_MAX_UNITS; i++) {
    Unidad *tropa = j->barco.tropas[i];
    if (!tropa)
      continue;
    int hp, dmg, def, crit;
    statsPorTipo(tropa->tipo, &hp, &dmg, &def, &crit);
    BattleUnit *u = &gBatalla.aliados[gBatalla.numAliados++];
    initUnit(u, tropa, false, hp, dmg, def, crit);
  }

  if (gBatalla.numAliados == 0)
    return false;

  poblarEnemigos(islasConquistadas);
  posicionarUnidades();
  gBatalla.tickAcumulado = 0.0f;
  gBatalla.activa = true;
  gBatalla.resultadoPendiente = BATALLA_RESULTADO_NONE;
  return true;
}

bool batallasEnCurso(void) { return gBatalla.activa; }

void batallasActualizar(float dt) {
  if (!gBatalla.activa)
    return;
  gBatalla.tickAcumulado += dt;
  gBatalla.animAccum += dt;
  if (gBatalla.animAccum >= BATTLE_ANIM_STEP_SEC) {
    gBatalla.animAccum = 0.0f;
    // Avance de animación para vivos
    for (int i = 0; i < gBatalla.numAliados; i++) {
      BattleUnit *u = &gBatalla.aliados[i];
      if (!u->vivo && u->animState != ANIM_DEATH)
        continue;
      u->animFrame =
          (u->animFrame + 1) % (u->animFrameMax > 0 ? u->animFrameMax : 1);
    }
    for (int i = 0; i < gBatalla.numEnemigos; i++) {
      BattleUnit *u = &gBatalla.enemigos[i];
      if (!u->vivo && u->animState != ANIM_DEATH)
        continue;
      u->animFrame =
          (u->animFrame + 1) % (u->animFrameMax > 0 ? u->animFrameMax : 1);
    }
  }

  if (gBatalla.tickAcumulado >= 1.0f) {
    gBatalla.tickAcumulado = 0.0f;
    resolverTick();
  }
}

static void dibujarUnidad(HDC hdc, BattleUnit *u, Camara cam) {
	if (!u->vivo) return;
	SpriteSet *spr = spriteSetPorTipo(u->tipo);
	int dirIdx = dirIndex(u->dir);
	HBITMAP frame = NULL;

	switch (u->animState) {
	case ANIM_MOVE:
		// Para caballero no hay walk: usar idle si falta
		frame = spr->walk[dirIdx] ? spr->walk[dirIdx] : spr->idle[dirIdx];
		break;
	case ANIM_ATTACK:
		frame = spr->attack[dirIdx][u->animFrame % 3];
		break;
	case ANIM_DEATH:
		frame = spr->death[dirIdx][(u->animFrame >= 2) ? 1 : u->animFrame];
		break;
	case ANIM_IDLE:
	default:
		frame = spr->idle[dirIdx];
		break;
	}

  if (!frame) {
    // Placeholder si falta sprite
    HBRUSH b =
        CreateSolidBrush(u->esEnemigo ? RGB(220, 40, 40) : RGB(40, 200, 60));
    int sx = (int)((u->x - cam.x) * cam.zoom);
    int sy = (int)((u->y - cam.y) * cam.zoom);
    RECT r = {sx, sy, sx + 48, sy + 48};
    FillRect(hdc, &r, b);
    DeleteObject(b);
    return;
  }

  HDC mem = CreateCompatibleDC(hdc);
  HBITMAP old = (HBITMAP)SelectObject(mem, frame);
  int size = (int)(64 * cam.zoom);
  int sx = (int)((u->x - cam.x) * cam.zoom);
  int sy = (int)((u->y - cam.y) * cam.zoom);
  TransparentBlt(hdc, sx, sy, size, size, mem, 0, 0, 64, 64,
                 RGB(255, 255, 255));
  SelectObject(mem, old);
  DeleteDC(mem);

  // Círculo de selección
  HBRUSH nullBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
  HPEN pen = CreatePen(PS_SOLID, 2,
                       u->esEnemigo ? RGB(220, 40, 40) : RGB(40, 200, 60));
  SelectObject(hdc, nullBrush);
  SelectObject(hdc, pen);
  Ellipse(hdc, sx, sy + size - 10, sx + size, sy + size + 5);
  DeleteObject(pen);

  // Barra de vida
  int barWidth = (int)(48 * cam.zoom);
  int barHeight = (int)(6 * cam.zoom);
  int barX = sx + (size - barWidth) / 2;
  int barY = sy - barHeight - 2;

  RECT bg = {barX, barY, barX + barWidth, barY + barHeight};
  HBRUSH rojo = CreateSolidBrush(RGB(120, 20, 20));
  FillRect(hdc, &bg, rojo);
  DeleteObject(rojo);

  int vidaPix = (u->hpMax > 0) ? (barWidth * u->hp) / u->hpMax : 0;
  RECT vida = {barX, barY, barX + vidaPix, barY + barHeight};
  HBRUSH verde = CreateSolidBrush(RGB(30, 200, 30));
  FillRect(hdc, &vida, verde);
  DeleteObject(verde);
}

void batallasRender(HDC hdc, RECT rect, Camara cam) {
  if (!gBatalla.activa)
    return;

  if (hFondoIsla) {
    HDC mem = CreateCompatibleDC(hdc);
    HBITMAP old = (HBITMAP)SelectObject(mem, hFondoIsla);
    BITMAP info;
    GetObject(hFondoIsla, sizeof(info), &info);

    int srcW = (int)((rect.right - rect.left) / cam.zoom);
    int srcH = (int)((rect.bottom - rect.top) / cam.zoom);
    int srcX = (int)cam.x;
    int srcY = (int)cam.y;
    if (srcW <= 0 || srcH <= 0) {
      srcW = info.bmWidth;
      srcH = info.bmHeight;
    }
    if (srcX < 0)
      srcX = 0;
    if (srcY < 0)
      srcY = 0;
    if (srcX + srcW > info.bmWidth)
      srcX = info.bmWidth - srcW;
    if (srcY + srcH > info.bmHeight)
      srcY = info.bmHeight - srcH;
    StretchBlt(hdc, 0, 0, rect.right - rect.left, rect.bottom - rect.top, mem,
               srcX, srcY, srcW, srcH, SRCCOPY);
    SelectObject(mem, old);
    DeleteDC(mem);
  } else {
    HBRUSH fondo = CreateSolidBrush(RGB(40, 120, 70));
    FillRect(hdc, &rect, fondo);
    DeleteObject(fondo);
  }

	BattleUnit *ordenados[BATTLE_MAX_UNITS * 2];
	int n = 0;
	for (int i = 0; i < gBatalla.numAliados; i++) {
		if (gBatalla.aliados[i].vivo) ordenados[n++] = &gBatalla.aliados[i];
	}
	for (int i = 0; i < gBatalla.numEnemigos; i++) {
		if (gBatalla.enemigos[i].vivo) ordenados[n++] = &gBatalla.enemigos[i];
	}
	qsort(ordenados, n, sizeof(BattleUnit *), cmpBattleUnitPtr);
	for (int i = 0; i < n; i++) dibujarUnidad(hdc, ordenados[i], cam);
}

bool batallasObtenerResultado(BatallaResultado *outResultado,
                              int *outIslaDestino) {
  if (gBatalla.resultadoPendiente == BATALLA_RESULTADO_NONE)
    return false;
  if (outResultado)
    *outResultado = gBatalla.resultadoPendiente;
  if (outIslaDestino)
    *outIslaDestino = gBatalla.islaDestino;
  gBatalla.resultadoPendiente = BATALLA_RESULTADO_NONE;
  return true;
}