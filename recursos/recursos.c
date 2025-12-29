#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include "../mapa/menu.h"
#include "../mapa/mapa.h"
#include "recursos.h"
#include "stdbool.h"
#include <math.h>

// --- Animaciones Lógicas ---
static const Animation gAnimFront = { DIR_FRONT, 4, 6 };
static const Animation gAnimBack  = { DIR_BACK,  4, 6 };
static const Animation gAnimLeft  = { DIR_LEFT,  4, 6 };
static const Animation gAnimRight = { DIR_RIGHT, 4, 6 };

static const Animation *animPorDireccion(Direccion d) {
    switch (d) {
        case DIR_BACK:  return &gAnimBack;
        case DIR_LEFT:  return &gAnimLeft;
        case DIR_RIGHT: return &gAnimRight;
        case DIR_FRONT:
        default:        return &gAnimFront;
    }
}

// --- Utilidades de Grid ---
static int clampInt(int v, int lo, int hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

static int pixelACelda(float px) {
    int c = (int)(px / (float)TILE_SIZE);
    int resultado = clampInt(c, 0, GRID_SIZE - 1);
    
    // DEBUG: Verificar conversión (comentar después de confirmar)
    // printf("[DEBUG pixelACelda] px=%.2f -> c=%d -> resultado=%d (TILE_SIZE=%d)\n", 
    //        px, c, resultado, TILE_SIZE);
    
    return resultado;
}

static float celdaCentroPixel(int celda) {
    return (float)(celda * TILE_SIZE) + (float)TILE_SIZE * 0.5f;
}

static float celdaAPixel(int celda) {
    return (float)(celda * TILE_SIZE);
}

static int obreroFilaActual(const Unidad *o) {
    return pixelACelda(o->y + 32.0f);
}

static int obreroColActual(const Unidad *o) {
    return pixelACelda(o->x + 32.0f);
}
// 1. Mueve marcarHuellaObrero ARRIBA de ocupacionActualizarUnidad
static void marcarHuellaObrero(int **collision, int fila, int col, int valor) {
    if (!collision || fila < 0 || col < 0 || fila >= GRID_SIZE - 1 || col >= GRID_SIZE - 1) return;
    for (int i = 0; i < 2; i++) {
        int *fila_ptr = *(collision + fila + i);
        for (int j = 0; j < 2; j++) {
            *(fila_ptr + col + j) = valor;
        }
    }
}

// Ahora esta función ya conoce a la anterior
static void ocupacionActualizarUnidad(int **collision, Unidad *o, int nuevaF, int nuevaC) {
    if (!collision) return;
    if (o->celdaFila >= 0 && o->celdaCol >= 0) {
        marcarHuellaObrero(collision, o->celdaFila, o->celdaCol, 0);
    }
    o->celdaFila = nuevaF;
    o->celdaCol = nuevaC;
    marcarHuellaObrero(collision, nuevaF, nuevaC, 3);
}

static bool buscarCeldaLibreCerca(int **collision, int startF, int startC, int *outF, int *outC) {
    for (int r = 1; r <= 5; r++) {
        for (int df = -r; df <= r; df++) {
            for (int dc = -r; dc <= r; dc++) {
                int f = startF + df, c = startC + dc;
                if (f >= 0 && f < GRID_SIZE && c >= 0 && c < GRID_SIZE) {
                    if (collision[f][c] == 0) {
                        *outF = f; *outC = c; return true;
                    }
                }
            }
        }
    }
    return false;
}

static void obreroLiberarRuta(Unidad *o) {
    if (o->rutaCeldas) free(o->rutaCeldas);
    o->rutaCeldas = NULL;
    o->rutaLen = 0;
    o->rutaIdx = 0;
}

// PATHFINDING SIMPLE: Movimiento greedy sin usar colas.
// Genera una ruta paso a paso moviéndose hacia el objetivo,
// eligiendo la dirección más cercana al destino que no esté bloqueada.
// CRÍTICO: Valida bloques de 2x2 celdas (64x64px) para el obrero.
static bool pathfindSimple(int startF, int startC, int goalF, int goalC, int **collision, int **outRuta, int *outLen) {
    if (startF == goalF && startC == goalC) return false;
    
    // Máximo de pasos para evitar bucles infinitos (diagonal máxima del mapa)
    const int MAX_PASOS = GRID_SIZE * 2;
    
    // Array temporal para guardar la ruta (máximo MAX_PASOS celdas)
    int *rutaTemp = (int*)malloc(MAX_PASOS * sizeof(int));
    if (!rutaTemp) return false;
    
    // Matriz de visitados para no volver a celdas ya recorridas
    // Usamos un array simple de tamaño GRID_SIZE * GRID_SIZE
    char *visitado = (char*)calloc(GRID_SIZE * GRID_SIZE, sizeof(char));
    if (!visitado) { free(rutaTemp); return false; }
    
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
            if (nf < 0 || nf >= GRID_SIZE - 1 || nc < 0 || nc >= GRID_SIZE - 1) continue;
            
            // ================================================================
            // VALIDACIÓN CRÍTICA: BLOQUE 2x2 COMPLETO (64x64px)
            // El obrero ocupa 2x2 celdas, por lo que debemos validar
            // las 4 celdas que ocuparía en la nueva posición.
            // Si CUALQUIERA de las 4 celdas está bloqueada (valor 1 o 2),
            // esta dirección NO es válida.
            // NOTA: Los personajes (valor 3) NO bloquean el pathfinding,
            // solo la ejecución del movimiento en actualizarPersonajes.
            // ================================================================
            bool bloqueado = false;
            for (int df = 0; df < 2; df++) {
                int *fila_ptr = *(collision + nf + df);
                for (int dc = 0; dc < 2; dc++) {
                    int valor = *(fila_ptr + nc + dc);
                    // Bloqueado SOLO si hay agua/árbol (1) o edificio (2)
                    // Los personajes (3) NO bloquean el pathfinding
                    if (valor == 1 || valor == 2) {
                        bloqueado = true;
                        break;
                    }
                }
                if (bloqueado) break;
            }
            
            if (bloqueado) continue;
            
            // Verificar si ya fue visitada
            if (visitado[nf * GRID_SIZE + nc]) continue;
            
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
    int *rutaFinal = (int*)malloc(pasos * sizeof(int));
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
    j->Comida = 200; j->Oro = 100; j->Madera = 150; j->Piedra = 100;
    for (int i = 0; i < 6; i++) {
        j->obreros[i].x = 900.0f + (i * 64.0f);
        j->obreros[i].y = 900.0f;
        j->obreros[i].moviendose = false;
        j->obreros[i].seleccionado = false;
        j->obreros[i].celdaFila = -1;
        j->obreros[i].celdaCol = -1;
        j->obreros[i].rutaCeldas = NULL;
        j->obreros[i].tipo = TIPO_OBRERO;  // Asignar tipo
        j->obreros[i].animActual = animPorDireccion(DIR_FRONT);
    }



for (int i = 0; i < 2; i++) {
  j->guerreros[i].x = 900.0f + (i * 64.0f);
  j->guerreros[i].y = 850.0f;
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
}
printf("[DEBUG] %d guerreros inicializados\\n", 2);

  // ================================================================
  // INICIALIZAR CABALLEROS (NUEVO)
  // ================================================================
  for (int i = 0; i < 4; i++) {
    j->caballeros[i].x = 900.0f + (i * 64.0f);
    j->caballeros[i].y = 800.0f;
    j->caballeros[i].destinoX = j->caballeros[i].x;
    j->caballeros[i].destinoY = j->caballeros[i].y;
    j->caballeros[i].moviendose = false;
    j->caballeros[i].seleccionado = false;
    j->caballeros[i].dir = DIR_FRONT;
    j->caballeros[i].frame = 0;
    j->caballeros[i].celdaFila = -1;
    j->caballeros[i].celdaCol = -1;
    j->caballeros[i].rutaCeldas = NULL;
    j->caballeros[i].tipo = TIPO_CABALLERO;  // Asignar tipo
  }
  
  printf("[DEBUG] %d caballeros inicializados\n", 4);

  // ================================================================
  // INICIALIZAR BARCO EN LA ORILLA (192x192px)
  // ================================================================
  j->barco.activo = false;  // Se activará después de detectar la orilla
  j->barco.x = 0.0f;
  j->barco.y = 0.0f;
  j->barco.dir = DIR_FRONT;
  
  printf("[DEBUG] Barco inicializado (pendiente de colocacion en orilla)\n");
}


void actualizarPersonajes(struct Jugador *j) {
    const float vel = 4.0f;
    int **col = mapaObtenerCollisionMap();
    if (!col) return;
    
    //Obreros
    for (int i = 0; i < 6; i++) {
        Unidad *o = &j->obreros[i];
        
        // 1. Sincronización inicial de la huella en la matriz (2x2 celdas)
        if (o->celdaFila == -1) {
            ocupacionActualizarUnidad(col, o, obreroFilaActual(o), obreroColActual(o));
        }

        if (!o->moviendose) continue;

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
        // Revisamos si el espacio al que se dirige está libre de obstáculos
        bool bloqueadoPermanente = false;  // Agua/Árbol/Edificio (cancelar ruta)
        bool bloqueadoTemporal = false;    // Otro personaje (esperar)
        
        for (int f = 0; f < 2; f++) {
            for (int c = 0; c < 2; c++) {
                int checkF = nextF + f;
                int checkC = nextC + c;

                // Verificar límites del mapa
                if (checkF >= GRID_SIZE || checkC >= GRID_SIZE) { 
                    bloqueadoPermanente = true; 
                    break; 
                }

                // Aritmética de punteros para obtener el valor de la celda
                int valor = *(*(col + checkF) + checkC);

                // Bloqueado PERMANENTEMENTE si es Agua/Árbol (1) o Edificio (2)
                if (valor == 1 || valor == 2) { 
                    bloqueadoPermanente = true; 
                    break; 
                }

                // Bloqueado TEMPORALMENTE si es OTRO Obrero (3)
                if (valor == 3) {
                    // Solo es bloqueante si la celda NO es parte de nuestra posición actual
                    bool esMiPropiaHuella = (checkF >= o->celdaFila && checkF <= o->celdaFila + 1 &&
                                            checkC >= o->celdaCol && checkC <= o->celdaCol + 1);
                    if (!esMiPropiaHuella) { 
                        bloqueadoTemporal = true; 
                        break; 
                    }
                }
            }
            if (bloqueadoPermanente || bloqueadoTemporal) break;
        }

        // Si hay obstáculo PERMANENTE (agua/edificio), cancelar movimiento
        if (bloqueadoPermanente) {
            o->moviendose = false;
            obreroLiberarRuta(o);
            continue;
        }

        // Si hay obstáculo TEMPORAL (otro personaje), ESPERAR sin cancelar ruta
        // El personaje mantiene su ruta y volverá a intentar en el siguiente frame
        if (bloqueadoTemporal) {
            continue; // Esperar, NO cancelar la ruta
        }

        // 4. Cálculo de movimiento hacia el centro de la celda destino
        float tx = celdaCentroPixel(nextC), ty = celdaCentroPixel(nextF);
        float cx = o->x + 32.0f, cy = o->y + 32.0f; // Centro actual del obrero
        float vx = tx - cx, vy = ty - cy;
        float dist = sqrtf(vx*vx + vy*vy);

        // 5. Animación y Dirección según el vector de movimiento
        if (dist > 0.1f) {
            if (fabsf(vx) > fabsf(vy)) o->dir = (vx > 0) ? DIR_RIGHT : DIR_LEFT;
            else o->dir = (vy > 0) ? DIR_FRONT : DIR_BACK;
            
            o->animActual = animPorDireccion(o->dir);
            o->animTick++;
            if (o->animTick >= o->animActual->ticksPerFrame) {
                o->animTick = 0;
                o->frame = (o->frame + 1) % o->animActual->frameCount;
            }
        }

        // 6. Aplicar desplazamiento o finalizar llegada a la celda
        if (dist <= vel) {
            // Llegó al centro de la celda: se posiciona y actualiza su ocupación en la matriz maestra
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
            if (newX < 0) newX = 0;
            if (newY < 0) newY = 0;
            if (newX > (float)(MAPA_SIZE - 64)) newX = (float)(MAPA_SIZE - 64);
            if (newY > (float)(MAPA_SIZE - 64)) newY = (float)(MAPA_SIZE - 64);
            
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

        bool bloqueadoPermanente = false;
        bool bloqueadoTemporal = false;
        
        for (int f = 0; f < 2; f++) {
            for (int c = 0; c < 2; c++) {
                int checkF = nextF + f;
                int checkC = nextC + c;

                if (checkF >= GRID_SIZE || checkC >= GRID_SIZE) { 
                    bloqueadoPermanente = true; 
                    break; 
                }

                int valor = *(*(col + checkF) + checkC);

                if (valor == 1 || valor == 2) { 
                    bloqueadoPermanente = true; 
                    break; 
                }

                if (valor == 3) {
                    bool esMiPropiaHuella = (checkF >= u->celdaFila && checkF <= u->celdaFila + 1 &&
                                            checkC >= u->celdaCol && checkC <= u->celdaCol + 1);
                    if (!esMiPropiaHuella) { 
                        bloqueadoTemporal = true; 
                        break; 
                    }
                }
            }
            if (bloqueadoPermanente || bloqueadoTemporal) break;
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
        float cx = u->x + 32.0f, cy = u->y + 32.0f;
        float vx = tx - cx, vy = ty - cy;
        float dist = sqrtf(vx*vx + vy*vy);

        if (dist > 0.1f) {
            if (fabsf(vx) > fabsf(vy)) u->dir = (vx > 0) ? DIR_RIGHT : DIR_LEFT;
            else u->dir = (vy > 0) ? DIR_FRONT : DIR_BACK;
            
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
            
            if (newX < 0) newX = 0;
            if (newY < 0) newY = 0;
            if (newX > (float)(MAPA_SIZE - 64)) newX = (float)(MAPA_SIZE - 64);
            if (newY > (float)(MAPA_SIZE - 64)) newY = (float)(MAPA_SIZE - 64);
            
            u->x = newX;
            u->y = newY;
        }
    }

    for (int i = 0; i < 2; i++) {
    Unidad *u = &j->guerreros[i];
    
    // Sincronización inicial
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

        bool bloqueadoPermanente = false;
        bool bloqueadoTemporal = false;
        
        for (int f = 0; f < 2; f++) {
            for (int c = 0; c < 2; c++) {
                int checkF = nextF + f;
                int checkC = nextC + c;

                if (checkF >= GRID_SIZE || checkC >= GRID_SIZE) { 
                    bloqueadoPermanente = true; 
                    break; 
                }

                int valor = *(*(col + checkF) + checkC);

                if (valor == 1 || valor == 2) { 
                    bloqueadoPermanente = true; 
                    break; 
                }

                if (valor == 3) {
                    bool esMiPropiaHuella = (checkF >= u->celdaFila && checkF <= u->celdaFila + 1 &&
                                            checkC >= u->celdaCol && checkC <= u->celdaCol + 1);
                    if (!esMiPropiaHuella) { 
                        bloqueadoTemporal = true; 
                        break; 
                    }
                }
            }
            if (bloqueadoPermanente || bloqueadoTemporal) break;
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
        float cx = u->x + 32.0f, cy = u->y + 32.0f;
        float vx = tx - cx, vy = ty - cy;
        float dist = sqrtf(vx*vx + vy*vy);

        if (dist > 0.1f) {
            if (fabsf(vx) > fabsf(vy)) u->dir = (vx > 0) ? DIR_RIGHT : DIR_LEFT;
            else u->dir = (vy > 0) ? DIR_FRONT : DIR_BACK;
            
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
            
            if (newX < 0) newX = 0;
            if (newY < 0) newY = 0;
            if (newX > (float)(MAPA_SIZE - 64)) newX = (float)(MAPA_SIZE - 64);
            if (newY > (float)(MAPA_SIZE - 64)) newY = (float)(MAPA_SIZE - 64);
            
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
// - Múltiples unidades seleccionadas reciben destinos diferentes para no solaparse.
// ============================================================================
void rtsComandarMovimiento(struct Jugador *j, float mundoX, float mundoY) {
    int **col = mapaObtenerCollisionMap();
    if (!col) return;
    
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
        printf("[DEBUG] RECHAZADO: Fuera de limites (gF=%d, gC=%d, GRID_SIZE=%d)\n", gF, gC, GRID_SIZE);
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
    // Validamos que TODAS las 4 celdas del bloque estén libres de
    // agua/árboles (1) y edificios (2).
    // Los personajes (3) NO bloquean las órdenes de movimiento.
    // ================================================================
    bool destinoBloqueado = false;
    int motivoBloqueo = -1; // Para debug: qué celda causó el bloqueo
    for (int df = 0; df < 2; df++) {
        int *fila_ptr = *(col + gF + df);
        for (int dc = 0; dc < 2; dc++) {
            int valor = *(fila_ptr + gC + dc);
            // Si hay Agua/Árbol (1) o Edificio (2), rechazar orden
            // Los personajes (3) NO bloquean las órdenes
            if (valor == 1 || valor == 2) {
                destinoBloqueado = true;
                motivoBloqueo = valor;
                break;
            }
        }
        if (destinoBloqueado) break;
    }
    
    // Si el destino clickeado es impasable, TERMINAR inmediatamente
    // NO buscar alternativas - el usuario debe clickear en tierra válida
    if (destinoBloqueado) {
        // SALIDA POR TERMINAL
        printf("[DEBUG] ********************************************\n");
        printf("[DEBUG] ORDEN RECHAZADA: El area de destino (F:%d, C:%d) contiene ", gF, gC);
        if (motivoBloqueo == 1) printf("AGUA/ARBOL.\n");
        else if (motivoBloqueo == 2) printf("EDIFICIO.\n");
        else printf("OBSTACULO (valor=%d).\n", motivoBloqueo);
        printf("[DEBUG] ********************************************\n");
        fflush(stdout);
        
        // SALIDA VISUAL (MessageBox) - SOLO PARA DEBUG
        char mensaje[256];
        sprintf(mensaje, "Click rechazado!\n\nDestino: Fila=%d, Col=%d\nMotivo: %s (valor=%d)", 
                gF, gC, 
                (motivoBloqueo == 1 ? "AGUA/ARBOL" : "EDIFICIO"),
                motivoBloqueo);
        MessageBoxA(NULL, mensaje, "DEBUG: Orden Rechazada", MB_OK | MB_ICONWARNING);
        
        return; // Orden rechazada: destino sobre agua/obstáculo
    }
    
    // Si llegamos aquí, el destino ES VÁLIDO
    printf("[DEBUG] Destino VALIDO: Procediendo a calcular pathfinding...\n");
    fflush(stdout);
    
    // Contador de unidades que ya recibieron destino (para separación)
    int unidadesAsignadas = 0;
    
    // Puntero base al array de obreros (aritmética de punteros)
    Unidad *base = j->obreros;
    
    // Recorrer todas las unidades usando aritmética de punteros
    for (Unidad *o = base; o < base + 6; o++) {
        if (!o->seleccionado) continue;

        // Usar el destino global para todas las unidades
        int destinoF = gF;
        int destinoC = gC;
        
        // Obtener posición actual de la unidad
        int sF = obreroFilaActual(o);
        int sC = obreroColActual(o);
        
        // Si ya estamos en el destino, no moverse
        if (sF == destinoF && sC == destinoC) continue;
        
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
            // NO marcar el destino aquí - se actualizará cuando llegue
            unidadesAsignadas++;
        }
    }
    
    // ================================================================
    // COMANDAR CABALLEROS (misma lógica)
    // ================================================================
    Unidad *baseCaballeros = j->caballeros;
    for (Unidad *u = baseCaballeros; u < baseCaballeros + 4; u++) {
        if (!u->seleccionado) continue;

        // Calcular offset para separación
        int offF = (unidadesAsignadas / 3) - 1;
        int offC = (unidadesAsignadas % 3) - 1;
        int targetF = gF + offF;
        int targetC = gC + offC;
        targetF = (targetF < 0) ? 0 : ((targetF >= GRID_SIZE) ? GRID_SIZE - 1 : targetF);
        targetC = (targetC < 0) ? 0 : ((targetC >= GRID_SIZE) ? GRID_SIZE - 1 : targetC);

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

    // ================================================================
    // COMANDAR GUERREROS (misma lógica)
    // ================================================================
    Unidad *baseGuerreros = j->guerreros;
    for (Unidad *u = baseGuerreros; u < baseGuerreros + 2; u++) {
        if (!u->seleccionado) continue;

        // Calcular offset para separación
        int offF = (unidadesAsignadas / 3) - 1;
        int offC = (unidadesAsignadas % 3) - 1;
        int targetF = gF + offF;
        int targetC = gC + offC;
        targetF = (targetF < 0) ? 0 : ((targetF >= GRID_SIZE) ? GRID_SIZE - 1 : targetF);
        targetC = (targetC < 0) ? 0 : ((targetC >= GRID_SIZE) ? GRID_SIZE - 1 : targetC);

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

void rtsLiberarMovimientoJugador(struct Jugador *j) {
    for (int i = 0; i < 6; i++) obreroLiberarRuta(&j->obreros[i]);
    for (int i = 0; i < 4; i++) obreroLiberarRuta(&j->caballeros[i]);
    for (int i = 0; i < 2; i++) obreroLiberarRuta(&j->guerreros[i]); 

}