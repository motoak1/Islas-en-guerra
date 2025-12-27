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
    return clampInt(c, 0, GRID_SIZE - 1);
}

static float celdaCentroPixel(int celda) {
    return (float)(celda * TILE_SIZE) + (float)TILE_SIZE * 0.5f;
}

static float celdaAPixel(int celda) {
    return (float)(celda * TILE_SIZE);
}

static int obreroFilaActual(const UnidadObrero *o) {
    return pixelACelda(o->y + 32.0f);
}

static int obreroColActual(const UnidadObrero *o) {
    return pixelACelda(o->x + 32.0f);
}

static void ocupacionActualizarUnidad(int **collision, UnidadObrero *o, int nuevaF, int nuevaC) {
    if (!collision) return;
    // Liberar celda vieja si era ocupada por unidad (2)
    if (o->celdaFila >= 0 && o->celdaCol >= 0) {
        if (collision[o->celdaFila][o->celdaCol] == 2) {
            collision[o->celdaFila][o->celdaCol] = 0;
        }
    }
    // Marcar nueva celda
    if (collision[nuevaF][nuevaC] == 0) {
        collision[nuevaF][nuevaC] = 2;
    }
    o->celdaFila = nuevaF;
    o->celdaCol = nuevaC;
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

static void obreroLiberarRuta(UnidadObrero *o) {
    if (o->rutaCeldas) free(o->rutaCeldas);
    o->rutaCeldas = NULL;
    o->rutaLen = 0;
    o->rutaIdx = 0;
}

// PATHFINDING SIMPLE: Movimiento greedy sin usar colas.
// Genera una ruta paso a paso moviéndose hacia el objetivo,
// eligiendo la dirección más cercana al destino que no esté bloqueada.
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
            
            // Verificar límites del mapa
            if (nf < 0 || nf >= GRID_SIZE || nc < 0 || nc >= GRID_SIZE) continue;
            
            // Verificar si es transitable (solo árboles bloquean, valor 1)
            if (collision[nf][nc] == 1) continue;
            
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
        j->obreros[i].x = 400.0f + (i * 70.0f);
        j->obreros[i].y = 400.0f;
        j->obreros[i].moviendose = false;
        j->obreros[i].seleccionado = false;
        j->obreros[i].celdaFila = -1;
        j->obreros[i].celdaCol = -1;
        j->obreros[i].rutaCeldas = NULL;
        j->obreros[i].animActual = animPorDireccion(DIR_FRONT);
    }
}

void actualizarObreros(struct Jugador *j) {
    const float vel = 4.0f;
    int **col = mapaObtenerCollisionMap();
    if (!col) return;

    for (int i = 0; i < 6; i++) {
        UnidadObrero *o = &j->obreros[i];
        
        // Sincronizar ocupación en el primer frame
        if (o->celdaFila == -1) {
            ocupacionActualizarUnidad(col, o, obreroFilaActual(o), obreroColActual(o));
        }

        if (!o->moviendose) continue;

        // ==================================================================
        // VALIDACIÓN CRÍTICA: Verificar que la siguiente celda sea pasable
        // ==================================================================
        int nextF, nextC;
        if (o->rutaCeldas && o->rutaIdx < o->rutaLen) {
            int targetCelda = o->rutaCeldas[o->rutaIdx];
            nextF = targetCelda / GRID_SIZE;
            nextC = targetCelda % GRID_SIZE;
            
            // Acceso a la matriz con aritmética de punteros
            int *fila = *(col + nextF);
            int valorCelda = *(fila + nextC);
            
            // Si la celda tiene valor 1 (AGUA o ÁRBOL), NO SE PUEDE PASAR
            if (valorCelda == 1) {
                // Detenerse inmediatamente
                o->moviendose = false;
                obreroLiberarRuta(o);
                continue;
            }
        } else {
            o->moviendose = false; continue;
        }

        float tx = celdaCentroPixel(nextC), ty = celdaCentroPixel(nextF);
        float cx = o->x + 32.0f, cy = o->y + 32.0f;
        float vx = tx - cx, vy = ty - cy;
        float dist = sqrtf(vx*vx + vy*vy);

        // Animación y Dirección
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

        if (dist <= vel) {
            // Llegó al centro de la celda
            o->x = tx - 32.0f; o->y = ty - 32.0f;
            o->rutaIdx++;
            int cf = obreroFilaActual(o), cc = obreroColActual(o);
            ocupacionActualizarUnidad(col, o, cf, cc);
            if (o->rutaIdx >= o->rutaLen) {
                o->moviendose = false; obreroLiberarRuta(o);
            }
        } else {
            // Calcular nueva posición
            float newX = o->x + (vx / dist) * vel;
            float newY = o->y + (vy / dist) * vel;
            
            // Validar que la nueva posición no salga de la isla
            if (newX < 0) newX = 0;
            if (newY < 0) newY = 0;
            if (newX > (float)(MAPA_SIZE - TILE_SIZE)) newX = (float)(MAPA_SIZE - TILE_SIZE);
            if (newY > (float)(MAPA_SIZE - TILE_SIZE)) newY = (float)(MAPA_SIZE - TILE_SIZE);
            
            // Verificar que la celda donde vamos a caer no sea agua/árbol
            int testF = pixelACelda(newY + 32.0f);
            int testC = pixelACelda(newX + 32.0f);
            int *filaTest = *(col + testF);
            int valorTest = *(filaTest + testC);
            
            if (valorTest == 1) {
                // La nueva posición es agua/árbol, detener
                o->moviendose = false;
                obreroLiberarRuta(o);
                continue;
            }
            
            // Aplicar movimiento
            o->x = newX;
            o->y = newY;
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
    
    // Celda objetivo inicial basada en el clic del mouse
    int gF = pixelACelda(mundoY);
    int gC = pixelACelda(mundoX);

    // Si el destino base es impasable (árbol/agua), buscar celda libre cercana
    if (*(*(col + gF) + gC) == 1) {
        if (!buscarCeldaLibreCerca(col, gF, gC, &gF, &gC)) return;
    }
    
    // Contador de unidades que ya recibieron destino (para separación)
    int unidadesAsignadas = 0;
    
    // Puntero base al array de obreros (aritmética de punteros)
    UnidadObrero *base = j->obreros;
    
    // Recorrer todas las unidades usando aritmética de punteros
    for (UnidadObrero *o = base; o < base + 6; o++) {
        if (!o->seleccionado) continue;

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
            if (!buscarCeldaLibreCerca(col, destinoF, destinoC, &destinoF, &destinoC)) {
                continue; // No hay celda libre, esta unidad no se mueve
            }
        }
        
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
            
            // Marcar la celda destino como "reservada" temporalmente (valor 2)
            // para que la siguiente unidad elija otro destino
            *(*(col + destinoF) + destinoC) = 2;
            unidadesAsignadas++;
        }
    }
}

void rtsLiberarMovimientoJugador(struct Jugador *j) {
    for (int i = 0; i < 6; i++) obreroLiberarRuta(&j->obreros[i]);
}