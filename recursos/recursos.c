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
    return (float)(celda * TILE_SIZE) + (float)(TILE_SIZE / 2);
}

static int obreroFilaActual(const Unidad *o) {
    return pixelACelda(o->y + (TILE_SIZE / 2));  // Centro de la celda, no hardcoded 32
}

static int obreroColActual(const Unidad *o) {
    return pixelACelda(o->x + (TILE_SIZE / 2));  // Centro de la celda, no hardcoded 32
}
// Marcar/desmarcar una ÚNICA CELDA en el collision map (1x1, no 2x2)
static void marcarHuellaObrero(int **collision, int fila, int col, int valor) {
    if (!collision || fila < 0 || col < 0 || fila >= GRID_SIZE || col >= GRID_SIZE) return;
    
    // Marcar solo UNA celda usando punteros
    int *fila_ptr = *(collision + fila);
    *(fila_ptr + col) = valor;
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


static void obreroLiberarRuta(Unidad *o) {
    if (o->rutaCeldas) free(o->rutaCeldas);
    o->rutaCeldas = NULL;
    o->rutaLen = 0;
    o->rutaIdx = 0;
}

// PATHFINDING MEJORADO: Explora múltiples rutas para encontrar caminos alternativos
// Usa backtracking cuando se atasca, permitiendo encontrar rutas aunque haya obstáculos
// CRÍTICO: Valida bloques de 2x2 celdas (64x64px) para el obrero.
static bool pathfindSimple(int startF, int startC, int goalF, int goalC, int **collision, int **outRuta, int *outLen) {
    if (startF == goalF && startC == goalC) return false;
    
    const int MAX_PASOS = GRID_SIZE * 4; // Más pasos para rutas complejas
    
    int *rutaTemp = (int*)malloc(MAX_PASOS * sizeof(int));
    if (!rutaTemp) return false;
    
    // Matriz de visitados para evitar ciclos
    char *visitado = (char*)calloc(GRID_SIZE * GRID_SIZE, sizeof(char));
    if (!visitado) { free(rutaTemp); return false; }
    
    // Matriz de distancias (para A*)
    int *distancias = (int*)malloc(GRID_SIZE * GRID_SIZE * sizeof(int));
    if (!distancias) { free(rutaTemp); free(visitado); return false; }
    
    // Inicializar distancias
    for (int i = 0; i < GRID_SIZE * GRID_SIZE; i++) distancias[i] = 999999;
    
    int pasos = 0;
    int actualF = startF;
    int actualC = startC;
    
    visitado[actualF * GRID_SIZE + actualC] = 1;
    distancias[actualF * GRID_SIZE + actualC] = 0;
    
    // Direcciones: arriba, abajo, izquierda, derecha
    int dF[4] = {-1, 1, 0, 0};
    int dC[4] = {0, 0, -1, 1};
    
    int intentosFallidos = 0;
    const int MAX_REINTENTOS = 5;
    
    while (pasos < MAX_PASOS) {
        if (actualF == goalF && actualC == goalC) {
            break; // ¡Llegamos!
        }
        
        // Evaluar todas las 4 direcciones
        typedef struct {
            int dir;
            float heuristica;
            int distancia;
        } Opcion;
        
        Opcion opciones[4];
        int numOpciones = 0;
        
        for (int i = 0; i < 4; i++) {
            int nf = actualF + dF[i];
            int nc = actualC + dC[i];
            
            // Validar límites (solo 1 celda ahora, no 2x2)
            if (nf < 0 || nf >= GRID_SIZE || nc < 0 || nc >= GRID_SIZE) continue;
            
            // Validar SOLO la celda destino (1x1, no bloque 2x2)
            int *fila_ptr = *(collision + nf);
            int valor = *(fila_ptr + nc);
            if (valor == 1 || valor == 2) continue; // Bloqueado
            
            // NUEVO: Permitir revisitar si ha pasado tiempo (para backtracking)
            bool yaVisitado = visitado[nf * GRID_SIZE + nc];
            int distActual = distancias[actualF * GRID_SIZE + actualC] + 1;
            int distPrevia = distancias[nf * GRID_SIZE + nc];
            
            // Solo considerar si no ha sido visitada O si encontramos ruta más corta
            if (yaVisitado && distActual >= distPrevia) continue;
            
            // Calcular heurística (distancia al objetivo)
            int distF = (nf > goalF) ? (nf - goalF) : (goalF - nf);
            int distC = (nc > goalC) ? (nc - goalC) : (goalC - nc);
            float heuristica = (float)(distF + distC) + distActual * 0.1f; // Favorece rutas más cortas
            
            opciones[numOpciones].dir = i;
            opciones[numOpciones].heuristica = heuristica;
            opciones[numOpciones].distancia = distActual;
            numOpciones++;
        }
        
        // Si no hay opciones, hemos llegado a un callejón sin salida
        if (numOpciones == 0) {
            intentosFallidos++;
            
            if (intentosFallidos > MAX_REINTENTOS || pasos == 0) {
                free(rutaTemp);
                free(visitado);
                free(distancias);
                return false; // No hay ruta posible
            }
            
            // BACKTRACKING: Retroceder un paso
            if (pasos > 0) {
                pasos--;
                int celdaPrevia = rutaTemp[pasos > 0 ? pasos - 1 : 0];
                actualF = celdaPrevia / GRID_SIZE;
                actualC = celdaPrevia % GRID_SIZE;
                continue;
            } else {
                // No podemos retroceder más
                free(rutaTemp);
                free(visitado);
                free(distancias);
                return false;
            }
        }
        
        // Ordenar opciones por heurística (mejor primero)
        for (int i = 0; i < numOpciones - 1; i++) {
            for (int j = i + 1; j < numOpciones; j++) {
                if (opciones[j].heuristica < opciones[i].heuristica) {
                    Opcion temp = opciones[i];
                    opciones[i] = opciones[j];
                    opciones[j] = temp;
                }
            }
        }
        
        // Elegir la mejor opción
        int mejorDir = opciones[0].dir;
        actualF += dF[mejorDir];
        actualC += dC[mejorDir];
        
        // Actualizar estructuras
        visitado[actualF * GRID_SIZE + actualC] = 1;
        distancias[actualF * GRID_SIZE + actualC] = opciones[0].distancia;
        
        rutaTemp[pasos] = actualF * GRID_SIZE + actualC;
        pasos++;
        intentosFallidos = 0; // Reset al progresar
    }
    
    free(visitado);
    free(distancias);
    
    // Verificar si llegamos
    if (actualF != goalF || actualC != goalC) {
        free(rutaTemp);
        return false;
    }
    
    // Crear ruta final
    int *rutaFinal = (int*)malloc(pasos * sizeof(int));
    if (!rutaFinal) {
        free(rutaTemp);
        return false;
    }
    
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
  
  // ================================================================
  // REGISTRAR TODOS LOS OBJETOS EN mapaObjetos
  // ================================================================
  printf("[DEBUG] Registrando objetos en mapaObjetos...\n");
  
  // Registrar obreros
  for (int i = 0; i < 6; i++) {
    mapaRegistrarObjeto(j->obreros[i].x, j->obreros[i].y, SIMBOLO_OBRERO);
  }
  printf("[DEBUG] %d obreros registrados en matriz\n", 6);
  
  // Registrar caballeros
  for (int i = 0; i < 4; i++) {
    mapaRegistrarObjeto(j->caballeros[i].x, j->caballeros[i].y, SIMBOLO_CABALLERO);
  }
  printf("[DEBUG] %d caballeros registrados en matriz\n", 4);
  
  // Registrar guerreros
  for (int i = 0; i < 2; i++) {
    mapaRegistrarObjeto(j->guerreros[i].x, j->guerreros[i].y, SIMBOLO_GUERRERO);
  }
  printf("[DEBUG] %d guerreros registrados en matriz\n", 2);
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
            
            // DEBUG: Ver qué celda estamos decodificando
            if (o->rutaIdx == 0) {  // Solo primera celda de la ruta
                printf("[DEBUG RUTA] Obrero %d: targetCelda=%d -> nextF=%d, nextC=%d\n", 
                       i, targetCelda, nextF, nextC);
            }
        } else {
            o->moviendose = false; 
            continue;
        }

        // 3. VALIDACION DE CELDA DESTINO (1x1 - UNA SOLA CELDA)
        // Revisamos si la celda destino esta libre
        bool bloqueadoPermanente = false;  // Agua/Arbol/Edificio (cancelar ruta)
        bool bloqueadoTemporal = false;    // Otro personaje en celda FINAL (esperar)
        
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

            // MODIFICADO: Solo bloquear por otro personaje si es la ULTIMA celda de la ruta
            // Esto permite atravesar personajes durante el transito
            bool esUltimaCelda = (o->rutaIdx + 1 >= o->rutaLen);
            if (esUltimaCelda && valor == 3 && (nextF != o->celdaFila || nextC != o->celdaCol)) {
                bloqueadoTemporal = true;
            }
        }

        // Si hay obstaculo PERMANENTE (agua/edificio), cancelar movimiento
        if (bloqueadoPermanente) {
            o->moviendose = false;
            obreroLiberarRuta(o);
            continue;
        }

        // Si hay obstaculo TEMPORAL en celda FINAL, ESPERAR sin cancelar ruta
        // El personaje mantiene su ruta y volvera a intentar en el siguiente frame
        if (bloqueadoTemporal) {
            continue; // Esperar, NO cancelar la ruta
        }

        // 4. Cálculo de movimiento hacia el centro de la celda destino
        float tx = celdaCentroPixel(nextC), ty = celdaCentroPixel(nextF);
        float cx = o->x + (TILE_SIZE / 2), cy = o->y + (TILE_SIZE / 2); // Centro actual del obrero
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

        //  6. Aplicar desplazamiento o finalizar llegada a la celda
        if (dist <= vel) {
            // Guarda posición anterior
            float viejoX = o->x;
            float viejoY = o->y;
            
            // Llegó al centro de la celda: se posiciona y actualiza su ocupación en la matriz maestra
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
            if (newX < 0) newX = 0;
            if (newY < 0) newY = 0;
            if (newX > (float)(MAPA_SIZE - 64)) newX = (float)(MAPA_SIZE - 64);
            if (newY > (float)(MAPA_SIZE - 64)) newY = (float)(MAPA_SIZE - 64);
            
            o->x = newX;
            o->y = newY;
            
            // NUEVO: Actualizar mapaObjetos durante movimiento suave
            mapaMoverObjeto(viejoX, viejoY, o->x, o->y, SIMBOLO_OBRERO);
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
        
        // Verificar límites del mapa
        if (nextF >= GRID_SIZE || nextC >= GRID_SIZE) { 
            bloqueadoPermanente = true;
        } else {
            int valor = *(*(col + nextF) + nextC);

            if (valor == 1 || valor == 2) { 
                bloqueadoPermanente = true;
            }

            // MODIFICADO: Solo bloquear si es la ULTIMA celda de la ruta
            bool esUltimaCelda = (u->rutaIdx + 1 >= u->rutaLen);
            if (esUltimaCelda && valor == 3 && (nextF != u->celdaFila || nextC != u->celdaCol)) {
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
            
            if (newX < 0) newX = 0;
            if (newY < 0) newY = 0;
            if (newX > (float)(MAPA_SIZE - 64)) newX = (float)(MAPA_SIZE - 64);
            if (newY > (float)(MAPA_SIZE - 64)) newY = (float)(MAPA_SIZE - 64);
            
            u->x = newX;
            u->y = newY;
            mapaMoverObjeto(viejoX, viejoY, u->x, u->y, SIMBOLO_CABALLERO);
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
        
        // Verificar límites del mapa
        if (nextF >= GRID_SIZE || nextC >= GRID_SIZE) { 
            bloqueadoPermanente = true;
        } else {
            int valor = *(*(col + nextF) + nextC);

            if (valor == 1 || valor == 2) { 
                bloqueadoPermanente = true;
            }

            // MODIFICADO: Solo bloquear si es la ULTIMA celda de la ruta
            bool esUltimaCelda = (u->rutaIdx + 1 >= u->rutaLen);
            if (esUltimaCelda && valor == 3 && (nextF != u->celdaFila || nextC != u->celdaCol)) {
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
            
            if (newX < 0) newX = 0;
            if (newY < 0) newY = 0;
            if (newX > (float)(MAPA_SIZE - 64)) newX = (float)(MAPA_SIZE - 64);
            if (newY > (float)(MAPA_SIZE - 64)) newY = (float)(MAPA_SIZE - 64);
            
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
    // IMPORTANTE: Usar la MISMA conversión que el hover (división simple)
    int gF = (int)(mundoY / TILE_SIZE);
    int gC = (int)(mundoX / TILE_SIZE);
    
    // DIAGNÓSTICO PASO 2: Mostrar conversión
    printf("[DEBUG] Conversion a matriz: gF=%d, gC=%d\n", gF, gC);
    fflush(stdout);
    
    if (gF < 0 || gF >= GRID_SIZE || gC < 0 || gC >= GRID_SIZE) {
        printf("[DEBUG] RECHAZADO: Fuera de limites (gF=%d, gC=%d, GRID_SIZE=%d)\n", gF, gC, GRID_SIZE);
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
    
    // Si hay Agua/Árbol (1) o Edificio (2), rechazar orden
    if (valor == 1 || valor == 2) {
        destinoBloqueado = true;
        motivoBloqueo = valor;
    }
    
    fflush(stdout);
    
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
    
    printf("[DEBUG] Buscando obreros seleccionados...\n");
    fflush(stdout);
    
    // Recorrer todas las unidades usando aritmética de punteros
    for (Unidad *o = base; o < base + 6; o++) {
        int idx = (int)(o - base);
        printf("[DEBUG] Obrero %d: seleccionado=%d\n", idx, o->seleccionado);
        fflush(stdout);
        
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
        
        printf("[DEBUG PATH] Obrero %d en[%d][%d] -> destino[%d][%d]\n", idx, sF, sC, destinoF, destinoC);
        fflush(stdout);
        
        if (pathfindSimple(sF, sC, destinoF, destinoC, col, &ruta, &len)) {
            // DEBUG: Primera celda de la ruta
            if (len > 0) {
                int primeraCelda = ruta[0];
                int primeraF = primeraCelda / GRID_SIZE;
                int primeraC = primeraCelda % GRID_SIZE;
                printf("[DEBUG PATH] Ruta OK: %d pasos. Primera celda=[%d][%d] (codificada=%d)\n",
                       len, primeraF, primeraC, primeraCelda);
                fflush(stdout);
            }
            
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
    // COMANDAR CABALLEROS (misma lógica que obreros)
    // ================================================================
    Unidad *baseCaballeros = j->caballeros;
    for (Unidad *u = baseCaballeros; u < baseCaballeros + 4; u++) {
        if (!u->seleccionado) continue;

        // Usar el destino global directamente (igual que obreros)
        int destinoF = gF;
        int destinoC = gC;
        
        // Obtener posición actual de la unidad
        int sF = obreroFilaActual(u);
        int sC = obreroColActual(u);
        
        // Si ya estamos en el destino, no moverse
        if (sF == destinoF && sC == destinoC) continue;

        // Pathfinding
        int *path = NULL;
        int len = 0;
        bool success = pathfindSimple(sF, sC, destinoF, destinoC, col, &path, &len);
        
        if (success) {
            obreroLiberarRuta(u);
            u->rutaCeldas = path;
            u->rutaLen = len;
            u->rutaIdx = 0;
            u->objetivoFila = destinoF;
            u->objetivoCol = destinoC;
            u->moviendose = true;
        }

        unidadesAsignadas++;
    }

    // ================================================================
    // COMANDAR GUERREROS (misma lógica que obreros)
    // ================================================================
    Unidad *baseGuerreros = j->guerreros;
    for (Unidad *u = baseGuerreros; u < baseGuerreros + 2; u++) {
        if (!u->seleccionado) continue;

        // Usar el destino global directamente (igual que obreros)
        int destinoF = gF;
        int destinoC = gC;
        
        // Obtener posición actual de la unidad
        int sF = obreroFilaActual(u);
        int sC = obreroColActual(u);
        
        // Si ya estamos en el destino, no moverse
        if (sF == destinoF && sC == destinoC) continue;

        // Pathfinding
        int *path = NULL;
        int len = 0;
        bool success = pathfindSimple(sF, sC, destinoF, destinoC, col, &path, &len);
        
        if (success) {
            obreroLiberarRuta(u);
            u->rutaCeldas = path;
            u->rutaLen = len;
            u->rutaIdx = 0;
            u->objetivoFila = destinoF;
            u->objetivoCol = destinoC;
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