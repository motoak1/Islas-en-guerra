#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <windows.h> //Necesario para poder posicionar en cualqueir lado de la pantalla los datos de los jugadores.
#include "../mapa/menu.h"
#include "../mapa/mapa.h"
#include "recursos.h"
#include "stdbool.h"
#include <math.h>

// --- Animaciones (estado lógico por puntero) ---
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

// --- Utilidades de grid (punteros) ---
static int clampInt(int v, int lo, int hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

static int pixelACelda(float px) {
    // Conversión px->celda: celda = floor(px / TILE_SIZE)
    int c = (int)(px / (float)TILE_SIZE);
    return clampInt(c, 0, GRID_SIZE - 1);
}

static int obreroFilaActual(const UnidadObrero *o) {
    // Usamos el centro del sprite (x+32, y+32) para mapear celda de ocupación.
    return pixelACelda(o->y + (float)TILE_SIZE * 0.5f);
}

static int obreroColActual(const UnidadObrero *o) {
    return pixelACelda(o->x + (float)TILE_SIZE * 0.5f);
}

static float celdaCentroPixel(int celda) {
    return (float)(celda * TILE_SIZE) + (float)TILE_SIZE * 0.5f;
}

static void ocupacionActualizarUnidad(int **collision, UnidadObrero *o, int nuevaF, int nuevaC) {
    if (!collision) return;

    // Limpiar celda anterior solo si era ocupación de unidad (2)
    if (o->celdaFila >= 0 && o->celdaCol >= 0) {
        int *row = *(collision + o->celdaFila);
        if (*(row + o->celdaCol) == 2) {
            *(row + o->celdaCol) = 0;
        }
    }

    // Marcar nueva celda como ocupada por unidad (2)
    *(*(collision + nuevaF) + nuevaC) = 2;
    o->celdaFila = nuevaF;
    o->celdaCol = nuevaC;
}

static bool buscarCeldaLibreCerca(int **collision, int startF, int startC, int *outF, int *outC) {
    if (!collision) return false;

    // Búsqueda en anillos (radio creciente) alrededor de la celda inicial.
    for (int r = 0; r <= 6; r++) {
        for (int df = -r; df <= r; df++) {
            for (int dc = -r; dc <= r; dc++) {
                int f = startF + df;
                int c = startC + dc;
                if (f < 0 || f >= GRID_SIZE || c < 0 || c >= GRID_SIZE) continue;
                if (*(*(collision + f) + c) == 0) {
                    *outF = f;
                    *outC = c;
                    return true;
                }
            }
        }
    }
    return false;
}

static float celdaAPixel(int celda) {
    // Conversión celda->px: px = celda * TILE_SIZE
    return (float)(celda * TILE_SIZE);
}

static int **gridAlloc(int n) {
    int **m = (int**)malloc((size_t)n * sizeof(int*));
    if (!m) return NULL;
    for (int i = 0; i < n; i++) {
        *(m + i) = (int*)malloc((size_t)n * sizeof(int));
        if (!*(m + i)) {
            for (int k = 0; k < i; k++) free(*(m + k));
            free(m);
            return NULL;
        }
    }
    return m;
}

static void gridFree(int **m, int n) {
    if (!m) return;
    for (int i = 0; i < n; i++) free(*(m + i));
    free(m);
}

static void gridCopy(int **dst, int **src, int n) {
    for (int i = 0; i < n; i++) {
        int *drow = *(dst + i);
        int *srow = *(src + i);
        for (int j = 0; j < n; j++) {
            *(drow + j) = *(srow + j);
        }
    }
}

static void obreroLiberarRuta(UnidadObrero *o) {
    if (o->rutaCeldas) {
        free(o->rutaCeldas);
        o->rutaCeldas = NULL;
    }
    o->rutaLen = 0;
    o->rutaIdx = 0;
}

// BFS en grid 4-dir: retorna ruta (lista de celdas) en memoria dinámica.
// - collision[f][c] == 1 => bloqueado
// - ruta incluye celdas a visitar desde el siguiente paso (no incluye celda inicial)
static bool pathfindBFS(int startF, int startC, int goalF, int goalC, int **collision,
                        int **outRutaCeldas, int *outLen) {
    const int n = GRID_SIZE;
    const int total = n * n;

    int start = startF * n + startC;
    int goal = goalF * n + goalC;
    if (start == goal) {
        *outRutaCeldas = NULL;
        *outLen = 0;
        return true;
    }

    // No permitir objetivo bloqueado
    if (*(*(collision + goalF) + goalC) != 0) return false;

    int *cola = (int*)malloc((size_t)total * sizeof(int));
    int *prev = (int*)malloc((size_t)total * sizeof(int));
    unsigned char *visit = (unsigned char*)malloc((size_t)total);
    if (!cola || !prev || !visit) {
        free(cola); free(prev); free(visit);
        return false;
    }

    // init
    for (int i = 0; i < total; i++) {
        *(prev + i) = -1;
        *(visit + i) = 0;
    }

    int head = 0, tail = 0;
    *(cola + tail++) = start;
    *(visit + start) = 1;

    const int dF[4] = { -1, 1, 0, 0 };
    const int dC[4] = { 0, 0, -1, 1 };

    bool found = false;
    while (head < tail) {
        int cur = *(cola + head++);
        if (cur == goal) { found = true; break; }

        int cf = cur / n;
        int cc = cur % n;

        for (int k = 0; k < 4; k++) {
            int nf = cf + *(dF + k);
            int nc = cc + *(dC + k);
            if (nf < 0 || nf >= n || nc < 0 || nc >= n) continue;
            if (*(*(collision + nf) + nc) != 0) continue;
            int nxt = nf * n + nc;
            if (*(visit + nxt)) continue;
            *(visit + nxt) = 1;
            *(prev + nxt) = cur;
            *(cola + tail++) = nxt;
        }
    }

    if (!found) {
        free(cola); free(prev); free(visit);
        return false;
    }

    // reconstruir ruta en orden inverso
    int pasos = 0;
    for (int v = goal; v != start; v = *(prev + v)) pasos++;

    int *ruta = NULL;
    if (pasos > 0) {
        ruta = (int*)malloc((size_t)pasos * sizeof(int));
        if (!ruta) {
            free(cola); free(prev); free(visit);
            return false;
        }
        int idx = pasos - 1;
        for (int v = goal; v != start; v = *(prev + v)) {
            *(ruta + idx--) = v;
        }
    }

    *outRutaCeldas = ruta;
    *outLen = pasos;

    free(cola); free(prev); free(visit);
    return true;
}

void IniciacionRecursos (struct Jugador *j ,const char *Nombre){
	strcpy(j->Nombre,Nombre);
	j->Comida=200;
	j->Oro=100;
	j->Madera=150;
	j->Piedra=100;	
	j->Ejercito= NULL;
	j->NumeroTropas=0;
	j->Capacidad=0;
	j->CantidadCaballeria=0;
	j->CantidadArqueros=0;
	j->CantidadPicas=0;
	j->CantidadEspadas=0;

for (int i = 0; i < 6; i++) {
        j->obreros[i].x = 400.0f + (i * 70.0f); // Aparecen cerca en el mapa
        j->obreros[i].y = 400.0f;
        j->obreros[i].destinoX = j->obreros[i].x;
        j->obreros[i].destinoY = j->obreros[i].y;
        j->obreros[i].moviendose = false;
        j->obreros[i].seleccionado = false;
        j->obreros[i].dir = DIR_FRONT;
    j->obreros[i].frame = 0;

    j->obreros[i].objetivoFila = pixelACelda(j->obreros[i].y);
    j->obreros[i].objetivoCol = pixelACelda(j->obreros[i].x);
    j->obreros[i].rutaCeldas = NULL;
    j->obreros[i].rutaLen = 0;
    j->obreros[i].rutaIdx = 0;

    j->obreros[i].animActual = animPorDireccion(DIR_FRONT);
    j->obreros[i].animTick = 0;

    // La collisionMap se construye al cargar el mapa (cargarRecursosGraficos),
    // que ocurre DESPUÉS del WM_CREATE; por eso dejamos esto sin inicializar aquí.
    j->obreros[i].celdaFila = -1;
    j->obreros[i].celdaCol = -1;
    }

}
void IniciacionTropa (struct Tropa *t, const char *Nombre, int Oro , int Comida, int Madera, int Piedra, int Vida, int Fuerza , int VelocidadAtaque,int DistanciaAtaque){
	strcpy (t->Nombre,Nombre);
	t->CostoComida= Comida;
	t->CostoOro= Oro;
	t->CostoMadera=Madera;
	t->CostoPiedra=Piedra;
	t->Vida=Vida;
	t->Fuerza=Fuerza;
	t->VelocidadAtaque=VelocidadAtaque;
	t->DistanciaAtaque=DistanciaAtaque;
	
	
}
void gotoxy(int x, int y) {
    COORD coord;
    coord.X = x;
    coord.Y = y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}
// void mostrarStats(struct Jugador j, int x, int y) {
//     // Ahora usa la funcion de mapa.c
//     dibujarPanelEnMapa(j);
// }

void actualizarObreros(struct Jugador *j) {
    const float vel = 3.5f;
    int **baseCollision = mapaObtenerCollisionMap();

    // 1) Sincronizar ocupación inicial (una sola vez por obrero, cuando ya exista collisionMap con árboles)
    if (baseCollision) {
        UnidadObrero *baseInit = j->obreros;
        for (UnidadObrero *o = baseInit; o < baseInit + 6; o++) {
            if (o->celdaFila >= 0 && o->celdaCol >= 0) continue;

            int f = obreroFilaActual(o);
            int c = obreroColActual(o);

            // Si cayó sobre una celda bloqueada por árbol (1) u otra unidad (2), recolocar cercano.
            if (*(*(baseCollision + f) + c) != 0) {
                int nf = f, nc = c;
                if (buscarCeldaLibreCerca(baseCollision, f, c, &nf, &nc)) {
                    // Posicionar top-left para que el centro coincida con el centro de celda.
                    float cx = celdaCentroPixel(nc);
                    float cy = celdaCentroPixel(nf);
                    o->x = cx - (float)TILE_SIZE * 0.5f;
                    o->y = cy - (float)TILE_SIZE * 0.5f;
                    o->destinoX = o->x;
                    o->destinoY = o->y;
                    o->moviendose = false;
                    f = nf; c = nc;
                }
            }
            ocupacionActualizarUnidad(baseCollision, o, f, c);
        }
    }

    // Movimiento por ruta y colisiones simples unidad/unidad (misma celda)
    UnidadObrero *base = j->obreros;
    for (UnidadObrero *o = base; o < base + 6; o++) {
        if (!o->moviendose) continue;

        // Si no hay ruta, intentar ir directo (pero respetando límites)
        float targetX = o->destinoX;
        float targetY = o->destinoY;

        // Sistema "siguiente nodo": usamos puntero al nodo actual dentro de la ruta.
        int nextF = -1, nextC = -1;
        if (o->rutaCeldas && o->rutaIdx < o->rutaLen) {
            int *pNodo = o->rutaCeldas + o->rutaIdx; // puntero al nodo actual
            int celda = *pNodo;
            nextF = celda / GRID_SIZE;
            nextC = celda % GRID_SIZE;

            // Regla: el movimiento se detiene si el siguiente nodo está bloqueado inesperadamente.
            // (Esto evita atravesar otras unidades u objetos.)
            if (baseCollision && *(*(baseCollision + nextF) + nextC) != 0) {
                o->moviendose = false;
                obreroLiberarRuta(o);
                continue;
            }

            // Steering hacia el centro de la celda siguiente.
            targetX = celdaCentroPixel(nextC); // centro (mundo) en X
            targetY = celdaCentroPixel(nextF); // centro (mundo) en Y
        } else {
            // Si no hay ruta, usamos el destino como punto del mundo, pero steer al centro del sprite.
            targetX = o->destinoX + (float)TILE_SIZE * 0.5f;
            targetY = o->destinoY + (float)TILE_SIZE * 0.5f;
        }

        // Vector de movimiento desde el centro del obrero hacia el objetivo.
        float curCX = o->x + (float)TILE_SIZE * 0.5f;
        float curCY = o->y + (float)TILE_SIZE * 0.5f;
        float vx = targetX - curCX;
        float vy = targetY - curCY;
        float dist = sqrtf(vx*vx + vy*vy);

        // Dirección (para render actual por BMP)
        if (fabsf(vx) > fabsf(vy)) o->dir = (vx > 0) ? DIR_RIGHT : DIR_LEFT;
        else o->dir = (vy > 0) ? DIR_FRONT : DIR_BACK;
        o->animActual = animPorDireccion(o->dir);

        // Animación lógica (frame) por ticks
        if (o->animActual && o->animActual->frameCount > 0) {
            o->animTick++;
            if (o->animTick >= o->animActual->ticksPerFrame) {
                o->animTick = 0;
                o->frame = (o->frame + 1) % o->animActual->frameCount;
            }
        }

        if (dist <= vel) {
            // Llegó al centro del waypoint: snap del centro y avanzar nodo.
            float snapCX = targetX;
            float snapCY = targetY;

            // Convertir centro -> top-left para render (no tocamos draw)
            o->x = snapCX - (float)TILE_SIZE * 0.5f;
            o->y = snapCY - (float)TILE_SIZE * 0.5f;

            if (o->rutaCeldas && o->rutaIdx < o->rutaLen) {
                o->rutaIdx++;
            }

            // Actualizar ocupación si cambió de celda completamente
            if (baseCollision) {
                int cf = obreroFilaActual(o);
                int cc = obreroColActual(o);
                if (cf != o->celdaFila || cc != o->celdaCol) {
                    ocupacionActualizarUnidad(baseCollision, o, cf, cc);
                }
            }

            if (!o->rutaCeldas || o->rutaIdx >= o->rutaLen) {
                // Llegó al destino final
                o->moviendose = false;
                obreroLiberarRuta(o);
            }
            continue;
        }

        // Paso propuesto (steering): mover el centro, luego convertir a top-left.
        float stepX = (vx / dist) * vel;
        float stepY = (vy / dist) * vel;

        float newCX = curCX + stepX;
        float newCY = curCY + stepY;

        // Restricción de isla (0..2048) aplicada al top-left (sprite 64x64)
        float newX = newCX - (float)TILE_SIZE * 0.5f;
        float newY = newCY - (float)TILE_SIZE * 0.5f;
        if (newX < 0) newX = 0;
        if (newY < 0) newY = 0;
        if (newX > (float)(MAPA_SIZE - TILE_SIZE)) newX = (float)(MAPA_SIZE - TILE_SIZE);
        if (newY > (float)(MAPA_SIZE - TILE_SIZE)) newY = (float)(MAPA_SIZE - TILE_SIZE);

        // Si cambia de celda, solo permitimos entrar si la celda destino está libre.
        if (baseCollision) {
            int oldF = obreroFilaActual(o);
            int oldC = obreroColActual(o);

            UnidadObrero temp = *o;
            temp.x = newX;
            temp.y = newY;
            int newF = obreroFilaActual(&temp);
            int newC = obreroColActual(&temp);

            if (newF != oldF || newC != oldC) {
                // Si la celda nueva está ocupada (árbol=1 u otra unidad=2), detener.
                if (*(*(baseCollision + newF) + newC) != 0) {
                    o->moviendose = false;
                    obreroLiberarRuta(o);
                    continue;
                }

                // Entramos a nueva celda => liberar vieja (solo si era unidad) y bloquear nueva.
                ocupacionActualizarUnidad(baseCollision, o, newF, newC);
            }
        }

        o->x = newX;
        o->y = newY;
    }
}

void rtsComandarMovimiento(struct Jugador *j, float mundoX, float mundoY) {
    // Clamp destino a la isla (top-left sprite 64x64)
    float dx = mundoX;
    float dy = mundoY;
    if (dx < 0) dx = 0;
    if (dy < 0) dy = 0;
    if (dx > (float)(MAPA_SIZE - TILE_SIZE)) dx = (float)(MAPA_SIZE - TILE_SIZE);
    if (dy > (float)(MAPA_SIZE - TILE_SIZE)) dy = (float)(MAPA_SIZE - TILE_SIZE);

    // Goal en grid basado en el centro del sprite (destino top-left + 32)
    int goalF = pixelACelda(dy + (float)TILE_SIZE * 0.5f);
    int goalC = pixelACelda(dx + (float)TILE_SIZE * 0.5f);

    int **baseCollision = mapaObtenerCollisionMap();
    if (!baseCollision) return;

    UnidadObrero *base = j->obreros;
    for (UnidadObrero *o = base; o < base + 6; o++) {
        if (!o->seleccionado) continue;

        // Objetivo de grid
        o->objetivoFila = goalF;
        o->objetivoCol = goalC;
        o->destinoX = dx;
        o->destinoY = dy;

        int startF = obreroFilaActual(o);
        int startC = obreroColActual(o);

        // Construir colisión temporal: usamos collisionMap (árboles + unidades) y
        // liberamos la celda actual del obrero para permitir arrancar.
        int **tmp = gridAlloc(GRID_SIZE);
        if (!tmp) continue;
        gridCopy(tmp, baseCollision, GRID_SIZE);
        *(*(tmp + startF) + startC) = 0;

        obreroLiberarRuta(o);
        int *ruta = NULL;
        int rutaLen = 0;
        bool ok = pathfindBFS(startF, startC, goalF, goalC, tmp, &ruta, &rutaLen);
        gridFree(tmp, GRID_SIZE);

        if (!ok) {
            free(ruta);
            o->moviendose = false;
            continue;
        }

        o->rutaCeldas = ruta;
        o->rutaLen = rutaLen;
        o->rutaIdx = 0;
        o->moviendose = true;
    }
}

void rtsLiberarMovimientoJugador(struct Jugador *j) {
    UnidadObrero *base = j->obreros;
    for (UnidadObrero *o = base; o < base + 6; o++) {
        obreroLiberarRuta(o);
    }
}