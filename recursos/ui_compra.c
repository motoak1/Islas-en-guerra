// recursos/ui_compra.c
#include "ui_compra.h"
#include <stdio.h>
#include <string.h>

void menuCompraInicializar(MenuCompra *menu) {
  menu->abierto = false;
  menu->ancho = 500; // Aumentar ancho
  menu->alto = 560;  // Aumentar alto para mensaje error
  menu->mensajeError[0] = '\0';
  menu->tiempoError = 0;
}

void menuCompraAbrir(MenuCompra *menu, int anchoPantalla, int altoPantalla) {
  menu->abierto = true;
  menu->pantallaX = (anchoPantalla - menu->ancho) / 2;
  menu->pantallaY = (altoPantalla - menu->alto) / 2;

  int xCentro = menu->pantallaX + (menu->ancho / 2);

  // Configurar botones (Centrados)
  // Opción 1
  menu->botonComprar.left = xCentro - 180;
  menu->botonComprar.top = menu->pantallaY + 160;
  menu->botonComprar.right = xCentro + 180;
  menu->botonComprar.bottom = menu->botonComprar.top + 50;

  // Opción 2
  menu->botonOpcion2.left = xCentro - 180;
  menu->botonOpcion2.top = menu->botonComprar.bottom + 20;
  menu->botonOpcion2.right = xCentro + 180;
  menu->botonOpcion2.bottom = menu->botonOpcion2.top + 50;

  // Opción 3
  menu->botonOpcion3.left = xCentro - 180;
  menu->botonOpcion3.top = menu->botonOpcion2.bottom + 20;
  menu->botonOpcion3.right = xCentro + 180;
  menu->botonOpcion3.bottom = menu->botonOpcion3.top + 50;

  // Opción 4
  menu->botonOpcion4.left = xCentro - 180;
  menu->botonOpcion4.top = menu->botonOpcion3.bottom + 20;
  menu->botonOpcion4.right = xCentro + 180;
  menu->botonOpcion4.bottom = menu->botonOpcion4.top + 50;

  // Cerrar
  menu->botonCerrar.left = xCentro - 70;
  menu->botonCerrar.top = menu->pantallaY + 480;
  menu->botonCerrar.right = xCentro + 70;
  menu->botonCerrar.bottom = menu->botonCerrar.top + 40;

  menu->mensajeError[0] = '\0';
  menu->tiempoError = 0;
}

void menuCompraCerrar(MenuCompra *menu) {
  menu->abierto = false;
  menu->mensajeError[0] = '\0';
  menu->tiempoError = 0;
}

// Helper para dibujar botones de opción
void dibujarOpcion(HDC hdc, RECT rect, const char *titulo, const char *costo,
                   const char *ganancia) {
  HBRUSH brush = CreateSolidBrush(RGB(60, 60, 60)); // Fondo oscuro
  SelectObject(hdc, brush);
  HPEN pen = CreatePen(PS_SOLID, 1, RGB(150, 150, 150));
  SelectObject(hdc, pen);
  RoundRect(hdc, rect.left, rect.top, rect.right, rect.bottom, 5, 5);

  DeleteObject(brush);
  DeleteObject(pen);

  SetBkMode(hdc, TRANSPARENT);

  // Título (Izq)
  SetTextColor(hdc, RGB(255, 255, 100));
  TextOut(hdc, rect.left + 10, rect.top + 15, titulo, strlen(titulo));

  // Costo (Centro/Der)
  SetTextColor(hdc, RGB(200, 200, 200));
  TextOut(hdc, rect.left + 10, rect.top + 32, costo, strlen(costo));

  // Ganancia (Der extrema)
  SetTextColor(hdc, RGB(100, 255, 100));
  RECT rText = rect;
  rText.right -= 10;
  DrawText(hdc, ganancia, -1, &rText, DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
}

void menuCompraDibujar(HDC hdc, MenuCompra *menu, struct Jugador *jugador) {
  if (!menu->abierto)
    return;

  // Fondo del menú
  HBRUSH brushFondo = CreateSolidBrush(RGB(40, 40, 40));
  HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, brushFondo);
  HPEN penBorde = CreatePen(PS_SOLID, 3, RGB(200, 180, 100));
  HPEN oldPen = (HPEN)SelectObject(hdc, penBorde);

  Rectangle(hdc, menu->pantallaX, menu->pantallaY,
            menu->pantallaX + menu->ancho, menu->pantallaY + menu->alto);

  SelectObject(hdc, oldPen);
  DeleteObject(penBorde);

  // Título
  SetBkMode(hdc, TRANSPARENT);
  SetTextColor(hdc, RGB(255, 220, 100));
  HFONT fontTitulo =
      CreateFont(28, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                 OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                 DEFAULT_PITCH | FF_SWISS, "Arial");
  HFONT oldFont = (HFONT)SelectObject(hdc, fontTitulo);

  RECT rectTitulo = {menu->pantallaX + 20, menu->pantallaY + 20,
                     menu->pantallaX + menu->ancho - 20, menu->pantallaY + 60};
  DrawText(hdc, "MERCADO DE ALIMENTOS", -1, &rectTitulo,
           DT_CENTER | DT_VCENTER | DT_SINGLELINE);

  SelectObject(hdc, oldFont);
  DeleteObject(fontTitulo);

  // Fuente normal
  HFONT fontNormal =
      CreateFont(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                 OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                 DEFAULT_PITCH | FF_SWISS, "Arial");
  SelectObject(hdc, fontNormal);

  // Recursos
  char buffer[100];
  SetTextColor(hdc, RGB(255, 255, 255));
  sprintf(buffer, "Oro: %d  |  Piedra: %d  |  Madera: %d", jugador->Oro,
          jugador->Piedra, jugador->Madera);
  TextOut(hdc, menu->pantallaX + 40, menu->pantallaY + 70, buffer,
          strlen(buffer));

  SetTextColor(hdc, RGB(100, 255, 100));
  sprintf(buffer, "Comida Actual: %d", jugador->Comida);
  TextOut(hdc, menu->pantallaX + 40, menu->pantallaY + 90, buffer,
          strlen(buffer));

  // Separador
  MoveToEx(hdc, menu->pantallaX + 20, menu->pantallaY + 120, NULL);
  LineTo(hdc, menu->pantallaX + menu->ancho - 20, menu->pantallaY + 120);

  // DIBUJAR OPCIONES
  char costo[100], ganancia[100];

  // Opción 1
  sprintf(costo, "Costo: %d Oro + %d Piedra", COSTO_CULTIVO_ORO,
          COSTO_CULTIVO_PIEDRA);
  sprintf(ganancia, "+%d Comida", GANANCIA_CULTIVO_COMIDA);
  dibujarOpcion(hdc, menu->botonComprar, "Cultivos Basicos", costo, ganancia);

  // Opción 2
  sprintf(costo, "Costo: %d Oro", COSTO_OP2_ORO);
  sprintf(ganancia, "+%d Comida", GANANCIA_OP2_COMIDA);
  dibujarOpcion(hdc, menu->botonOpcion2, "Suministros Ext.", costo, ganancia);

  // Opción 3
  sprintf(costo, "Costo: %d Oro", COSTO_OP3_ORO);
  sprintf(ganancia, "+%d Comida", GANANCIA_OP3_COMIDA);
  dibujarOpcion(hdc, menu->botonOpcion3, "Contratar Cazadores", costo,
                ganancia);

  // Opción 4
  sprintf(costo, "Costo: %d Oro + %d Madera", COSTO_OP4_ORO, COSTO_OP4_MADERA);
  sprintf(ganancia, "+%d Comida", GANANCIA_OP4_COMIDA);
  dibujarOpcion(hdc, menu->botonOpcion4, "Pescadores", costo, ganancia);

  SelectObject(hdc, oldFont);
  DeleteObject(fontNormal);

  // Botón CERRAR
  HBRUSH brushCerrar = CreateSolidBrush(RGB(150, 50, 50));
  SelectObject(hdc, brushCerrar);
  RoundRect(hdc, menu->botonCerrar.left, menu->botonCerrar.top,
            menu->botonCerrar.right, menu->botonCerrar.bottom, 10, 10);
  SetTextColor(hdc, RGB(255, 255, 255));
  DrawText(hdc, "CERRAR", -1, &menu->botonCerrar,
           DT_CENTER | DT_VCENTER | DT_SINGLELINE);
  DeleteObject(brushCerrar);

  // Mensaje de Error
  // Mensaje de Error
  if (menu->tiempoError > 0) {
    SetTextColor(hdc, RGB(255, 50, 50));
    
    // Fuente negrita para error
    HFONT fontError =
        CreateFont(20, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                   OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                   DEFAULT_PITCH | FF_SWISS, "Arial");
    HFONT prevFont = (HFONT)SelectObject(hdc, fontError);

    RECT rErr = {menu->pantallaX, menu->pantallaY + 520,
                 menu->pantallaX + menu->ancho, menu->pantallaY + 560};
    DrawText(hdc, menu->mensajeError, -1, &rErr,
             DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    SelectObject(hdc, prevFont);
    DeleteObject(fontError);
  }

  SelectObject(hdc, oldBrush);
  DeleteObject(brushFondo);
}

bool menuCompraClick(MenuCompra *menu, struct Jugador *jugador, int pantallaX,
                     int pantallaY) {
  if (!menu->abierto)
    return false;

  // Fuera del menú?
  if (pantallaX < menu->pantallaX ||
      pantallaX > menu->pantallaX + menu->ancho ||
      pantallaY < menu->pantallaY || pantallaY > menu->pantallaY + menu->alto) {
    return false;
  }

  // --- OPCIÓN 1 ---
  if (pantallaX >= menu->botonComprar.left &&
      pantallaX <= menu->botonComprar.right &&
      pantallaY >= menu->botonComprar.top &&
      pantallaY <= menu->botonComprar.bottom) {
    if (jugador->Oro >= COSTO_CULTIVO_ORO &&
        jugador->Piedra >= COSTO_CULTIVO_PIEDRA) {
      jugador->Oro -= COSTO_CULTIVO_ORO;
      jugador->Piedra -= COSTO_CULTIVO_PIEDRA;
      jugador->Comida += GANANCIA_CULTIVO_COMIDA;
      // No cerramos el menú para permitir compras múltiples
    } else {
      strcpy(menu->mensajeError, "Falta Oro o Piedra!");
      menu->tiempoError = 60;
    }
    return true;
  }

  // --- OPCIÓN 2 ---
  if (pantallaX >= menu->botonOpcion2.left &&
      pantallaX <= menu->botonOpcion2.right &&
      pantallaY >= menu->botonOpcion2.top &&
      pantallaY <= menu->botonOpcion2.bottom) {
    if (jugador->Oro >= COSTO_OP2_ORO) {
      jugador->Oro -= COSTO_OP2_ORO;
      jugador->Comida += GANANCIA_OP2_COMIDA;
    } else {
      strcpy(menu->mensajeError, "Falta Oro!");
      menu->tiempoError = 60;
    }
    return true;
  }

  // --- OPCIÓN 3 ---
  if (pantallaX >= menu->botonOpcion3.left &&
      pantallaX <= menu->botonOpcion3.right &&
      pantallaY >= menu->botonOpcion3.top &&
      pantallaY <= menu->botonOpcion3.bottom) {
    if (jugador->Oro >= COSTO_OP3_ORO) {
      jugador->Oro -= COSTO_OP3_ORO;
      jugador->Comida += GANANCIA_OP3_COMIDA;
    } else {
      strcpy(menu->mensajeError, "Falta Oro!");
      menu->tiempoError = 60;
    }
    return true;
  }

  // --- OPCIÓN 4 ---
  if (pantallaX >= menu->botonOpcion4.left &&
      pantallaX <= menu->botonOpcion4.right &&
      pantallaY >= menu->botonOpcion4.top &&
      pantallaY <= menu->botonOpcion4.bottom) {
    if (jugador->Oro >= COSTO_OP4_ORO && jugador->Madera >= COSTO_OP4_MADERA) {
      jugador->Oro -= COSTO_OP4_ORO;
      jugador->Madera -= COSTO_OP4_MADERA;
      jugador->Comida += GANANCIA_OP4_COMIDA;
    } else {
      strcpy(menu->mensajeError, "Falta Oro o Madera!");
      menu->tiempoError = 60;
    }
    return true;
  }

  // Cerrar
  if (pantallaX >= menu->botonCerrar.left &&
      pantallaX <= menu->botonCerrar.right &&
      pantallaY >= menu->botonCerrar.top &&
      pantallaY <= menu->botonCerrar.bottom) {
    menuCompraCerrar(menu);
    return true;
  }

  return true; // Click dentro el menú
}

void menuCompraActualizar(MenuCompra *menu) {
  if (menu->tiempoError > 0) {
    menu->tiempoError--;
  }
}
