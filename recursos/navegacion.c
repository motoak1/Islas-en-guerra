#include "navegacion.h"
#include "../edificios/edificios.h"
#include "../mapa/mapa.h"
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

// Estado persistente por isla (1..3)
typedef struct {
  bool inicializado;
  int Comida;
  int Oro;
  int Madera;
  int Piedra;
  int Hierro;
  bool tieneAyuntamiento;
  bool tieneMina;
  bool tieneCuartel;
  Edificio ayuntamiento;
  Edificio mina;
  Edificio cuartel;
  Unidad obreros[MAX_OBREROS];
  Unidad caballeros[MAX_CABALLEROS];
  Unidad caballerosSinEscudo[MAX_CABALLEROS_SIN_ESCUDO];
  Unidad guerreros[MAX_GUERREROS];
  Unidad enemigos[8];
  int numEnemigos;
  bool enemigosGenerados;
} EstadoIsla;

static EstadoIsla sIslas[4];
static int sIslaInicial = 1;
static bool sIslaInicialDefinida = false;
static Unidad sEnemigosActivos[8];
static int sNumEnemigosActivos = 0;
static int sEnemigosCooldownMs[8] = {0};
static void limpiarEnemigosActivos(void);
static void activarEnemigosDesdeEstado(EstadoIsla *estado);
static void unidadASerializable(const Unidad *src,
                                UnidadIslaSerializable *dst);
static void serializableAUnidad(const UnidadIslaSerializable *src,
                                Unidad *dst);
static void edificioASerializable(const Edificio *src,
                                  EdificioIslaSerializable *dst);
static void serializableAEdificio(const EdificioIslaSerializable *src,
                                  Edificio *dst);

static void unidadASerializable(const Unidad *src,
                                UnidadIslaSerializable *dst) {
  if (!dst || !src)
    return;
  dst->x = src->x;
  dst->y = src->y;
  dst->destinoX = src->destinoX;
  dst->destinoY = src->destinoY;
  dst->moviendose = src->moviendose;
  dst->seleccionado = src->seleccionado;
  dst->dir = (int)src->dir;
  dst->frame = src->frame;
  dst->objetivoFila = src->objetivoFila;
  dst->objetivoCol = src->objetivoCol;
  dst->celdaFila = src->celdaFila;
  dst->celdaCol = src->celdaCol;
  dst->tipo = (int)src->tipo;
  dst->vida = src->vida;
  dst->vidaMax = src->vidaMax;
  dst->damage = src->damage;
  dst->critico = src->critico;
  dst->defensa = src->defensa;
  dst->alcance = src->alcance;
  dst->recibiendoAtaque = src->recibiendoAtaque;
  dst->tiempoMuerteMs = src->tiempoMuerteMs;
  dst->frameMuerte = src->frameMuerte;
}

static void serializableAUnidad(const UnidadIslaSerializable *src,
                                Unidad *dst) {
  if (!dst || !src)
    return;
  if (dst->rutaCeldas) {
    free(dst->rutaCeldas);
    dst->rutaCeldas = NULL;
  }
  dst->rutaLen = 0;
  dst->rutaIdx = 0;
  dst->animActual = NULL;
  dst->animTick = 0;
  dst->x = src->x;
  dst->y = src->y;
  dst->destinoX = src->destinoX;
  dst->destinoY = src->destinoY;
  dst->moviendose = src->moviendose;
  dst->seleccionado = src->seleccionado;
  dst->dir = (Direccion)src->dir;
  dst->frame = src->frame;
  dst->objetivoFila = src->objetivoFila;
  dst->objetivoCol = src->objetivoCol;
  dst->celdaFila = src->celdaFila;
  dst->celdaCol = src->celdaCol;
  dst->tipo = (TipoUnidad)src->tipo;
  dst->vida = src->vida;
  dst->vidaMax = src->vidaMax;
  dst->damage = src->damage;
  dst->critico = src->critico;
  dst->defensa = src->defensa;
  dst->alcance = src->alcance;
  dst->recibiendoAtaque = src->recibiendoAtaque;
  dst->tiempoMuerteMs = src->tiempoMuerteMs;
  dst->frameMuerte = src->frameMuerte;
}

static void edificioASerializable(const Edificio *src,
                                  EdificioIslaSerializable *dst) {
  if (!src || !dst)
    return;
  dst->tipo = (int)src->tipo;
  dst->x = src->x;
  dst->y = src->y;
  dst->ancho = src->ancho;
  dst->alto = src->alto;
  dst->construido = src->construido;
  dst->oroAcumulado = src->oroAcumulado;
  dst->piedraAcumulada = src->piedraAcumulada;
  dst->hierroAcumulado = src->hierroAcumulado;
  dst->ultimoTickGeneracion = src->ultimoTickGeneracion;
  dst->oroRestante = src->oroRestante;
  dst->piedraRestante = src->piedraRestante;
  dst->hierroRestante = src->hierroRestante;
  dst->agotada = src->agotada;
}

static void serializableAEdificio(const EdificioIslaSerializable *src,
                                  Edificio *dst) {
  if (!src || !dst)
    return;
  dst->tipo = (TipoEdificio)src->tipo;
  dst->x = src->x;
  dst->y = src->y;
  dst->ancho = src->ancho;
  dst->alto = src->alto;
  dst->construido = src->construido;
  dst->oroAcumulado = src->oroAcumulado;
  dst->piedraAcumulada = src->piedraAcumulada;
  dst->hierroAcumulado = src->hierroAcumulado;
  dst->ultimoTickGeneracion = src->ultimoTickGeneracion;
  dst->oroRestante = src->oroRestante;
  dst->piedraRestante = src->piedraRestante;
  dst->hierroRestante = src->hierroRestante;
  dst->agotada = src->agotada;
  dst->sprite = NULL;
}

// Guardado: expone el snapshot lógico de cada isla para que guardado.c pueda
// escribirlo en disco junto con el mapa.
void navegacionExportarEstadosIsla(EstadoIslaSerializable estados[4]) {
  if (!estados)
    return;
  for (int isla = 0; isla < 4; isla++) {
    EstadoIsla *src = &sIslas[isla];
    EstadoIslaSerializable *dst = &estados[isla];
    dst->inicializado = src->inicializado;
    dst->Comida = src->Comida;
    dst->Oro = src->Oro;
    dst->Madera = src->Madera;
    dst->Piedra = src->Piedra;
    dst->Hierro = src->Hierro;
    dst->tieneAyuntamiento = src->tieneAyuntamiento;
    dst->tieneMina = src->tieneMina;
    dst->tieneCuartel = src->tieneCuartel;
    if (src->tieneAyuntamiento)
      edificioASerializable(&src->ayuntamiento, &dst->ayuntamiento);
    if (src->tieneMina)
      edificioASerializable(&src->mina, &dst->mina);
    if (src->tieneCuartel)
      edificioASerializable(&src->cuartel, &dst->cuartel);

    for (int i = 0; i < 6; i++)
      unidadASerializable(&src->obreros[i], &dst->obreros[i]);
    for (int i = 0; i < 4; i++) {
      unidadASerializable(&src->caballeros[i], &dst->caballeros[i]);
      unidadASerializable(&src->caballerosSinEscudo[i],
                          &dst->caballerosSinEscudo[i]);
      unidadASerializable(&src->guerreros[i], &dst->guerreros[i]);
    }
    for (int i = 0; i < 8; i++)
      unidadASerializable(&src->enemigos[i], &dst->enemigos[i]);
    dst->numEnemigos = src->numEnemigos;
    dst->enemigosGenerados = src->enemigosGenerados;
  }
}

// Carga: restaura el snapshot de cada isla desde el archivo sin tener que
// regenerar edificios, enemigos o recursos.
void navegacionImportarEstadosIsla(const EstadoIslaSerializable estados[4]) {
  if (!estados)
    return;
  for (int isla = 0; isla < 4; isla++) {
    EstadoIsla *dst = &sIslas[isla];
    const EstadoIslaSerializable *src = &estados[isla];
    dst->inicializado = src->inicializado;
    dst->Comida = src->Comida;
    dst->Oro = src->Oro;
    dst->Madera = src->Madera;
    dst->Piedra = src->Piedra;
    dst->Hierro = src->Hierro;
    dst->tieneAyuntamiento = src->tieneAyuntamiento;
    dst->tieneMina = src->tieneMina;
    dst->tieneCuartel = src->tieneCuartel;
    if (dst->tieneAyuntamiento) {
      serializableAEdificio(&src->ayuntamiento, &dst->ayuntamiento);
      dst->ayuntamiento.sprite = g_spriteAyuntamiento;
    }
    if (dst->tieneMina) {
      serializableAEdificio(&src->mina, &dst->mina);
      dst->mina.sprite = g_spriteMina;
    }
    if (dst->tieneCuartel) {
      serializableAEdificio(&src->cuartel, &dst->cuartel);
      dst->cuartel.sprite = g_spriteCuartel;
    }

    for (int i = 0; i < 6; i++)
      serializableAUnidad(&src->obreros[i], &dst->obreros[i]);
    for (int i = 0; i < 4; i++) {
      serializableAUnidad(&src->caballeros[i], &dst->caballeros[i]);
      serializableAUnidad(&src->caballerosSinEscudo[i],
                          &dst->caballerosSinEscudo[i]);
      serializableAUnidad(&src->guerreros[i], &dst->guerreros[i]);
    }
    for (int i = 0; i < 8; i++)
      serializableAUnidad(&src->enemigos[i], &dst->enemigos[i]);
    dst->numEnemigos = src->numEnemigos;
    dst->enemigosGenerados = src->enemigosGenerados;
  }
}

// Carga: repone la isla inicial almacenada en el guardado para que el flujo de
// viaje siga coherente tras reanudar.
void navegacionRestaurarIslaInicial(int isla, bool definida) {
  sIslaInicial = (isla >= 1 && isla <= 3) ? isla : 1;
  sIslaInicialDefinida = definida;
}

// Guardado/carga: expone si la isla inicial ya fue fijada; usado al serializar
// y al validar datos cargados.
bool navegacionIslaInicialDefinida(void) { return sIslaInicialDefinida; }

// Carga: reinyecta los enemigos pasivos guardados para la isla restaurada.
void navegacionActivarEnemigosIsla(int isla) {
  if (isla < 1 || isla > 3) {
    limpiarEnemigosActivos();
    return;
  }
  EstadoIsla *estado = &sIslas[isla];
  if (estado->enemigosGenerados) {
    activarEnemigosDesdeEstado(estado);
  } else {
    limpiarEnemigosActivos();
  }
}

// Adelanto de helper usado antes de su definición
static bool buscarCeldaLibreCerca(int preferX, int preferY, int ancho, int alto,
                                  int radioMax, int *outX, int *outY);

// ============================================================================
// POSICIONES FIJAS DEL BARCO POR ISLA (EN COORDENADAS DE MATRIZ)
// ============================================================================
// Las posiciones se definen como (fila, columna) de la matriz 32x32.
// Se convierten a píxeles multiplicando por TILE_SIZE.
// Direcciones: 0=DIR_FRONT, 1=DIR_BACK, 2=DIR_LEFT, 3=DIR_RIGHT
// ============================================================================

// Array de posiciones por isla [isla][0=fila, 1=columna, 2=direccion]
// Isla 0 no se usa, islas 1-3 son válidas
static int sPosicionesBarco[4][3] = {
    {0, 0, 0},   // Índice 0: no usado
    {15, 2, 3},  // Isla 1: fila=15, col=2, dir=DIR_RIGHT (mirando hacia isla)
    {15, 29, 2}, // Isla 2: fila=15, col=29, dir=DIR_LEFT
    {2, 15, 0}   // Isla 3: fila=2, col=15, dir=DIR_FRONT
};

static void obtenerPosicionBarcoIsla(int isla, float *outX, float *outY,
                                     int *outDir) {
  // Validar isla
  if (isla < 1 || isla > 3) {
    // Fallback: usar detección automática
    mapaDetectarOrilla(outX, outY, outDir);
    return;
  }

  // Obtener posición usando aritmética de punteros sobre el array
  int *posIsla = *(sPosicionesBarco + isla); // Puntero a la fila del array
  int fila = *(posIsla + 0);                 // Primera posición: fila
  int columna = *(posIsla + 1);              // Segunda posición: columna
  int direccion = *(posIsla + 2);            // Tercera posición: dirección

  // Convertir coordenadas de matriz a píxeles
  *outX = (float)(columna * TILE_SIZE);
  *outY = (float)(fila * TILE_SIZE);
  *outDir = direccion;

  printf(
      "[DEBUG BARCO] Isla %d: Matriz[%d][%d] -> Pixeles(%.1f, %.1f), dir=%d\n",
      isla, fila, columna, *outX, *outY, *outDir);
}

// Función exportable para usar desde main.c
void navegacionObtenerPosicionBarcoIsla(int isla, float *outX, float *outY,
                                        int *outDir) {
  obtenerPosicionBarcoIsla(isla, outX, outY, outDir);
}
static int contarIslasConquistadas(void) {
  int total = 0;
  for (int i = 1; i <= 3; i++) {
    if (sIslas[i].inicializado)
      total++;
  }
  return total;
}

static void normalizarVidaSiVacio(Unidad *u) {
  if (!u)
    return;
  if (u->vidaMax <= 0) {
    switch (u->tipo) {
    case TIPO_CABALLERO:
      u->vidaMax = CABALLERO_VIDA;
      u->vida = CABALLERO_VIDA;
      break;
    case TIPO_CABALLERO_SIN_ESCUDO:
      u->vidaMax = CABALLERO_SIN_ESCUDO_VIDA;
      u->vida = CABALLERO_SIN_ESCUDO_VIDA;
      break;
    case TIPO_GUERRERO:
      u->vidaMax = GUERRERO_VIDA;
      u->vida = GUERRERO_VIDA;
      break;
    default:
      break;
    }
  }
}

static void limpiarEnemigosActivos(void) {
  sNumEnemigosActivos = 0;
  for (int i = 0; i < 8; i++) {
    sEnemigosActivos[i].x = -1000.0f;
    sEnemigosActivos[i].y = -1000.0f;
    sEnemigosActivos[i].moviendose = false;
    sEnemigosActivos[i].seleccionado = false;
  }
}

Unidad *navegacionObtenerEnemigosActivos(int *cantidad) {
  if (cantidad)
    *cantidad = sNumEnemigosActivos;
  return sEnemigosActivos;
}

static void marcarEnemigosEnMapa(const Unidad *enemigos, int cantidad) {
  int **col = mapaObtenerCollisionMap();
  for (int i = 0; i < cantidad; i++) {
    const Unidad *e = enemigos + i;
    mapaRegistrarObjeto(e->x, e->y, SIMBOLO_ENEMIGO);
    if (!col)
      continue;
    int celdaX = (int)(e->x / TILE_SIZE);
    int celdaY = (int)(e->y / TILE_SIZE);
    if (celdaX >= 0 && celdaX < GRID_SIZE && celdaY >= 0 && celdaY < GRID_SIZE)
      *(*(col + celdaY) + celdaX) = 3;
  }
}

static void activarEnemigosDesdeEstado(EstadoIsla *estado) {
  limpiarEnemigosActivos();
  sNumEnemigosActivos = estado->numEnemigos;
  for (int i = 0; i < estado->numEnemigos && i < 8; i++) {
    sEnemigosActivos[i] = estado->enemigos[i];
  }
  marcarEnemigosEnMapa(sEnemigosActivos, sNumEnemigosActivos);
}

static void statsBasicosEnemigo(Unidad *u, TipoUnidad tipo) {
  u->tipo = tipo;
  u->moviendose = false;
  u->seleccionado = false;
  u->rutaCeldas = NULL;
  u->rutaIdx = 0;
  u->rutaLen = 0;
  u->destinoX = u->x;
  u->destinoY = u->y;
  u->dir = DIR_LEFT;
  u->alcance = TILE_SIZE;
  u->recibiendoAtaque = false;
  u->tiempoMuerteMs = 0;
  u->frameMuerte = 0;

  if (tipo == TIPO_CABALLERO) {
    u->vida = CABALLERO_VIDA;
    u->vidaMax = CABALLERO_VIDA;
    u->damage = CABALLERO_DANO;
    u->defensa = CABALLERO_DEFENSA;
    u->critico = CABALLERO_CRITICO;
  } else {
    u->vida = GUERRERO_VIDA;
    u->vidaMax = GUERRERO_VIDA;
    u->damage = GUERRERO_DANO;
    u->defensa = GUERRERO_DEFENSA;
    u->critico = GUERRERO_CRITICO;
  }
}

static int clampIntLocal(int v, int lo, int hi) {
  if (v < lo)
    return lo;
  if (v > hi)
    return hi;
  return v;
}

// Función para contar unidades globales (incluyendo otras islas)
int navegacionContarUnidadesGlobal(const struct Jugador *j, TipoUnidad tipo) {
  int total = 0;

  // 1. Contar unidades ACTIVAS en la isla actual (j->obreros, etc)
  int maxLimit = 0;
  const Unidad *ptr = NULL;

  switch(tipo) {
    case TIPO_OBRERO: 
        maxLimit = MAX_OBREROS; ptr = j->obreros; break;
    case TIPO_CABALLERO: 
        maxLimit = MAX_CABALLEROS; ptr = j->caballeros; break;
    case TIPO_CABALLERO_SIN_ESCUDO: 
        maxLimit = MAX_CABALLEROS_SIN_ESCUDO; ptr = j->caballerosSinEscudo; break;
    case TIPO_GUERRERO: 
        maxLimit = MAX_GUERREROS; ptr = j->guerreros; break;
    default: return 0;
  }

  for (int i = 0; i < maxLimit; i++) {
    if (ptr[i].x >= 0 && ptr[i].y >= 0) total++;
  }

  // 2. Sumar unidades de OTROS islas guardadas en sIslas
  for (int k = 1; k <= 3; k++) {
    if (k == j->islaActual) continue; // Ya contadas (o por guardar)
    if (!sIslas[k].inicializado) continue;

    const Unidad *ptrGuardado = NULL;
    switch(tipo) {
        case TIPO_OBRERO: ptrGuardado = sIslas[k].obreros; break;
        case TIPO_CABALLERO: ptrGuardado = sIslas[k].caballeros; break;
        case TIPO_CABALLERO_SIN_ESCUDO: ptrGuardado = sIslas[k].caballerosSinEscudo; break;
        case TIPO_GUERRERO: ptrGuardado = sIslas[k].guerreros; break;
    }

    if (ptrGuardado) {
        for (int i = 0; i < maxLimit; i++) {
            if (ptrGuardado[i].x >= 0 && ptrGuardado[i].y >= 0) total++;
        }
    }
  }

  // No necesitamos sumar las del barco explícitamente si ya se consideran "activas" o "fuera".
  // Pero las del barco están en j->barco.tropas.
  // IMPORTANTE: Cuando una unidad sube al barco, ¿sigue en j->obreros con coords validas?
  // Normalmente se desactiva (x=-1000) o se mantiene hasta que viajas?
  // Revisando ui_embarque.c (no visible), asumimos que al subir al barco desaparecen del mapa (x<0).
  // Si es así, debemos sumarlas aquí.
  
  if (j->barco.numTropas > 0) {
      for(int i=0; i<j->barco.numTropas; i++) {
          if (j->barco.tropas[i] && j->barco.tropas[i]->tipo == tipo) {
              total++;
          }
      }
  }

  return total;
}

static DWORD sStartMs = 0;

// ---------------------------------------------------------------------------
// VALIDACIONES DE VIAJE
// ---------------------------------------------------------------------------
static bool islaConquistada(int isla) {
  if (isla < 1 || isla > 3) return false;
  EstadoIsla *e = &sIslas[isla];
  // Isla inicial no tiene enemigos por diseño; si aún no está inicializada,
  // consideramos que no está conquistada salvo que sea la inicial definida.
  if (!e->inicializado) {
    return (isla == sIslaInicial); // permitir si el destino es la isla inicial
  }
  int vivos = 0;
  for (int i = 0; i < e->numEnemigos; i++) {
    if (e->enemigos[i].vida > 0 && e->enemigos[i].x >= 0) vivos++;
  }
  return vivos == 0;
}

static bool barcoTieneObreros(const Barco *barco) {
  if (!barco) return false;
  for (int i = 0; i < barco->numTropas; i++) {
    const Unidad *u = barco->tropas[i];
    if (u && u->tipo == TIPO_OBRERO) return true;
  }
  return false;
}

// Limpia un rectángulo de celdas en mapaObjetos y collision map
static void limpiarAreaCeldas(int celdaX, int celdaY, int ancho, int alto) {
  int **col = mapaObtenerCollisionMap();

  for (int f = celdaY; f < celdaY + alto; f++) {
    for (int c = celdaX; c < celdaX + ancho; c++) {
      if (f < 0 || c < 0 || f >= GRID_SIZE || c >= GRID_SIZE)
        continue;
      mapaObjetos[f][c] = SIMBOLO_VACIO;
      if (col) {
        *(*(col + f) + c) = 0;
      }
    }
  }
}

void navegacionRegistrarIslaInicial(int isla) {
  sIslaInicial = (isla >= 1 && isla <= 3) ? isla : 1;
  sIslaInicialDefinida = true;
}

int navegacionObtenerIslaInicial(void) {
  return sIslaInicial;
}

bool navegacionIsIslaConquistada(int isla) {
  return islaConquistada(isla);
}

bool navegacionIslaActualNoConquistada(const struct Jugador *j) {
  if (!j) return false;
  return !islaConquistada(j->islaActual);
}

// Verifica si un rectángulo de celdas está libre en collision map y no es agua
static bool areaLibre(int celdaX, int celdaY, int ancho, int alto) {
  int **col = mapaObtenerCollisionMap();
  if (!col)
    return false;

  for (int f = celdaY; f < celdaY + alto; f++) {
    for (int c = celdaX; c < celdaX + ancho; c++) {
      if (f < 0 || c < 0 || f >= GRID_SIZE || c >= GRID_SIZE)
        return false;
      int valor = *(*(col + f) + c);
      char simbolo = mapaObjetos[f][c];
      if (valor != 0)
        return false; // Bloqueado por agua/árbol/edificio
      if (simbolo == SIMBOLO_AGUA)
        return false; // Evitar agua marcada
    }
  }
  return true;
}

// Determina si la celda es tierra pegada al agua (orilla)
static bool esTierraOrilla(int cx, int cy, int **col) {
  if (cx < 0 || cy < 0 || cx >= GRID_SIZE || cy >= GRID_SIZE)
    return false;
  if (!mapaCeldaEsTierra(cy, cx))
    return false;

  const int dx[4] = {1, -1, 0, 0};
  const int dy[4] = {0, 0, 1, -1};
  for (int k = 0; k < 4; k++) {
    int nx = cx + dx[k];
    int ny = cy + dy[k];
    if (nx < 0 || ny < 0 || nx >= GRID_SIZE || ny >= GRID_SIZE)
      continue;
    int v2 = *(*(col + ny) + nx);
    char s2 = mapaObjetos[ny][nx];
    if (v2 == 1 || s2 == SIMBOLO_AGUA)
      return true;
  }
  return false;
}

// Busca la mejor celda disponible alrededor de una preferencia
static bool buscarCeldaLibreCerca(int preferX, int preferY, int ancho, int alto,
                                  int radioMax, int *outX, int *outY) {
  for (int radio = 0; radio <= radioMax; radio++) {
    for (int dy = -radio; dy <= radio; dy++) {
      for (int dx = -radio; dx <= radio; dx++) {
        int cx = preferX + dx;
        int cy = preferY + dy;
        if (areaLibre(cx, cy, ancho, alto)) {
          if (outX)
            *outX = cx;
          if (outY)
            *outY = cy;
          return true;
        }
      }
    }
  }
  return false;
}

// Inicializa edificios en las posiciones base usadas al inicio del juego
static void inicializarEstructurasIslaBase(struct Jugador *j,
                                           EstadoIsla *estado) {
  const float AYUNT_X = 1024.0f - 64.0f;
  const float AYUNT_Y = 1024.0f - 64.0f;
  const float MINA_X = 1024.0f - 64.0f;
  const float MINA_Y = 450.0f;
  const float CUAR_X = 1024.0f - 200.0f;
  const float CUAR_Y = 1350.0f;

  edificioInicializar(&estado->ayuntamiento, EDIFICIO_AYUNTAMIENTO, AYUNT_X,
                      AYUNT_Y);
  edificioInicializar(&estado->mina, EDIFICIO_MINA, MINA_X, MINA_Y);
  edificioInicializar(&estado->cuartel, EDIFICIO_CUARTEL, CUAR_X, CUAR_Y);

  estado->tieneAyuntamiento = true;
  estado->tieneMina = true;
  estado->tieneCuartel = true;

  // Marcar en mapa de colisión y objetos (para asegurar consistencia)
  mapaMarcarEdificio(AYUNT_X, AYUNT_Y, estado->ayuntamiento.ancho,
                     estado->ayuntamiento.alto);
  mapaRegistrarObjeto(AYUNT_X, AYUNT_Y, SIMBOLO_EDIFICIO);

  mapaMarcarEdificio(MINA_X, MINA_Y, estado->mina.ancho, estado->mina.alto);
  mapaRegistrarObjeto(MINA_X, MINA_Y, SIMBOLO_MINA);

  mapaMarcarEdificio(CUAR_X, CUAR_Y, estado->cuartel.ancho,
                     estado->cuartel.alto);
  mapaRegistrarObjeto(CUAR_X, CUAR_Y, SIMBOLO_CUARTEL);

  // Apuntar jugador a estas instancias
  j->ayuntamiento = &estado->ayuntamiento;
  j->mina = &estado->mina;
  j->cuartel = &estado->cuartel;
}

// Envía todas las unidades fuera del mapa (se usarán solo las del barco al
// desembarcar)
static void vaciarUnidades(struct Jugador *j) {
  for (int i = 0; i < MAX_OBREROS; i++) {
    j->obreros[i].x = -1000.0f;
    j->obreros[i].y = -1000.0f;
    j->obreros[i].moviendose = false;
    j->obreros[i].seleccionado = false;
    j->obreros[i].celdaFila = -1;
    j->obreros[i].celdaCol = -1;
  }
  for (int i = 0; i < MAX_CABALLEROS; i++) {
    j->caballeros[i].x = -1000.0f;
    j->caballeros[i].y = -1000.0f;
    j->caballeros[i].moviendose = false;
    j->caballeros[i].seleccionado = false;
    j->caballeros[i].celdaFila = -1;
    j->caballeros[i].celdaCol = -1;
  }
  for (int i = 0; i < MAX_CABALLEROS_SIN_ESCUDO; i++) {
    j->caballerosSinEscudo[i].x = -1000.0f;
    j->caballerosSinEscudo[i].y = -1000.0f;
    j->caballerosSinEscudo[i].moviendose = false;
    j->caballerosSinEscudo[i].seleccionado = false;
    j->caballerosSinEscudo[i].celdaFila = -1;
    j->caballerosSinEscudo[i].celdaCol = -1;
  }
  for (int i = 0; i < MAX_GUERREROS; i++) {
    j->guerreros[i].x = -1000.0f;
    j->guerreros[i].y = -1000.0f;
    j->guerreros[i].moviendose = false;
    j->guerreros[i].seleccionado = false;
    j->guerreros[i].celdaFila = -1;
    j->guerreros[i].celdaCol = -1;
  }
}

// Desembarca tropas juntas cerca del centro de la isla
static void desembarcarTropasEnCentro(Barco *barco, struct Jugador *j) {
  int baseCeldaX = GRID_SIZE / 2;
  int baseCeldaY = (GRID_SIZE / 2) + 2;

  int desembarcadas = 0;
  for (int i = 0; i < barco->numTropas; i++) {
    Unidad *tropa = barco->tropas[i];
    if (!tropa)
      continue;

    int preferX = baseCeldaX + (i % 3);
    int preferY = baseCeldaY + (i / 3);
    int celdaX = preferX, celdaY = preferY;

    if (!buscarCeldaLibreCerca(preferX, preferY, 1, 1, 6, &celdaX, &celdaY)) {
      continue;
    }

    tropa->x = (float)(celdaX * TILE_SIZE);
    tropa->y = (float)(celdaY * TILE_SIZE);
    tropa->destinoX = tropa->x;
    tropa->destinoY = tropa->y;
    tropa->moviendose = false;
    desembarcadas++;
  }

  for (int k = 0; k < 6; k++)
    barco->tropas[k] = NULL;
  barco->numTropas = 0;

  printf("[DEBUG] Tropas desembarcadas en el centro: %d\n", desembarcadas);
}

// Guarda el estado de recursos y edificios del jugador para la isla actual
static void guardarEstadoIslaJugador(struct Jugador *j) {
  int isla = j->islaActual;
  if (isla < 1 || isla > 3)
    return;
  EstadoIsla *estado = &sIslas[isla];

  estado->Comida = j->Comida;
  estado->Oro = j->Oro;
  estado->Madera = j->Madera;
  estado->Piedra = j->Piedra;
  estado->Hierro = j->Hierro;

  estado->tieneAyuntamiento = (j->ayuntamiento != NULL);
  if (estado->tieneAyuntamiento)
    estado->ayuntamiento = *((Edificio *)j->ayuntamiento);

  estado->tieneMina = (j->mina != NULL);
  if (estado->tieneMina)
    estado->mina = *((Edificio *)j->mina);

  estado->tieneCuartel = (j->cuartel != NULL);
  if (estado->tieneCuartel)
    estado->cuartel = *((Edificio *)j->cuartel);

  // Copiar unidades (posiciones actuales en esta isla)
  for (int i = 0; i < MAX_OBREROS; i++)
    estado->obreros[i] = j->obreros[i];
  for (int i = 0; i < MAX_CABALLEROS; i++)
    estado->caballeros[i] = j->caballeros[i];
  for (int i = 0; i < MAX_CABALLEROS_SIN_ESCUDO; i++)
    estado->caballerosSinEscudo[i] = j->caballerosSinEscudo[i];
  for (int i = 0; i < MAX_GUERREROS; i++)
    estado->guerreros[i] = j->guerreros[i];

  estado->numEnemigos = sNumEnemigosActivos;
  estado->enemigosGenerados =
      sNumEnemigosActivos > 0 || estado->enemigosGenerados;
  for (int i = 0; i < sNumEnemigosActivos && i < 8; i++) {
    estado->enemigos[i] = sEnemigosActivos[i];
  }

  estado->inicializado = true;
  printf("[DEBUG] Jugador: estado de isla %d guardado\n", isla);
}

// Restaura recursos y edificios del jugador al cambiar a otra isla
static void restaurarEstadoIslaJugador(struct Jugador *j, int isla) {
  if (isla < 1 || isla > 3)
    return;
  EstadoIsla *estado = &sIslas[isla];
  if (!estado->inicializado)
    return;

  // RECURSOS GLOBALES: No sobrescribir con el estado guardado de la isla
  // j->Comida = estado->Comida;
  // j->Oro = estado->Oro;
  // j->Madera = estado->Madera;
  // j->Piedra = estado->Piedra;
  // j->Hierro = estado->Hierro;

  j->ayuntamiento = estado->tieneAyuntamiento ? &estado->ayuntamiento : NULL;
  j->mina = estado->tieneMina ? &estado->mina : NULL;
  j->cuartel = estado->tieneCuartel ? &estado->cuartel : NULL;

  // Restaurar unidades de esta isla
  for (int i = 0; i < MAX_OBREROS; i++)
    j->obreros[i] = estado->obreros[i];
  for (int i = 0; i < MAX_CABALLEROS; i++)
    j->caballeros[i] = estado->caballeros[i];
  for (int i = 0; i < MAX_CABALLEROS_SIN_ESCUDO; i++)
    j->caballerosSinEscudo[i] = estado->caballerosSinEscudo[i];
  for (int i = 0; i < MAX_GUERREROS; i++)
    j->guerreros[i] = estado->guerreros[i];

  if (estado->enemigosGenerados) {
    activarEnemigosDesdeEstado(estado);
  } 

  printf("[DEBUG] Jugador: estado de isla %d restaurado (Recursos mantenidos "
         "globales)\n",
         isla);
}

// ============================================================================
// FUNCIONES AUXILIARES
// ============================================================================

bool barcoContienePunto(Barco *barco, float mundoX, float mundoY) {
  const float BARCO_SIZE = 192.0f;

  return (mundoX >= barco->x && mundoX < barco->x + BARCO_SIZE &&
          mundoY >= barco->y && mundoY < barco->y + BARCO_SIZE);
}

void desembarcarTropas(Barco *barco, struct Jugador *j) {
  printf("[DEBUG] Desembarcando %d tropas...\n", barco->numTropas);

  // CRÍTICO: Buscar punto de tierra más cercano al barco
  // El barco está en agua, necesitamos encontrar la tierra adyacente

  // Obtener mapa de colisión para verificar dónde hay tierra
  int **col = mapaObtenerCollisionMap();
  if (!col) {
    printf("[ERROR] No se pudo obtener mapa de colisión para desembarcar\n");
    return;
  }

  // Convertir posición del barco a celdas (alineado con TILE_SIZE)
  int barcoCeldaX = (int)(barco->x / (float)TILE_SIZE);
  int barcoCeldaY = (int)(barco->y / (float)TILE_SIZE);

  // Buscar tierra adyacente al barco (búsqueda en espiral) dando prioridad a
  // orilla (tierra pegada a agua)
  int tierraX = -1, tierraY = -1;
  bool tierraEncontrada = false;

  // Buscar en radio creciente alrededor del barco (hasta 12 celdas)
  for (int radio = 1; radio <= 12 && !tierraEncontrada; radio++) {
    for (int dy = -radio; dy <= radio && !tierraEncontrada; dy++) {
      for (int dx = -radio; dx <= radio; dx++) {
        int celdaX = barcoCeldaX + dx;
        int celdaY = barcoCeldaY + dy;
        if (celdaX < 0 || celdaX >= GRID_SIZE || celdaY < 0 ||
            celdaY >= GRID_SIZE)
          continue;
        if (esTierraOrilla(celdaX, celdaY, col)) {
          tierraX = celdaX;
          tierraY = celdaY;
          tierraEncontrada = true;
          break;
        }
      }
    }
  }

  // Si no se encontró tierra, usar posición de emergencia
  if (!tierraEncontrada) {
    printf("[WARNING] No se encontró tierra cerca del barco, desembarcando EN EL SITIO (emergencia)\n");
    tierraX = barcoCeldaX; 
    tierraY = barcoCeldaY;
  }

  printf("[DEBUG] Punto de desembarco en celda: [%d][%d]\n", tierraY, tierraX);

  // Colocar tropas una por celda de tierra cercana para evitar agua
  // Usar un buffer de celdas usadas cuyo tamaño se adapta a la cantidad real
  int maxColocables = barco->numTropas;
  if (maxColocables < 0) maxColocables = 0;
  if (maxColocables > 15) maxColocables = 15; // seguridad absoluta
  int colocadas = 0;
  int (*usados)[2] = NULL;
  if (maxColocables > 0) {
    usados = (int (*)[2])malloc(sizeof(int[2]) * (size_t)maxColocables);
    for (int u = 0; u < maxColocables; u++) {
      usados[u][0] = usados[u][1] = -1;
    }
  }

  for (int i = 0; i < barco->numTropas; i++) {
    Unidad *tropa = barco->tropas[i];
    if (!tropa)
      continue;

    bool puesta = false;
    // Búsqueda en radio creciente desde la celda de tierra encontrada
    for (int radio = 0; radio <= 3 && !puesta; radio++) {
      for (int dy = -radio; dy <= radio && !puesta; dy++) {
        for (int dx = -radio; dx <= radio; dx++) {
          int cx = tierraX + dx;
          int cy = tierraY + dy;
          if (cx < 0 || cy < 0 || cx >= GRID_SIZE || cy >= GRID_SIZE)
            continue;

          // Evitar reutilizar la misma celda
          bool yaUsada = false;
          if (usados) {
            for (int k = 0; k < colocadas; k++) {
              if (usados[k][0] == cx && usados[k][1] == cy) {
                yaUsada = true;
                break;
              }
            }
          }
          if (yaUsada)
            continue;

          int valor = *(*(col + cy) + cx);
          if (valor != 0)
            continue; // bloqueado por agua/obstáculo
          if (!mapaCeldaEsTierra(cy, cx))
            continue; // seguridad anti-agua

          // Asignar posición (esquina de celda)
          tropa->x = (float)(cx * TILE_SIZE);
          tropa->y = (float)(cy * TILE_SIZE);
          tropa->destinoX = tropa->x;
          tropa->destinoY = tropa->y;
          tropa->moviendose = false;
          tropa->seleccionado = true; // mostrar círculo al llegar a la isla

          if (usados && colocadas < maxColocables) {
            usados[colocadas][0] = cx;
            usados[colocadas][1] = cy;
          }
          colocadas++;
          printf(
              "[DEBUG] Tropa %d desembarcada en celda [%d][%d] (%.1f, %.1f)\n",
              i, cy, cx, tropa->x, tropa->y);
          puesta = true;
          if (colocadas >= maxColocables) {
            // No colocar más de las esperadas por seguridad
            break;
          }
        }
      }
    }
  }

  // Limpiar punteros usados en el barco y vaciarlo
  for (int i = 0; i < barco->numTropas; i++) {
    barco->tropas[i] = NULL;
  }
  barco->numTropas = 0;

  if (usados) {
    free(usados);
  }
}

// ============================================================================
// REINICIAR ISLA DESCONOCIDA
// ============================================================================
// Cuando el jugador llega a una nueva isla, esta es una isla "desconocida"
// sin edificios desarrollados, con recursos básicos, etc.
// Esta función resetea el estado del jugador para la nueva isla.
// ============================================================================
void reiniciarIslaDesconocida(struct Jugador *j) {
  printf("[DEBUG] Reiniciando isla desconocida...\n");

  // RECURSOS GLOBALES: Mantener los recursos actuales del jugador
  // j->Comida = 50; // Menos recursos que al inicio
  // j->Oro = 30;
  // j->Madera = 40;
  // j->Piedra = 30;

  // CRÍTICO: Eliminar edificios (la nueva isla no tiene edificios)
  j->ayuntamiento = NULL;
  j->mina = NULL;
  j->cuartel = NULL;

  // NOTA: Los personajes ya fueron desembarcados correctamente ANTES de llamar
  // esta función Por lo tanto, NO necesitamos hacer nada con ellos aquí Solo
  // las tropas desembarcadas existen en esta isla nueva

  printf("[DEBUG] Isla reiniciada (Recursos globales mantenidos): Oro=%d\n",
         j->Oro);
  printf("[DEBUG] Solo las tropas desembarcadas están disponibles\n");
}

// ============================================================================
// VIAJE DIRECTO A ISLA (SIN ANIMACIÓN)
// ============================================================================
// Cuando el jugador selecciona una isla después de embarcar tropas,
// viaja directamente sin animación del barco.
// ============================================================================

bool viajarAIsla(struct Jugador *j, int islaDestino) {
  printf("[DEBUG] Viajando directamente a isla %d\n", islaDestino);
  if (!sIslaInicialDefinida) {
    sIslaInicial = j->islaActual;
    sIslaInicialDefinida = true;
  }
  // Validación: no permitir llevar obreros a islas no conquistadas
  if (islaDestino != j->islaActual) {
    if (barcoTieneObreros(&j->barco) && !islaConquistada(islaDestino)) {
      MessageBox(NULL, "No puedes llevar obreros hasta conquistar la isla",
                 "Embarque restringido", MB_OK | MB_ICONWARNING);
      return false; // Cancelar viaje
    }
  }
  
  // Si es la misma isla, desembarcar y listo
  if (islaDestino == j->islaActual) {
    printf("[DEBUG] Ya estás en la isla %d, desembarcando tropas\n",
           islaDestino);
    desembarcarTropas(&j->barco, j);
    return true;
  }

  typedef struct {
    Unidad *ptr;
    Unidad copia;
  } TropaSnapshot;
  TropaSnapshot tropasEnRuta[15];
  int numTropasEnRuta = 0;
  for (int i = 0; i < 15; i++) {
    Unidad *t = j->barco.tropas[i];
    if (!t)
      continue;
    tropasEnRuta[numTropasEnRuta].ptr = t;
    tropasEnRuta[numTropasEnRuta].copia = *t;
    numTropasEnRuta++;
  }

  bool islaYaVisitada = (islaDestino >= 1 && islaDestino <= 3) &&
                        sIslas[islaDestino].inicializado;

  // Guardar estado de la isla actual antes de salir
  guardarEstadoIslaJugador(j);
  mapaGuardarEstadoIsla(j->islaActual);

  limpiarEnemigosActivos();

  // Cambiar isla activa
  j->islaActual = islaDestino;

  // Cambiar mapa de isla y recargar gráficos PRIMERO
  mapaSeleccionarIsla(islaDestino);
  mapaSetGenerarRecursos(true);
  cargarRecursosGraficos();

  EstadoIsla *estadoDestino = &sIslas[islaDestino];
  // Si la isla ya tiene estado guardado, restaurarlo
  if (islaYaVisitada) {
    mapaRestaurarEstadoIsla(islaDestino);
    restaurarEstadoIslaJugador(j, islaDestino);
    if (numTropasEnRuta > 0) {
      for (int i = 0; i < numTropasEnRuta; i++) {
        if (tropasEnRuta[i].ptr) {
          *(tropasEnRuta[i].ptr) = tropasEnRuta[i].copia;
        }
      }
    }
  } else {
    // Primera vez en la isla: resetear y generar base
    reiniciarIslaDesconocida(j);
    vaciarUnidades(j); // Solo tropas desembarcadas estarán presentes
    inicializarEstructurasIslaBase(j, estadoDestino);
    estadoDestino->inicializado = true;
    // Generar enemigos iniciales rodeando el ayuntamiento
    if (!estadoDestino->enemigosGenerados) {
      int maxEnemigos = 6; // base
      int minutos = (int)(GetTickCount64() / 60000ULL);
      if (minutos > 0) maxEnemigos += (minutos % 3); // ligera escala
      if (maxEnemigos > 8) maxEnemigos = 8;

      estadoDestino->numEnemigos = maxEnemigos;
      float baseX = estadoDestino->ayuntamiento.x + 64.0f;
      float baseY = estadoDestino->ayuntamiento.y + 64.0f;
      // Colocar en círculo alrededor del ayuntamiento
      for (int i = 0; i < maxEnemigos; i++) {
        Unidad *e = &estadoDestino->enemigos[i];
        float ang = (float)(i * (360.0 / maxEnemigos));
        float rad = 300.0f;
        float ex = baseX + rad * cosf(ang * (3.1415926f / 180.0f));
        float ey = baseY + rad * sinf(ang * (3.1415926f / 180.0f));
        e->x = ex;
        e->y = ey;
        statsBasicosEnemigo(e, (i % 2 == 0) ? TIPO_CABALLERO : TIPO_GUERRERO);
      }
      estadoDestino->enemigosGenerados = true;
    }
    activarEnemigosDesdeEstado(estadoDestino);
    guardarEstadoIslaJugador(j); // Guardar snapshot inicial
    mapaGuardarEstadoIsla(islaDestino);
  }

  // Posicionar barco en orilla de la nueva isla (posiciones fijas por isla)
  float nuevoBarcoX, nuevoBarcoY;
  int nuevoDir;
  obtenerPosicionBarcoIsla(islaDestino, &nuevoBarcoX, &nuevoBarcoY, &nuevoDir);
  j->barco.x = nuevoBarcoX;
  j->barco.y = nuevoBarcoY;
  j->barco.dir = (Direccion)nuevoDir;

  printf("[DEBUG] Barco reposicionado en isla %d: (%.1f, %.1f)\n", islaDestino,
         j->barco.x, j->barco.y);

  // Desembarcar SOLO tropas que venían en el barco
  desembarcarTropas(&j->barco, j);

  printf("[DEBUG] Viaje completado; enemigos pasivos listos\n");
  return true;
}
