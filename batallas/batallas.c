// batallas/batallas.c
#include "batallas.h"
#include "../recursos/navegacion.h"
#include "../mapa/mapa.h"
#include <windows.h>
#include <math.h>

// Emparejamientos 1 vs 1: índice del enemigo <-> índice del aliado
// Aliados: 12 máximo (4 caballeros, 4 sin escudo, 4 guerreros)
// Enemigos: 8 máximo
static int sPairEnemyToAlly[8] = {-1, -1, -1, -1, -1, -1, -1, -1};
static int sPairAllyToEnemy[12] = {-1};
static ULONGLONG sLastAttackMsEnemy[8] = {0};
static ULONGLONG sLastAttackMsAlly[12] = {0};
static bool sHuboAliadoEnBatalla = false; // evita derrota falsa cuando no hay tropas desplegadas
// Garantiza que una unidad tenga stats básicos asignados
static void asegurarStatsUnidad(Unidad *u) {
	if (!u) return;
	// Vida por tipo si falta
	if (u->vidaMax <= 0) {
		if (u->tipo == TIPO_CABALLERO) { u->vidaMax = CABALLERO_VIDA; u->vida = CABALLERO_VIDA; }
		else if (u->tipo == TIPO_CABALLERO_SIN_ESCUDO) { u->vidaMax = CABALLERO_SIN_ESCUDO_VIDA; u->vida = CABALLERO_SIN_ESCUDO_VIDA; }
		else if (u->tipo == TIPO_GUERRERO) { u->vidaMax = GUERRERO_VIDA; u->vida = GUERRERO_VIDA; }
	}
	// Daño, defensa y crítico por tipo si no están configurados
	if (u->tipo == TIPO_CABALLERO) {
		if (u->damage <= 0) u->damage = CABALLERO_DANO;
		if (u->defensa <= 0) u->defensa = CABALLERO_DEFENSA;
		if (u->critico <= 0) u->critico = CABALLERO_CRITICO;
	} else if (u->tipo == TIPO_CABALLERO_SIN_ESCUDO) {
		if (u->damage <= 0) u->damage = CABALLERO_SIN_ESCUDO_DANO;
		if (u->defensa <= 0) u->defensa = CABALLERO_SIN_ESCUDO_DEFENSA;
		if (u->critico <= 0) u->critico = CABALLERO_SIN_ESCUDO_CRITICO;
	} else if (u->tipo == TIPO_GUERRERO) {
		if (u->damage <= 0) u->damage = GUERRERO_DANO;
		if (u->defensa <= 0) u->defensa = GUERRERO_DEFENSA;
		if (u->critico <= 0) u->critico = GUERRERO_CRITICO;
	}
}

static int clampIndex(int v, int lo, int hi) { return (v < lo) ? lo : (v > hi ? hi : v); }

// Calcula daño usando stats de recursos.h
static int calcularDanio(const Unidad *atacante, const Unidad *defensor) {
	int danoBase = atacante->damage;
	int defensa = defensor->defensa;
	int neto = danoBase - defensa;
	if (neto < 5) neto = 5; // daño mínimo
	// Crítico simple
	int crit = atacante->critico;
	if (crit > 0) {
		int r = rand() % 100;
		if (r < crit) neto = (int)(neto * 1.5f);
	}
	return neto;
}

// Actualiza una pequeña persecución hacia el objetivo evitando solapamiento básico
static void moverHaciaObjetivo(Unidad *u, const Unidad *obj, float vel) {
	float dx = obj->x - u->x;
	float dy = obj->y - u->y;
	float d = sqrtf(dx * dx + dy * dy);
	if (d < 1.0f) { u->moviendose = false; return; }
	u->moviendose = true;
	float ux = u->x + vel * (dx / d);
	float uy = u->y + vel * (dy / d);
	// Evitar compartir casilla
	int **col = mapaObtenerCollisionMap();
	if (col) {
		int nx = (int)(ux / (float)TILE_SIZE);
		int ny = (int)(uy / (float)TILE_SIZE);
		if (nx >= 0 && nx < GRID_SIZE && ny >= 0 && ny < GRID_SIZE) {
			int cx = (int)(u->x / (float)TILE_SIZE);
			int cy = (int)(u->y / (float)TILE_SIZE);
			bool mismaCelda = (nx == cx && ny == cy);
			// Permitir avanzar dentro de la misma celda aunque esté marcada como ocupada por la propia unidad
			if (mismaCelda || col[ny][nx] == 0) {
				if (!mismaCelda && cy >= 0 && cy < GRID_SIZE && cx >= 0 && cx < GRID_SIZE) col[cy][cx] = 0;
				u->x = ux; u->y = uy;
				col[ny][nx] = 3;
			} else {
				// celda ocupada, no mover
				u->moviendose = false;
			}
		}
	}
}

void batallasActualizar(struct Jugador *j) {
	// Obtener enemigos activos
	int numEnemigos = 0;
	Unidad *enemigos = navegacionObtenerEnemigosActivos(&numEnemigos);
	if (!enemigos || numEnemigos <= 0) {
		sHuboAliadoEnBatalla = false;
		return;
	}

	// Asegurar stats de enemigos (por seguridad en restauraciones)
	for (int e = 0; e < numEnemigos; e++) {
		asegurarStatsUnidad(&enemigos[e]);
	}

	// Asegurar emparejamientos limpios (evita estado inicial incorrecto)
	for (int i = 0; i < 8; i++) {
		sPairEnemyToAlly[i] = -1;
	}
	for (int i = 0; i < 12; i++) {
		sPairAllyToEnemy[i] = -1;
	}

	// Construir lista de aliados (cab, cse, gue)
	Unidad *aliados[12];
	int nAliados = 0;
	for (int i = 0; i < 4; i++) aliados[nAliados++] = &j->caballeros[i];
	for (int i = 0; i < 4; i++) aliados[nAliados++] = &j->caballerosSinEscudo[i];
	for (int i = 0; i < 4; i++) aliados[nAliados++] = &j->guerreros[i];

	// Asegurar stats de aliados (entrenados o restaurados)
	for (int a = 0; a < nAliados; a++) {
		asegurarStatsUnidad(aliados[a]);
	}

	// 1) Detección: si aliado cerca, emparejar 1vs1
	const float rangoDeteccion = 180.0f;
	const float rangoAtk = 64.0f; // melee

	for (int e = 0; e < numEnemigos; e++) {
		Unidad *en = &enemigos[e];
		if (en->vida <= 0 || en->x < 0) continue;
		int empActual = sPairEnemyToAlly[e];
		if (empActual >= 0) {
			// Validar que aliado sigue vivo
			Unidad *al = aliados[empActual];
			if (!al || al->vida <= 0 || al->x < 0) { sPairEnemyToAlly[e] = -1; }
			continue;
		}
		// Buscar aliado disponible
		for (int a = 0; a < nAliados; a++) {
			Unidad *al = aliados[a];
			if (!al || al->vida <= 0 || al->x < 0) continue;
			if (sPairAllyToEnemy[a] >= 0) continue; // ya emparejado
			float dx = al->x - en->x; float dy = al->y - en->y; float d2 = dx*dx + dy*dy;
			if (d2 <= rangoDeteccion * rangoDeteccion) {
				sPairEnemyToAlly[e] = a; sPairAllyToEnemy[a] = e; break;
			}
		}
	}

	// 2) Movimiento y ataques por turnos, lento
	ULONGLONG ahora = GetTickCount64();
	const ULONGLONG cadenciaMs = 1200; // ataques lentos
	const float velMov = 1.5f; // movimiento lento

	for (int e = 0; e < numEnemigos; e++) {
		int a = sPairEnemyToAlly[e];
		if (a < 0 || a >= nAliados) continue;
		Unidad *en = &enemigos[e];
		Unidad *al = aliados[a];
		if (en->vida <= 0 || al->vida <= 0) continue;

		float dx = al->x - en->x; float dy = al->y - en->y; float d2 = dx*dx + dy*dy;
		if (d2 > rangoAtk * rangoAtk) {
			// acercarse mutuamente (enemigo se mueve)
			moverHaciaObjetivo(en, al, velMov);
			en->recibiendoAtaque = false; al->recibiendoAtaque = false;
			continue;
		}

		// Turnos: alternar según últimos tiempos
		bool turnoEnemigo = (sLastAttackMsEnemy[e] <= sLastAttackMsAlly[a]);
		if (turnoEnemigo) {
			if (ahora - sLastAttackMsEnemy[e] >= cadenciaMs) {
				int danio = calcularDanio(en, al);
				al->recibiendoAtaque = true;
				if (al->vida > 0) {
					al->vida -= danio; if (al->vida < 0) al->vida = 0;
					if (al->vida == 0 && al->tiempoMuerteMs == 0) {
						al->tiempoMuerteMs = ahora;
						al->moviendose = false;
					}
				}
				sLastAttackMsEnemy[e] = ahora;
			}
			en->moviendose = false;
		} else {
			if (ahora - sLastAttackMsAlly[a] >= cadenciaMs) {
				int danio = calcularDanio(al, en);
				en->recibiendoAtaque = true;
				if (en->vida > 0) {
					en->vida -= danio; if (en->vida < 0) en->vida = 0;
					if (en->vida == 0 && en->tiempoMuerteMs == 0) {
						en->tiempoMuerteMs = ahora;
						en->moviendose = false;
					}
				}
				sLastAttackMsAlly[a] = ahora;
			}
			al->moviendose = false;
		}
	}

	// Limpiar flags de impacto tras breve tiempo
	static ULONGLONG sLastClearMs = 0; if (sLastClearMs == 0) sLastClearMs = ahora;
	if (ahora - sLastClearMs >= 500) {
		for (int a = 0; a < nAliados; a++) if (aliados[a]) aliados[a]->recibiendoAtaque = false;
		for (int e = 0; e < numEnemigos; e++) enemigos[e].recibiendoAtaque = false;
		sLastClearMs = ahora;
	}

	// 3) Chequear victoria/derrota
	int vivosEnemigos = 0; for (int e = 0; e < numEnemigos; e++) if (enemigos[e].vida > 0 && enemigos[e].x >= 0) vivosEnemigos++;
	int vivosAliados = 0; for (int a = 0; a < nAliados; a++) if (aliados[a] && aliados[a]->vida > 0 && aliados[a]->x >= 0) vivosAliados++;
	int tropasEmbarcadas = j ? j->barco.numTropas : 0;
	int aliadosPresentes = vivosAliados + tropasEmbarcadas;
	if (aliadosPresentes > 0) sHuboAliadoEnBatalla = true;

	if (vivosEnemigos == 0) {
		// Victoria
		MessageBox(NULL, "Has conquistado la isla", "Batalla", MB_OK | MB_ICONINFORMATION);
		sHuboAliadoEnBatalla = false;
	}
	if (vivosAliados == 0 && vivosEnemigos > 0 && sHuboAliadoEnBatalla && tropasEmbarcadas == 0) {
		// Derrota: regresar a isla inicial primero, luego mostrar mensaje
		extern bool viajarAIsla(struct Jugador * j, int islaDestino);
		extern int navegacionObtenerIslaInicial(void);
		int islaInicial = navegacionObtenerIslaInicial();
		viajarAIsla(j, islaInicial);
		MessageBox(NULL, "Perdiste la batalla, todas tus tropas han muerto", "Batalla", MB_OK | MB_ICONWARNING);
		sHuboAliadoEnBatalla = false;
	}
}
