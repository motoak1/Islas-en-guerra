#include "batallas.h"
#include "../recursos/navegacion.h"
#include "../mapa/mapa.h"
#include <windows.h>
#include <math.h>

#define MAX_ALIADOS_EN_BATALLA (MAX_CABALLEROS + MAX_CABALLEROS_SIN_ESCUDO + MAX_GUERREROS)

#include "batallas.h"
#include "../recursos/navegacion.h"
#include "../mapa/mapa.h"
#include <windows.h>
#include <math.h>

#define MAX_ALIADOS (MAX_CABALLEROS + MAX_CABALLEROS_SIN_ESCUDO + MAX_GUERREROS)

static int sPairEnemy[8], sPairAlly[MAX_ALIADOS]; // Indices -1
static ULONGLONG sAttTimeEnemy[8] = {0}, sAttTimeAlly[MAX_ALIADOS] = {0};
static bool sHuboAliado = false, sMsgVictoria = false;
static int sLastEnemigos = -1, sLastIsla = -1;

void batallasReiniciarEstado(void) {
    for (int i=0; i<8; i++) { sPairEnemy[i] = -1; sAttTimeEnemy[i] = 0; }
    for (int i=0; i<MAX_ALIADOS; i++) { sPairAlly[i] = -1; sAttTimeAlly[i] = 0; }
    sHuboAliado = false; sMsgVictoria = false; sLastEnemigos = -1; sLastIsla = -1;
}

static void asegurarStats(Unidad *u) {
    if (!u || u->vidaMax > 0) return;
    int stats[][4] = { // vida, dano, defensa, critico
        {CABALLERO_VIDA, CABALLERO_DANO, CABALLERO_DEFENSA, CABALLERO_CRITICO},
        {CABALLERO_SIN_ESCUDO_VIDA, CABALLERO_SIN_ESCUDO_DANO, CABALLERO_SIN_ESCUDO_DEFENSA, CABALLERO_SIN_ESCUDO_CRITICO},
        {GUERRERO_VIDA, GUERRERO_DANO, GUERRERO_DEFENSA, GUERRERO_CRITICO}
    };
    int idx = (u->tipo == TIPO_CABALLERO) ? 0 : (u->tipo == TIPO_CABALLERO_SIN_ESCUDO ? 1 : 2);
    u->vidaMax = u->vida = stats[idx][0];
    u->dano = stats[idx][1]; u->defensa = stats[idx][2]; u->critico = stats[idx][3];
}

static bool moverSuelo(Unidad *u, float nx, float ny, int **col) {
    int cx = (int)(nx / TILE_SIZE), cy = (int)(ny / TILE_SIZE);
    if (cx < 0 || cx >= GRID_SIZE || cy < 0 || cy >= GRID_SIZE) return false;
    int ox = (int)(u->x / TILE_SIZE), oy = (int)(u->y / TILE_SIZE);
    if ((cx != ox || cy != oy) && col && col[cy][cx] != 0) return false;
    if (col && ox >= 0 && ox < GRID_SIZE && oy >= 0 && oy < GRID_SIZE) col[oy][ox] = 0;
    u->x = nx; u->y = ny;
    if (col) col[cy][cx] = 3;
    return true;
}

static void moverAU(Unidad *u, const Unidad *obj, float spd) {
    float dx = obj->x - u->x, dy = obj->y - u->y;
    float d = sqrtf(dx*dx + dy*dy);
    if (d < 1.0f) { u->moviendose = false; return; }
    u->moviendose = true;
    u->dir = (fabsf(dx) > fabsf(dy)) ? (dx > 0 ? DIR_RIGHT : DIR_LEFT) : (dy > 0 ? DIR_FRONT : DIR_BACK);

    int **col = mapaObtenerCollisionMap();
    float nx = u->x + spd * dx/d, ny = u->y + spd * dy/d;
    if (moverSuelo(u, nx, ny, col)) return;
    // Slide perpendicular
    float perX = -dy/d, perY = dx/d;
    for (int s=-1; s<=1; s+=2)
        if (moverSuelo(u, nx + perX * 22.0f * s, ny + perY * 22.0f * s, col)) return;
    u->moviendose = false;
}

void simularBatalla(struct Jugador *j) {
    if (!j) return;
    if (j->islaActual != sLastIsla) { sLastIsla = j->islaActual; sLastEnemigos = -1; }
    
    int nEnemigos = 0;
    Unidad *enemigos = navegacionObtenerEnemigosActivos(&nEnemigos);
    if (!enemigos || nEnemigos <= 0) { sHuboAliado = false; return; }

    for (int e=0; e<nEnemigos; e++) {
        asegurarStats(&enemigos[e]);
        if (enemigos[e].vida > 0) sMsgVictoria = false;
    }

    // Reset pairs
    for (int i=0; i<8; i++) sPairEnemy[i] = -1;
    for (int i=0; i<MAX_ALIADOS; i++) sPairAlly[i] = -1;

    // Build Allies List
    Unidad *aliados[MAX_ALIADOS];
    int nAliados = 0;
    Unidad *arrays[] = {j->caballeros, j->caballerosSinEscudo, j->guerreros};
    int counts[] = {MAX_CABALLEROS, MAX_CABALLEROS_SIN_ESCUDO, MAX_GUERREROS};
    for(int k=0; k<3; k++) {
        for(int i=0; i<counts[k] && nAliados < MAX_ALIADOS; i++) {
            if (arrays[k][i].x >= 0) aliados[nAliados++] = &arrays[k][i];
        }
    }
    for (int i=0; i<nAliados; i++) asegurarStats(aliados[i]);

    // Pairing (128 range)
    for (int e=0; e<nEnemigos; e++) {
        Unidad *en = &enemigos[e];
        if (en->vida <= 0 || en->x < 0 || sPairEnemy[e] >= 0) continue;
        for (int a=0; a<nAliados; a++) {
            Unidad *al = aliados[a];
            if (sPairAlly[a] < 0 && al->vida > 0 && al->x >= 0) {
                float dx = al->x - en->x, dy = al->y - en->y;
                if (dx*dx + dy*dy <= 128.0f*128.0f) { sPairEnemy[e] = a; sPairAlly[a] = e; break; }
            }
        }
    }

    // Combat
    DWORD tick = GetTickCount();
    for (int e=0; e<nEnemigos; e++) {
        int a = sPairEnemy[e];
        if (a < 0) continue;
        Unidad *en = &enemigos[e], *al = aliados[a];
        if (en->vida <= 0 || al->vida <= 0) continue;

        float d2 = (al->x - en->x)*(al->x - en->x) + (al->y - en->y)*(al->y - en->y);
        if (d2 > RANGO_GOLPE_MELEE * RANGO_GOLPE_MELEE) {
            moverAU(en, al, 1.5f); moverAU(al, en, 1.5f);
            en->recibiendoAtaque = al->recibiendoAtaque = false;
            continue;
        }

        // Logic fusion
        bool enTurn = sAttTimeEnemy[e] < sAttTimeAlly[a];
        Unidad *atkr = enTurn ? en : al;
        Unidad *def = enTurn ? al : en;
        ULONGLONG *lastT = enTurn ? &sAttTimeEnemy[e] : &sAttTimeAlly[a];

        if (tick - *lastT >= 1000) {
            int dmg = atkr->dano - def->defensa;
            if (dmg < 5) dmg = 5;
            if (atkr->critico > rand()%100) dmg = (int)(dmg*1.5f);
            def->recibiendoAtaque = true;
            def->vida = (def->vida > dmg) ? def->vida - dmg : 0;
            if (def->vida == 0 && def->tiempoMuerteMs == 0) { def->tiempoMuerteMs = (ULONGLONG)tick; def->moviendose = false; }
            *lastT = tick;
        }
        atkr->moviendose = false;
    }

    // Visual cleanup
    static DWORD tClean = 0;
    if (tick - tClean >= 500) {
        for(int i=0; i<nAliados; i++) aliados[i]->recibiendoAtaque = false;
        for(int i=0; i<nEnemigos; i++) enemigos[i].recibiendoAtaque = false;
        tClean = tick;
    }

    // Victory Check
    int nEnVivos=0; for(int i=0; i<nEnemigos; i++) if(enemigos[i].vida > 0 && enemigos[i].x >= 0) nEnVivos++;
    int nAlVivos=0; for(int i=0; i<nAliados; i++) if(aliados[i]->vida > 0 && aliados[i]->x >= 0) nAlVivos++;
    if (nAlVivos + j->barco.numTropas > 0) sHuboAliado = true;

    if (sLastEnemigos < 0) sLastEnemigos = nEnVivos;
    if (nEnVivos == 0 && sLastEnemigos > 0 && !sMsgVictoria) {
        MessageBox(NULL, "Has conquistado la isla", "Batalla", MB_OK | MB_ICONINFORMATION);
        sMsgVictoria = true; sHuboAliado = false;
    }
    sLastEnemigos = nEnVivos;

    if (nAlVivos == 0 && nEnVivos > 0 && sHuboAliado && j->barco.numTropas == 0) {
        extern bool viajarAIsla(struct Jugador *, int);
        extern int navegacionObtenerIslaInicial(void);
        viajarAIsla(j, navegacionObtenerIslaInicial());
        MessageBox(NULL, "Perdiste la batalla", "Batalla", MB_OK | MB_ICONWARNING);
        sHuboAliado = false;
    }
}
