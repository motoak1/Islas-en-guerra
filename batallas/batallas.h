// Sistema de batallas por islas
#ifndef BATALLAS_H
#define BATALLAS_H

#include <windows.h>
#include <stdbool.h>
#include "../recursos/recursos.h"
#include "../mapa/mapa.h"

typedef enum {
	BATALLA_RESULTADO_NONE = 0,
	BATALLA_RESULTADO_VICTORIA,
	BATALLA_RESULTADO_DERROTA
} BatallaResultado;

// Inicializa semilla y carga mínima del módulo (debe llamarse una vez al arrancar)
void batallasInicializar(void);

// Arranca una batalla al viajar; devuelve true si la batalla fue creada.
// No genera batalla si es isla inicial o ya visitada.
bool batallasPrepararDesdeViaje(struct Jugador *j, int islaDestino,
																int islasConquistadas, bool esIslaInicial);

// Estado de la escena de batalla
bool batallasEnCurso(void);
void batallasActualizar(float dt);    // dt en segundos (usar 0.016f con el timer actual)
void batallasRender(HDC hdc, RECT rect, Camara cam);

// Resultado listo tras finalizar; al llamar, limpia el resultado almacenado.
bool batallasObtenerResultado(BatallaResultado *outResultado, int *outIslaDestino);

#endif // BATALLAS_H
