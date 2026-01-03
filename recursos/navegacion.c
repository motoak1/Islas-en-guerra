#include "navegacion.h"
#include "../mapa/mapa.h"
#include "../edificios/edificios.h"
#include <stdio.h>
#include <math.h>

// Estado persistente por isla (1..3)
typedef struct {
  bool inicializado;
  int Comida, Oro, Madera, Piedra;
  bool tieneAyuntamiento;
  bool tieneMina;
  bool tieneCuartel;
  Edificio ayuntamiento;
  Edificio mina;
  Edificio cuartel;
  Unidad obreros[6];
  Unidad caballeros[4];
  Unidad guerreros[4];
} EstadoIsla;

static EstadoIsla sIslas[4];

// Limpia un rectángulo de celdas en mapaObjetos y collision map
static void limpiarAreaCeldas(int celdaX, int celdaY, int ancho, int alto) {
  int **col = mapaObtenerCollisionMap();

  for (int f = celdaY; f < celdaY + alto; f++) {
    for (int c = celdaX; c < celdaX + ancho; c++) {
      if (f < 0 || c < 0 || f >= GRID_SIZE || c >= GRID_SIZE) continue;
      mapaObjetos[f][c] = SIMBOLO_VACIO;
      if (col) {
        *(*(col + f) + c) = 0;
      }
    }
  }
}

// Verifica si un rectángulo de celdas está libre en collision map y no es agua
static bool areaLibre(int celdaX, int celdaY, int ancho, int alto) {
  int **col = mapaObtenerCollisionMap();
  if (!col) return false;

  for (int f = celdaY; f < celdaY + alto; f++) {
    for (int c = celdaX; c < celdaX + ancho; c++) {
      if (f < 0 || c < 0 || f >= GRID_SIZE || c >= GRID_SIZE) return false;
      int valor = *(*(col + f) + c);
      char simbolo = mapaObjetos[f][c];
      if (valor != 0) return false;       // Bloqueado por agua/árbol/edificio
      if (simbolo == SIMBOLO_AGUA) return false; // Evitar agua marcada
    }
  }
  return true;
}

// Determina si la celda es tierra pegada al agua (orilla)
static bool esTierraOrilla(int cx, int cy, int **col) {
  if (cx < 0 || cy < 0 || cx >= GRID_SIZE || cy >= GRID_SIZE) return false;
  if (!mapaCeldaEsTierra(cy, cx)) return false;

  const int dx[4] = {1, -1, 0, 0};
  const int dy[4] = {0, 0, 1, -1};
  for (int k = 0; k < 4; k++) {
    int nx = cx + dx[k];
    int ny = cy + dy[k];
    if (nx < 0 || ny < 0 || nx >= GRID_SIZE || ny >= GRID_SIZE) continue;
    int v2 = *(*(col + ny) + nx);
    char s2 = mapaObjetos[ny][nx];
    if (v2 == 1 || s2 == SIMBOLO_AGUA) return true;
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
          if (outX) *outX = cx;
          if (outY) *outY = cy;
          return true;
        }
      }
    }
  }
  return false;
}

// Inicializa edificios en las posiciones base usadas al inicio del juego
static void inicializarEstructurasIslaBase(struct Jugador* j, EstadoIsla* estado) {
  const float AYUNT_X = 1024.0f - 64.0f;
  const float AYUNT_Y = 1024.0f - 64.0f;
  const float MINA_X  = 1024.0f - 64.0f;
  const float MINA_Y  = 450.0f;
  const float CUAR_X  = 1024.0f - 64.0f;
  const float CUAR_Y  = 1600.0f;

  edificioInicializar(&estado->ayuntamiento, EDIFICIO_AYUNTAMIENTO, AYUNT_X, AYUNT_Y);
  edificioInicializar(&estado->mina, EDIFICIO_MINA, MINA_X, MINA_Y);
  edificioInicializar(&estado->cuartel, EDIFICIO_CUARTEL, CUAR_X, CUAR_Y);

  estado->tieneAyuntamiento = true;
  estado->tieneMina = true;
  estado->tieneCuartel = true;

  // Marcar en mapa de colisión y objetos (para asegurar consistencia)
  mapaMarcarEdificio(AYUNT_X, AYUNT_Y, estado->ayuntamiento.ancho, estado->ayuntamiento.alto);
  mapaRegistrarObjeto(AYUNT_X, AYUNT_Y, SIMBOLO_EDIFICIO);

  mapaMarcarEdificio(MINA_X, MINA_Y, estado->mina.ancho, estado->mina.alto);
  mapaRegistrarObjeto(MINA_X, MINA_Y, SIMBOLO_MINA);

  mapaMarcarEdificio(CUAR_X, CUAR_Y, estado->cuartel.ancho, estado->cuartel.alto);
  mapaRegistrarObjeto(CUAR_X, CUAR_Y, SIMBOLO_CUARTEL);

  // Apuntar jugador a estas instancias
  j->ayuntamiento = &estado->ayuntamiento;
  j->mina = &estado->mina;
  j->cuartel = &estado->cuartel;
}

// Envía todas las unidades fuera del mapa (se usarán solo las del barco al desembarcar)
static void vaciarUnidades(struct Jugador* j) {
  for (int i = 0; i < 6; i++) {
    j->obreros[i].x = -1000.0f; j->obreros[i].y = -1000.0f;
    j->obreros[i].moviendose = false; j->obreros[i].seleccionado = false;
    j->obreros[i].celdaFila = -1; j->obreros[i].celdaCol = -1;
  }
  for (int i = 0; i < 4; i++) {
    j->caballeros[i].x = -1000.0f; j->caballeros[i].y = -1000.0f;
    j->caballeros[i].moviendose = false; j->caballeros[i].seleccionado = false;
    j->caballeros[i].celdaFila = -1; j->caballeros[i].celdaCol = -1;
  }
  for (int i = 0; i < 4; i++) {
    j->guerreros[i].x = -1000.0f; j->guerreros[i].y = -1000.0f;
    j->guerreros[i].moviendose = false; j->guerreros[i].seleccionado = false;
    j->guerreros[i].celdaFila = -1; j->guerreros[i].celdaCol = -1;
  }
}

// Desembarca tropas juntas cerca del centro de la isla
static void desembarcarTropasEnCentro(Barco* barco, struct Jugador* j) {
  int baseCeldaX = GRID_SIZE / 2;
  int baseCeldaY = (GRID_SIZE / 2) + 2;

  int desembarcadas = 0;
  for (int i = 0; i < barco->numTropas; i++) {
    Unidad* tropa = barco->tropas[i];
    if (!tropa) continue;

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

  for (int k = 0; k < 6; k++) barco->tropas[k] = NULL;
  barco->numTropas = 0;

  printf("[DEBUG] Tropas desembarcadas en el centro: %d\n", desembarcadas);
}

// Guarda el estado de recursos y edificios del jugador para la isla actual
static void guardarEstadoIslaJugador(struct Jugador* j) {
  int isla = j->islaActual;
  if (isla < 1 || isla > 3) return;
  EstadoIsla* estado = &sIslas[isla];

  estado->Comida = j->Comida;
  estado->Oro = j->Oro;
  estado->Madera = j->Madera;
  estado->Piedra = j->Piedra;

  estado->tieneAyuntamiento = (j->ayuntamiento != NULL);
  if (estado->tieneAyuntamiento) estado->ayuntamiento = *((Edificio*)j->ayuntamiento);

  estado->tieneMina = (j->mina != NULL);
  if (estado->tieneMina) estado->mina = *((Edificio*)j->mina);

  estado->tieneCuartel = (j->cuartel != NULL);
  if (estado->tieneCuartel) estado->cuartel = *((Edificio*)j->cuartel);

  // Copiar unidades (posiciones actuales en esta isla)
  for (int i = 0; i < 6; i++) estado->obreros[i] = j->obreros[i];
  for (int i = 0; i < 4; i++) estado->caballeros[i] = j->caballeros[i];
  for (int i = 0; i < 4; i++) estado->guerreros[i] = j->guerreros[i];

  estado->inicializado = true;
  printf("[DEBUG] Jugador: estado de isla %d guardado\n", isla);
}

// Restaura recursos y edificios del jugador al cambiar a otra isla
static void restaurarEstadoIslaJugador(struct Jugador* j, int isla) {
  if (isla < 1 || isla > 3) return;
  EstadoIsla* estado = &sIslas[isla];
  if (!estado->inicializado) return;

  j->Comida = estado->Comida;
  j->Oro = estado->Oro;
  j->Madera = estado->Madera;
  j->Piedra = estado->Piedra;

  j->ayuntamiento = estado->tieneAyuntamiento ? &estado->ayuntamiento : NULL;
  j->mina = estado->tieneMina ? &estado->mina : NULL;
  j->cuartel = estado->tieneCuartel ? &estado->cuartel : NULL;

  // Restaurar unidades de esta isla
  for (int i = 0; i < 6; i++) j->obreros[i] = estado->obreros[i];
  for (int i = 0; i < 4; i++) j->caballeros[i] = estado->caballeros[i];
  for (int i = 0; i < 4; i++) j->guerreros[i] = estado->guerreros[i];

  printf("[DEBUG] Jugador: estado de isla %d restaurado\n", isla);
}

// ============================================================================
// FUNCIONES AUXILIARES
// ============================================================================

bool barcoContienePunto(Barco* barco, float mundoX, float mundoY) {
  const float BARCO_SIZE = 192.0f;
  
  return (mundoX >= barco->x && mundoX < barco->x + BARCO_SIZE &&
          mundoY >= barco->y && mundoY < barco->y + BARCO_SIZE);
}

void desembarcarTropas(Barco* barco, struct Jugador* j) {
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
  
  // Buscar tierra adyacente al barco (búsqueda en espiral) dando prioridad a orilla (tierra pegada a agua)
  int tierraX = -1, tierraY = -1;
  bool tierraEncontrada = false;

  // Buscar en radio creciente alrededor del barco (hasta 12 celdas)
  for (int radio = 1; radio <= 12 && !tierraEncontrada; radio++) {
    for (int dy = -radio; dy <= radio && !tierraEncontrada; dy++) {
      for (int dx = -radio; dx <= radio; dx++) {
        int celdaX = barcoCeldaX + dx;
        int celdaY = barcoCeldaY + dy;
        if (celdaX < 0 || celdaX >= GRID_SIZE || celdaY < 0 || celdaY >= GRID_SIZE) continue;
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
    printf("[WARNING] No se encontró tierra cerca del barco, usando posición de emergencia\n");
    tierraX = 32; // Centro del mapa
    tierraY = 32;
  }

  printf("[DEBUG] Punto de desembarco en celda: [%d][%d]\n", tierraY, tierraX);

  // Colocar tropas una por celda de tierra cercana para evitar agua
  int colocadas = 0;
  int usados[6][2];
  for (int u = 0; u < 6; u++) { usados[u][0] = usados[u][1] = -1; }

  for (int i = 0; i < barco->numTropas; i++) {
    Unidad* tropa = barco->tropas[i];
    if (!tropa) continue;

    bool puesta = false;
    // Búsqueda en radio creciente desde la celda de tierra encontrada
    for (int radio = 0; radio <= 3 && !puesta; radio++) {
      for (int dy = -radio; dy <= radio && !puesta; dy++) {
        for (int dx = -radio; dx <= radio; dx++) {
          int cx = tierraX + dx;
          int cy = tierraY + dy;
          if (cx < 0 || cy < 0 || cx >= GRID_SIZE || cy >= GRID_SIZE) continue;

          // Evitar reutilizar la misma celda
          bool yaUsada = false;
          for (int k = 0; k < colocadas; k++) {
            if (usados[k][0] == cx && usados[k][1] == cy) { yaUsada = true; break; }
          }
          if (yaUsada) continue;

          int valor = *(*(col + cy) + cx);
          if (valor != 0) continue; // bloqueado por agua/obstáculo
          if (!mapaCeldaEsTierra(cy, cx)) continue; // seguridad anti-agua

          // Asignar posición (esquina de celda)
          tropa->x = (float)(cx * TILE_SIZE);
          tropa->y = (float)(cy * TILE_SIZE);
          tropa->destinoX = tropa->x;
          tropa->destinoY = tropa->y;
          tropa->moviendose = false;

          usados[colocadas][0] = cx;
          usados[colocadas][1] = cy;
          colocadas++;
          printf("[DEBUG] Tropa %d desembarcada en celda [%d][%d] (%.1f, %.1f)\n",
                 i, cy, cx, tropa->x, tropa->y);
          puesta = true;
        }
      }
    }
  }

  // Vaciar barco
  barco->numTropas = 0;
}

// ============================================================================
// REINICIAR ISLA DESCONOCIDA
// ============================================================================
// Cuando el jugador llega a una nueva isla, esta es una isla "desconocida"
// sin edificios desarrollados, con recursos básicos, etc.
// Esta función resetea el estado del jugador para la nueva isla.
// ============================================================================
void reiniciarIslaDesconocida(struct Jugador* j) {
  printf("[DEBUG] Reiniciando isla desconocida...\n");
  
  // CRÍTICO: Resetear recursos a valores iniciales bajos (isla sin desarrollar)
  j->Comida = 50;    // Menos recursos que al inicio
  j->Oro = 30;
  j->Madera = 40;
  j->Piedra = 30;
  
  // CRÍTICO: Eliminar edificios (la nueva isla no tiene edificios)
  j->ayuntamiento = NULL;
  j->mina = NULL;
  j->cuartel = NULL;
  
  // NOTA: Los personajes ya fueron desembarcados correctamente ANTES de llamar esta función
  // Por lo tanto, NO necesitamos hacer nada con ellos aquí
  // Solo las tropas desembarcadas existen en esta isla nueva
  
  printf("[DEBUG] Isla reiniciada: Recursos=%d oro, %d comida, %d madera, %d piedra\n",
         j->Oro, j->Comida, j->Madera, j->Piedra);
  printf("[DEBUG] Solo las tropas desembarcadas están disponibles\n");
}


// ============================================================================
// VIAJE DIRECTO A ISLA (SIN ANIMACIÓN)
// ============================================================================
// Cuando el jugador selecciona una isla después de embarcar tropas,
// viaja directamente sin animación del barco.
// ============================================================================

void viajarAIsla(struct Jugador* j, int islaDestino) {
  printf("[DEBUG] Viajando directamente a isla %d\n", islaDestino);
  
  // Si es la misma isla, desembarcar y listo
  if (islaDestino == j->islaActual) {
    printf("[DEBUG] Ya estás en la isla %d, desembarcando tropas\n", islaDestino);
    desembarcarTropas(&j->barco, j);
    return;
  }
  
  // Guardar estado de la isla actual antes de salir
  guardarEstadoIslaJugador(j);
  mapaGuardarEstadoIsla(j->islaActual);

  // Cambiar isla activa
  j->islaActual = islaDestino;
  bool islaYaVisitada = (islaDestino >= 1 && islaDestino <= 3) && sIslas[islaDestino].inicializado;
  
  // Cambiar mapa de isla y recargar gráficos PRIMERO
  mapaSeleccionarIsla(islaDestino);
  cargarRecursosGraficos(); // Esto crea el nuevo mapa de colisión

  // Si la isla ya tiene estado guardado, restaurarlo
  if (islaYaVisitada) {
    mapaRestaurarEstadoIsla(islaDestino);
    restaurarEstadoIslaJugador(j, islaDestino);
  } else {
    // Primera vez en la isla: resetear y generar base
    reiniciarIslaDesconocida(j);
    inicializarEstructurasIslaBase(j, &sIslas[islaDestino]);
    sIslas[islaDestino].inicializado = true;
    vaciarUnidades(j); // No generar tropas nuevas en islas visitadas en barco
    guardarEstadoIslaJugador(j); // Guardar snapshot inicial de recursos/edificios
  }
  
  // Posicionar barco en orilla de la nueva isla
  float nuevoBarcoX, nuevoBarcoY;
  int nuevoDir;
  mapaDetectarOrilla(&nuevoBarcoX, &nuevoBarcoY, &nuevoDir);
  j->barco.x = nuevoBarcoX;
  j->barco.y = nuevoBarcoY;
  j->barco.dir = (Direccion)nuevoDir;
  
  printf("[DEBUG] Barco reposicionado en isla %d: (%.1f, %.1f)\n",
         islaDestino, j->barco.x, j->barco.y);

  // Desembarcar tropas en la orilla más cercana al barco
  desembarcarTropas(&j->barco, j);
  
  printf("[DEBUG] Viaje completado\n");
}

