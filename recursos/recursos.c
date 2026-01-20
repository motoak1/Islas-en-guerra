#include "recursos.h"
#include "navegacion.h"
#include "edificios/edificios.h"
#include "../mapa/mapa.h"
#include "../mapa/menu.h"
#include "stdbool.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

// --- Animaciones Lógicas ---
// Definimos animaciones constantes para cada dirección.
// Cada animación tiene una dirección base, cantidad de cuadros y velocidad (ticks por cuadro).
static const Animacion gAnimFront = {DIR_FRONT, 4, 6};
static const Animacion gAnimBack = {DIR_BACK, 4, 6};
static const Animacion gAnimLeft = {DIR_LEFT, 4, 6};
static const Animacion gAnimRight = {DIR_RIGHT, 4, 6};

static const Animacion *animPorDireccion(Direccion d) {
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

static void sincronizarIslasConquistadas(struct Jugador *j);

// Pathfinding BFS - Ruta más corta usando arrays básicos
// Implementación del algoritmo de Búsqueda en Anchura (Breadth-First Search).
// Explora el mapa nivel por nivel para encontrar el camino más corto sin usar pesos.
// Se utilizan arrays planos 'padreIdx' y 'bufferBFS' simulando colas para evitar estructuras de datos complejas.
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

static void initUnitArray(Unidad *arr, int count, int type, float baseX, float baseY) {
    for(int i=0; i<count; i++) {
        arr[i].x = (baseX >= 0) ? (baseX + (i * 64.0f)) : -1000.0f;
        arr[i].y = (baseY >= 0) ? baseY : -1000.0f;
        arr[i].destinoX = arr[i].x; arr[i].destinoY = arr[i].y;
        arr[i].moviendose = false; arr[i].seleccionado = false;
        arr[i].celdaFila = -1; arr[i].celdaCol = -1;
        arr[i].rutaCeldas = NULL; arr[i].tipo = type;
        arr[i].animActual = animPorDireccion(DIR_FRONT);
        arr[i].recibiendoAtaque = false;
        
        if (type == TIPO_OBRERO) { arr[i].vidaMax = OBRERO_VIDA_MAX; arr[i].vida = OBRERO_VIDA_MAX; }
        else if (type == TIPO_GUERRERO) { arr[i].vidaMax = 120; arr[i].vida = 120; } // Initial dummy values
        else { arr[i].vidaMax = CABALLERO_VIDA; arr[i].vida = CABALLERO_VIDA; } 
    }
}

void IniciacionRecursos(struct Jugador *j, const char *Nombre) {
  strcpy(j->Nombre, Nombre);
  j->Comida = 200; j->Oro = 0; j->Madera = 150; j->Piedra = 100; j->Hierro = 100;
  j->CantidadEspadas = 0;

  initUnitArray(j->obreros, MAX_OBREROS, TIPO_OBRERO, 900.0f, 900.0f);
  // Ocultar obreros restantes (logic handles > 6 check inside init?)
  // Actually the original code had distinct logic for first 6 obreros. 
  // Let's just fix the positions manually for the first 6 after init or make init smarter.
  // Re-doing Init simplified:
  
  for(int i=0; i<MAX_OBREROS; i++) {
      if(i<6) { j->obreros[i].x = 900.0f + (i*64.0f); j->obreros[i].y = 900.0f; }
      else { j->obreros[i].x = -1000.0f; j->obreros[i].y = -1000.0f; }
      j->obreros[i].destinoX = j->obreros[i].x; j->obreros[i].destinoY = j->obreros[i].y;
      j->obreros[i].moviendose = false; j->obreros[i].seleccionado = false;
      j->obreros[i].celdaFila = -1; j->obreros[i].celdaCol = -1;
      j->obreros[i].rutaCeldas = NULL; j->obreros[i].tipo = TIPO_OBRERO;
      j->obreros[i].animActual = animPorDireccion(DIR_FRONT);
      j->obreros[i].vidaMax = OBRERO_VIDA_MAX; j->obreros[i].vida = OBRERO_VIDA_MAX;
      j->obreros[i].recibiendoAtaque = false;
  }

  initUnitArray(j->caballeros, MAX_CABALLEROS, TIPO_CABALLERO, -1, -1);
  initUnitArray(j->caballerosSinEscudo, MAX_CABALLEROS_SIN_ESCUDO, TIPO_CABALLERO_SIN_ESCUDO, -1, -1);
  initUnitArray(j->guerreros, MAX_GUERREROS, TIPO_GUERRERO, -1, -1);

  for(int i=0; i<6; i++) j->islasConquistadas[i] = false;

  // Inicializar barco
  j->barco.activo = false; j->barco.x = j->barco.y = 0.0f; j->barco.dir = DIR_FRONT;
  j->barco.numTropas = 0; j->barco.nivelMejora = 1; j->barco.capacidadMaxima = 6; j->barco.construido = false;

  // Registrar objetos init
  for(int i=0; i<MAX_OBREROS; i++) if(j->obreros[i].x >= 0) mapaRegistrarObjeto(j->obreros[i].x, j->obreros[i].y, SIMBOLO_OBRERO);
  // Others are x=-1000 so no register needed
}

static void actualizarGrupoUnidades(Unidad *grupo, int count, int **col) {
  const float vel = 5.0f; 
  const float umbralLlegada = 8.0f;
  
  for (int i = 0; i < count; i++) {
    Unidad *u = &grupo[i];
    if (u->x < 0) continue; // Inactivo

    if (u->celdaFila == -1) {
      ocupacionActualizarUnidad(col, u, obreroFilaActual(u), obreroColActual(u));
    }

    if (!u->moviendose) continue;

    int nextF, nextC;
    if (u->rutaCeldas && u->rutaIdx < u->rutaLen) {
      int targetCelda = u->rutaCeldas[u->rutaIdx];
      nextF = targetCelda / GRID_SIZE;
      nextC = targetCelda % GRID_SIZE;
    } else {
      u->moviendose = false;
      continue;
    }

    // Verificar colisiones
    if (nextF >= GRID_SIZE || nextC >= GRID_SIZE) {
       u->moviendose = false; obreroLiberarRuta(u); continue;
    }
    
    int valor = *(*(col + nextF) + nextC);
    if (valor == 1 || valor == 2) { // Bloqueado permanente
       u->moviendose = false; obreroLiberarRuta(u); continue;
    }
    
    // Bloqueo temporal (solo si es destino final)
    if ((u->rutaIdx + 1 >= u->rutaLen) && valor == 3 && (nextF != u->celdaFila || nextC != u->celdaCol)) {
       continue; // Esperar
    }

    // Movimiento
    float tx = celdaCentroPixel(nextC), ty = celdaCentroPixel(nextF);
    float cx = u->x + (TILE_SIZE / 2), cy = u->y + (TILE_SIZE / 2);
    float vx = tx - cx, vy = ty - cy;
    float dist = sqrtf(vx * vx + vy * vy);

    if (dist > 0.1f) {
      if (fabsf(vx) > fabsf(vy)) u->dir = (vx > 0) ? DIR_RIGHT : DIR_LEFT;
      else u->dir = (vy > 0) ? DIR_FRONT : DIR_BACK;
      
      u->animActual = animPorDireccion(u->dir);
      u->animTick++;
      if (u->animTick >= u->animActual->ticksPorCuadro) {
        u->animTick = 0;
        u->frame = (u->frame + 1) % u->animActual->cantidadCuadros;
      }
    }

    if (dist <= umbralLlegada) {
      float viejoX = u->x, viejoY = u->y;
      u->x = tx - (TILE_SIZE / 2); u->y = ty - (TILE_SIZE / 2);
      u->rutaIdx++;
      ocupacionActualizarUnidad(col, u, nextF, nextC);
      mapaMoverObjeto(viejoX, viejoY, u->x, u->y, (char)(u->tipo == TIPO_OBRERO ? SIMBOLO_OBRERO : (u->tipo == TIPO_GUERRERO ? SIMBOLO_GUERRERO : SIMBOLO_CABALLERO)));
      
      if (u->rutaIdx >= u->rutaLen) {
        u->moviendose = false;
        obreroLiberarRuta(u);
      }
    } else {
       float viejoX = u->x, viejoY = u->y;
       float newX = u->x + (vx / dist) * vel;
       float newY = u->y + (vy / dist) * vel;
       // Clamp
       if (newX < 0) newX = 0; 
       if (newY < 0) newY = 0;
       if (newX > (float)(MAPA_SIZE - 64)) newX = (float)(MAPA_SIZE - 64);
       if (newY > (float)(MAPA_SIZE - 64)) newY = (float)(MAPA_SIZE - 64);
       
       u->x = newX; u->y = newY;
       mapaMoverObjeto(viejoX, viejoY, u->x, u->y, (char)(u->tipo == TIPO_OBRERO ? SIMBOLO_OBRERO : (u->tipo == TIPO_GUERRERO ? SIMBOLO_GUERRERO : SIMBOLO_CABALLERO)));
    }
  }
}

void actualizarPersonajes(struct Jugador *j) {
  int **col = mapaObtenerCollisionMap();
  if (!col) return;
  
  actualizarGrupoUnidades(j->obreros, MAX_OBREROS, col);
  actualizarGrupoUnidades(j->caballeros, MAX_CABALLEROS, col);
  actualizarGrupoUnidades(j->caballerosSinEscudo, MAX_CABALLEROS_SIN_ESCUDO, col);
  actualizarGrupoUnidades(j->guerreros, MAX_GUERREROS, col);

  sincronizarIslasConquistadas(j);
}
// Comandar movimiento RTS con separación de unidades
static bool checkCeldaAsignada(int f, int c, int (*destinos)[2], int count) {
    for (int i = 0; i < count; i++) {
        if (destinos[i][0] == f && destinos[i][1] == c) return true;
    }
    return false;
}

static void rtsComandarGrupo(Unidad *array, int count, int gF, int gC, int **col, int (*destinos)[2], int *numDestinos) {
    for (int i = 0; i < count; i++) {
        Unidad *u = &array[i];
        if (!u->seleccionado || u->x < 0 || u->vida <= 0) continue;

        int destinoF = gF, destinoC = gC;

        if (checkCeldaAsignada(destinoF, destinoC, destinos, *numDestinos) || (col[destinoF][destinoC] == 3)) {
            bool encontrado = false;
            for (int r = 1; r <= 3 && !encontrado; r++) {
               for (int dy = -r; dy <= r && !encontrado; dy++) {
                  for (int dx = -r; dx <= r && !encontrado; dx++) {
                     int nf = gF + dy, nc = gC + dx;
                     if (nf >= 0 && nf < GRID_SIZE && nc >= 0 && nc < GRID_SIZE) {
                        if (col[nf][nc] == 0 && !checkCeldaAsignada(nf, nc, destinos, *numDestinos)) {
                           destinoF = nf; destinoC = nc; encontrado = true;
                        }
                     }
                  }
               }
            }
            if (!encontrado) continue; 
        }

        destinos[*numDestinos][0] = destinoF;
        destinos[*numDestinos][1] = destinoC;
        (*numDestinos)++;

        int sF = obreroFilaActual(u), sC = obreroColActual(u);
        if (sF == destinoF && sC == destinoC) continue;

        int *ruta = NULL, len = 0;
        if (pathfindSimple(sF, sC, destinoF, destinoC, col, &ruta, &len)) {
            obreroLiberarRuta(u); u->rutaCeldas = ruta; u->rutaLen = len; u->rutaIdx = 0; u->moviendose = true;
        }
    }
}

void rtsComandarMovimiento(struct Jugador *j, float mundoX, float mundoY) {
  int **col = mapaObtenerCollisionMap();
  if (!col) return;

  int gF = (int)(mundoY / TILE_SIZE);
  int gC = (int)(mundoX / TILE_SIZE);
  if (gF < 0 || gF >= GRID_SIZE || gC < 0 || gC >= GRID_SIZE) return;

  // Validar destino principal
  if (col[gF][gC] == 1 || col[gF][gC] == 2) {
    bool encontrado = false;
    for (int r = 1; r <= 3 && !encontrado; r++) {
       for (int dy = -r; dy <= r && !encontrado; dy++) {
          for (int dx = -r; dx <= r && !encontrado; dx++) {
             int nf = gF + dy, nc = gC + dx;
             if (nf >= 0 && nf < GRID_SIZE && nc >= 0 && nc < GRID_SIZE && col[nf][nc] == 0) {
                gF = nf; gC = nc; encontrado = true;
             }
          }
       }
    }
    if (!encontrado) return;
  }

  int destinosAsignados[MAX_OBREROS + MAX_CABALLEROS + MAX_CABALLEROS_SIN_ESCUDO + MAX_GUERREROS][2];
  int numDestinos = 0;

  rtsComandarGrupo(j->obreros, MAX_OBREROS, gF, gC, col, destinosAsignados, &numDestinos);
  rtsComandarGrupo(j->caballeros, MAX_CABALLEROS, gF, gC, col, destinosAsignados, &numDestinos);
  rtsComandarGrupo(j->caballerosSinEscudo, MAX_CABALLEROS_SIN_ESCUDO, gF, gC, col, destinosAsignados, &numDestinos);
  rtsComandarGrupo(j->guerreros, MAX_GUERREROS, gF, gC, col, destinosAsignados, &numDestinos);
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

// Funciones de entrenamiento de tropas
static bool entrenarUnidadGenerico(struct Jugador *j, Unidad *array, int count, Edificio *cuartel, TipoUnidad tipo) {
    if(!cuartel) return false;
    
    for(int i=0; i<count; i++) {
        if(array[i].x >= 0) continue; // Ocupado

        // Generar posición
        float offsetX = (float)((i % 3) * 70);
        float offsetY = (float)((i / 3) * 70);
        float baseX = cuartel->x + offsetX;
        float baseY = cuartel->y + cuartel->alto + 20 + offsetY;
        
        array[i].x = baseX; array[i].y = baseY;

        // Validar spawn
        int **col = mapaObtenerCollisionMap();
        if(col) {
            int cX = (int)(array[i].x / (float)TILE_SIZE);
            int cY = (int)(array[i].y / (float)TILE_SIZE);

            if(cY < 0 || cY >= GRID_SIZE || cX < 0 || cX >= GRID_SIZE || col[cY][cX] != 0 || mapaObjetos[cY][cX] != 0) {
                bool encontrado = false;
                int centroCX = (int)((cuartel->x + 64)/TILE_SIZE);
                int centroCY = (int)((cuartel->y + 64)/TILE_SIZE);
                
                for(int r=2; r<8 && !encontrado; r++) {
                    for(int dy=-r; dy<=r && !encontrado; dy++) {
                        for(int dx=-r; dx<=r; dx++) {
                            int ny = centroCY + dy; int nx = centroCX + dx;
                             if(ny>=0 && ny<GRID_SIZE && nx>=0 && nx<GRID_SIZE && col[ny][nx]==0 && mapaObjetos[ny][nx]==0) {
                                array[i].x = nx*(float)TILE_SIZE;
                                array[i].y = ny*(float)TILE_SIZE;
                                col[ny][nx] = 3; 
                                mapaRegistrarObjeto(array[i].x, array[i].y, (char)(tipo==TIPO_OBRERO?SIMBOLO_OBRERO:tipo==TIPO_GUERRERO?SIMBOLO_GUERRERO:SIMBOLO_CABALLERO));
                                encontrado = true; break;
                             }
                        }
                    }
                }
            } else {
                col[cY][cX] = 3;
                mapaRegistrarObjeto(array[i].x, array[i].y, (char)(tipo==TIPO_OBRERO?SIMBOLO_OBRERO:tipo==TIPO_GUERRERO?SIMBOLO_GUERRERO:SIMBOLO_CABALLERO));
            }
        }
        
        array[i].destinoX = array[i].x; array[i].destinoY = array[i].y;
        array[i].moviendose = false; array[i].seleccionado = false;
        array[i].dir = DIR_FRONT; array[i].frame = 0;
        array[i].celdaFila = -1; array[i].celdaCol = -1;
        array[i].rutaCeldas = NULL; array[i].tipo = tipo;
        array[i].animActual = animPorDireccion(DIR_FRONT);
        array[i].recibiendoAtaque = false; array[i].tiempoMuerteMs = 0; array[i].frameMuerte = 0;

        if(tipo == TIPO_OBRERO) { array[i].vidaMax = OBRERO_VIDA_MAX; }
        else if(tipo == TIPO_GUERRERO) { array[i].vidaMax = 120; array[i].dano = 30; array[i].critico = GUERRERO_CRITICO; array[i].defensa = 20; array[i].alcance = 64; }
        else { array[i].vidaMax = CABALLERO_VIDA; } // Caballero
        array[i].vida = array[i].vidaMax;

        return true;
    }
    return false;
}

bool entrenarObrero(struct Jugador *j, float x, float y) {
    return entrenarUnidadGenerico(j, j->obreros, MAX_OBREROS, (Edificio*)j->cuartel, TIPO_OBRERO);
}
bool entrenarCaballero(struct Jugador *j, float x, float y) {
    return entrenarUnidadGenerico(j, j->caballeros, MAX_CABALLEROS, (Edificio*)j->cuartel, TIPO_CABALLERO);
}
bool entrenarGuerrero(struct Jugador *j, float x, float y) {
    return entrenarUnidadGenerico(j, j->guerreros, MAX_GUERREROS, (Edificio*)j->cuartel, TIPO_GUERRERO);
}

bool recursosIntentarCazar(struct Jugador *j, float mundoX, float mundoY) {
  // Buscar vaca por posición directa del array
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
      // CORRECCIÓN BUG DE SINCRONIZACIÓN:
      // Usamos el ÍNDICE de la vaca (vacaEncontrada) para eliminarla,
      // no su posición anterior (vacaFila, vacaCol).
      // Esto garantiza que aunque la vaca se haya movido mientras el
      // usuario decidía en el MessageBox, se eliminará la vaca correcta.
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

// Lógica de talar árboles
bool recursosIntentarTalar(struct Jugador *j, float mundoX, float mundoY) {
  // Buscar árbol en celdas 2x2 alrededor del click
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
    // Restricción: Solo obreros y guerreros pueden talar
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
        // --- GOLPEAR ÁRBOL ---
        // Se elimina el MessageBox. Cada click resta 1 vida. 
        // Al 3er golpe (vida=0) se destruye y da madera.
        
        // Declaracion extern para la funcion en mapa.c
        extern bool mapaGolpearArbol(int f, int c);
        
        if (mapaGolpearArbol(fArbol, cArbol)) {
            // Árbol destruido (el último golpe)
            j->Madera += 50;
        } else {
            // Golpe dado pero no destruido
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

  // Fondo del panel (estilo medieval)
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

  // Contador global de islas conquistadas (1..5)
  int totalConquistadas = 0;
  for (int i = 1; i <= 5; i++) {
    if (j->islasConquistadas[i])
      totalConquistadas++;
  }
  sprintf(buffer, "Islas conquistadas: %d/5", totalConquistadas);
  TextOutA(hdcBuffer, panelX + 5, yPos + 43, buffer, strlen(buffer));

  // ================================================================
  // LIMPIEZA DE RECURSOS GDI
  // ================================================================
  SelectObject(hdcBuffer, oldFont);
  DeleteObject(fontTitulo);
  DeleteObject(fontValor);
}

static void sincronizarIslasConquistadas(struct Jugador *j) {
  if (!j)
    return;
  for (int isla = 1; isla <= 5; isla++) {
    if (j->islasConquistadas[isla])
      continue;
    if (navegacionIsIslaConquistada(isla)) {
      j->islasConquistadas[isla] = true;
    }
  }
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



bool entrenarCaballeroSinEscudo(struct Jugador *j, float x, float y) {
    return entrenarUnidadGenerico(j, j->caballerosSinEscudo, MAX_CABALLEROS_SIN_ESCUDO, (Edificio*)j->cuartel, TIPO_CABALLERO_SIN_ESCUDO);
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
    return false;
  }

  // Verificar recursos
  if (j->Oro < COSTO_CONSTRUIR_BARCO_ORO ||
      j->Madera < COSTO_CONSTRUIR_BARCO_MADERA ||
      j->Piedra < COSTO_CONSTRUIR_BARCO_PIEDRA ||
      j->Hierro < COSTO_CONSTRUIR_BARCO_HIERRO) {
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
      } else {
           // Fallback extremo: Posición 100,100 (probable agua)
           j->barco.x = 100.0f;
           j->barco.y = 100.0f;
      }
  }

  j->barco.dir = (Direccion)dir;


  return true;
}
