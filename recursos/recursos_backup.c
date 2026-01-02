#include "recursos.h"
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
  return (float)(celda * TILE_SIZE) + (float)TILE_SIZE * 0.5f;
}

static float celdaAPixel(int celda) { return (float)(celda * TILE_SIZE); }

static int obreroFilaActual(const Unidad *o) {
  return pixelACelda(o->y + 32.0f);
}

static int obreroColActual(const Unidad *o) {
  return pixelACelda(o->x + 32.0f);
}
// 1. Mueve marcarHuellaObrero ARRIBA de ocupacionActualizarUnidad
static void marcarHuellaObrero(int **collision, int fila, int col, int valor) {
  if (!collision || fila < 0 || col < 0 || fila >= GRID_SIZE - 1 ||
      col >= GRID_SIZE - 1)
    return;
  for (int i = 0; i < 2; i++) {
    int *fila_ptr = *(collision + fila + i);
    for (int j = 0; j < 2; j++) {
      *(fila_ptr + col + j) = valor;
    }
  }
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

static bool buscarCeldaLibreCerca(int **collision, int startF, int startC,
                                  int *outF, int *outC) {
  for (int r = 1; r <= 5; r++) {
    for (int df = -r; df <= r; df++) {
      for (int dc = -r; dc <= r; dc++) {
        int f = startF + df, c = startC + dc;
        if (f >= 0 && f < GRID_SIZE && c >= 0 && c < GRID_SIZE) {
          if (collision[f][c] == 0) {
            *outF = f;
            *outC = c;
            return true;
          }
        }
      }
    }
  }
  return false;
}

static void obreroLiberarRuta(Unidad *o) {
  if (o->rutaCeldas)
    free(o->rutaCeldas);
  o->rutaCeldas = NULL;
  o->rutaLen = 0;
  o->rutaIdx = 0;
}

// PATHFINDING SIMPLE: Movimiento greedy sin usar colas.
// Genera una ruta paso a paso moviéndose hacia el objetivo,
// eligiendo la dirección más cercana al destino que no esté bloqueada.
// CRÍTICO: Valida bloques de 2x2 celdas (64x64px) para el obrero.
static bool pathfindSimple(int startF, int startC, int goalF, int goalC,
                           int **collision, int **outRuta, int *outLen) {
  if (startF == goalF && startC == goalC)
    return false;

  // Máximo de pasos para evitar bucles infinitos (diagonal máxima del mapa)
  const int MAX_PASOS = GRID_SIZE * 2;

  // Array temporal para guardar la ruta (máximo MAX_PASOS celdas)
  int *rutaTemp = (int *)malloc(MAX_PASOS * sizeof(int));
  if (!rutaTemp)
    return false;

  // Matriz de visitados para no volver a celdas ya recorridas
  // Usamos un array simple de tamaño GRID_SIZE * GRID_SIZE
  char *visitado = (char *)calloc(GRID_SIZE * GRID_SIZE, sizeof(char));
  if (!visitado) {
    free(rutaTemp);
    return false;
  }

  int pasos = 0;
  int actualF = startF;
  int actualC = startC;

  // Marcar inicio como visitado
  visitado[actualF * GRID_SIZE + actualC] = 1;

  // Direcciones: arriba, abajo, izquierda, derecha
  int dF[4] = {-1, 1, 0, 0};
  int dC[4] = {0, 0, -1, 1};

  // Bucle principal: intentar llegar al objetivo
  while (pasos < MAX_PASOS) {
    // Si llegamos al objetivo, terminamos
    if (actualF == goalF && actualC == goalC) {
      break;
    }

    // Buscar la mejor dirección (la que más nos acerca al objetivo)
    int mejorDir = -1;
    float mejorDist = 999999.0f;

    for (int i = 0; i < 4; i++) {
      int nf = actualF + dF[i];
      int nc = actualC + dC[i];

      // Verificar límites del mapa (considerando bloque 2x2)
      if (nf < 0 || nf >= GRID_SIZE - 1 || nc < 0 || nc >= GRID_SIZE - 1)
        continue;

      // ================================================================
      // VALIDACIÓN CRÍTICA: BLOQUE 2x2 COMPLETO (64x64px)
      // El obrero ocupa 2x2 celdas, por lo que debemos validar
      // las 4 celdas que ocuparía en la nueva posición.
      // Si CUALQUIERA de las 4 celdas está bloqueada (valor 1),
      // esta dirección NO es válida.
      // ================================================================
      bool bloqueado = false;
      for (int df = 0; df < 2; df++) {
        int *fila_ptr = *(collision + nf + df);
        for (int dc = 0; dc < 2; dc++) {
          int valor = *(fila_ptr + nc + dc);
          // Bloqueado si hay agua/árbol (1) o edificio (2)
          if (valor == 1 || valor == 2) {
            bloqueado = true;
            break;
          }
        }
        if (bloqueado)
          break;
      }

      if (bloqueado)
        continue;

      // Verificar si ya fue visitada
      if (visitado[nf * GRID_SIZE + nc])
        continue;

      // Calcular distancia al objetivo (distancia Manhattan)
      int distF = (nf > goalF) ? (nf - goalF) : (goalF - nf);
      int distC = (nc > goalC) ? (nc - goalC) : (goalC - nc);
      float dist = (float)(distF + distC);

      if (dist < mejorDist) {
        mejorDist = dist;
        mejorDir = i;
      }
    }

    // Si no hay dirección válida, no podemos continuar
    if (mejorDir == -1) {
      free(rutaTemp);
      free(visitado);
      return false;
    }

    // Moverse en la mejor dirección
    actualF += dF[mejorDir];
    actualC += dC[mejorDir];

    // Marcar como visitado
    visitado[actualF * GRID_SIZE + actualC] = 1;

    // Agregar a la ruta
    rutaTemp[pasos] = actualF * GRID_SIZE + actualC;
    pasos++;
  }

  free(visitado);

  // Si no llegamos al objetivo
  if (actualF != goalF || actualC != goalC) {
    free(rutaTemp);
    return false;
  }

  // Crear la ruta final con el tamaño exacto
  int *rutaFinal = (int *)malloc(pasos * sizeof(int));
  if (!rutaFinal) {
    free(rutaTemp);
    return false;
  }

  // Copiar la ruta
  for (int i = 0; i < pasos; i++) {
    rutaFinal[i] = rutaTemp[i];
  }

  free(rutaTemp);

  *outRuta = rutaFinal;
  *outLen = pasos;
  return true;
}

void IniciacionRecursos(struct Jugador *j, const char *Nombre) {
  strcpy(j->Nombre, Nombre);
  j->Comida = 200;
  j->Oro = 100;
  j->Madera = 150;
  j->Piedra = 100;
  // Inicializar solo 3 OBREROS (indices 0,1,2), dejar libres 3,4,5
  for (int i = 0; i < 6; i++) {
    if (i < 3) {
      // OBREROS ACTIVOS
      j->obreros[i].x = 400.0f + (i * 70.0f);
      j->obreros[i].y = 400.0f;
    } else {
      // OBREROS INACTIVOS (Fuera de pantalla)
      j->obreros[i].x = -5000.0f;
      j->obreros[i].y = -5000.0f;
    }

    j->obreros[i].moviendose = false;
    j->obreros[i].seleccionado = false;
    j->obreros[i].celdaFila = -1;
    j->obreros[i].celdaCol = -1;
    j->obreros[i].rutaCeldas = NULL;
    j->obreros[i].tipo = TIPO_OBRERO;
    j->obreros[i].animActual = animPorDireccion(DIR_FRONT);
  }

  // ================================================================
  // INICIALIZAR CABALLEROS (NUEVO)
  // Inicializar solo 1 CABALLERO (indice 0), dejar libres 1,2,3
  // ================================================================
  struct {
    int x, y;
  } posCaballeros[4] = {
      {500, 500},
      {532, 500}, // Fila 1
      {500, 532},
      {532, 532} // Fila 2
  };

  for (int i = 0; i < 4; i++) {
    if (i < 1) {
      // CABALLERO ACTIVO
      j->caballeros[i].x = posCaballeros[i].x;
      j->caballeros[i].y = posCaballeros[i].y;
      j->caballeros[i].destinoX = j->caballeros[i].x;
      j->caballeros[i].destinoY = j->caballeros[i].y;
    } else {
      // CABALLEROS INACTIVOS (Fuera de pantalla)
      j->caballeros[i].x = -5000.0f;
      j->caballeros[i].y = -5000.0f;
      j->caballeros[i].destinoX = -5000.0f;
      j->caballeros[i].destinoY = -5000.0f;
    }

    j->caballeros[i].moviendose = false;
    j->caballeros[i].seleccionado = false;
    j->caballeros[i].dir = DIR_FRONT;
    j->caballeros[i].frame = 0;
    j->caballeros[i].celdaFila = -1;
    j->caballeros[i].celdaCol = -1;
    j->caballeros[i].rutaCeldas = NULL;
    j->caballeros[i].tipo = TIPO_CABALLERO;
  }

  printf("[DEBUG] %d caballeros inicializados\n", 4);

  // ================================================================
  // INICIALIZAR BARCO EN LA ORILLA (192x192px)
  // ================================================================
  j->barco.activo = false; // Se activará después de detectar la orilla
  j->barco.x = 0.0f;
  j->barco.y = 0.0f;
  j->barco.dir = DIR_FRONT;

  printf("[DEBUG] Barco inicializado (pendiente de colocacion en orilla)\n");
}

void actualizarObreros(struct Jugador *j) {
  const float vel = 4.0f;
  int **col = mapaObtenerCollisionMap();
  if (!col)
    return;

  for (int i = 0; i < 6; i++) {
    Unidad *o = &j->obreros[i];

    // IGNORAR UNIDADES INACTIVAS (Fuera de pantalla)
    if (o->x < 0)
      continue;

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
    } else {
      o->moviendose = false;
      continue;
    }

    // 3. VALIDACIÓN DE BLOQUE 2x2 (Huella del obrero)
    // Revisamos si el espacio al que se dirige está libre de obstáculos u otros
    // obreros
    bool bloqueado = false;
    for (int f = 0; f < 2; f++) {
      for (int c = 0; c < 2; c++) {
        int checkF = nextF + f;
        int checkC = nextC + c;

        // Verificar límites del mapa
        if (checkF >= GRID_SIZE || checkC >= GRID_SIZE) {
          bloqueado = true;
          break;
        }

        // Aritmética de punteros para obtener el valor de la celda
        int valor = *(*(col + checkF) + checkC);

        // Bloqueado si es Agua/Árbol (1) o Edificio (2)
        if (valor == 1 || valor == 2) {
          bloqueado = true;
          break;
        }

        // Bloqueado si es OTRO Obrero (3)
        if (valor == 3) {
          // Solo es bloqueante si la celda NO es parte de nuestra posición
          // actual
          bool esMiPropiaHuella =
              (checkF >= o->celdaFila && checkF <= o->celdaFila + 1 &&
               checkC >= o->celdaCol && checkC <= o->celdaCol + 1);
          if (!esMiPropiaHuella) {
            bloqueado = true;
            break;
          }
        }
      }
      if (bloqueado)
        break;
    }

    if (bloqueado) {
      o->moviendose = false;
      obreroLiberarRuta(o);
      continue;
    }

    // 4. Cálculo de movimiento hacia el centro de la celda destino
    float tx = celdaCentroPixel(nextC), ty = celdaCentroPixel(nextF);
    float cx = o->x + 32.0f, cy = o->y + 32.0f; // Centro actual del obrero
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

    // 6. Aplicar desplazamiento o finalizar llegada a la celda
    if (dist <= vel) {
      // Llegó al centro de la celda: se posiciona y actualiza su ocupación en
      // la matriz maestra
      o->x = tx - 32.0f;
      o->y = ty - 32.0f;
      o->rutaIdx++;

      // Actualizar la huella 2x2 en la matriz de colisiones
      ocupacionActualizarUnidad(col, o, nextF, nextC);

      if (o->rutaIdx >= o->rutaLen) {
        o->moviendose = false;
        obreroLiberarRuta(o);
      }
    } else {
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
    }
  }

  // ================================================================
  // ACTUALIZAR CABALLEROS (misma lógica)
  // ================================================================
  for (int i = 0; i < 4; i++) {
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

    bool bloqueado = false;
    for (int f = 0; f < 2; f++) {
      for (int c = 0; c < 2; c++) {
        int checkF = nextF + f;
        int checkC = nextC + c;

        if (checkF >= GRID_SIZE || checkC >= GRID_SIZE) {
          bloqueado = true;
          break;
        }

        int valor = *(*(col + checkF) + checkC);

        if (valor == 1 || valor == 2) {
          bloqueado = true;
          break;
        }

        if (valor == 3) {
          bool esMiPropiaHuella =
              (checkF >= u->celdaFila && checkF <= u->celdaFila + 1 &&
               checkC >= u->celdaCol && checkC <= u->celdaCol + 1);
          if (!esMiPropiaHuella) {
            bloqueado = true;
            break;
          }
        }
      }
      if (bloqueado)
        break;
    }

    if (bloqueado) {
      u->moviendose = false;
      obreroLiberarRuta(u);
      continue;
    }

    float tx = celdaCentroPixel(nextC), ty = celdaCentroPixel(nextF);
    float cx = u->x + 32.0f, cy = u->y + 32.0f;
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

    if (dist <= vel) {
      u->x = tx - 32.0f;
      u->y = ty - 32.0f;
      u->rutaIdx++;

      ocupacionActualizarUnidad(col, u, nextF, nextC);

      if (u->rutaIdx >= u->rutaLen) {
        u->moviendose = false;
        obreroLiberarRuta(u);
      }
    } else {
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
  int gF = pixelACelda(mundoY);
  int gC = pixelACelda(mundoX);

  // DIAGNÓSTICO PASO 2: Mostrar conversión
  printf("[DEBUG] Conversion a matriz: gF=%d, gC=%d\n", gF, gC);
  fflush(stdout);

  // RESTRICCIÓN: Verificar límites para bloque 2x2
  // Un obrero necesita espacio para 2x2 celdas (64x64px)
  if (gF < 0 || gF >= GRID_SIZE - 1 || gC < 0 || gC >= GRID_SIZE - 1) {
    printf("[DEBUG] RECHAZADO: Fuera de limites (gF=%d, gC=%d, GRID_SIZE=%d)\n",
           gF, gC, GRID_SIZE);
    fflush(stdout);
    return; // Fuera de límites
  }

  // DIAGNÓSTICO PASO 3: Leer valores de la matriz en el destino
  printf("[DEBUG] Inspeccionando bloque 2x2 en destino:\n");
  for (int df = 0; df < 2; df++) {
    int *fila_ptr = *(col + gF + df);
    for (int dc = 0; dc < 2; dc++) {
      int valor = *(fila_ptr + gC + dc);
      printf("[DEBUG]   Celda[%d][%d] = %d\n", gF + df, gC + dc, valor);
    }
  }
  fflush(stdout);

  // ================================================================
  // VALIDACIÓN DE BLOQUE 2x2 COMPLETO EN EL DESTINO
  // ================================================================
  // El jugador debe ser "sordo" a órdenes sobre agua/obstáculos.
  // Validamos que TODAS las 4 celdas del bloque estén libres.
  // ================================================================
  bool destinoBloqueado = false;
  int motivoBloqueo = -1; // Para debug: qué celda causó el bloqueo
  for (int df = 0; df < 2; df++) {
    int *fila_ptr = *(col + gF + df);
    for (int dc = 0; dc < 2; dc++) {
      int valor = *(fila_ptr + gC + dc);
      // Si hay Agua/Árbol (1) o Edificio (2), rechazar orden
      if (valor == 1 || valor == 2) {
        destinoBloqueado = true;
        motivoBloqueo = valor;
        break;
      }
    }
    if (destinoBloqueado)
      break;
  }

  if (destinoBloqueado) {
    // En lugar de rechazar, buscamos el lugar mas cercano libre para que el
    // obrero camine hacia alla
    if (!buscarCeldaLibreCerca(col, gF, gC, &gF, &gC)) {
      printf("[DEBUG] RECHAZADO: No hay espacio libre cerca del objetivo.\n");
      return;
    }
    printf("[DEBUG] Destino ajustado a punto libre cercano: F:%d, C:%d\n", gF,
           gC);
  }

  // Si llegamos aquí, el destino ES VÁLIDO (original o ajustado)
  printf("[DEBUG] Destino VALIDO: Procediendo a calcular pathfinding...\n");
  fflush(stdout);

  // Contador de unidades que ya recibieron destino (para separación)
  int unidadesAsignadas = 0;

  // Puntero base al array de obreros (aritmética de punteros)
  Unidad *base = j->obreros;

  // Recorrer todas las unidades usando aritmética de punteros
  for (Unidad *o = base; o < base + 6; o++) {
    if (!o->seleccionado)
      continue;

    // Calcular destino para esta unidad
    int destinoF = gF;
    int destinoC = gC;

    // ============================================================
    // SEPARACIÓN DE UNIDADES:
    // Si la celda destino está ocupada (árbol=1 o unidad=2),
    // buscar la celda libre más cercana.
    // ============================================================
    int valorCelda = *(*(col + destinoF) + destinoC);
    if (valorCelda == 1 || valorCelda == 2) {
      // La celda está bloqueada, buscar alternativa
      if (!buscarCeldaLibreCerca(col, destinoF, destinoC, &destinoF,
                                 &destinoC)) {
        continue; // No hay celda libre, esta unidad no se mueve
      }
    }

    // Obtener posición actual de la unidad
    int sF = obreroFilaActual(o);
    int sC = obreroColActual(o);

    // Si ya estamos en el destino, no moverse
    if (sF == destinoF && sC == destinoC)
      continue;

    // Calcular ruta con pathfinding simple (sin colas)
    int *ruta = NULL;
    int len = 0;

    if (pathfindSimple(sF, sC, destinoF, destinoC, col, &ruta, &len)) {
      // Liberar ruta anterior y asignar nueva
      obreroLiberarRuta(o);
      o->rutaCeldas = ruta;
      o->rutaLen = len;
      o->rutaIdx = 0;
      o->moviendose = true;

      // Marcar la celda destino como "reservada" temporalmente (valor 2)
      // para que la siguiente unidad elija otro destino
      *(*(col + destinoF) + destinoC) = 2;
      unidadesAsignadas++;
    }
  }

  // ================================================================
  // COMANDAR CABALLEROS (misma lógica)
  // ================================================================
  Unidad *baseCaballeros = j->caballeros;
  for (Unidad *u = baseCaballeros; u < baseCaballeros + 4; u++) {
    if (!u->seleccionado)
      continue;

    // Calcular offset para separación
    int offF = (unidadesAsignadas / 3) - 1;
    int offC = (unidadesAsignadas % 3) - 1;
    int targetF = gF + offF;
    int targetC = gC + offC;
    targetF =
        (targetF < 0) ? 0 : ((targetF >= GRID_SIZE) ? GRID_SIZE - 1 : targetF);
    targetC =
        (targetC < 0) ? 0 : ((targetC >= GRID_SIZE) ? GRID_SIZE - 1 : targetC);

    // ============================================================
    // VALIDACIÓN DE DESTINO (NUEVO)
    // ============================================================
    int valorCelda = *(*(col + targetF) + targetC);
    if (valorCelda == 1 || valorCelda == 2) {
      // La celda está bloqueada, buscar alternativa
      if (!buscarCeldaLibreCerca(col, targetF, targetC, &targetF, &targetC)) {
        continue; // No hay celda libre, esta unidad no se mueve
      }
    }

    // Pathfinding
    int *path = NULL;
    int len = 0;
    bool success = pathfindSimple(obreroFilaActual(u), obreroColActual(u),
                                  targetF, targetC, col, &path, &len);

    if (success) {
      obreroLiberarRuta(u);
      u->rutaCeldas = path;
      u->rutaLen = len;
      u->rutaIdx = 0;
      u->objetivoFila = targetF;
      u->objetivoCol = targetC;
      u->moviendose = true;
    }

    unidadesAsignadas++;
  }
}

// ============================================================================
// LÓGICA DE TALAR ÁRBOLES
// ============================================================================
bool recursosIntentarTalar(struct Jugador *j, float mundoX, float mundoY) {
  // 1. Verificar qué objeto hay en el mapa (coordenadas mundo -> celda)
  int f = (int)(mundoY / TILE_SIZE);
  int c = (int)(mundoX / TILE_SIZE);

  // Validar límites
  if (f < 0 || f >= GRID_SIZE || c < 0 || c >= GRID_SIZE)
    return false;

  int tipoObjeto = mapaObtenerTipoObjeto(f, c);

  // Los árboles son tipos 1 a 4
  if (tipoObjeto >= 1 && tipoObjeto <= 4) {
    printf("[DEBUG] Talar: Click en arbol tipo %d en Celda[%d][%d]\n",
           tipoObjeto, f, c);

    // 2. Buscar si hay algún obrero SELECCIONADO cerca
    Unidad *obreroCercano = NULL;
    float distMinima = 9999.0f;
    const float DISTANCIA_MAXIMA = 150.0f; // Aprox 4-5 celdas

    for (int i = 0; i < 6; i++) {
      Unidad *o = &j->obreros[i];
      if (!o->seleccionado)
        continue;

      // Distancia Euclídea
      float dx = (o->x + 32.0f) - mundoX;
      float dy = (o->y + 32.0f) - mundoY;
      float dist = sqrtf(dx * dx + dy * dy);

      if (dist < DISTANCIA_MAXIMA && dist < distMinima) {
        distMinima = dist;
        obreroCercano = o;
      }
    }

    if (obreroCercano != NULL) {
      // 3. Confirmación del usuario
      int respuesta =
          MessageBox(NULL, "¿Quieres talar este arbol y obtener madera?",
                     "Talar Arbol", MB_YESNO | MB_ICONQUESTION);

      if (respuesta == IDYES) {
        // ... ejecutar accion ...
        mapaEliminarObjeto(f, c);
        j->Madera += 50;
        MessageBox(NULL, "Arbol talado! +50 Madera", "Recursos",
                   MB_OK | MB_ICONINFORMATION);
      }
      return true; // Accion manejada (confirmada o cancelada)
    }

    // Si no hay obrero cerca, retornamos FALSE para que el click
    // pase a la logica de movimiento y el obrero camine hacia el arbol
    return false;
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
    // 2. Verificar si hay recursos
    if (e->oroAcumulado <= 0 && e->piedraAcumulada <= 0) {
      MessageBox(NULL, "La mina no tiene recursos acumulados aun.",
                 "Mina Vacia", MB_OK | MB_ICONINFORMATION);
      return true; // Click manejado
    }

    // 3. Buscar obrero seleccionado cerca
    Unidad *obreroCercano = NULL;
    float distMinima = 9999.0f;
    const float DISTANCIA_MAXIMA = 200.0f;

    for (int i = 0; i < 6; i++) {
      Unidad *o = &j->obreros[i];
      if (!o->seleccionado)
        continue;

      float dx = (o->x + 32.0f) - mundoX;
      float dy = (o->y + 32.0f) - mundoY;
      float dist = sqrtf(dx * dx + dy * dy);

      if (dist < DISTANCIA_MAXIMA && dist < distMinima) {
        distMinima = dist;
        obreroCercano = o;
      }
    }

    if (obreroCercano != NULL) {
      char msg[256];
      sprintf(msg,
              "¿Quieres recoger los recursos de la mina?\n\nOro acumulado: "
              "%d\nPiedra acumulada: %d",
              e->oroAcumulado, e->piedraAcumulada);

      int respuesta =
          MessageBox(NULL, msg, "Recoger Recursos", MB_YESNO | MB_ICONQUESTION);

      if (respuesta == IDYES) {
        j->Oro += e->oroAcumulado;
        j->Piedra += e->piedraAcumulada;
        e->oroAcumulado = 0;
        e->piedraAcumulada = 0;
        MessageBox(NULL, "¡Recursos recogidos con exito!", "Recursos",
                   MB_OK | MB_ICONINFORMATION);
      }
      return true;
    }

    // Si no hay obrero cerca, permitimos el movimiento hacia la mina
    return false;
  }

  return false;
}

void rtsLiberarMovimientoJugador(struct Jugador *j) {
  // Liberar rutas de obreros
  for (int i = 0; i < 6; i++) {
    obreroLiberarRuta(&j->obreros[i]);
  }
  // Liberar rutas de caballeros
  for (int i = 0; i < 4; i++) {
    obreroLiberarRuta(&j->caballeros[i]);
  }
}

// ============================================================================
// FUNCIONES DE ENTRENAMIENTO DE TROPAS
// ============================================================================

bool entrenarObrero(struct Jugador *j, float x, float y) {
  // Buscar un espacio disponible en el array de obreros
  for (int i = 0; i < 6; i++) {
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
        int cX = (int)(j->obreros[i].x / 32.0f);
        int cY = (int)(j->obreros[i].y / 32.0f);

        // Si la posición inicial es inválida (no es 0), buscar vecino libre
        if (cY < 0 || cY >= GRID_SIZE || cX < 0 || cX >= GRID_SIZE ||
            col[cY][cX] != 0) {
          bool encontrado = false;
          // Buscar en un radio creciente alrededor del cuartel
          int centroCX = (int)((cuartel->x + 64) / 32.0f);
          int centroCY = (int)((cuartel->y + 64) / 32.0f);

          for (int r = 2; r < 8; r++) { // Radio 2 a 8 tiles
            for (int dy = -r; dy <= r; dy++) {
              for (int dx = -r; dx <= r; dx++) {
                int ny = centroCY + dy;
                int nx = centroCX + dx;
                if (ny >= 0 && ny < GRID_SIZE && nx >= 0 && nx < GRID_SIZE) {
                  if (col[ny][nx] == 0) { // Celda libre encontrada
                    j->obreros[i].x = nx * 32.0f;
                    j->obreros[i].y = ny * 32.0f;
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
  for (int i = 0; i < 4; i++) {
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
        int cX = (int)(j->caballeros[i].x / 32.0f);
        int cY = (int)(j->caballeros[i].y / 32.0f);

        // Si la posición inicial es inválida, buscar vecino libre
        if (cY < 0 || cY >= GRID_SIZE || cX < 0 || cX >= GRID_SIZE ||
            col[cY][cX] != 0) {
          bool encontrado = false;
          // Buscar en un radio creciente alrededor del cuartel
          int centroCX = (int)((cuartel->x + 64) / 32.0f);
          int centroCY = (int)((cuartel->y + 64) / 32.0f);

          for (int r = 2; r < 8; r++) {
            for (int dy = -r; dy <= r; dy++) {
              for (int dx = -r; dx <= r; dx++) {
                int ny = centroCY + dy;
                int nx = centroCX + dx;
                if (ny >= 0 && ny < GRID_SIZE && nx >= 0 && nx < GRID_SIZE) {
                  if (col[ny][nx] == 0) { // Celda libre
                    j->caballeros[i].x = nx * 32.0f;
                    j->caballeros[i].y = ny * 32.0f;
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

      printf("[CUARTEL] Nuevo caballero entrenado en posición (%.1f, %.1f)\n",
             j->caballeros[i].x, j->caballeros[i].y);
      return true;
    }
  }

  // No hay espacio disponible
  return false;
}

bool recursosIntentarCazar(struct Jugador *j, float mundoX, float mundoY) {
  // 1. Verificar qué objeto hay en el mapa (coordenadas mundo -> celda)
  int clickF = (int)(mundoY / TILE_SIZE);
  int clickC = (int)(mundoX / TILE_SIZE);

  // Buscar en la celda clickeada y sus vecinas (arriba/izquierda)
  // porque la vaca mide 2x2 tiles pero solo se registra en la esquina superior
  // izquierda.
  int targets[4][2] = {{0, 0}, {0, -1}, {-1, 0}, {-1, -1}};
  int tipoObjeto = 0;
  int f = -1, c = -1;

  for (int k = 0; k < 4; k++) {
    int tf = clickF + targets[k][0];
    int tc = clickC + targets[k][1];

    if (tf >= 0 && tf < GRID_SIZE && tc >= 0 && tc < GRID_SIZE) {
      int t = mapaObtenerTipoObjeto(tf, tc);
      if (t >= 5 && t <= 8) {
        tipoObjeto = t;
        f = tf;
        c = tc;
        break; // Vaca encontrada
      }
    }
  }

  // Las vacas son tipos 5 a 8 (Front, Back, Left, Right)
  if (tipoObjeto >= 5 && tipoObjeto <= 8) {
    printf("[DEBUG] Cazar: Click (o vecindad) en vaca (tipo %d) anclada en "
           "Celda[%d][%d]\n",
           tipoObjeto, f, c);

    // 2. Buscar si hay algún obrero O caballero SELECCIONADO cerca
    bool tropaCercana = false;
    const float DISTANCIA_MAXIMA = 200.0f; // Aumentado un poco por seguridad

    // Revisar Obreros
    for (int i = 0; i < 6; i++) {
      Unidad *o = &j->obreros[i];
      if (!o->seleccionado)
        continue;
      float dx = (o->x + 32.0f) - mundoX;
      float dy = (o->y + 32.0f) - mundoY;
      float dist = sqrtf(dx * dx + dy * dy);
      if (dist < DISTANCIA_MAXIMA) {
        tropaCercana = true;
        break;
      }
    }

    // Revisar Caballeros (si no se encontró obrero)
    if (!tropaCercana) {
      for (int i = 0; i < 4; i++) {
        Unidad *cab = &j->caballeros[i];
        if (!cab->seleccionado)
          continue;
        float dx = (cab->x + 32.0f) - mundoX;
        float dy = (cab->y + 32.0f) - mundoY;
        float dist = sqrtf(dx * dx + dy * dy);
        if (dist < DISTANCIA_MAXIMA) {
          tropaCercana = true;
          break;
        }
      }
    }

    if (tropaCercana) {
      // 3. Confirmación
      int respuesta = MessageBox(NULL, "¿Quieres cazar esta vaca por comida?",
                                 "Cazar Vaca", MB_YESNO | MB_ICONQUESTION);

      if (respuesta == IDYES) {
        // Eliminar objeto de la matriz
        mapaEliminarObjeto(f, c);
        j->Comida += 100;
        MessageBox(NULL, "¡Vaca cazada! +100 Comida", "Recursos",
                   MB_OK | MB_ICONINFORMATION);
      }
      return true; // Click manejado
    }

    // Si no hay tropa cerca, retornamos FALSE para que el click pase a la
    // logica de movimiento y la unidad camine hacia la vaca.
    return false;
  }

  // No era una vaca
  return false;
}
