#include "navegacion.h"
#include "../mapa/mapa.h"
#include <stdio.h>
#include <math.h>

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
  
  // Convertir posición del barco a celdas
  int barcoCeldaX = (int)(barco->x / 32.0f);
  int barcoCeldaY = (int)(barco->y / 32.0f);
  
  // Buscar tierra adyacente al barco (búsqueda en espiral)
  int tierraX = -1, tierraY = -1;
  bool tierraEncontrada = false;
  
  // Buscar en radio creciente alrededor del barco (hasta 10 celdas)
  for (int radio = 1; radio <= 10 && !tierraEncontrada; radio++) {
    for (int dy = -radio; dy <= radio && !tierraEncontrada; dy++) {
      for (int dx = -radio; dx <= radio; dx++) {
        int celdaX = barcoCeldaX + dx;
        int celdaY = barcoCeldaY + dy;
        
        // Verificar límites
        if (celdaX < 0 || celdaX >= 64 || celdaY < 0 || celdaY >= 64) continue;
        
        // Verificar si es TIERRA (valor 0 = tierra libre)
        if (*(*(col + celdaY) + celdaX) == 0) {
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
  
  // Convertir a coordenadas de píxeles
  float desembarcoX = (float)(tierraX * 32);
  float desembarcoY = (float)(tierraY * 32);
  
  // Colocar tropas en formación 3x2 cerca del punto de desembarco
  for (int i = 0; i < barco->numTropas; i++) {
    Unidad* tropa = barco->tropas[i];
    if (tropa) {
      // Formación: 3 columnas, 2 filas
      int col = i % 3;
      int fila = i / 3;
      
      tropa->x = desembarcoX + (col * 70.0f);
      tropa->y = desembarcoY + (fila * 70.0f);
      
      printf("[DEBUG] Tropa %d desembarcada en (%.1f, %.1f)\n", 
             i, tropa->x, tropa->y);
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
  
  // Cambiar isla activa
  j->islaActual = islaDestino;
  
  // Cambiar mapa de isla y recargar gráficos PRIMERO
  mapaSeleccionarIsla(islaDestino);
  cargarRecursosGraficos(); // Esto crea el nuevo mapa de colisión
  
  // Posicionar barco en orilla de la nueva isla
  float nuevoBarcoX, nuevoBarcoY;
  int nuevoDir;
  mapaDetectarOrilla(&nuevoBarcoX, &nuevoBarcoY, &nuevoDir);
  j->barco.x = nuevoBarcoX;
  j->barco.y = nuevoBarcoY;
  j->barco.dir = (Direccion)nuevoDir;
  
  printf("[DEBUG] Barco reposicionado en isla %d: (%.1f, %.1f)\n",
         islaDestino, j->barco.x, j->barco.y);
  
  // CRÍTICO: Desembarcar tropas DESPUÉS de tener el mapa actualizado
  desembarcarTropas(&j->barco, j);
  
  // AHORA sí, reiniciar isla desconocida (resetear recursos, eliminar edificios)
  // Esto se hace AL FINAL para no afectar las tropas ya desembarcadas
  reiniciarIslaDesconocida(j);
  
  printf("[DEBUG] Viaje completado\n");
}

