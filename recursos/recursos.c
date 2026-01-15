#include "recursos.h"
#include "navegacion.h" // Incluir para navegacionContarUnidadesGlobal
#include "../edificios/edificios.h"
#include "../mapa/mapa.h"
#include "../mapa/menu.h"
#include "stdbool.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

// --- Animaciones Lógicas ---
static const Animation gAnimFront = {DIR_FRONT, 4, 6};
static const Animation gAnimBack = {DIR_BACK, 4, 6};
static const Animation gAnimLeft = {DIR_LEFT, 4, 6};
static const Animation gAnimRight = {DIR_RIGHT, 4, 6};

static const Animation *animPorDireccion(Direccion d) {
  switch (d) {
  case DIR_BACK:
    return &gAnimBack;
  case DIR_LEFT:
    return &gAnimLeft;
  case DIR_RIGHT:
    return &gAnimRight;
  case DIR_FRONT:
  default:
    return &gAnimFront;
  }
}

// --- Utilidades de Grid ---
static int clampInt(int v, int lo, int hi) {
  if (v < lo)
    return lo;
  if (v > hi)
    return hi;
  return v;
}

static int pixelACelda(float px) {
  int c = (int)(px / (float)TILE_SIZE);
  int resultado = clampInt(c, 0, GRID_SIZE - 1);

  // DEBUG: Verificar conversión (comentar después de confirmar)
  // printf("[DEBUG pixelACelda] px=%.2f -> c=%d -> resultado=%d
  // (TILE_SIZE=%d)\n",
  //        px, c, resultado, TILE_SIZE);

  return resultado;
}

static float celdaCentroPixel(int celda) {
  return (float)(celda * TILE_SIZE) + (float)(TILE_SIZE / 2);
}

static int obreroFilaActual(const Unidad *o) {
  return pixelACelda(o->y +
                     (TILE_SIZE / 2)); // Centro de la celda, no hardcoded 32
}

static int obreroColActual(const Unidad *o) {
  return pixelACelda(o->x +
                     (TILE_SIZE / 2)); // Centro de la celda, no hardcoded 32
}
// Marcar/desmarcar una ÚNICA CELDA en el collision map (1x1, no 2x2)
static void marcarHuellaObrero(int **collision, int fila, int col, int valor) {
  if (!collision || fila < 0 || col < 0 || fila >= GRID_SIZE ||
      col >= GRID_SIZE)
    return;

  // Marcar solo UNA celda usando punteros
  int *fila_ptr = *(collision + fila);
  *(fila_ptr + col) = valor;
}

// Ahora esta función ya conoce a la anterior
static void ocupacionActualizarUnidad(int **collision, Unidad *o, int nuevaF,
                                      int nuevaC) {
  if (!collision)
    return;
  if (o->celdaFila >= 0 && o->celdaCol >= 0) {
    marcarHuellaObrero(collision, o->celdaFila, o->celdaCol, 0);
  }
  o->celdaFila = nuevaF;
  o->celdaCol = nuevaC;
  marcarHuellaObrero(collision, nuevaF, nuevaC, 3);
}

static void obreroLiberarRuta(Unidad *o) {
  if (o->rutaCeldas)
    free(o->rutaCeldas);
  o->rutaCeldas = NULL;
  o->rutaLen = 0;
  o->rutaIdx = 0;
}

// ============================================================================
// PATHFINDING BFS - RUTA MÁS CORTA GARANTIZADA
// ============================================================================
// Implementación de Breadth-First Search usando SOLO arrays básicos.
// Garantiza encontrar la ruta más corta explorando nivel por nivel.
// NO usa colas, heaps, ni estructuras avanzadas - solo arrays planos.
// ============================================================================
static bool pathfindSimple(int startF, int startC, int goalF, int goalC,
                           int **collision, int **outRuta, int *outLen) {
  if (startF == goalF && startC == goalC)
    return false;

  const int TOTAL_CELDAS = GRID_SIZE * GRID_SIZE;

  // Arrays para BFS - solo arrays básicos, sin estructuras avanzadas
  int *padreIdx =
      (int *)malloc(TOTAL_CELDAS * sizeof(int)); // De donde vino cada celda
  int *bufferBFS =
      (int *)malloc(TOTAL_CELDAS * sizeof(int)); // Buffer de exploración

  if (!padreIdx || !bufferBFS) {
    if (padreIdx)
      free(padreIdx);
    if (bufferBFS)
      free(bufferBFS);
    return false;
  }

  // Inicializar: -1 significa no visitado
  for (int i = 0; i < TOTAL_CELDAS; i++) {
    *(padreIdx + i) = -1;
  }

  // Direcciones: arriba, abajo, izquierda, derecha
  int dF[4] = {-1, 1, 0, 0};
  int dC[4] = {0, 0, -1, 1};

  // BFS usando dos índices sobre el buffer (simula cola sin estructuras)
  int inicioBuffer = 0; // Índice de lectura
  int finBuffer = 0;    // Índice de escritura

  // Agregar posición inicial al buffer
  int startIdx = startF * GRID_SIZE + startC;
  int goalIdx = goalF * GRID_SIZE + goalC;

  *(bufferBFS + finBuffer) = startIdx;
  finBuffer++;
  *(padreIdx + startIdx) =
      startIdx; // Se apunta a sí mismo (marca como visitado)

  bool encontrado = false;

  // Explorar nivel por nivel (BFS garantiza ruta más corta)
  while (inicioBuffer < finBuffer) {
    // Obtener siguiente celda a procesar
    int actualIdx = *(bufferBFS + inicioBuffer);
    inicioBuffer++;

    int actualF = actualIdx / GRID_SIZE;
    int actualC = actualIdx % GRID_SIZE;

    // ¿Llegamos al objetivo?
    if (actualIdx == goalIdx) {
      encontrado = true;
      break;
    }

    // Explorar los 4 vecinos
    for (int i = 0; i < 4; i++) {
      int nf = actualF + dF[i];
      int nc = actualC + dC[i];

      // Validar límites del mapa
      if (nf < 0 || nf >= GRID_SIZE || nc < 0 || nc >= GRID_SIZE)
        continue;

      int neighborIdx = nf * GRID_SIZE + nc;

      // Saltar si ya visitado (padreIdx != -1)
      if (*(padreIdx + neighborIdx) != -1)
        continue;

      // Verificar colisión usando aritmética de punteros
      int *fila_ptr = *(collision + nf);
      int valor = *(fila_ptr + nc);

      // Bloqueado por agua/árbol (1) o edificio (2) - NO pasar
      if (valor == 1 || valor == 2)
        continue;

      // Marcar como visitado y agregar al buffer
      *(padreIdx + neighborIdx) = actualIdx;
      *(bufferBFS + finBuffer) = neighborIdx;
      finBuffer++;
    }
  }

  free(bufferBFS);

  if (!encontrado) {
    free(padreIdx);
    return false;
  }

  // Contar longitud de la ruta reconstruyendo hacia atrás
  int pathLen = 0;
  int idx = goalIdx;
  while (idx != startIdx) {
    pathLen++;
    idx = *(padreIdx + idx);
    if (pathLen > TOTAL_CELDAS) { // Protección contra ciclos
      free(padreIdx);
      return false;
    }
  }

  if (pathLen == 0) {
    free(padreIdx);
    return false;
  }

  // Crear array de ruta
  int *rutaFinal = (int *)malloc(pathLen * sizeof(int));
  if (!rutaFinal) {
    free(padreIdx);
    return false;
  }

  // Llenar ruta en orden correcto (inicio -> fin)
  idx = goalIdx;
  for (int i = pathLen - 1; i >= 0; i--) {
    *(rutaFinal + i) = idx;
    idx = *(padreIdx + idx);
  }

  free(padreIdx);

  *outRuta = rutaFinal;
  *outLen = pathLen;
  return true;
}

void IniciacionRecursos(struct Jugador *j, const char *Nombre) {
  strcpy(j->Nombre, Nombre);
  j->Comida = 200;
  j->Oro = 0;
  j->Madera = 150;
  j->Piedra = 100;
  j->Hierro = 100;
  j->CantidadEspadas = 0;
  for (int i = 0; i < MAX_OBREROS; i++) {
    if (i < 6) {
        j->obreros[i].x = 900.0f + (i * 64.0f);
        j->obreros[i].y = 900.0f;
    } else {
        j->obreros[i].x = -1000.0f;
        j->obreros[i].y = -1000.0f;
    }
    j->obreros[i].destinoX = j->obreros[i].x;
    j->obreros[i].destinoY = j->obreros[i].y;
    j->obreros[i].moviendose = false;
    j->obreros[i].seleccionado = false;
    j->obreros[i].celdaFila = -1;
    j->obreros[i].celdaCol = -1;
    j->obreros[i].rutaCeldas = NULL;
    j->obreros[i].tipo = TIPO_OBRERO; // Asignar tipo
    j->obreros[i].animActual = animPorDireccion(DIR_FRONT);
    // Inicializar vida para mostrar barra de salud desde el inicio
    j->obreros[i].vidaMax = OBRERO_VIDA_MAX;
    j->obreros[i].vida = OBRERO_VIDA_MAX;
    j->obreros[i].recibiendoAtaque = false;
  }

  // No generar guerreros ni caballeros al inicio
  for (int i = 0; i < MAX_CABALLEROS; i++) {
    j->caballeros[i].x = -1000.0f;
    j->caballeros[i].y = -1000.0f;
    j->caballeros[i].destinoX = j->caballeros[i].x;
    j->caballeros[i].destinoY = j->caballeros[i].y;
    j->caballeros[i].moviendose = false;
    j->caballeros[i].seleccionado = false;
    j->caballeros[i].celdaFila = -1;
    j->caballeros[i].celdaCol = -1;
    j->caballeros[i].rutaCeldas = NULL;
    j->caballeros[i].tipo = TIPO_CABALLERO;
    j->caballeros[i].recibiendoAtaque = false;
  }

  for (int i = 0; i < MAX_CABALLEROS_SIN_ESCUDO; i++) {
    j->caballerosSinEscudo[i].x = -1000.0f;
    j->caballerosSinEscudo[i].y = -1000.0f;
    j->caballerosSinEscudo[i].destinoX = j->caballerosSinEscudo[i].x;
    j->caballerosSinEscudo[i].destinoY = j->caballerosSinEscudo[i].y;
    j->caballerosSinEscudo[i].moviendose = false;
    j->caballerosSinEscudo[i].seleccionado = false;
    j->caballerosSinEscudo[i].celdaFila = -1;
    j->caballerosSinEscudo[i].celdaCol = -1;
    j->caballerosSinEscudo[i].rutaCeldas = NULL;
    j->caballerosSinEscudo[i].tipo = TIPO_CABALLERO_SIN_ESCUDO;
    j->caballerosSinEscudo[i].recibiendoAtaque = false;
  }
  
  // Inicializar estado de conquista
  for(int i=0; i<6; i++) j->islasConquistadas[i] = false;


  for (int i = 0; i < MAX_GUERREROS; i++) {
    j->guerreros[i].x = -1000.0f;
    j->guerreros[i].y = -1000.0f;
    j->guerreros[i].destinoX = j->guerreros[i].x;
    j->guerreros[i].destinoY = j->guerreros[i].y;
    j->guerreros[i].moviendose = false;
    j->guerreros[i].seleccionado = false;
    j->guerreros[i].celdaFila = -1;
    j->guerreros[i].celdaCol = -1;
    j->guerreros[i].rutaCeldas = NULL;
    j->guerreros[i].tipo = TIPO_GUERRERO;
    j->guerreros[i].recibiendoAtaque = false;
  }

  // ================================================================
  // INICIALIZAR BARCO EN LA ORILLA (192x192px)
  // ================================================================
  j->barco.activo = false; // Se activará después de detectar la orilla
  j->barco.x = 0.0f;
  j->barco.y = 0.0f;
  j->barco.dir = DIR_FRONT;
  j->barco.numTropas = 0;

  // Inicializar sistema de mejoras (nivel 1 por defecto)
  j->barco.nivelMejora = 1;
  j->barco.capacidadMaxima = 6;
  j->barco.construido = false; // Barco comienza destruido

  printf("[DEBUG] Barco inicializado (pendiente de colocacion en orilla)\\n");

  // ================================================================
  // REGISTRAR TODOS LOS OBJETOS EN mapaObjetos
  // ================================================================
  printf("[DEBUG] Registrando objetos en mapaObjetos...\n");

  // Registrar obreros
  for (int i = 0; i < MAX_OBREROS; i++) {
    if (j->obreros[i].x >= 0) {
        mapaRegistrarObjeto(j->obreros[i].x, j->obreros[i].y, SIMBOLO_OBRERO);
    }
  }
  printf("[DEBUG] %d obreros registrados en matriz\n", 6);

  printf("[DEBUG] %d caballeros registrados en matriz\n", 4);

  // Registrar caballeros sin escudo
  for (int i = 0; i < MAX_CABALLEROS_SIN_ESCUDO; i++) {
    if (j->caballerosSinEscudo[i].x >= 0 && j->caballerosSinEscudo[i].y >= 0) {
      mapaRegistrarObjeto(j->caballerosSinEscudo[i].x,
                          j->caballerosSinEscudo[i].y, SIMBOLO_CABALLERO);
    }
  }
  printf("[DEBUG] %d caballeros sin escudo registrados en matriz\n", 4);

  // Registrar guerreros
  for (int i = 0; i < MAX_GUERREROS; i++) {
    if (j->guerreros[i].x >= 0 && j->guerreros[i].y >= 0) {
      mapaRegistrarObjeto(j->guerreros[i].x, j->guerreros[i].y,
                          SIMBOLO_GUERRERO);
    }
  }
  printf("[DEBUG] %d guerreros registrados en matriz\n", 2);
}

void actualizarPersonajes(struct Jugador *j) {
  const float vel = 5.0f; // Velocidad de movimiento en píxeles/frame
  const float umbralLlegada =
      8.0f; // Umbral para considerar que llegó a la celda (más fluido)
  int **col = mapaObtenerCollisionMap();
  if (!col)
    return;

  // Obreros
  for (int i = 0; i < MAX_OBREROS; i++) {
    Unidad *o = &j->obreros[i];

    // 1. Sincronización inicial de la huella en la matriz (2x2 celdas)
    if (o->celdaFila == -1) {
      ocupacionActualizarUnidad(col, o, obreroFilaActual(o),
                                obreroColActual(o));
    }

    if (!o->moviendose)
      continue;

    // 2. Obtener la siguiente celda de la ruta
    int nextF, nextC;
    if (o->rutaCeldas && o->rutaIdx < o->rutaLen) {
      int targetCelda = o->rutaCeldas[o->rutaIdx];
      nextF = targetCelda / GRID_SIZE;
      nextC = targetCelda % GRID_SIZE;

      // DEBUG: Ver qué celda estamos decodificando
      if (o->rutaIdx == 0) { // Solo primera celda de la ruta
        printf("[DEBUG RUTA] Obrero %d: targetCelda=%d -> nextF=%d, nextC=%d\n",
               i, targetCelda, nextF, nextC);
      }
    } else {
      o->moviendose = false;
      continue;
    }

    // 3. VALIDACION DE CELDA DESTINO (1x1 - UNA SOLA CELDA)
    // Revisamos si la celda destino esta libre
    bool bloqueadoPermanente = false; // Agua/Arbol/Edificio (cancelar ruta)
    bool bloqueadoTemporal = false;   // Otro personaje en celda FINAL (esperar)

    // Verificar limites del mapa
    if (nextF >= GRID_SIZE || nextC >= GRID_SIZE) {
      bloqueadoPermanente = true;
    } else {
      // Aritmetica de punteros para obtener el valor de la celda
      int valor = *(*(col + nextF) + nextC);

      // Bloqueado PERMANENTEMENTE si es Agua/Arbol (1) o Edificio (2)
      if (valor == 1 || valor == 2) {
        bloqueadoPermanente = true;
      }

      // Bloqueo TEMPORAL: Solo si la celda es la ULTIMA de la ruta (destino
      // final) y hay otro personaje (valor == 3). Permite pasar por al lado
      // durante transito.
      bool esUltimaCelda = (o->rutaIdx + 1 >= o->rutaLen);
      if (esUltimaCelda && valor == 3 &&
          (nextF != o->celdaFila || nextC != o->celdaCol)) {
        bloqueadoTemporal = true;
      }
    }

    // Si hay obstaculo PERMANENTE (agua/edificio), cancelar movimiento
    if (bloqueadoPermanente) {
      o->moviendose = false;
      obreroLiberarRuta(o);
      continue;
    }

    // Si hay otro personaje en la CELDA FINAL, ESPERAR sin cancelar ruta
    // El personaje mantiene su ruta y volvera a intentar en el siguiente frame
    if (bloqueadoTemporal) {
      continue; // Esperar, NO cancelar la ruta
    }

    // 4. Cálculo de movimiento hacia el centro de la celda destino
    float tx = celdaCentroPixel(nextC), ty = celdaCentroPixel(nextF);
    float cx = o->x + (TILE_SIZE / 2),
          cy = o->y + (TILE_SIZE / 2); // Centro actual del obrero
    float vx = tx - cx, vy = ty - cy;
    float dist = sqrtf(vx * vx + vy * vy);

    // 5. Animación y Dirección según el vector de movimiento
    if (dist > 0.1f) {
      if (fabsf(vx) > fabsf(vy))
        o->dir = (vx > 0) ? DIR_RIGHT : DIR_LEFT;
      else
        o->dir = (vy > 0) ? DIR_FRONT : DIR_BACK;

      o->animActual = animPorDireccion(o->dir);
      o->animTick++;
      if (o->animTick >= o->animActual->ticksPerFrame) {
        o->animTick = 0;
        o->frame = (o->frame + 1) % o->animActual->frameCount;
      }
    }

    //  6. Aplicar desplazamiento o finalizar llegada a la celda
    if (dist <= umbralLlegada) {
      // Guarda posición anterior
      float viejoX = o->x;
      float viejoY = o->y;

      // Llegó al centro de la celda: se posiciona y actualiza su ocupación en
      // la matriz maestra
      o->x = tx - (TILE_SIZE / 2);
      o->y = ty - (TILE_SIZE / 2);
      o->rutaIdx++;

      // Actualizar la huella 2x2 en la matriz de colisiones
      ocupacionActualizarUnidad(col, o, nextF, nextC);

      // NUEVO: Actualizar mapaObjetos
      mapaMoverObjeto(viejoX, viejoY, o->x, o->y, SIMBOLO_OBRERO);

      if (o->rutaIdx >= o->rutaLen) {
        o->moviendose = false;
        obreroLiberarRuta(o);
      }
    } else {
      // Guarda posición anterior
      float viejoX = o->x;
      float viejoY = o->y;

      // Moverse suavemente hacia el objetivo
      float newX = o->x + (vx / dist) * vel;
      float newY = o->y + (vy / dist) * vel;

      // Límites físicos de la isla (2048x2048)
      if (newX < 0)
        newX = 0;
      if (newY < 0)
        newY = 0;
      if (newX > (float)(MAPA_SIZE - 64))
        newX = (float)(MAPA_SIZE - 64);
      if (newY > (float)(MAPA_SIZE - 64))
        newY = (float)(MAPA_SIZE - 64);

      o->x = newX;
      o->y = newY;

      // NUEVO: Actualizar mapaObjetos durante movimiento suave
      mapaMoverObjeto(viejoX, viejoY, o->x, o->y, SIMBOLO_OBRERO);
    }
  }

  // ================================================================
  // ACTUALIZAR CABALLEROS (misma lógica)
  // ================================================================
  for (int i = 0; i < MAX_CABALLEROS; i++) {
    Unidad *u = &j->caballeros[i];

    // Misma lógica que obreros
    if (u->celdaFila == -1) {
      ocupacionActualizarUnidad(col, u, obreroFilaActual(u),
                                obreroColActual(u));
    }

    if (!u->moviendose)
      continue;

    int nextF, nextC;
    if (u->rutaCeldas && u->rutaIdx < u->rutaLen) {
      int targetCelda = u->rutaCeldas[u->rutaIdx];
      nextF = targetCelda / GRID_SIZE;
      nextC = targetCelda % GRID_SIZE;
    } else {
      u->moviendose = false;
      continue;
    }

    bool bloqueadoPermanente = false;
    bool bloqueadoTemporal = false;

    // Verificar límites del mapa
    if (nextF >= GRID_SIZE || nextC >= GRID_SIZE) {
      bloqueadoPermanente = true;
    } else {
      int valor = *(*(col + nextF) + nextC);

      if (valor == 1 || valor == 2) {
        bloqueadoPermanente = true;
      }

      // Bloqueo TEMPORAL: Solo en la celda FINAL de la ruta (destino)
      bool esUltimaCelda = (u->rutaIdx + 1 >= u->rutaLen);
      if (esUltimaCelda && valor == 3 &&
          (nextF != u->celdaFila || nextC != u->celdaCol)) {
        bloqueadoTemporal = true;
      }
    }

    if (bloqueadoPermanente) {
      u->moviendose = false;
      obreroLiberarRuta(u);
      continue;
    }

    if (bloqueadoTemporal) {
      continue;
    }

    float tx = celdaCentroPixel(nextC), ty = celdaCentroPixel(nextF);
    float cx = u->x + (TILE_SIZE / 2), cy = u->y + (TILE_SIZE / 2);
    float vx = tx - cx, vy = ty - cy;
    float dist = sqrtf(vx * vx + vy * vy);

    if (dist > 0.1f) {
      if (fabsf(vx) > fabsf(vy))
        u->dir = (vx > 0) ? DIR_RIGHT : DIR_LEFT;
      else
        u->dir = (vy > 0) ? DIR_FRONT : DIR_BACK;

      u->animActual = animPorDireccion(u->dir);
      u->animTick++;
      if (u->animTick >= u->animActual->ticksPerFrame) {
        u->animTick = 0;
        u->frame = (u->frame + 1) % u->animActual->frameCount;
      }
    }

    if (dist <= umbralLlegada) {
      float viejoX = u->x;
      float viejoY = u->y;

      u->x = tx - (TILE_SIZE / 2);
      u->y = ty - (TILE_SIZE / 2);
      u->rutaIdx++;

      ocupacionActualizarUnidad(col, u, nextF, nextC);
      mapaMoverObjeto(viejoX, viejoY, u->x, u->y, SIMBOLO_CABALLERO);

      if (u->rutaIdx >= u->rutaLen) {
        u->moviendose = false;
        obreroLiberarRuta(u);
      }
    } else {
      float viejoX = u->x;
      float viejoY = u->y;

      float newX = u->x + (vx / dist) * vel;
      float newY = u->y + (vy / dist) * vel;

      if (newX < 0)
        newX = 0;
      if (newY < 0)
        newY = 0;
      if (newX > (float)(MAPA_SIZE - 64))
        newX = (float)(MAPA_SIZE - 64);
      if (newY > (float)(MAPA_SIZE - 64))
        newY = (float)(MAPA_SIZE - 64);

      u->x = newX;
      u->y = newY;
      mapaMoverObjeto(viejoX, viejoY, u->x, u->y, SIMBOLO_CABALLERO);
    }
  }

  for (int i = 0; i < MAX_CABALLEROS_SIN_ESCUDO; i++) {
    Unidad *u = &j->caballerosSinEscudo[i];
    if (u->x < 0)
      continue;
    if (u->celdaFila == -1)
      ocupacionActualizarUnidad(col, u, obreroFilaActual(u),
                                obreroColActual(u));
    if (!u->moviendose)
      continue;

    if (!u->rutaCeldas || u->rutaIdx < 0 || u->rutaIdx >= u->rutaLen) {
      // Batalla puede marcar moviendose sin path (persecucion manual)
      u->moviendose = false;
      continue;
    }

    int target = u->rutaCeldas[u->rutaIdx];
    int nF = target / GRID_SIZE, nC = target % GRID_SIZE;

    // Bloqueo TEMPORAL: Solo en la celda FINAL de la ruta (destino)
    bool esUltimaCelda = (u->rutaIdx + 1 >= u->rutaLen);
    int valorCelda = *(*(col + nF) + nC);
    if (esUltimaCelda && valorCelda == 3 &&
        (nF != u->celdaFila || nC != u->celdaCol)) {
      continue; // Esperar, otro personaje ocupa la celda destino
    }

    float tx = celdaCentroPixel(nC), ty = celdaCentroPixel(nF);
    float vx = tx - (u->x + 32), vy = ty - (u->y + 32);
    float d = sqrtf(vx * vx + vy * vy);
    if (d > 0.1f) {
      if (fabsf(vx) > fabsf(vy))
        u->dir = (vx > 0) ? DIR_RIGHT : DIR_LEFT;
      else
        u->dir = (vy > 0) ? DIR_FRONT : DIR_BACK;
      u->animActual = animPorDireccion(u->dir);
      u->animTick++;
      if (u->animTick >= u->animActual->ticksPerFrame) {
        u->animTick = 0;
        u->frame = (u->frame + 1) % u->animActual->frameCount;
      }
      u->x += (vx / d) * vel;
      u->y += (vy / d) * vel;
      int nf = obreroFilaActual(u), nc = obreroColActual(u);
      if (nf != u->celdaFila || nc != u->celdaCol)
        ocupacionActualizarUnidad(col, u, nf, nc);
      mapaMoverObjeto(u->x - (vx / d) * vel, u->y - (vy / d) * vel, u->x, u->y,
                      SIMBOLO_CABALLERO);
      if (d < umbralLlegada) {
        u->rutaIdx++;
        if (u->rutaIdx >= u->rutaLen) {
          u->moviendose = false;
          obreroLiberarRuta(u);
        }
      }
    }
  }

  for (int i = 0; i < MAX_GUERREROS; i++) {
    Unidad *u = &j->guerreros[i];

    // Sincronización inicial
    if (u->celdaFila == -1) {
      ocupacionActualizarUnidad(col, u, obreroFilaActual(u),
                                obreroColActual(u));
    }
    if (!u->moviendose)
      continue;

    int nextF, nextC;
    if (u->rutaCeldas && u->rutaIdx < u->rutaLen) {
      int targetCelda = u->rutaCeldas[u->rutaIdx];
      nextF = targetCelda / GRID_SIZE;
      nextC = targetCelda % GRID_SIZE;
    } else {
      u->moviendose = false;
      continue;
    }

    bool bloqueadoPermanente = false;
    bool bloqueadoTemporal = false;

    // Verificar límites del mapa
    if (nextF >= GRID_SIZE || nextC >= GRID_SIZE) {
      bloqueadoPermanente = true;
    } else {
      int valor = *(*(col + nextF) + nextC);

      if (valor == 1 || valor == 2) {
        bloqueadoPermanente = true;
      }

      // Bloqueo TEMPORAL: Solo en la celda FINAL de la ruta (destino)
      bool esUltimaCelda = (u->rutaIdx + 1 >= u->rutaLen);
      if (esUltimaCelda && valor == 3 &&
          (nextF != u->celdaFila || nextC != u->celdaCol)) {
        bloqueadoTemporal = true;
      }
    }

    if (bloqueadoPermanente) {
      u->moviendose = false;
      obreroLiberarRuta(u);
      continue;
    }

    if (bloqueadoTemporal) {
      continue;
    }

    float tx = celdaCentroPixel(nextC), ty = celdaCentroPixel(nextF);
    float cx = u->x + (TILE_SIZE / 2), cy = u->y + (TILE_SIZE / 2);
    float vx = tx - cx, vy = ty - cy;
    float dist = sqrtf(vx * vx + vy * vy);

    if (dist > 0.1f) {
      if (fabsf(vx) > fabsf(vy))
        u->dir = (vx > 0) ? DIR_RIGHT : DIR_LEFT;
      else
        u->dir = (vy > 0) ? DIR_FRONT : DIR_BACK;

      u->animActual = animPorDireccion(u->dir);
      u->animTick++;
      if (u->animTick >= u->animActual->ticksPerFrame) {
        u->animTick = 0;
        u->frame = (u->frame + 1) % u->animActual->frameCount;
      }
    }

    if (dist <= umbralLlegada) {
      float viejoX = u->x;
      float viejoY = u->y;

      u->x = tx - (TILE_SIZE / 2);
      u->y = ty - (TILE_SIZE / 2);
      u->rutaIdx++;

      ocupacionActualizarUnidad(col, u, nextF, nextC);
      mapaMoverObjeto(viejoX, viejoY, u->x, u->y, SIMBOLO_GUERRERO);

      if (u->rutaIdx >= u->rutaLen) {
        u->moviendose = false;
        obreroLiberarRuta(u);
      }
    } else {
      float viejoX = u->x;
      float viejoY = u->y;

      float newX = u->x + (vx / dist) * vel;
      float newY = u->y + (vy / dist) * vel;

      if (newX < 0)
        newX = 0;
      if (newY < 0)
        newY = 0;
      if (newX > (float)(MAPA_SIZE - 64))
        newX = (float)(MAPA_SIZE - 64);
      if (newY > (float)(MAPA_SIZE - 64))
        newY = (float)(MAPA_SIZE - 64);

      u->x = newX;
      u->y = newY;
      mapaMoverObjeto(viejoX, viejoY, u->x, u->y, SIMBOLO_GUERRERO);
    }
  }
}
// ============================================================================
// COMANDAR MOVIMIENTO RTS CON SEPARACIÓN DE UNIDADES
// ============================================================================
// - Si el destino es impasable (agua/árbol = 1), buscar celda libre cercana.
// - Si el destino está ocupado por otra unidad (= 2), buscar celda adyacente.
// - Múltiples unidades seleccionadas reciben destinos diferentes para no
// solaparse.
// ============================================================================
void rtsComandarMovimiento(struct Jugador *j, float mundoX, float mundoY) {
  int **col = mapaObtenerCollisionMap();
  if (!col)
    return;

  // ================================================================
  // DIAGNÓSTICO PASO 1: Verificar coordenadas recibidas
  // ================================================================
  printf("\n[DEBUG] ===== NUEVO COMANDO DE MOVIMIENTO =====\n");
  printf("[DEBUG] Click en coordenadas mundo: (%.2f, %.2f)\n", mundoX, mundoY);
  fflush(stdout);

  // ================================================================
  // VALIDACIÓN CRÍTICA #1: FILTRO DE DESTINO ANTES DE PATHFINDING
  // ================================================================
  // Convertir coordenadas del clic a índices de la matriz
  // IMPORTANTE: Usar la MISMA conversión que el hover (división simple)
  int gF = (int)(mundoY / TILE_SIZE);
  int gC = (int)(mundoX / TILE_SIZE);

  // DIAGNÓSTICO PASO 2: Mostrar conversión
  printf("[DEBUG] Conversion a matriz: gF=%d, gC=%d\n", gF, gC);
  fflush(stdout);

  if (gF < 0 || gF >= GRID_SIZE || gC < 0 || gC >= GRID_SIZE) {
    printf("[DEBUG] RECHAZADO: Fuera de limites (gF=%d, gC=%d, GRID_SIZE=%d)\n",
           gF, gC, GRID_SIZE);
    fflush(stdout);
    return; // Fuera de límites
  }

  // DIAGNÓSTICO PASO 3: Leer valor de la celda destino
  printf("[DEBUG] Inspeccionando celda destino [%d][%d]:\n", gF, gC);

  // ================================================================
  // VALIDACIÓN DE CELDA DESTINO (1x1 - NO 2x2)
  // ================================================================
  // El jugador debe ser "sordo" a órdenes sobre agua/obstáculos.
  // Validamos que LA CELDA esté libre de agua/árboles (1) y edificios (2).
  // Los personajes (3) NO bloquean las órdenes de movimiento.
  // ================================================================
  bool destinoBloqueado = false;
  int motivoBloqueo = -1;

  int *fila_ptr = *(col + gF);
  int valor = *(fila_ptr + gC);
  printf("[DEBUG]   Celda[%d][%d] = %d\n", gF, gC, valor);

  // Si hay Agua/Árbol (1) o Edificio (2), intentar buscar una celda libre
  // cercana para permitir interactuar o acercarse.
  if (valor == 1 || valor == 2) {
    printf("[DEBUG] Destino inicial bloqueado (%d), buscando celda libre "
           "adyacente...\n",
           valor);
    bool encontrado = false;
    for (int r = 1; r <= 3 && !encontrado; r++) {
      for (int dy = -r; dy <= r && !encontrado; dy++) {
        for (int dx = -r; dx <= r && !encontrado; dx++) {
          int nf = gF + dy;
          int nc = gC + dx;
          if (nf >= 0 && nf < GRID_SIZE && nc >= 0 && nc < GRID_SIZE) {
            if (col[nf][nc] == 0) {
              gF = nf;
              gC = nc;
              encontrado = true;
              printf("[DEBUG] Reubicado a celda libre cercana: [%d][%d]\n", gF,
                     gC);
            }
          }
        }
      }
    }

    if (!encontrado) {
      printf("[DEBUG] ********************************************\n");
      printf("[DEBUG] ORDEN RECHAZADA: No hay celdas libres cerca de (F:%d, "
             "C:%d).\n",
             gF, gC);
      printf("[DEBUG] ********************************************\n");
      fflush(stdout);
      return;
    }
  }

  // Si llegamos aquí, el destino ES VÁLIDO (o reubicado)
  printf("[DEBUG] Destino VALIDO: Procediendo a calcular pathfinding...\n");
  fflush(stdout);

  // ================================================================
  // SEPARACIÓN DE DESTINOS: Cada unidad recibe un destino diferente
  // ================================================================
  // Array para marcar celdas ya asignadas como destino (máximo 18 unidades)
  int destinosAsignados[18][2]; // [fila, col] de cada destino asignado
  int numDestinos = 0;

// Función inline para verificar si una celda ya fue asignada
#define CELDA_YA_ASIGNADA(f, c)                                                \
  ({                                                                           \
    int _asignada = 0;                                                         \
    for (int _i = 0; _i < numDestinos; _i++) {                                 \
      if (destinosAsignados[_i][0] == (f) &&                                   \
          destinosAsignados[_i][1] == (c)) {                                   \
        _asignada = 1;                                                         \
        break;                                                                 \
      }                                                                        \
    }                                                                          \
    _asignada;                                                                 \
  })

  // Puntero base al array de obreros (aritmética de punteros)
  Unidad *base = j->obreros;

  printf("[DEBUG] Buscando obreros seleccionados...\n");
  fflush(stdout);

  // Recorrer todas las unidades usando aritmética de punteros
  for (Unidad *o = base; o < base + MAX_OBREROS; o++) {
    int idx = (int)(o - base);
    printf("[DEBUG] Obrero %d: seleccionado=%d\n", idx, o->seleccionado);
    fflush(stdout);

    if (!o->seleccionado || o->vida <= 0)
      continue;

    // Buscar destino libre (el original o una celda adyacente)
    int destinoF = gF;
    int destinoC = gC;

    // Si la celda destino ya fue asignada a otra unidad, buscar una adyacente
    if (CELDA_YA_ASIGNADA(destinoF, destinoC) ||
        (*(*(col + destinoF) + destinoC) == 3)) {
      bool encontrado = false;
      for (int r = 1; r <= 3 && !encontrado; r++) {
        for (int dy = -r; dy <= r && !encontrado; dy++) {
          for (int dx = -r; dx <= r && !encontrado; dx++) {
            int nf = gF + dy;
            int nc = gC + dx;
            if (nf >= 0 && nf < GRID_SIZE && nc >= 0 && nc < GRID_SIZE) {
              int valorCelda = *(*(col + nf) + nc);
              // Solo aceptar celdas vacías (0) que no hayan sido asignadas
              if (valorCelda == 0 && !CELDA_YA_ASIGNADA(nf, nc)) {
                destinoF = nf;
                destinoC = nc;
                encontrado = true;
              }
            }
          }
        }
      }
      if (!encontrado)
        continue; // No hay celda libre, saltar esta unidad
    }

    // Registrar este destino como asignado
    destinosAsignados[numDestinos][0] = destinoF;
    destinosAsignados[numDestinos][1] = destinoC;
    numDestinos++;

    // Obtener posición actual de la unidad
    int sF = obreroFilaActual(o);
    int sC = obreroColActual(o);

    // Si ya estamos en el destino, no moverse
    if (sF == destinoF && sC == destinoC)
      continue;

    // Calcular ruta con pathfinding simple (sin colas)
    int *ruta = NULL;
    int len = 0;

    printf("[DEBUG PATH] Obrero %d en[%d][%d] -> destino[%d][%d]\n", idx, sF,
           sC, destinoF, destinoC);
    fflush(stdout);

    if (pathfindSimple(sF, sC, destinoF, destinoC, col, &ruta, &len)) {
      // DEBUG: Primera celda de la ruta
      if (len > 0) {
        int primeraCelda = ruta[0];
        int primeraF = primeraCelda / GRID_SIZE;
        int primeraC = primeraCelda % GRID_SIZE;
        printf("[DEBUG PATH] Ruta OK: %d pasos. Primera celda=[%d][%d] "
               "(codificada=%d)\n",
               len, primeraF, primeraC, primeraCelda);
        fflush(stdout);
      }

      // Liberar ruta anterior y asignar nueva
      obreroLiberarRuta(o);
      o->rutaCeldas = ruta;
      o->rutaLen = len;
      o->rutaIdx = 0;
      o->moviendose = true;
    }
  }

  // --- COMANDAR CABALLEROS ---
  for (int i = 0; i < MAX_CABALLEROS; i++) {
    Unidad *u = &j->caballeros[i];
    if (u->seleccionado && u->x >= 0 && u->vida > 0) {
      // Buscar destino libre
      int destinoF = gF;
      int destinoC = gC;

      if (CELDA_YA_ASIGNADA(destinoF, destinoC) ||
          (*(*(col + destinoF) + destinoC) == 3)) {
        bool encontrado = false;
        for (int r = 1; r <= 3 && !encontrado; r++) {
          for (int dy = -r; dy <= r && !encontrado; dy++) {
            for (int dx = -r; dx <= r && !encontrado; dx++) {
              int nf = gF + dy;
              int nc = gC + dx;
              if (nf >= 0 && nf < GRID_SIZE && nc >= 0 && nc < GRID_SIZE) {
                int valorCelda = *(*(col + nf) + nc);
                if (valorCelda == 0 && !CELDA_YA_ASIGNADA(nf, nc)) {
                  destinoF = nf;
                  destinoC = nc;
                  encontrado = true;
                }
              }
            }
          }
        }
        if (!encontrado)
          continue;
      }

      destinosAsignados[numDestinos][0] = destinoF;
      destinosAsignados[numDestinos][1] = destinoC;
      numDestinos++;

      int sF = obreroFilaActual(u), sC = obreroColActual(u);
      int *path = NULL;
      int len = 0;
      if (pathfindSimple(sF, sC, destinoF, destinoC, col, &path, &len)) {
        obreroLiberarRuta(u);
        u->rutaCeldas = path;
        u->rutaLen = len;
        u->rutaIdx = 0;
        u->moviendose = true;
      }
    }
  }

  // --- COMANDAR CABALLEROS SIN ESCUDO ---
  for (int i = 0; i < MAX_CABALLEROS_SIN_ESCUDO; i++) {
    Unidad *u = &j->caballerosSinEscudo[i];
    if (u->seleccionado && u->x >= 0 && u->vida > 0) {
      // Buscar destino libre
      int destinoF = gF;
      int destinoC = gC;

      if (CELDA_YA_ASIGNADA(destinoF, destinoC) ||
          (*(*(col + destinoF) + destinoC) == 3)) {
        bool encontrado = false;
        for (int r = 1; r <= 3 && !encontrado; r++) {
          for (int dy = -r; dy <= r && !encontrado; dy++) {
            for (int dx = -r; dx <= r && !encontrado; dx++) {
              int nf = gF + dy;
              int nc = gC + dx;
              if (nf >= 0 && nf < GRID_SIZE && nc >= 0 && nc < GRID_SIZE) {
                int valorCelda = *(*(col + nf) + nc);
                if (valorCelda == 0 && !CELDA_YA_ASIGNADA(nf, nc)) {
                  destinoF = nf;
                  destinoC = nc;
                  encontrado = true;
                }
              }
            }
          }
        }
        if (!encontrado)
          continue;
      }

      destinosAsignados[numDestinos][0] = destinoF;
      destinosAsignados[numDestinos][1] = destinoC;
      numDestinos++;

      int sF = obreroFilaActual(u), sC = obreroColActual(u);
      int *path = NULL;
      int len = 0;
      if (pathfindSimple(sF, sC, destinoF, destinoC, col, &path, &len)) {
        obreroLiberarRuta(u);
        u->rutaCeldas = path;
        u->rutaLen = len;
        u->rutaIdx = 0;
        u->moviendose = true;
      }
    }
  }

  // --- COMANDAR GUERREROS ---
  for (int i = 0; i < MAX_GUERREROS; i++) {
    Unidad *u = &j->guerreros[i];
    if (u->seleccionado && u->x >= 0 && u->vida > 0) {
      // Buscar destino libre
      int destinoF = gF;
      int destinoC = gC;

      if (CELDA_YA_ASIGNADA(destinoF, destinoC) ||
          (*(*(col + destinoF) + destinoC) == 3)) {
        bool encontrado = false;
        for (int r = 1; r <= 3 && !encontrado; r++) {
          for (int dy = -r; dy <= r && !encontrado; dy++) {
            for (int dx = -r; dx <= r && !encontrado; dx++) {
              int nf = gF + dy;
              int nc = gC + dx;
              if (nf >= 0 && nf < GRID_SIZE && nc >= 0 && nc < GRID_SIZE) {
                int valorCelda = *(*(col + nf) + nc);
                if (valorCelda == 0 && !CELDA_YA_ASIGNADA(nf, nc)) {
                  destinoF = nf;
                  destinoC = nc;
                  encontrado = true;
                }
              }
            }
          }
        }
        if (!encontrado)
          continue;
      }

      destinosAsignados[numDestinos][0] = destinoF;
      destinosAsignados[numDestinos][1] = destinoC;
      numDestinos++;

      int sF = obreroFilaActual(u), sC = obreroColActual(u);
      int *path = NULL;
      int len = 0;
      if (pathfindSimple(sF, sC, destinoF, destinoC, col, &path, &len)) {
        obreroLiberarRuta(u);
        u->rutaCeldas = path;
        u->rutaLen = len;
        u->rutaIdx = 0;
        u->moviendose = true;
      }
    }
  }

#undef CELDA_YA_ASIGNADA

  // --- NUEVA LÓGICA DE CONQUISTA: Verificar si la isla actual está limpia ---
  // Solo verificar si estamos en una isla válida (1-5) y no ha sido conquistada aun
  if (j->islaActual >= 1 && j->islaActual <= 5 && !j->islasConquistadas[j->islaActual]) {
      int enemCount = 0;
      // navegacionObtenerEnemigosActivos devuelve puntero a array, pero necesitamos saber si hay activos.
      // Si la funcion retorna NULL o count es 0, no hay enemigos.
      navegacionObtenerEnemigosActivos(&enemCount);
      
      if (enemCount == 0) {
          j->islasConquistadas[j->islaActual] = true;
          printf("[CONQUISTA] ¡Isla %d conquistada! Ahora puedes viajar a nuevas tierras si completas las 3 primeras.\n", j->islaActual);
      }
  }
}


void rtsLiberarMovimientoJugador(struct Jugador *j) {
  for (int i = 0; i < MAX_OBREROS; i++)
    obreroLiberarRuta(&j->obreros[i]);
  for (int i = 0; i < MAX_CABALLEROS; i++)
    obreroLiberarRuta(&j->caballeros[i]);
  for (int i = 0; i < MAX_CABALLEROS_SIN_ESCUDO; i++)
    obreroLiberarRuta(&j->caballerosSinEscudo[i]);
  for (int i = 0; i < MAX_GUERREROS; i++) // Guerreros tiene capacidad 4 segun struct
    obreroLiberarRuta(&j->guerreros[i]);
}

// ============================================================================
// FUNCIONES DE ENTRENAMIENTO DE TROPAS
// ============================================================================

bool entrenarObrero(struct Jugador *j, float x, float y) {
  // Buscar un espacio disponible en el array de obreros
  for (int i = 0; i < MAX_OBREROS; i++) {
    // Si el obrero está fuera de pantalla, está libre
    if (j->obreros[i].x < 0) {
      // Obtener la posición del cuartel
      if (j->cuartel == NULL)
        return false;

      Edificio *cuartel = (Edificio *)j->cuartel;

      // Generar posición cerca del cuartel (offset aleatorio pequeño)
      // Generar posición base cerca del cuartel
      float offsetX = (float)((i % 3) * 70);
      float offsetY = (float)((i / 3) * 70);
      float baseX = cuartel->x + offsetX;
      float baseY = cuartel->y + cuartel->alto + 20 + offsetY;

      j->obreros[i].x = baseX;
      j->obreros[i].y = baseY;

      // VALIDAR POSICIÓN DE SPAWN (Evitar mar/obstáculos)
      int **col = mapaObtenerCollisionMap();
      if (col) {
        int cX = (int)(j->obreros[i].x / (float)TILE_SIZE);
        int cY = (int)(j->obreros[i].y / (float)TILE_SIZE);

        // Si la posición inicial es inválida, buscar vecino libre
        if (cY < 0 || cY >= GRID_SIZE || cX < 0 || cX >= GRID_SIZE ||
            col[cY][cX] != 0 || mapaObjetos[cY][cX] != 0) {
          bool encontrado = false;
          // Buscar en un radio creciente alrededor del cuartel
          int centroCX = (int)((cuartel->x + 64) / (float)TILE_SIZE);
          int centroCY = (int)((cuartel->y + 64) / (float)TILE_SIZE);

          for (int r = 2; r < 8; r++) { // Radio 2 a 8 tiles
            for (int dy = -r; dy <= r; dy++) {
              for (int dx = -r; dx <= r; dx++) {
                int ny = centroCY + dy;
                int nx = centroCX + dx;
                if (ny >= 0 && ny < GRID_SIZE && nx >= 0 && nx < GRID_SIZE) {
                  // Verificar que esté libre en colisiones Y en objetos
                  if (col[ny][nx] == 0 && mapaObjetos[ny][nx] == 0) {
                    j->obreros[i].x = nx * (float)TILE_SIZE;
                    j->obreros[i].y = ny * (float)TILE_SIZE;

                    // NUEVO: Reservar posición inmediatamente
                    col[ny][nx] = 3; // Ocupado por unidad
                    mapaRegistrarObjeto(j->obreros[i].x, j->obreros[i].y,
                                        SIMBOLO_OBRERO);

                    encontrado = true;
                    break;
                  }
                }
              }
              if (encontrado)
                break;
            }
            if (encontrado)
              break;
          }
        } else {
          // Si la posición inicial ya era válida, también reservarla
          col[cY][cX] = 3;
          mapaRegistrarObjeto(j->obreros[i].x, j->obreros[i].y, SIMBOLO_OBRERO);
        }
      }

      j->obreros[i].destinoX = j->obreros[i].x;
      j->obreros[i].destinoY = j->obreros[i].y;
      j->obreros[i].moviendose = false;
      j->obreros[i].seleccionado = false;
      j->obreros[i].dir = DIR_FRONT;
      j->obreros[i].frame = 0;
      j->obreros[i].celdaFila = -1;
      j->obreros[i].celdaCol = -1;
      j->obreros[i].rutaCeldas = NULL;
      j->obreros[i].tipo = TIPO_OBRERO;
      j->obreros[i].animActual = animPorDireccion(DIR_FRONT);
      j->obreros[i].vidaMax = OBRERO_VIDA_MAX;
      j->obreros[i].vida = OBRERO_VIDA_MAX;
      j->obreros[i].recibiendoAtaque = false;
      // Guardado: mantener en cero los campos de muerte para que el snapshot los trate como vivos
      j->obreros[i].tiempoMuerteMs = 0;
      j->obreros[i].frameMuerte = 0;

      printf("[CUARTEL] Nuevo obrero entrenado en posición (%.1f, %.1f)\n",
             j->obreros[i].x, j->obreros[i].y);
      return true;
    }
  }

  // No hay espacio disponible
  return false;
}

bool entrenarCaballero(struct Jugador *j, float x, float y) {
  // Buscar un espacio disponible en el array de caballeros
  for (int i = 0; i < MAX_CABALLEROS; i++) {
    // Si el caballero está fuera de pantalla, está libre
    if (j->caballeros[i].x < 0) {
      // Obtener la posición del cuartel
      if (j->cuartel == NULL)
        return false;

      Edificio *cuartel = (Edificio *)j->cuartel;

      // Generar posición cerca del cuartel
      float offsetX = (float)((i % 2) * 70);
      float offsetY = (float)((i / 2) * 70);
      float baseX = cuartel->x + offsetX;
      float baseY = cuartel->y + cuartel->alto + 20 + offsetY;

      j->caballeros[i].x = baseX;
      j->caballeros[i].y = baseY;

      // VALIDAR POSICIÓN DE SPAWN (Evitar mar/obstáculos)
      int **col = mapaObtenerCollisionMap();
      if (col) {
        int cX = (int)(j->caballeros[i].x / (float)TILE_SIZE);
        int cY = (int)(j->caballeros[i].y / (float)TILE_SIZE);

        // Si la posición inicial es inválida, buscar vecino libre
        if (cY < 0 || cY >= GRID_SIZE || cX < 0 || cX >= GRID_SIZE ||
            col[cY][cX] != 0 || mapaObjetos[cY][cX] != 0) {
          bool encontrado = false;
          // Buscar en un radio creciente alrededor del cuartel
          int centroCX = (int)((cuartel->x + 64) / (float)TILE_SIZE);
          int centroCY = (int)((cuartel->y + 64) / (float)TILE_SIZE);

          for (int r = 2; r < 8; r++) {
            for (int dy = -r; dy <= r; dy++) {
              for (int dx = -r; dx <= r; dx++) {
                int ny = centroCY + dy;
                int nx = centroCX + dx;
                if (ny >= 0 && ny < GRID_SIZE && nx >= 0 && nx < GRID_SIZE) {
                  // Verificar colisión Y ocupación visual
                  if (col[ny][nx] == 0 && mapaObjetos[ny][nx] == 0) {
                    j->caballeros[i].x = nx * (float)TILE_SIZE;
                    j->caballeros[i].y = ny * (float)TILE_SIZE;

                    // NUEVO: Reservar posición inmediatamente
                    col[ny][nx] = 3;
                    mapaRegistrarObjeto(j->caballeros[i].x, j->caballeros[i].y,
                                        SIMBOLO_CABALLERO);

                    encontrado = true;
                    break;
                  }
                }
              }
              if (encontrado)
                break;
            }
            if (encontrado)
              break;
          }
        } else {
          // Si la posición inicial ya era válida, también reservarla
          col[cY][cX] = 3;
          mapaRegistrarObjeto(j->caballeros[i].x, j->caballeros[i].y,
                              SIMBOLO_CABALLERO);
        }
      }

      j->caballeros[i].destinoX = j->caballeros[i].x;
      j->caballeros[i].destinoY = j->caballeros[i].y;
      j->caballeros[i].moviendose = false;
      j->caballeros[i].seleccionado = false;
      j->caballeros[i].dir = DIR_FRONT;
      j->caballeros[i].frame = 0;
      j->caballeros[i].celdaFila = -1;
      j->caballeros[i].celdaCol = -1;
      j->caballeros[i].rutaCeldas = NULL;
      j->caballeros[i].tipo = TIPO_CABALLERO;
      j->caballeros[i].animActual = animPorDireccion(DIR_FRONT);
      j->caballeros[i].vidaMax = CABALLERO_VIDA;
      j->caballeros[i].vida = CABALLERO_VIDA;
      j->caballeros[i].recibiendoAtaque = false;
      // Guardado: limpiar estado de muerte heredado de slots reutilizados
      j->caballeros[i].tiempoMuerteMs = 0;
      j->caballeros[i].frameMuerte = 0;

      printf("[CUARTEL] Nuevo caballero entrenado en posición (%.1f, %.1f)\n",
             j->caballeros[i].x, j->caballeros[i].y);
      return true;
    }
  }

  // No hay espacio disponible
  return false;
}

bool entrenarGuerrero(struct Jugador *j, float x, float y) {
  // Buscar un espacio disponible en el array de guerreros
  for (int i = 0; i < MAX_GUERREROS; i++) {
    // Si el guerrero está fuera de pantalla, está libre
    if (j->guerreros[i].x < 0) {
      // Obtener la posición del cuartel
      if (j->cuartel == NULL)
        return false;

      Edificio *cuartel = (Edificio *)j->cuartel;

      // Generar posición cerca del cuartel
      float offsetX = (float)((i % 2) * 70);
      float offsetY = (float)((i / 2) * 70);
      float baseX = cuartel->x + offsetX;
      float baseY = cuartel->y + cuartel->alto + 20 + offsetY;

      j->guerreros[i].x = baseX;
      j->guerreros[i].y = baseY;

      // VALIDAR POSICIÓN DE SPAWN (Evitar mar/obstáculos)
      int **col = mapaObtenerCollisionMap();
      if (col) {
        int cX = (int)(j->guerreros[i].x / (float)TILE_SIZE);
        int cY = (int)(j->guerreros[i].y / (float)TILE_SIZE);

        // Si la posición inicial es inválida, buscar vecino libre
        if (cY < 0 || cY >= GRID_SIZE || cX < 0 || cX >= GRID_SIZE ||
            col[cY][cX] != 0 || mapaObjetos[cY][cX] != 0) {
          bool encontrado = false;
          // Buscar en un radio creciente alrededor del cuartel
          int centroCX = (int)((cuartel->x + 64) / (float)TILE_SIZE);
          int centroCY = (int)((cuartel->y + 64) / (float)TILE_SIZE);

          for (int r = 2; r < 8; r++) {
            for (int dy = -r; dy <= r; dy++) {
              for (int dx = -r; dx <= r; dx++) {
                int ny = centroCY + dy;
                int nx = centroCX + dx;
                if (ny >= 0 && ny < GRID_SIZE && nx >= 0 && nx < GRID_SIZE) {
                  // Verificar colisión Y ocupación visual
                  if (col[ny][nx] == 0 && mapaObjetos[ny][nx] == 0) {
                    j->guerreros[i].x = nx * (float)TILE_SIZE;
                    j->guerreros[i].y = ny * (float)TILE_SIZE;

                    // NUEVO: Reservar posición inmediatamente
                    col[ny][nx] = 3;
                    mapaRegistrarObjeto(j->guerreros[i].x, j->guerreros[i].y,
                                        SIMBOLO_GUERRERO);

                    encontrado = true;
                    break;
                  }
                }
              }
              if (encontrado)
                break;
            }
            if (encontrado)
              break;
          }
        } else {
          // Si la posición inicial ya era válida, también reservarla
          col[cY][cX] = 3;
          mapaRegistrarObjeto(j->guerreros[i].x, j->guerreros[i].y,
                              SIMBOLO_GUERRERO);
        }
      }

      j->guerreros[i].destinoX = j->guerreros[i].x;
      j->guerreros[i].destinoY = j->guerreros[i].y;
      j->guerreros[i].moviendose = false;
      j->guerreros[i].seleccionado = false;
      j->guerreros[i].dir = DIR_FRONT;
      j->guerreros[i].frame = 0;
      j->guerreros[i].celdaFila = -1;
      j->guerreros[i].celdaCol = -1;
      j->guerreros[i].rutaCeldas = NULL;
      j->guerreros[i].tipo = TIPO_GUERRERO;
      j->guerreros[i].animActual = animPorDireccion(DIR_FRONT);

      // Stats Guerrero: Vida:120, Daño:30, Crítico:10%, Defensa:20
      j->guerreros[i].vidaMax = 120;
      j->guerreros[i].vida = 120;
      j->guerreros[i].damage = 30;
      j->guerreros[i].critico = GUERRERO_CRITICO;
      j->guerreros[i].defensa = 20;
      j->guerreros[i].alcance = 64; // Melee (1 tile)
      j->guerreros[i].recibiendoAtaque = false;
      // Guardado: igualar estado de muerte para nuevas tropas
      j->guerreros[i].tiempoMuerteMs = 0;
      j->guerreros[i].frameMuerte = 0;

      printf("[CUARTEL] Nuevo guerrero entrenado en posición (%.1f, %.1f)\n",
             j->guerreros[i].x, j->guerreros[i].y);
      return true;
    }
  }

  // No hay espacio disponible
  return false;
}

bool recursosIntentarCazar(struct Jugador *j, float mundoX, float mundoY) {
  // ================================================================
  // VERIFICACIÓN DIRECTA: Buscar vaca por posición real del array
  // ================================================================
  // En lugar de confiar solo en mapaObjetos, verificamos directamente
  // las coordenadas de cada vaca en el array dinámico.
  // ================================================================

  // Convertir click a celda de la matriz
  int clickF = (int)(mundoY / TILE_SIZE);
  int clickC = (int)(mundoX / TILE_SIZE);

  // Validar límites
  if (clickF < 0 || clickF >= GRID_SIZE || clickC < 0 || clickC >= GRID_SIZE) {
    return false;
  }

  // Obtener array de vacas
  int numVacas = 0;
  Vaca *vacas = mapaObtenerVacas(&numVacas);

  if (!vacas || numVacas == 0) {
    return false;
  }

  // Buscar una vaca en la celda clickeada (usando aritmética de punteros)
  int vacaEncontrada = -1;
  int vacaFila = -1, vacaCol = -1;

  Vaca *pVaca = vacas;
  for (int i = 0; i < numVacas; i++, pVaca++) {
    // Calcular celda de esta vaca
    int vF = (int)(pVaca->y / TILE_SIZE);
    int vC = (int)(pVaca->x / TILE_SIZE);

    // Verificar si coincide con el click (celda exacta o adyacentes)
    // La vaca ocupa 64x64 pero solo se ancla en una celda
    if (vF == clickF && vC == clickC) {
      vacaEncontrada = i;
      vacaFila = vF;
      vacaCol = vC;
      break;
    }

    // También verificar celdas adyacentes por el tamaño del sprite
    int dF = clickF - vF;
    int dC = clickC - vC;
    if (dF >= 0 && dF <= 1 && dC >= 0 && dC <= 1) {
      vacaEncontrada = i;
      vacaFila = vF;
      vacaCol = vC;
      break;
    }
  }

  // Si no encontramos vaca en la posición real, no procesar
  if (vacaEncontrada < 0) {
    return false;
  }

  printf("[DEBUG] Cazar: Vaca #%d encontrada en posicion real Celda[%d][%d]\n",
         vacaEncontrada, vacaFila, vacaCol);

  // Usar centro de la vaca para proximidad (aprox 64px de offset del ancla)
  float vacaCentroX = (float)(vacaCol * TILE_SIZE) + 64.0f;
  float vacaCentroY = (float)(vacaFila * TILE_SIZE) + 64.0f;

  // 2. Buscar si hay algún OBRERO o CABALLERO seleccionado cerca
  bool alguienCerca = false;

  // Buscar Obreros
  if (recursosObreroCercaDePunto(j, vacaCentroX, vacaCentroY, 200.0f))
    alguienCerca = true;

  // Buscar Caballeros (con y sin escudo)
  if (!alguienCerca) {
    for (int i = 0; i < MAX_CABALLEROS; i++) {
      Unidad *u1 = &j->caballeros[i];
      Unidad *u2 = &j->caballerosSinEscudo[i];
      if ((u1->seleccionado && u1->x >= 0) ||
          (u2->seleccionado && u2->x >= 0)) {
        float dx1 = (u1->x + 32.0f) - vacaCentroX;
        float dy1 = (u1->y + 32.0f) - vacaCentroY;
        float dx2 = (u2->x + 32.0f) - vacaCentroX;
        float dy2 = (u2->y + 32.0f) - vacaCentroY;
        if ((u1->seleccionado && sqrtf(dx1 * dx1 + dy1 * dy1) < 200.0f) ||
            (u2->seleccionado && sqrtf(dx2 * dx2 + dy2 * dy2) < 200.0f)) {
          alguienCerca = true;
          break;
        }
      }
    }
  }

  if (alguienCerca) {
    // 3. Confirmación
    int respuesta = MessageBox(GetActiveWindow(), "Quieres cazar esta vaca por comida?",
                               "Cazar Vaca", MB_YESNO | MB_ICONQUESTION);

    if (respuesta == IDYES) {
      // ================================================================
      // CORRECCIÓN BUG DE SINCRONIZACIÓN:
      // ================================================================
      // Usamos el ÍNDICE de la vaca (vacaEncontrada) para eliminarla,
      // no su posición anterior (vacaFila, vacaCol).
      // Esto garantiza que aunque la vaca se haya movido mientras el
      // usuario decidía en el MessageBox, se eliminará la vaca correcta.
      // ================================================================
      if (mapaEliminarVacaPorIndice(vacaEncontrada)) {
        j->Comida += 100;
        MessageBox(GetActiveWindow(), "Vaca cazada! +100 Comida", "Recursos",
                   MB_OK | MB_ICONINFORMATION);
      } else {
        // La vaca ya no existe (índice inválido o fue eliminada por otra razón)
        MessageBox(GetActiveWindow(), "La vaca ya no está disponible.", "Error",
                   MB_OK | MB_ICONWARNING);
      }
    }
    return true; // Click manejado
  }

  // Si no hay tropa cerca, mandarlos a caminar
  rtsComandarMovimiento(j, mundoX, mundoY);
  return true; // Click manejado (movimiento comandado)
}

// ============================================================================
// LÓGICA DE TALAR ÁRBOLES
// ============================================================================
bool recursosIntentarTalar(struct Jugador *j, float mundoX, float mundoY) {
  // 1. Verificar qué objeto hay en el mapa (Búsqueda en 2x2 celdas)
  // El árbol visual es 128x128 (2x2 tiles) pero se registra en una celda.
  // Buscamos en la celda clickeada y sus vecinas para mayor tolerancia.
  int targets[4][2] = {{0, 0}, {0, -1}, {-1, 0}, {-1, -1}};
  int tipoObjeto = 0;
  int fArbol = -1, cArbol = -1;

  for (int k = 0; k < 4; k++) {
    int tf = (int)(mundoY / TILE_SIZE) + targets[k][0];
    int tc = (int)(mundoX / TILE_SIZE) + targets[k][1];

    if (tf >= 0 && tf < GRID_SIZE && tc >= 0 && tc < GRID_SIZE) {
      int t = mapaObtenerTipoObjeto(tf, tc);
      if (t == SIMBOLO_ARBOL) {
        tipoObjeto = t;
        fArbol = tf;
        cArbol = tc;
        break;
      }
    }
  }

  // Los árboles se identifican con SIMBOLO_ARBOL ('A')
  if (tipoObjeto == SIMBOLO_ARBOL) {
    printf("[DEBUG] Talar: Arbol detectado en Celda[%d][%d]\n", fArbol, cArbol);

    // ================================================================
    // RESTRICCIÓN DE TALA: OBREROS Y GUERREROS PUEDEN TALAR
    // ================================================================
    // Según las reglas del juego:
    // - Obreros: Pueden talar árboles ✓
    // - Guerreros: Pueden talar árboles ✓
    // - Caballeros (con escudo): NO pueden talar (solo cazar vacas) ✗
    // - Caballeros sin escudo: NO pueden talar (solo cazar vacas) ✗
    // ================================================================
    
    // Usamos el CENTRO del árbol (un tile de 64px) para la distancia
    float centroArbolX = (float)(cArbol * TILE_SIZE) + 32.0f;
    float centroArbolY = (float)(fArbol * TILE_SIZE) + 32.0f;

    Unidad *taladorCercano = NULL;
    float distMinima = 9999.0f;
    const float DISTANCIA_MAXIMA = 180.0f;

    // Buscar Obreros seleccionados cerca del árbol
    // Usamos aritmética de punteros para recorrer el array
    Unidad *ptrObrero = j->obreros;
    for (int i = 0; i < MAX_OBREROS; i++, ptrObrero++) {
      // Solo considerar obreros seleccionados y activos
      if (!ptrObrero->seleccionado || ptrObrero->x < 0 || ptrObrero->vida <= 0)
        continue;
      
      // Calcular distancia desde el centro del obrero al centro del árbol
      float dx = (ptrObrero->x + 32.0f) - centroArbolX;
      float dy = (ptrObrero->y + 32.0f) - centroArbolY;
      float dist = sqrtf(dx * dx + dy * dy);
      
      if (dist < DISTANCIA_MAXIMA && dist < distMinima) {
        distMinima = dist;
        taladorCercano = ptrObrero;
      }
    }

    // Buscar Guerreros seleccionados cerca del árbol
    // Los guerreros también pueden talar árboles
    Unidad *ptrGuerrero = j->guerreros;
    for (int i = 0; i < MAX_GUERREROS; i++, ptrGuerrero++) {
      // Solo considerar guerreros seleccionados y activos
      if (!ptrGuerrero->seleccionado || ptrGuerrero->x < 0 || ptrGuerrero->vida <= 0)
        continue;
      
      // Calcular distancia desde el centro del guerrero al centro del árbol
      float dx = (ptrGuerrero->x + 32.0f) - centroArbolX;
      float dy = (ptrGuerrero->y + 32.0f) - centroArbolY;
      float dist = sqrtf(dx * dx + dy * dy);
      
      if (dist < DISTANCIA_MAXIMA && dist < distMinima) {
        distMinima = dist;
        taladorCercano = ptrGuerrero;
      }
    }

    // NOTA: NO buscamos caballeros (con o sin escudo) porque NO pueden talar
    // Los caballeros solo pueden cazar vacas, no tienen habilidad de tala

    // Permitir talar si hay un OBRERO o GUERRERO cerca
    if (taladorCercano != NULL) {
        // --- NUEVA LÓGICA: GOLPEAR ÁRBOL ---
        // Se elimina el MessageBox. Cada click resta 1 vida. 
        // Al 3er golpe (vida=0) se destruye y da madera.
        
        // Declaracion extern para la funcion en mapa.c
        extern bool mapaGolpearArbol(int f, int c);
        
        if (mapaGolpearArbol(fArbol, cArbol)) {
            // Árbol destruido (el último golpe)
            j->Madera += 50;
            printf("[RECURSOS] Arbol talado! +50 Madera. Total: %d\n", j->Madera);
        } else {
            // Golpe dado pero no destruido
            printf("[RECURSOS] Arbol golpeado. Vida restante...\n");
        }
        
        return true; // Click manejado
    }

    // Si no hay obrero cerca, mandamos caminar al CENTRO del arbol
    rtsComandarMovimiento(j, centroArbolX, centroArbolY);
    return true; // Click manejado
  }

  // No era un árbol, permitir que el click pase a la lógica de movimiento
  return false;
}

bool recursosIntentarRecogerMina(struct Jugador *j, float mundoX,
                                 float mundoY) {
  if (j->mina == NULL)
    return false;
  Edificio *e = (Edificio *)j->mina;

  // 1. Verificar si el click fue sobre la mina
  if (edificioContienePunto(e, mundoX, mundoY)) {
    // 1.5 Verificar si la mina está agotada
    if (e->agotada) {
      MessageBox(GetActiveWindow(),
                 "Esta mina está completamente agotada.\\n\\nYa no quedan "
                 "recursos por extraer.",
                 "Mina Agotada", MB_OK | MB_ICONWARNING);
      return true; // Click manejado
    }

    // 2. Verificar si hay recursos acumulados
    if (e->oroAcumulado <= 0 && e->piedraAcumulada <= 0 &&
        e->hierroAcumulado <= 0) {
      MessageBox(GetActiveWindow(), "La mina no tiene recursos acumulados aun.",
                 "Mina Vacia", MB_OK | MB_ICONINFORMATION);
      return true; // Click manejado
    }

    // 3. Buscar obrero seleccionado cerca del CENTRO de la mina
    float minaCentroX = e->x + 64.0f;
    float minaCentroY = e->y + 64.0f;

    Unidad *obreroCercano = NULL;
    float distMinima = 9999.0f;
    const float DISTANCIA_MAXIMA = 250.0f; // Mayor rango para edificios grandes

    for (int i = 0; i < MAX_OBREROS; i++) {
      Unidad *o = &j->obreros[i];
      if (!o->seleccionado)
        continue;

      float dx = (o->x + 32.0f) - minaCentroX;
      float dy = (o->y + 32.0f) - minaCentroY;
      float dist = sqrtf(dx * dx + dy * dy);

      if (dist < DISTANCIA_MAXIMA && dist < distMinima) {
        distMinima = dist;
        obreroCercano = o;
      }
    }

    if (obreroCercano != NULL) {
      char msg[256];
      sprintf(msg,
              "Quieres recoger los recursos de la mina?\n\nOro: %d\nPiedra: "
              "%d\nHierro: %d",
              e->oroAcumulado, e->piedraAcumulada, e->hierroAcumulado);

      int respuesta =
          MessageBox(GetActiveWindow(), msg, "Recoger Recursos", MB_YESNO | MB_ICONQUESTION);

      if (respuesta == IDYES) {
        j->Oro += e->oroAcumulado;
        j->Piedra += e->piedraAcumulada;
        j->Hierro += e->hierroAcumulado;
        e->oroAcumulado = 0;
        e->piedraAcumulada = 0;
        e->hierroAcumulado = 0;
        MessageBox(GetActiveWindow(), "Recursos recogidos con exito!", "Recursos",
                   MB_OK | MB_ICONINFORMATION);
      }
      return true;
    }

    // Si no hay obrero cerca, mandarlo a caminar
    rtsComandarMovimiento(j, mundoX, mundoY);
    return true; // Click manejado (movimiento comandado)
  }

  return false;
}

// ============================================================================
// PANEL HUD DE RECURSOS (Esquina superior derecha)
// ============================================================================
// Dibuja un panel con los recursos actuales del jugador y conteo de unidades.
// Se renderiza sobre el buffer para evitar parpadeo.
// Estilo visual: Medieval (pergamino, cuero, bronce)
// ============================================================================
void panelRecursosDibujar(HDC hdcBuffer, struct Jugador *j, int anchoPantalla) {
  if (!j)
    return;

  // ================================================================
  // CONFIGURACIÓN DEL PANEL
  // ================================================================
  const int PANEL_ANCHO = 230;
  const int PANEL_ALTO =
      230; // Aumentado para incluir Nivel de Barco
  const int MARGEN = 10;
  const int ESPACIADO_LINEA = 22;

  // Posición: Esquina superior DERECHA con margen
  int panelX = anchoPantalla - PANEL_ANCHO - MARGEN;
  int panelY = MARGEN;

  // ================================================================
  // FONDO DEL PANEL (Estilo medieval: cuero/pergamino oscuro)
  // ================================================================
  // Fondo principal: Marrón cuero oscuro
  HBRUSH brushFondo = CreateSolidBrush(RGB(60, 40, 25));
  HBRUSH oldBrush = (HBRUSH)SelectObject(hdcBuffer, brushFondo);

  // Borde exterior: Bronce oscuro
  HPEN penBordeExt = CreatePen(PS_SOLID, 3, RGB(140, 100, 55));
  HPEN oldPen = (HPEN)SelectObject(hdcBuffer, penBordeExt);

  // Dibujar rectángulo con esquinas redondeadas
  RoundRect(hdcBuffer, panelX, panelY, panelX + PANEL_ANCHO,
            panelY + PANEL_ALTO, 12, 12);

  // Borde interior: Dorado para efecto de relieve
  SelectObject(hdcBuffer, oldBrush);
  DeleteObject(brushFondo);

  HPEN penBordeInt = CreatePen(PS_SOLID, 1, RGB(180, 140, 60));
  SelectObject(hdcBuffer, penBordeInt);

  // Marco interior decorativo
  HBRUSH nullBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
  SelectObject(hdcBuffer, nullBrush);
  RoundRect(hdcBuffer, panelX + 4, panelY + 4, panelX + PANEL_ANCHO - 4,
            panelY + PANEL_ALTO - 4, 8, 8);

  // Limpiar pens
  SelectObject(hdcBuffer, oldPen);
  DeleteObject(penBordeExt);
  DeleteObject(penBordeInt);

  // ================================================================
  // CONFIGURACIÓN DE TEXTO
  // ================================================================
  SetBkMode(hdcBuffer, TRANSPARENT);

  // Fuente para el título (estilo medieval)
  HFONT fontTitulo =
      CreateFont(18, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                 OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                 DEFAULT_PITCH | FF_SWISS, "Times New Roman");

  // Fuente para los valores
  HFONT fontValor =
      CreateFont(15, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                 OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                 DEFAULT_PITCH | FF_SWISS, "Arial");

  HFONT oldFont = (HFONT)SelectObject(hdcBuffer, fontTitulo);

  // ================================================================
  // TÍTULO "RECURSOS" (Dorado brillante)
  // ================================================================
  SetTextColor(hdcBuffer, RGB(255, 200, 80)); // Dorado cálido
  TextOutA(hdcBuffer, panelX + 12, panelY + 10, "RECURSOS", 8);

  // Línea separadora bajo el título (bronce)
  HPEN penSeparador = CreatePen(PS_SOLID, 1, RGB(160, 120, 60));
  SelectObject(hdcBuffer, penSeparador);
  MoveToEx(hdcBuffer, panelX + 12, panelY + 32, NULL);
  LineTo(hdcBuffer, panelX + PANEL_ANCHO - 12, panelY + 32);
  DeleteObject(penSeparador);

  // ================================================================
  // RECURSOS (con colores distintivos sobre fondo cuero)
  // ================================================================
  SelectObject(hdcBuffer, fontValor);

  char buffer[64];
  int yPos = panelY + 40;

  // Oro (amarillo dorado brillante)
  SetTextColor(hdcBuffer, RGB(255, 215, 50));
  sprintf(buffer, "Oro:       %d", j->Oro);
  TextOutA(hdcBuffer, panelX + 18, yPos, buffer, strlen(buffer));
  yPos += ESPACIADO_LINEA;

  // Madera (verde trigo/hierba - INVERTIDO)
  SetTextColor(hdcBuffer, RGB(150, 220, 100));
  sprintf(buffer, "Madera:    %d", j->Madera);
  TextOutA(hdcBuffer, panelX + 18, yPos, buffer, strlen(buffer));
  yPos += ESPACIADO_LINEA;

  // Comida (marrón claro/arena - INVERTIDO)
  SetTextColor(hdcBuffer, RGB(230, 180, 100));
  sprintf(buffer, "Comida:    %d", j->Comida);
  TextOutA(hdcBuffer, panelX + 18, yPos, buffer, strlen(buffer));
  yPos += ESPACIADO_LINEA;

  // Piedra (gris piedra caliza)
  SetTextColor(hdcBuffer, RGB(200, 195, 180));
  sprintf(buffer, "Piedra:    %d", j->Piedra);
  TextOutA(hdcBuffer, panelX + 18, yPos, buffer, strlen(buffer));
  yPos += ESPACIADO_LINEA;

  // Hierro (gris azulado/metal)
  SetTextColor(hdcBuffer, RGB(176, 196, 222));
  sprintf(buffer, "Hierro:    %d", j->Hierro);
  TextOutA(hdcBuffer, panelX + 18, yPos, buffer, strlen(buffer));
  yPos += ESPACIADO_LINEA + 8; // Espacio extra antes de tropas

  // ================================================================
  // CONTEO DE UNIDADES (parte inferior)
  // ================================================================
  // Línea separadora (bronce)
  HPEN penSep2 = CreatePen(PS_SOLID, 1, RGB(160, 120, 60));
  SelectObject(hdcBuffer, penSep2);
  MoveToEx(hdcBuffer, panelX + 12, yPos - 5, NULL);
  LineTo(hdcBuffer, panelX + PANEL_ANCHO - 12, yPos - 5);
  DeleteObject(penSep2);

  // Contar unidades globales (Isla actual + Otras islas + Barco)
  int numObreros = navegacionContarUnidadesGlobal(j, TIPO_OBRERO);
  int numCaballeros = navegacionContarUnidadesGlobal(j, TIPO_CABALLERO);
  int numCabSinEsc = navegacionContarUnidadesGlobal(j, TIPO_CABALLERO_SIN_ESCUDO);
  int numGuerreros = navegacionContarUnidadesGlobal(j, TIPO_GUERRERO);

  // Mostrar conteo de unidades (color pergamino claro) - Acentos corregidos
  SetTextColor(hdcBuffer, RGB(240, 220, 180));
  sprintf(buffer, "Ob:%d CE:%d CS:%d Gu:%d", numObreros, numCaballeros,
          numCabSinEsc, numGuerreros);
  TextOutA(hdcBuffer, panelX + 5, yPos + 3, buffer, strlen(buffer));

  // Mostrar Nivel de Barco
  sprintf(buffer, "Nivel Barco: %d", j->barco.nivelMejora);
  TextOutA(hdcBuffer, panelX + 5, yPos + 23, buffer, strlen(buffer));

  // ================================================================
  // LIMPIEZA DE RECURSOS GDI
  // ================================================================
  SelectObject(hdcBuffer, oldFont);
  DeleteObject(fontTitulo);
  DeleteObject(fontValor);
}

bool recursosObreroCercaDePunto(struct Jugador *j, float x, float y,
                                float distMax) {
  for (int i = 0; i < MAX_OBREROS; i++) {
    Unidad *o = &j->obreros[i];
    if (!o->seleccionado)
      continue;

    float dx = (o->x + 32.0f) - x;
    float dy = (o->y + 32.0f) - y;
    float dist = sqrtf(dx * dx + dy * dy);

    if (dist < distMax) {
      return true;
    }
  }
  return false;
}

bool recursosCualquierTropaCercaDePunto(struct Jugador *j, float x, float y,
                                        float distMax) {
  // Obreros
  if (recursosObreroCercaDePunto(j, x, y, distMax))
    return true;

  // Caballeros
  for (int i = 0; i < MAX_CABALLEROS; i++) {
    Unidad *u = &j->caballeros[i];
    if (!u->seleccionado)
      continue;
    float dx = (u->x + 32.0f) - x;
    float dy = (u->y + 32.0f) - y;
    float dist = sqrtf(dx * dx + dy * dy);
    if (dist < distMax)
      return true;
  }

  // Caballeros sin escudo
  for (int i = 0; i < MAX_CABALLEROS_SIN_ESCUDO; i++) {
    Unidad *u = &j->caballerosSinEscudo[i];
    if (!u->seleccionado)
      continue;
    float dx = (u->x + 32.0f) - x;
    float dy = (u->y + 32.0f) - y;
    float dist = sqrtf(dx * dx + dy * dy);
    if (dist < distMax)
      return true;
  }

  // Guerreros
  for (int i = 0; i < MAX_GUERREROS; i++) {
    Unidad *u = &j->guerreros[i];
    if (!u->seleccionado)
      continue;
    float dx = (u->x + 32.0f) - x;
    float dy = (u->y + 32.0f) - y;
    float dist = sqrtf(dx * dx + dy * dy);
    if (dist < distMax)
      return true;
  }

  return false;
}

bool entrenarCaballeroSinEscudo(struct Jugador *j, float x, float y) {
  // Buscar un espacio disponible en el array de caballeros sin escudo
  for (int i = 0; i < MAX_CABALLEROS_SIN_ESCUDO; i++) {
    if (j->caballerosSinEscudo[i].x < 0) {
      if (j->cuartel == NULL)
        return false;

      Edificio *cuartel = (Edificio *)j->cuartel;

      // Generar posición cerca del cuartel
      float offsetX = (float)((i % 2) * 70);
      float offsetY = (float)((i / 2) * 70);
      float baseX = cuartel->x + offsetX;
      float baseY = cuartel->y + cuartel->alto + 20 + offsetY;

      j->caballerosSinEscudo[i].x = baseX;
      j->caballerosSinEscudo[i].y = baseY;

      // VALIDAR POSICIÓN DE SPAWN (Evitar mar/obstáculos)
      int **col = mapaObtenerCollisionMap();
      if (col) {
        int cX = (int)(j->caballerosSinEscudo[i].x / (float)TILE_SIZE);
        int cY = (int)(j->caballerosSinEscudo[i].y / (float)TILE_SIZE);

        if (cY < 0 || cY >= GRID_SIZE || cX < 0 || cX >= GRID_SIZE ||
            col[cY][cX] != 0 || mapaObjetos[cY][cX] != 0) {
          bool encontrado = false;
          int centroCX = (int)((cuartel->x + 64) / (float)TILE_SIZE);
          int centroCY = (int)((cuartel->y + 64) / (float)TILE_SIZE);

          for (int r = 2; r < 8; r++) {
            for (int dy = -r; dy <= r; dy++) {
              for (int dx = -r; dx <= r; dx++) {
                int ny = centroCY + dy;
                int nx = centroCX + dx;
                if (ny >= 0 && ny < GRID_SIZE && nx >= 0 && nx < GRID_SIZE) {
                  if (col[ny][nx] == 0 && mapaObjetos[ny][nx] == 0) {
                    j->caballerosSinEscudo[i].x = nx * (float)TILE_SIZE;
                    j->caballerosSinEscudo[i].y = ny * (float)TILE_SIZE;
                    col[ny][nx] = 3;
                    mapaRegistrarObjeto(j->caballerosSinEscudo[i].x,
                                        j->caballerosSinEscudo[i].y,
                                        SIMBOLO_CABALLERO);
                    encontrado = true;
                    break;
                  }
                }
              }
              if (encontrado)
                break;
            }
            if (encontrado)
              break;
          }
        } else {
          col[cY][cX] = 3;
          mapaRegistrarObjeto(j->caballerosSinEscudo[i].x,
                              j->caballerosSinEscudo[i].y, SIMBOLO_CABALLERO);
        }
      }

      j->caballerosSinEscudo[i].destinoX = j->caballerosSinEscudo[i].x;
      j->caballerosSinEscudo[i].destinoY = j->caballerosSinEscudo[i].y;
      j->caballerosSinEscudo[i].moviendose = false;
      j->caballerosSinEscudo[i].seleccionado = false;
      j->caballerosSinEscudo[i].dir = DIR_FRONT;
      j->caballerosSinEscudo[i].frame = 0;
      j->caballerosSinEscudo[i].celdaFila = -1;
      j->caballerosSinEscudo[i].celdaCol = -1;
      j->caballerosSinEscudo[i].rutaCeldas = NULL;
      j->caballerosSinEscudo[i].tipo = TIPO_CABALLERO_SIN_ESCUDO;
      j->caballerosSinEscudo[i].animActual = animPorDireccion(DIR_FRONT);
      j->caballerosSinEscudo[i].vidaMax = CABALLERO_SIN_ESCUDO_VIDA;
      j->caballerosSinEscudo[i].vida = CABALLERO_SIN_ESCUDO_VIDA;

      // Stats para caballero sin escudo (Cuerpo a cuerpo)
      j->caballerosSinEscudo[i].vida = CABALLERO_SIN_ESCUDO_VIDA;
      j->caballerosSinEscudo[i].vidaMax = CABALLERO_SIN_ESCUDO_VIDA;
      j->caballerosSinEscudo[i].damage = CABALLERO_SIN_ESCUDO_DANO;
      j->caballerosSinEscudo[i].critico = CABALLERO_SIN_ESCUDO_CRITICO;
      j->caballerosSinEscudo[i].defensa = CABALLERO_SIN_ESCUDO_DEFENSA;
      j->caballerosSinEscudo[i].alcance = 64; // Cuerpo a cuerpo
      j->caballerosSinEscudo[i].recibiendoAtaque = false;
      // Guardado: asegurar que nuevas unidades reporten estado vivo
      j->caballerosSinEscudo[i].tiempoMuerteMs = 0;
      j->caballerosSinEscudo[i].frameMuerte = 0;

      printf("[CUARTEL] Nuevo caballero sin escudo entrenado en posición "
             "(%.1f, %.1f)\n",
             j->caballerosSinEscudo[i].x, j->caballerosSinEscudo[i].y);
      return true;
    }
  }

  return false;
}

// ============================================================================
// FUNCIÓN DE MEJORA DEL BARCO
// ============================================================================
// Incrementa el nivel de mejora del barco, ampliando su capacidad de tropas.
// Niveles: 1 (6 tropas) → 2 (9 tropas) → 3 (12 tropas) → 4 (15 tropas)
// Cada mejora requiere recursos significativos.
// ============================================================================
bool mejorarBarco(struct Jugador *j) {
  if (!j)
    return false;

  int nivelActual = j->barco.nivelMejora;

  // Verificar que aún se puede mejorar
  if (nivelActual >= 4) {
    printf("[MEJORA BARCO] El barco ya está al nivel máximo (%d)\n",
           nivelActual);
    return false;
  }

  int siguienteNivel = nivelActual + 1;
  int oro, madera, piedra, hierro;

  // Determinar costos según el siguiente nivel (Valores alineados con ui_entrena.h)
  if (siguienteNivel == 2) {
    oro = COSTO_MEJORA_BARCO_2_ORO;
    madera = COSTO_MEJORA_BARCO_2_MADERA;
    piedra = COSTO_MEJORA_BARCO_2_PIEDRA;
    hierro = COSTO_MEJORA_BARCO_2_HIERRO;
  } else if (siguienteNivel == 3) {
    oro = COSTO_MEJORA_BARCO_3_ORO;
    madera = COSTO_MEJORA_BARCO_3_MADERA;
    piedra = COSTO_MEJORA_BARCO_3_PIEDRA;
    hierro = COSTO_MEJORA_BARCO_3_HIERRO;
  } else { // siguienteNivel == 4
    oro = COSTO_MEJORA_BARCO_4_ORO;
    madera = COSTO_MEJORA_BARCO_4_MADERA;
    piedra = COSTO_MEJORA_BARCO_4_PIEDRA;
    hierro = COSTO_MEJORA_BARCO_4_HIERRO;
  }

  // Verificar que el jugador tiene suficientes recursos
  if (j->Oro < oro || j->Madera < madera || j->Piedra < piedra ||
      j->Hierro < hierro) {
    printf("[MEJORA BARCO] Recursos insuficientes. Requiere: %d Oro, %d "
           "Madera, %d Piedra, %d Hierro\n",
           oro, madera, piedra, hierro);
    return false;
  }

  // Descontar recursos
  j->Oro -= oro;
  j->Madera -= madera;
  j->Piedra -= piedra;
  j->Hierro -= hierro;

  // Incrementar nivel de mejora
  j->barco.nivelMejora = siguienteNivel;
  j->barco.capacidadMaxima = siguienteNivel * 3 + 3; // 9, 12, 15

  printf(
      "[MEJORA BARCO] ¡Barco mejorado a nivel %d! Nueva capacidad: %d tropas\n",
      siguienteNivel, j->barco.capacidadMaxima);

  return true;
}

// ============================================================================
// FUNCIÓN DE CONSTRUCCIÓN DEL BARCO
// ============================================================================
// Construye el barco inicialmente destruido, permitiendo su uso para navegar.
// Una vez construido, se pueden aplicar mejoras desde el cuartel.
// ============================================================================
bool construirBarco(struct Jugador *j) {
  if (!j)
    return false;

  // Verificar que el barco no esté ya construido
  if (j->barco.construido) {
    printf("[CONSTRUIR BARCO] El barco ya está construido\n");
    return false;
  }

  // Verificar recursos
  if (j->Oro < COSTO_CONSTRUIR_BARCO_ORO ||
      j->Madera < COSTO_CONSTRUIR_BARCO_MADERA ||
      j->Piedra < COSTO_CONSTRUIR_BARCO_PIEDRA ||
      j->Hierro < COSTO_CONSTRUIR_BARCO_HIERRO) {
    printf("[CONSTRUIR BARCO] Recursos insuficientes. Requiere: %d Oro, "
           "%d Madera, %d Piedra, %d Hierro\n",
           COSTO_CONSTRUIR_BARCO_ORO, COSTO_CONSTRUIR_BARCO_MADERA,
           COSTO_CONSTRUIR_BARCO_PIEDRA, COSTO_CONSTRUIR_BARCO_HIERRO);
    return false;
  }

  // Descontar recursos
  j->Oro -= COSTO_CONSTRUIR_BARCO_ORO;
  j->Madera -= COSTO_CONSTRUIR_BARCO_MADERA;
  j->Piedra -= COSTO_CONSTRUIR_BARCO_PIEDRA;
  j->Hierro -= COSTO_CONSTRUIR_BARCO_HIERRO;

  // Marcar como construido
  j->barco.construido = true;
  j->barco.activo = true;

  // LÓGICA DE ACTIVACIÓN / REPARACIÓN:
  // Si el barco ya tiene una posición válida (reparación), NO moverlo.
  // Solo buscar orilla si es la primera construcción (x=0, y=0).
  if (j->barco.x > 64.0f && j->barco.y > 64.0f) {
      printf("[CONSTRUIR BARCO] Reparando barco en posición existente: (%.1f, %.1f)\n", 
             j->barco.x, j->barco.y);
      return true;
  }

  // Si no tiene posición, buscar orilla
  // Asegurar que el mapa de colisiones esté limpio y actualizado (con agua detectada)
  mapaReconstruirCollisionMap();

  int dir = 0;
  float oldX = j->barco.x;
  float oldY = j->barco.y;
  
  mapaDetectarOrilla(&j->barco.x, &j->barco.y, &dir);
  
  // Si retorno coordenadas de error (1000,1000), intentar usar la posicion anterior si era valida
  if (j->barco.x == 1000.0f && j->barco.y == 1000.0f) {
      if (oldX > 64.0f || oldY > 64.0f) {
           j->barco.x = oldX;
           j->barco.y = oldY;
           printf("[CONSTRUIR BARCO] Fallo deteccion, manteniendo posicion: (%.1f, %.1f)\n", oldX, oldY);
      } else {
           // Fallback extremo: Posición 100,100 (probable agua)
           j->barco.x = 100.0f;
           j->barco.y = 100.0f;
           printf("[CONSTRUIR BARCO] Fallo deteccion total, usando fallback 100,100\n");
      }
  }

  j->barco.dir = (Direccion)dir;

  printf("[CONSTRUIR BARCO] ¡Barco construido! Posición: (%.1f, %.1f), Dir: %d\n", 
         j->barco.x, j->barco.y, dir);

  return true;
}
