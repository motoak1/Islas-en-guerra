#include "ui_embarque.h"
#include "../mapa/menu.h"
#include "navegacion.h"
#include "recursos.h"
#include <stdio.h>
#include <windows.h>


// Una unidad disponible está visible en el mapa y no está ocupada moviéndose
static bool unidadDisponible(const Unidad *u) {
  return u && u->x >= 0 && u->y >= 0 && !u->moviendose;
}

// Requiere que la unidad esté en la orilla junto al barco (zona corta de
// abordaje)
static bool unidadCercaDeBarco(const Unidad *u, const Barco *barco) {
  if (!u || !barco)
    return false;
  if (u->x < 0 || u->y < 0)
    return false;

  const float BARCO_SIZE = 192.0f;
  const float RADIO_EMBARQUE = 490.0f; // ~1.5 tiles desde el casco

  // Distancia mínima desde la unidad al rectángulo del barco
  float nearestX = u->x;
  if (u->x < barco->x)
    nearestX = barco->x;
  else if (u->x > barco->x + BARCO_SIZE)
    nearestX = barco->x + BARCO_SIZE;

  float nearestY = u->y;
  if (u->y < barco->y)
    nearestY = barco->y;
  else if (u->y > barco->y + BARCO_SIZE)
    nearestY = barco->y + BARCO_SIZE;

  float dx = u->x - nearestX;
  float dy = u->y - nearestY;

  return (dx * dx + dy * dy) <= (RADIO_EMBARQUE * RADIO_EMBARQUE);
}

static bool unidadListaParaEmbarcar(const Unidad *u, const Barco *barco) {
  return unidadDisponible(u) && unidadCercaDeBarco(u, barco);
}

void menuEmbarqueInicializar(MenuEmbarque *menu) {
  menu->activo = false;
  menu->eligiendoIsla = false;
  menu->obrerosSeleccionados = 0;
  menu->caballerosSeleccionados = 0;
  menu->guerrerosSeleccionados = 0;
  menu->totalSeleccionados = 0;
  menu->x = 0;
  menu->y = 0;
  menu->ancho = 400;
  menu->alto = 350;
}

void menuEmbarqueAbrir(MenuEmbarque *menu, int anchoVentana, int altoVentana) {
  menu->activo = true;
  menu->eligiendoIsla = false;
  menu->obrerosSeleccionados = 0;
  menu->caballerosSeleccionados = 0;
  menu->guerrerosSeleccionados = 0;
  menu->totalSeleccionados = 0;

  // Centrar el menú en la ventana
  menu->x = (anchoVentana - menu->ancho) / 2;
  menu->y = (altoVentana - menu->alto) / 2;
}

void menuEmbarqueCerrar(MenuEmbarque *menu) { menu->activo = false; }

void menuEmbarqueDibujar(HDC hdc, MenuEmbarque *menu, struct Jugador *j) {
  if (!menu->activo)
    return;

  // Fondo del menú
  HBRUSH hBrushFondo = CreateSolidBrush(RGB(40, 40, 40));
  HBRUSH hOld = (HBRUSH)SelectObject(hdc, hBrushFondo);
  Rectangle(hdc, menu->x, menu->y, menu->x + menu->ancho, menu->y + menu->alto);
  SelectObject(hdc, hOld);
  DeleteObject(hBrushFondo);

  // Borde
  HPEN hPenBorde = CreatePen(PS_SOLID, 3, RGB(200, 200, 200));
  HPEN hPenOld = (HPEN)SelectObject(hdc, hPenBorde);
  SelectObject(hdc, GetStockObject(NULL_BRUSH));
  Rectangle(hdc, menu->x, menu->y, menu->x + menu->ancho, menu->y + menu->alto);
  SelectObject(hdc, hPenOld);
  DeleteObject(hPenBorde);

  // Título
  SetBkMode(hdc, TRANSPARENT);
  SetTextColor(hdc, RGB(255, 255, 255));
  RECT rectTitulo = {menu->x, menu->y + 10, menu->x + menu->ancho,
                     menu->y + 40};
  if (menu->eligiendoIsla) {
    DrawTextA(hdc, "SELECCIONA ISLA", -1, &rectTitulo,
              DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    int opciones[3];
    int total = 0;
    for (int i = 1; i <= 3; i++) {
      if (i != j->islaActual) {
        opciones[total++] = i;
      }
    }

    int btnWidth = 260;
    int btnHeight = 50;
    int startY = menu->y + 100;
    int startX = menu->x + (menu->ancho - btnWidth) / 2;

    for (int i = 0; i < total; i++) {
      RECT btn = {startX, startY + i * (btnHeight + 20), startX + btnWidth,
                  startY + i * (btnHeight + 20) + btnHeight};
      HBRUSH hBrushIsla = CreateSolidBrush(RGB(0, 80, 140));
      HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hBrushIsla);
      Rectangle(hdc, btn.left, btn.top, btn.right, btn.bottom);
      SelectObject(hdc, hOldBrush);
      DeleteObject(hBrushIsla);

      char label[64];
      snprintf(label, sizeof(label), "Viajar a Isla %d", opciones[i]);
      DrawTextA(hdc, label, -1, &btn, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }

    // Botón cancelar
    RECT btnCancelar = {menu->x + 50, menu->y + menu->alto - 60,
                        menu->x + menu->ancho - 50, menu->y + menu->alto - 20};
    HBRUSH hBrushCancelar = CreateSolidBrush(RGB(120, 0, 0));
    HBRUSH hOldBrush2 = (HBRUSH)SelectObject(hdc, hBrushCancelar);
    Rectangle(hdc, btnCancelar.left, btnCancelar.top, btnCancelar.right,
              btnCancelar.bottom);
    SelectObject(hdc, hOldBrush2);
    DeleteObject(hBrushCancelar);
    DrawTextA(hdc, "CANCELAR", -1, &btnCancelar,
              DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    return;
  }

  DrawTextA(hdc, "EMBARCAR TROPAS", -1, &rectTitulo,
            DT_CENTER | DT_VCENTER | DT_SINGLELINE);

  // Contador de tropas
  char bufferContador[60];
  snprintf(bufferContador, sizeof(bufferContador),
           "Tropas seleccionadas: %d / %d", menu->totalSeleccionados,
           j->barco.capacidadMaxima);
  RECT rectContador = {menu->x, menu->y + 45, menu->x + menu->ancho,
                       menu->y + 70};
  SetTextColor(hdc, (menu->totalSeleccionados == j->barco.capacidadMaxima)
                        ? RGB(255, 200, 0)
                        : RGB(200, 200, 200));
  DrawTextA(hdc, bufferContador, -1, &rectContador,
            DT_CENTER | DT_VCENTER | DT_SINGLELINE);

  int yOffset = 90;
  int lineHeight = 60;

  // Contar tropas disponibles
  int obrerosDisponibles = 0;
  int caballerosDisponibles = 0;
  int guerrerosDisponibles = 0;

  for (int i = 0; i < 6; i++) {
    if (unidadListaParaEmbarcar(&j->obreros[i], &j->barco))
      obrerosDisponibles++;
  }
  for (int i = 0; i < 4; i++) {
    if (unidadListaParaEmbarcar(&j->caballeros[i], &j->barco))
      caballerosDisponibles++;
  }
  for (int i = 0; i < 4; i++) {
    if (unidadListaParaEmbarcar(&j->caballerosSinEscudo[i], &j->barco))
      caballerosDisponibles++;
  }
  for (int i = 0; i < 4; i++) {
    if (unidadListaParaEmbarcar(&j->guerreros[i], &j->barco))
      guerrerosDisponibles++;
  }

  // Obreros
  SetTextColor(hdc, RGB(255, 255, 255));
  char bufferObreros[50];
  snprintf(bufferObreros, sizeof(bufferObreros), "Obreros: %d / %d",
           menu->obrerosSeleccionados, obrerosDisponibles);
  RECT rectObreros = {menu->x + 20, menu->y + yOffset, menu->x + 200,
                      menu->y + yOffset + 20};
  DrawTextA(hdc, bufferObreros, -1, &rectObreros,
            DT_LEFT | DT_VCENTER | DT_SINGLELINE);

  // Botones +/- para obreros
  RECT btnObreroMenos = {menu->x + 260, menu->y + yOffset, menu->x + 300,
                         menu->y + yOffset + 30};
  RECT btnObreroMas = {menu->x + 310, menu->y + yOffset, menu->x + 350,
                       menu->y + yOffset + 30};

  Rectangle(hdc, btnObreroMenos.left, btnObreroMenos.top, btnObreroMenos.right,
            btnObreroMenos.bottom);
  Rectangle(hdc, btnObreroMas.left, btnObreroMas.top, btnObreroMas.right,
            btnObreroMas.bottom);

  DrawTextA(hdc, "-", -1, &btnObreroMenos,
            DT_CENTER | DT_VCENTER | DT_SINGLELINE);
  DrawTextA(hdc, "+", -1, &btnObreroMas,
            DT_CENTER | DT_VCENTER | DT_SINGLELINE);

  yOffset += lineHeight;

  // Caballeros
  char bufferCaballeros[50];
  snprintf(bufferCaballeros, sizeof(bufferCaballeros), "Caballeros: %d / %d",
           menu->caballerosSeleccionados, caballerosDisponibles);
  RECT rectCaballeros = {menu->x + 20, menu->y + yOffset, menu->x + 200,
                         menu->y + yOffset + 20};
  DrawTextA(hdc, bufferCaballeros, -1, &rectCaballeros,
            DT_LEFT | DT_VCENTER | DT_SINGLELINE);

  // Botones +/- para caballeros
  RECT btnCaballeroMenos = {menu->x + 260, menu->y + yOffset, menu->x + 300,
                            menu->y + yOffset + 30};
  RECT btnCaballeroMas = {menu->x + 310, menu->y + yOffset, menu->x + 350,
                          menu->y + yOffset + 30};

  Rectangle(hdc, btnCaballeroMenos.left, btnCaballeroMenos.top,
            btnCaballeroMenos.right, btnCaballeroMenos.bottom);
  Rectangle(hdc, btnCaballeroMas.left, btnCaballeroMas.top,
            btnCaballeroMas.right, btnCaballeroMas.bottom);

  DrawTextA(hdc, "-", -1, &btnCaballeroMenos,
            DT_CENTER | DT_VCENTER | DT_SINGLELINE);
  DrawTextA(hdc, "+", -1, &btnCaballeroMas,
            DT_CENTER | DT_VCENTER | DT_SINGLELINE);

  yOffset += lineHeight;

  // Guerreros
  char bufferGuerreros[50];
  snprintf(bufferGuerreros, sizeof(bufferGuerreros), "Guerreros: %d / %d",
           menu->guerrerosSeleccionados, guerrerosDisponibles);
  RECT rectGuerreros = {menu->x + 20, menu->y + yOffset, menu->x + 200,
                        menu->y + yOffset + 20};
  DrawTextA(hdc, bufferGuerreros, -1, &rectGuerreros,
            DT_LEFT | DT_VCENTER | DT_SINGLELINE);

  // Botones +/- para guerreros
  RECT btnGuerreroMenos = {menu->x + 260, menu->y + yOffset, menu->x + 300,
                           menu->y + yOffset + 30};
  RECT btnGuerreroMas = {menu->x + 310, menu->y + yOffset, menu->x + 350,
                         menu->y + yOffset + 30};

  Rectangle(hdc, btnGuerreroMenos.left, btnGuerreroMenos.top,
            btnGuerreroMenos.right, btnGuerreroMenos.bottom);
  Rectangle(hdc, btnGuerreroMas.left, btnGuerreroMas.top, btnGuerreroMas.right,
            btnGuerreroMas.bottom);

  DrawTextA(hdc, "-", -1, &btnGuerreroMenos,
            DT_CENTER | DT_VCENTER | DT_SINGLELINE);
  DrawTextA(hdc, "+", -1, &btnGuerreroMas,
            DT_CENTER | DT_VCENTER | DT_SINGLELINE);

  // Botones de acción
  yOffset = menu->alto - 60;

  RECT btnEmbarcar = {menu->x + 50, menu->y + yOffset, menu->x + 180,
                      menu->y + yOffset + 40};
  RECT btnCancelar = {menu->x + 220, menu->y + yOffset, menu->x + 350,
                      menu->y + yOffset + 40};

  // Botón Embarcar (habilitado solo si hay tropas seleccionadas)
  if (menu->totalSeleccionados > 0) {
    HBRUSH hBrushEmbarcar = CreateSolidBrush(RGB(0, 120, 0));
    HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hBrushEmbarcar);
    Rectangle(hdc, btnEmbarcar.left, btnEmbarcar.top, btnEmbarcar.right,
              btnEmbarcar.bottom);
    SelectObject(hdc, hOldBrush);
    DeleteObject(hBrushEmbarcar);
  } else {
    Rectangle(hdc, btnEmbarcar.left, btnEmbarcar.top, btnEmbarcar.right,
              btnEmbarcar.bottom);
  }

  SetTextColor(hdc, RGB(255, 255, 255));
  DrawTextA(hdc, "EMBARCAR", -1, &btnEmbarcar,
            DT_CENTER | DT_VCENTER | DT_SINGLELINE);

  // Botón Cancelar
  HBRUSH hBrushCancelar = CreateSolidBrush(RGB(120, 0, 0));
  HBRUSH hOldBrush2 = (HBRUSH)SelectObject(hdc, hBrushCancelar);
  Rectangle(hdc, btnCancelar.left, btnCancelar.top, btnCancelar.right,
            btnCancelar.bottom);
  SelectObject(hdc, hOldBrush2);
  DeleteObject(hBrushCancelar);

  DrawTextA(hdc, "CANCELAR", -1, &btnCancelar,
            DT_CENTER | DT_VCENTER | DT_SINGLELINE);
}

bool menuEmbarqueClick(MenuEmbarque *menu, struct Jugador *j, int x, int y) {
  if (!menu->activo)
    return false;

  // Verificar si el click está dentro del menú
  if (x < menu->x || x > menu->x + menu->ancho || y < menu->y ||
      y > menu->y + menu->alto) {
    return false;
  }

  if (menu->eligiendoIsla) {
    int opciones[3];
    int total = 0;
    for (int i = 1; i <= 3; i++) {
      if (i != j->islaActual) {
        opciones[total++] = i;
      }
    }

    int btnWidth = 260;
    int btnHeight = 50;
    int startY = menu->y + 100;
    int startX = menu->x + (menu->ancho - btnWidth) / 2;

    for (int i = 0; i < total; i++) {
      RECT btn = {startX, startY + i * (btnHeight + 20), startX + btnWidth,
                  startY + i * (btnHeight + 20) + btnHeight};
      if (x >= btn.left && x <= btn.right && y >= btn.top && y <= btn.bottom) {
        bool ok = viajarAIsla(j, opciones[i]);
        if (ok) {
          // Cerrar y limpiar selección si el viaje se realizó o se desembarcó en misma isla
          menu->activo = false;
          menu->eligiendoIsla = false;
          menu->obrerosSeleccionados = 0;
          menu->caballerosSeleccionados = 0;
          menu->guerrerosSeleccionados = 0;
          menu->totalSeleccionados = 0;
        } else {
          // Mantener selección de isla abierta; el mensaje ya fue mostrado en viajarAIsla
          menu->activo = true;
          menu->eligiendoIsla = true;
        }
        return true;
      }
    }

    // Botón cancelar
    RECT btnCancelar = {menu->x + 50, menu->y + menu->alto - 60, menu->x + menu->ancho - 50, menu->y + menu->alto - 20};
    if (x >= btnCancelar.left && x <= btnCancelar.right && y >= btnCancelar.top && y <= btnCancelar.bottom) {
      // Si hay tropas en el barco (ocultas), devolverlas a tierra
      if (j->barco.numTropas > 0) {
        desembarcarTropas(&j->barco, j);
      }
      // Cerrar y limpiar selección
      menuEmbarqueCerrar(menu);
      menu->obrerosSeleccionados = 0;
      menu->caballerosSeleccionados = 0;
      menu->guerrerosSeleccionados = 0;
      menu->totalSeleccionados = 0;
      menu->eligiendoIsla = false;
      return true;
    }
    return true;
  }

  int yOffset = 90;
  int lineHeight = 60;

  // Contar tropas disponibles
  int obrerosDisponibles = 0;
  int caballerosDisponibles = 0;
  int guerrerosDisponibles = 0;

  for (int i = 0; i < 6; i++) {
    if (unidadListaParaEmbarcar(&j->obreros[i], &j->barco))
      obrerosDisponibles++;
  }
  for (int i = 0; i < 4; i++) {
    if (unidadListaParaEmbarcar(&j->caballeros[i], &j->barco))
      caballerosDisponibles++;
  }
  for (int i = 0; i < 4; i++) {
    if (unidadListaParaEmbarcar(&j->caballerosSinEscudo[i], &j->barco))
      caballerosDisponibles++;
  }
  for (int i = 0; i < 4; i++) {
    if (unidadListaParaEmbarcar(&j->guerreros[i], &j->barco))
      guerrerosDisponibles++;
  }

  // Botones de obreros
  if (x >= menu->x + 260 && x <= menu->x + 300 && y >= menu->y + yOffset &&
      y <= menu->y + yOffset + 30) {
    // Botón - obreros
    if (menu->obrerosSeleccionados > 0) {
      menu->obrerosSeleccionados--;
      menu->totalSeleccionados--;
    }
    return true;
  }

  if (x >= menu->x + 310 && x <= menu->x + 350 && y >= menu->y + yOffset &&
      y <= menu->y + yOffset + 30) {
    // Botón + obreros
    if (menu->obrerosSeleccionados < obrerosDisponibles &&
        menu->totalSeleccionados < j->barco.capacidadMaxima) {
      menu->obrerosSeleccionados++;
      menu->totalSeleccionados++;
    }
    return true;
  }

  yOffset += lineHeight;

  // Botones de caballeros
  if (x >= menu->x + 260 && x <= menu->x + 300 && y >= menu->y + yOffset &&
      y <= menu->y + yOffset + 30) {
    // Botón - caballeros
    if (menu->caballerosSeleccionados > 0) {
      menu->caballerosSeleccionados--;
      menu->totalSeleccionados--;
    }
    return true;
  }

  if (x >= menu->x + 310 && x <= menu->x + 350 && y >= menu->y + yOffset &&
      y <= menu->y + yOffset + 30) {
    // Botón + caballeros
    if (menu->caballerosSeleccionados < caballerosDisponibles &&
        menu->totalSeleccionados < j->barco.capacidadMaxima) {
      menu->caballerosSeleccionados++;
      menu->totalSeleccionados++;
    }
    return true;
  }

  yOffset += lineHeight;

  // Botones de guerreros
  if (x >= menu->x + 260 && x <= menu->x + 300 && y >= menu->y + yOffset &&
      y <= menu->y + yOffset + 30) {
    // Botón - guerreros
    if (menu->guerrerosSeleccionados > 0) {
      menu->guerrerosSeleccionados--;
      menu->totalSeleccionados--;
    }
    return true;
  }

  if (x >= menu->x + 310 && x <= menu->x + 350 && y >= menu->y + yOffset &&
      y <= menu->y + yOffset + 30) {
    // Botón + guerreros
    if (menu->guerrerosSeleccionados < guerrerosDisponibles &&
        menu->totalSeleccionados < j->barco.capacidadMaxima) {
      menu->guerrerosSeleccionados++;
      menu->totalSeleccionados++;
    }
    return true;
  }

  // Botones de acción
  yOffset = menu->alto - 60;

  // Botón EMBARCAR
  if (x >= menu->x + 50 && x <= menu->x + 180 && y >= menu->y + yOffset &&
      y <= menu->y + yOffset + 40) {
    if (menu->totalSeleccionados > 0) {
      menuEmbarqueEmbarcar(menu, j);
    }
    return true;
  }

  // Botón CANCELAR
  if (x >= menu->x + 220 && x <= menu->x + 350 && 
      y >= menu->y + yOffset && y <= menu->y + yOffset + 40) {
    // Si el usuario cancela antes de elegir isla, devolver tropas al mapa si ya se embarcaron
    if (j->barco.numTropas > 0) {
      desembarcarTropas(&j->barco, j);
    }
    menuEmbarqueCerrar(menu);
    menu->obrerosSeleccionados = 0;
    menu->caballerosSeleccionados = 0;
    menu->guerrerosSeleccionados = 0;
    menu->totalSeleccionados = 0;
    return true;
  }

  return true; // Click dentro del menú, consumir evento
}

void menuEmbarqueEmbarcar(MenuEmbarque *menu, struct Jugador *j) {
  // Embarcar obreros
  int obrerosEmbarcados = 0;
  for (int i = 0; i < 6 && obrerosEmbarcados < menu->obrerosSeleccionados;
       i++) {
    if (unidadListaParaEmbarcar(&j->obreros[i], &j->barco) &&
        j->barco.numTropas < j->barco.capacidadMaxima) {
      // Guardar puntero en el barco
      j->barco.tropas[j->barco.numTropas++] = &j->obreros[i];

      // Ocultar obrero del mapa (mover fuera del área visible)
      j->obreros[i].x = -1000;
      j->obreros[i].y = -1000;

      obrerosEmbarcados++;
    }
  }

  // Embarcar caballeros (con y sin escudo)
  int caballerosEmbarcados = 0;
  for (int i = 0; i < 4 && caballerosEmbarcados < menu->caballerosSeleccionados;
       i++) {
    if (unidadListaParaEmbarcar(&j->caballeros[i], &j->barco) &&
        j->barco.numTropas < j->barco.capacidadMaxima) {
      j->barco.tropas[j->barco.numTropas++] = &j->caballeros[i];
      j->caballeros[i].x = -1000;
      j->caballeros[i].y = -1000;
      caballerosEmbarcados++;
    }
  }
  for (int i = 0; i < 4 && caballerosEmbarcados < menu->caballerosSeleccionados;
       i++) {
    if (unidadListaParaEmbarcar(&j->caballerosSinEscudo[i], &j->barco) &&
        j->barco.numTropas < j->barco.capacidadMaxima) {
      j->barco.tropas[j->barco.numTropas++] = &j->caballerosSinEscudo[i];
      j->caballerosSinEscudo[i].x = -1000;
      j->caballerosSinEscudo[i].y = -1000;
      caballerosEmbarcados++;
    }
  }

  // Embarcar guerreros
  int guerrerosEmbarcados = 0;
  for (int i = 0; i < 4 && guerrerosEmbarcados < menu->guerrerosSeleccionados;
       i++) {
    if (unidadListaParaEmbarcar(&j->guerreros[i], &j->barco) &&
        j->barco.numTropas < j->barco.capacidadMaxima) {
      j->barco.tropas[j->barco.numTropas++] = &j->guerreros[i];
      j->guerreros[i].x = -1000;
      j->guerreros[i].y = -1000;
      guerrerosEmbarcados++;
    }
  }

  printf("[DEBUG] Tropas embarcadas: %d obreros, %d caballeros, %d guerreros "
         "(Total: %d)\n",
         obrerosEmbarcados, caballerosEmbarcados, guerrerosEmbarcados,
         j->barco.numTropas);

  // Abrir selección de isla dentro del juego
  menu->eligiendoIsla = true;
}
