#ifndef NAVEGACION_H
#define NAVEGACION_H

#include "recursos.h"
#include "../edificios/edificios.h"
#include <windows.h>

// Verifica si un punto está dentro del barco (para detección de clicks)
bool barcoContienePunto(Barco* barco, float mundoX, float mundoY);

// Desembarca las tropas del barco en tierra cercana
void desembarcarTropas(Barco* barco, struct Jugador* j);

// Reinicia recursos y estado al llegar a una isla desconocida
void reiniciarIslaDesconocida(struct Jugador* j);

// NUEVO: Viaja directamente a una isla sin animación
bool viajarAIsla(struct Jugador* j, int islaDestino);

// Registrar la isla inicial
void navegacionRegistrarIslaInicial(int isla);

// Posición fija del barco por isla (exportada para uso externo)
void navegacionObtenerPosicionBarcoIsla(int isla, float *outX, float *outY, int *outDir);

// Acceso a enemigos activos en la isla actual
Unidad *navegacionObtenerEnemigosActivos(int *cantidad);

// Obtiene la isla inicial registrada al inicio de la partida
int navegacionObtenerIslaInicial(void);

// Consulta si una isla está conquistada
bool navegacionIsIslaConquistada(int isla);

// True si la isla actual del jugador NO está conquistada
bool navegacionIslaActualNoConquistada(const struct Jugador *j);

// Guardado: sincroniza la isla activa para que el snapshot refleje muertes y tropas actuales
void navegacionSincronizarIslaActual(struct Jugador *j);

typedef struct {
	float x, y;
	float destinoX, destinoY;
	bool moviendose;
	bool seleccionado;
	int dir;
	int frame;
	int objetivoFila;
	int objetivoCol;
	int celdaFila;
	int celdaCol;
	int tipo;
	int vida;
	int vidaMax;
	int damage;
	int critico;
	int defensa;
	int alcance;
	bool recibiendoAtaque;
	unsigned long long tiempoMuerteMs;
	int frameMuerte;
} UnidadIslaSerializable;

typedef struct {
	int tipo;
	float x, y;
	int ancho, alto;
	bool construido;
	int oroAcumulado;
	int piedraAcumulada;
	int hierroAcumulado;
	unsigned int ultimoTickGeneracion;
	int oroRestante;
	int piedraRestante;
	int hierroRestante;
	bool agotada;
} EdificioIslaSerializable;

typedef struct {
	bool inicializado;
	int Comida;
	int Oro;
	int Madera;
	int Piedra;
	int Hierro;
	bool tieneAyuntamiento;
	bool tieneMina;
	bool tieneCuartel;
	EdificioIslaSerializable ayuntamiento;
	EdificioIslaSerializable mina;
	EdificioIslaSerializable cuartel;
	UnidadIslaSerializable obreros[6];
	UnidadIslaSerializable caballeros[4];
	UnidadIslaSerializable caballerosSinEscudo[4];
	UnidadIslaSerializable guerreros[4];
	UnidadIslaSerializable enemigos[8];
	int numEnemigos;
	bool enemigosGenerados;
} EstadoIslaSerializable;

void navegacionExportarEstadosIsla(EstadoIslaSerializable estados[6]);
void navegacionImportarEstadosIsla(const EstadoIslaSerializable estados[6]);
void navegacionRestaurarIslaInicial(int isla, bool definida);
bool navegacionIslaInicialDefinida(void);
void navegacionActivarEnemigosIsla(int isla);

// Cuenta las unidades de un tipo en TODAS las islas (activas y guardadas)
int navegacionContarUnidadesGlobal(const struct Jugador *j, TipoUnidad tipo);

// Guardado: reinicia el snapshot persistente entre sesiones/menus
void navegacionReiniciarEstado(void);


#endif

