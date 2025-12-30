#include "ui_embarque.h"
#include "recursos.h"
#include "navegacion.h"
#include "../mapa/menu.h"
#include <stdio.h>
#include <windows.h>

void menuEmbarqueInicializar(MenuEmbarque* menu) {
  menu->activo = false;
  menu->obrerosSeleccionados = 0;
  menu->caballerosSeleccionados = 0;
  menu->guerrerosSeleccionados = 0;
  menu->totalSeleccionados = 0;
  menu->x = 0;
  menu->y = 0;
  menu->ancho = 400;
  menu->alto = 350;
}

void menuEmbarqueAbrir(MenuEmbarque* menu, int anchoVentana, int altoVentana) {
  menu->activo = true;
  menu->obrerosSeleccionados = 0;
  menu->caballerosSeleccionados = 0;
  menu->guerrerosSeleccionados = 0;
  menu->totalSeleccionados = 0;
  
  // Centrar el menú en la ventana
  menu->x = (anchoVentana - menu->ancho) / 2;
  menu->y = (altoVentana - menu->alto) / 2;
}

void menuEmbarqueCerrar(MenuEmbarque* menu) {
  menu->activo = false;
}

void menuEmbarqueDibujar(HDC hdc, MenuEmbarque* menu, struct Jugador* j) {
  if (!menu->activo) return;
  
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
  RECT rectTitulo = {menu->x, menu->y + 10, menu->x + menu->ancho, menu->y + 40};
  DrawTextA(hdc, "EMBARCAR TROPAS", -1, &rectTitulo, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
  
  // Contador de tropas
  char bufferContador[50];
  snprintf(bufferContador, sizeof(bufferContador), "Tropas seleccionadas: %d / 6", menu->totalSeleccionados);
  RECT rectContador = {menu->x, menu->y + 45, menu->x + menu->ancho, menu->y + 70};
  SetTextColor(hdc, (menu->totalSeleccionados == 6) ? RGB(255, 200, 0) : RGB(200, 200, 200));
  DrawTextA(hdc, bufferContador, -1, &rectContador, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
  
  int yOffset = 90;
  int lineHeight = 60;
  
  // Contar tropas disponibles
  int obrerosDisponibles = 0;
  int caballerosDisponibles = 0;
  int guerrerosDisponibles = 0;
  
  for (int i = 0; i < 6; i++) {
    if (j->obreros[i].x >= 0 && j->obreros[i].y >= 0) obrerosDisponibles++;
  }
  for (int i = 0; i < 4; i++) {
    if (j->caballeros[i].x >= 0 && j->caballeros[i].y >= 0) caballerosDisponibles++;
  }
  for (int i = 0; i < 4; i++) {
    if (j->guerreros[i].x >= 0 && j->guerreros[i].y >= 0) guerrerosDisponibles++;
  }
  
  // Obreros
  SetTextColor(hdc, RGB(255, 255, 255));
  char bufferObreros[50];
  snprintf(bufferObreros, sizeof(bufferObreros), "Obreros: %d / %d", 
           menu->obrerosSeleccionados, obrerosDisponibles);
  RECT rectObreros = {menu->x + 20, menu->y + yOffset, menu->x + 200, menu->y + yOffset + 20};
  DrawTextA(hdc, bufferObreros, -1, &rectObreros, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
  
  // Botones +/- para obreros
  RECT btnObreroMenos = {menu->x + 260, menu->y + yOffset, menu->x + 300, menu->y + yOffset + 30};
  RECT btnObreroMas = {menu->x + 310, menu->y + yOffset, menu->x + 350, menu->y + yOffset + 30};
  
  Rectangle(hdc, btnObreroMenos.left, btnObreroMenos.top, btnObreroMenos.right, btnObreroMenos.bottom);
  Rectangle(hdc, btnObreroMas.left, btnObreroMas.top, btnObreroMas.right, btnObreroMas.bottom);
  
  DrawTextA(hdc, "-", -1, &btnObreroMenos, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
  DrawTextA(hdc, "+", -1, &btnObreroMas, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
  
  yOffset += lineHeight;
  
  // Caballeros
  char bufferCaballeros[50];
  snprintf(bufferCaballeros, sizeof(bufferCaballeros), "Caballeros: %d / %d", 
           menu->caballerosSeleccionados, caballerosDisponibles);
  RECT rectCaballeros = {menu->x + 20, menu->y + yOffset, menu->x + 200, menu->y + yOffset + 20};
  DrawTextA(hdc, bufferCaballeros, -1, &rectCaballeros, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
  
  // Botones +/- para caballeros
  RECT btnCaballeroMenos = {menu->x + 260, menu->y + yOffset, menu->x + 300, menu->y + yOffset + 30};
  RECT btnCaballeroMas = {menu->x + 310, menu->y + yOffset, menu->x + 350, menu->y + yOffset + 30};
  
  Rectangle(hdc, btnCaballeroMenos.left, btnCaballeroMenos.top, btnCaballeroMenos.right, btnCaballeroMenos.bottom);
  Rectangle(hdc, btnCaballeroMas.left, btnCaballeroMas.top, btnCaballeroMas.right, btnCaballeroMas.bottom);
  
  DrawTextA(hdc, "-", -1, &btnCaballeroMenos, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
  DrawTextA(hdc, "+", -1, &btnCaballeroMas, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
  
  yOffset += lineHeight;
  
  // Guerreros
  char bufferGuerreros[50];
  snprintf(bufferGuerreros, sizeof(bufferGuerreros), "Guerreros: %d / %d", 
           menu->guerrerosSeleccionados, guerrerosDisponibles);
  RECT rectGuerreros = {menu->x + 20, menu->y + yOffset, menu->x + 200, menu->y + yOffset + 20};
  DrawTextA(hdc, bufferGuerreros, -1, &rectGuerreros, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
  
  // Botones +/- para guerreros
  RECT btnGuerreroMenos = {menu->x + 260, menu->y + yOffset, menu->x + 300, menu->y + yOffset + 30};
  RECT btnGuerreroMas = {menu->x + 310, menu->y + yOffset, menu->x + 350, menu->y + yOffset + 30};
  
  Rectangle(hdc, btnGuerreroMenos.left, btnGuerreroMenos.top, btnGuerreroMenos.right, btnGuerreroMenos.bottom);
  Rectangle(hdc, btnGuerreroMas.left, btnGuerreroMas.top, btnGuerreroMas.right, btnGuerreroMas.bottom);
  
  DrawTextA(hdc, "-", -1, &btnGuerreroMenos, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
  DrawTextA(hdc, "+", -1, &btnGuerreroMas, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
  
  // Botones de acción
  yOffset = menu->alto - 60;
  
  RECT btnEmbarcar = {menu->x + 50, menu->y + yOffset, menu->x + 180, menu->y + yOffset + 40};
  RECT btnCancelar = {menu->x + 220, menu->y + yOffset, menu->x + 350, menu->y + yOffset + 40};
  
  // Botón Embarcar (habilitado solo si hay tropas seleccionadas)
  if (menu->totalSeleccionados > 0) {
    HBRUSH hBrushEmbarcar = CreateSolidBrush(RGB(0, 120, 0));
    HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hBrushEmbarcar);
    Rectangle(hdc, btnEmbarcar.left, btnEmbarcar.top, btnEmbarcar.right, btnEmbarcar.bottom);
    SelectObject(hdc, hOldBrush);
    DeleteObject(hBrushEmbarcar);
  } else {
    Rectangle(hdc, btnEmbarcar.left, btnEmbarcar.top, btnEmbarcar.right, btnEmbarcar.bottom);
  }
  
  SetTextColor(hdc, RGB(255, 255, 255));
  DrawTextA(hdc, "EMBARCAR", -1, &btnEmbarcar, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
  
  // Botón Cancelar
  HBRUSH hBrushCancelar = CreateSolidBrush(RGB(120, 0, 0));
  HBRUSH hOldBrush2 = (HBRUSH)SelectObject(hdc, hBrushCancelar);
  Rectangle(hdc, btnCancelar.left, btnCancelar.top, btnCancelar.right, btnCancelar.bottom);
  SelectObject(hdc, hOldBrush2);
  DeleteObject(hBrushCancelar);
  
  DrawTextA(hdc, "CANCELAR", -1, &btnCancelar, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
}

bool menuEmbarqueClick(MenuEmbarque* menu, struct Jugador* j, int x, int y) {
  if (!menu->activo) return false;
  
  // Verificar si el click está dentro del menú
  if (x < menu->x || x > menu->x + menu->ancho || 
      y < menu->y || y > menu->y + menu->alto) {
    return false;
  }
  
  int yOffset = 90;
  int lineHeight = 60;
  
  // Contar tropas disponibles
  int obrerosDisponibles = 0;
  int caballerosDisponibles = 0;
  int guerrerosDisponibles = 0;
  
  for (int i = 0; i < 6; i++) {
    if (j->obreros[i].x >= 0 && j->obreros[i].y >= 0) obrerosDisponibles++;
  }
  for (int i = 0; i < 4; i++) {
    if (j->caballeros[i].x >= 0 && j->caballeros[i].y >= 0) caballerosDisponibles++;
  }
  for (int i = 0; i < 4; i++) {
    if (j->guerreros[i].x >= 0 && j->guerreros[i].y >= 0) guerrerosDisponibles++;
  }
  
  // Botones de obreros
  if (x >= menu->x + 260 && x <= menu->x + 300 && 
      y >= menu->y + yOffset && y <= menu->y + yOffset + 30) {
    // Botón - obreros
    if (menu->obrerosSeleccionados > 0) {
      menu->obrerosSeleccionados--;
      menu->totalSeleccionados--;
    }
    return true;
  }
  
  if (x >= menu->x + 310 && x <= menu->x + 350 && 
      y >= menu->y + yOffset && y <= menu->y + yOffset + 30) {
    // Botón + obreros
    if (menu->obrerosSeleccionados < obrerosDisponibles && menu->totalSeleccionados < 6) {
      menu->obrerosSeleccionados++;
      menu->totalSeleccionados++;
    }
    return true;
  }
  
  yOffset += lineHeight;
  
  // Botones de caballeros
  if (x >= menu->x + 260 && x <= menu->x + 300 && 
      y >= menu->y + yOffset && y <= menu->y + yOffset + 30) {
    // Botón - caballeros
    if (menu->caballerosSeleccionados > 0) {
      menu->caballerosSeleccionados--;
      menu->totalSeleccionados--;
    }
    return true;
  }
  
  if (x >= menu->x + 310 && x <= menu->x + 350 && 
      y >= menu->y + yOffset && y <= menu->y + yOffset + 30) {
    // Botón + caballeros
    if (menu->caballerosSeleccionados < caballerosDisponibles && menu->totalSeleccionados < 6) {
      menu->caballerosSeleccionados++;
      menu->totalSeleccionados++;
    }
    return true;
  }
  
  yOffset += lineHeight;
  
  // Botones de guerreros
  if (x >= menu->x + 260 && x <= menu->x + 300 && 
      y >= menu->y + yOffset && y <= menu->y + yOffset + 30) {
    // Botón - guerreros
    if (menu->guerrerosSeleccionados > 0) {
      menu->guerrerosSeleccionados--;
      menu->totalSeleccionados--;
    }
    return true;
  }
  
  if (x >= menu->x + 310 && x <= menu->x + 350 && 
      y >= menu->y + yOffset && y <= menu->y + yOffset + 30) {
    // Botón + guerreros
    if (menu->guerrerosSeleccionados < guerrerosDisponibles && menu->totalSeleccionados < 6) {
      menu->guerrerosSeleccionados++;
      menu->totalSeleccionados++;
    }
    return true;
  }
  
  // Botones de acción
  yOffset = menu->alto - 60;
  
  // Botón EMBARCAR
  if (x >= menu->x + 50 && x <= menu->x + 180 && 
      y >= menu->y + yOffset && y <= menu->y + yOffset + 40) {
    if (menu->totalSeleccionados > 0) {
      menuEmbarqueEmbarcar(menu, j);
    }
    return true;
  }
  
  // Botón CANCELAR
  if (x >= menu->x + 220 && x <= menu->x + 350 && 
      y >= menu->y + yOffset && y <= menu->y + yOffset + 40) {
    menuEmbarqueCerrar(menu);
    return true;
  }
  
  return true; // Click dentro del menú, consumir evento
}

void menuEmbarqueEmbarcar(MenuEmbarque* menu, struct Jugador* j) {
  // Embarcar obreros
  int obrerosEmbarcados = 0;
  for (int i = 0; i < 6 && obrerosEmbarcados < menu->obrerosSeleccionados; i++) {
    if (j->obreros[i].x >= 0 && j->obreros[i].y >= 0) {
      // Guardar puntero en el barco
      j->barco.tropas[j->barco.numTropas++] = &j->obreros[i];
      
      // Ocultar obrero del mapa (mover fuera del área visible)
      j->obreros[i].x = -1000;
      j->obreros[i].y = -1000;
      
      obrerosEmbarcados++;
    }
  }
  
  // Embarcar caballeros
  int caballerosEmbarcados = 0;
  for (int i = 0; i < 4 && caballerosEmbarcados < menu->caballerosSeleccionados; i++) {
    if (j->caballeros[i].x >= 0 && j->caballeros[i].y >= 0) {
      j->barco.tropas[j->barco.numTropas++] = &j->caballeros[i];
      j->caballeros[i].x = -1000;
      j->caballeros[i].y = -1000;
      caballerosEmbarcados++;
    }
  }
  
  // Embarcar guerreros
  int guerrerosEmbarcados = 0;
  for (int i = 0; i < 4 && guerrerosEmbarcados < menu->guerrerosSeleccionados; i++) {
    if (j->guerreros[i].x >= 0 && j->guerreros[i].y >= 0) {
      j->barco.tropas[j->barco.numTropas++] = &j->guerreros[i];
      j->guerreros[i].x = -1000;
      j->guerreros[i].y = -1000;
      guerrerosEmbarcados++;
    }
  }
  
  printf("[DEBUG] Tropas embarcadas: %d obreros, %d caballeros, %d guerreros (Total: %d)\n",
         obrerosEmbarcados, caballerosEmbarcados, guerrerosEmbarcados, j->barco.numTropas);
  
  menuEmbarqueCerrar(menu);
  
  // NUEVO FLUJO: Mostrar menú de selección de isla
  printf("[DEBUG] Mostrando menu de seleccion de isla...\n");
  
  // Llamar al menú de isla (consola)
  mostrarMenu();
  int islaSeleccionada = menuObtenerIsla();
  
  printf("[DEBUG] Isla seleccionada: %d\n", islaSeleccionada);
  
  // Viajar directamente a la isla (sin animación)
  viajarAIsla(j, islaSeleccionada);
}
