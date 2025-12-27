// recursos/ui_compra.h
#ifndef UI_COMPRA_H
#define UI_COMPRA_H

#include "recursos.h"
#include <stdbool.h>
#include <windows.h>

// --- CONSTANTES DE COMPRA ---
#define COSTO_CULTIVO_ORO 20
#define COSTO_CULTIVO_PIEDRA 15
#define GANANCIA_CULTIVO_COMIDA 50

// --- ESTRUCTURA DEL MENÚ DE COMPRA ---
typedef struct MenuCompra {
  bool abierto;
  int pantallaX, pantallaY; // Posición en pantalla del menú
  int ancho, alto;          // Dimensiones del menú
  RECT botonComprar;        // Rectángulo del botón comprar
  RECT botonCerrar;         // Rectángulo del botón cerrar
  char mensajeError[100];   // Mensaje de error si recursos insuficientes
  int tiempoError;          // Contador para mostrar error temporalmente
} MenuCompra;

// --- FUNCIONES DEL MENÚ ---

// Inicializa el menú de compra
void menuCompraInicializar(MenuCompra *menu);

// Abre el menú centrado en la pantalla
void menuCompraAbrir(MenuCompra *menu, int anchoPantalla, int altoPantalla);

// Cierra el menú
void menuCompraCerrar(MenuCompra *menu);

// Dibuja el menú en pantalla
void menuCompraDibujar(HDC hdc, MenuCompra *menu, struct Jugador *jugador);

// Maneja clicks del mouse en el menú, retorna true si se procesó el click
bool menuCompraClick(MenuCompra *menu, struct Jugador *jugador, int pantallaX,
                     int pantallaY);

// Actualiza el estado del menú (para animaciones, timers, etc)
void menuCompraActualizar(MenuCompra *menu);

#endif
