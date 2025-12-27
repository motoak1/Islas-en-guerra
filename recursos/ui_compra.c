// recursos/ui_compra.c
#include "ui_compra.h"
#include <stdio.h>
#include <string.h>

void menuCompraInicializar(MenuCompra *menu) {
  menu->abierto = false;
  menu->ancho = 400;
  menu->alto = 300;
  menu->mensajeError[0] = '\0';
  menu->tiempoError = 0;
}

void menuCompraAbrir(MenuCompra *menu, int anchoPantalla, int altoPantalla) {
  menu->abierto = true;
  menu->pantallaX = (anchoPantalla - menu->ancho) / 2;
  menu->pantallaY = (altoPantalla - menu->alto) / 2;

  // Configurar botones
  menu->botonComprar.left = menu->pantallaX + 50;
  menu->botonComprar.top = menu->pantallaY + 200;
  menu->botonComprar.right = menu->botonComprar.left + 140;
  menu->botonComprar.bottom = menu->botonComprar.top + 40;

  menu->botonCerrar.left = menu->pantallaX + 210;
  menu->botonCerrar.top = menu->pantallaY + 200;
  menu->botonCerrar.right = menu->botonCerrar.left + 140;
  menu->botonCerrar.bottom = menu->botonCerrar.top + 40;

  menu->mensajeError[0] = '\0';
  menu->tiempoError = 0;
}

void menuCompraCerrar(MenuCompra *menu) {
  menu->abierto = false;
  menu->mensajeError[0] = '\0';
  menu->tiempoError = 0;
}

void menuCompraDibujar(HDC hdc, MenuCompra *menu, struct Jugador *jugador) {
  if (!menu->abierto)
    return;

  // Fondo del menú (semi-transparente simulado con gris)
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
  DrawText(hdc, "AYUNTAMIENTO", -1, &rectTitulo,
           DT_CENTER | DT_VCENTER | DT_SINGLELINE);

  SelectObject(hdc, oldFont);
  DeleteObject(fontTitulo);

  // Fuente normal para el resto
  HFONT fontNormal =
      CreateFont(18, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                 OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                 DEFAULT_PITCH | FF_SWISS, "Arial");
  SelectObject(hdc, fontNormal);

  // Mostrar recursos actuales del jugador
  SetTextColor(hdc, RGB(200, 200, 200));
  char textoRecursos[200];
  sprintf(textoRecursos, "Tus recursos:");
  TextOut(hdc, menu->pantallaX + 30, menu->pantallaY + 70, textoRecursos,
          strlen(textoRecursos));

  sprintf(textoRecursos, "Oro: %d  |  Piedra: %d  |  Comida: %d", jugador->Oro,
          jugador->Piedra, jugador->Comida);
  TextOut(hdc, menu->pantallaX + 30, menu->pantallaY + 95, textoRecursos,
          strlen(textoRecursos));

  // Separador
  HPEN penLinea = CreatePen(PS_SOLID, 1, RGB(100, 100, 100));
  SelectObject(hdc, penLinea);
  MoveToEx(hdc, menu->pantallaX + 20, menu->pantallaY + 125, NULL);
  LineTo(hdc, menu->pantallaX + menu->ancho - 20, menu->pantallaY + 125);
  DeleteObject(penLinea);

  // Información de compra de cultivos
  SetTextColor(hdc, RGB(255, 255, 100));
  char textoCultivo[150];
  sprintf(textoCultivo, "Comprar Cultivos");
  TextOut(hdc, menu->pantallaX + 30, menu->pantallaY + 140, textoCultivo,
          strlen(textoCultivo));

  SetTextColor(hdc, RGB(200, 200, 200));
  sprintf(textoCultivo, "Costo: %d Oro + %d Piedra", COSTO_CULTIVO_ORO,
          COSTO_CULTIVO_PIEDRA);
  TextOut(hdc, menu->pantallaX + 30, menu->pantallaY + 165, textoCultivo,
          strlen(textoCultivo));

  SetTextColor(hdc, RGB(100, 255, 100));
  sprintf(textoCultivo, "Ganancia: +%d Comida", GANANCIA_CULTIVO_COMIDA);
  TextOut(hdc, menu->pantallaX + 230, menu->pantallaY + 165, textoCultivo,
          strlen(textoCultivo));

  SelectObject(hdc, oldFont);
  DeleteObject(fontNormal);

  // Dibujar botón COMPRAR
  HBRUSH brushBotonComprar = CreateSolidBrush(RGB(60, 120, 60));
  SelectObject(hdc, brushBotonComprar);
  HPEN penBoton = CreatePen(PS_SOLID, 2, RGB(100, 200, 100));
  SelectObject(hdc, penBoton);
  RoundRect(hdc, menu->botonComprar.left, menu->botonComprar.top,
            menu->botonComprar.right, menu->botonComprar.bottom, 10, 10);

  SetTextColor(hdc, RGB(255, 255, 255));
  HFONT fontBoton =
      CreateFont(20, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                 OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                 DEFAULT_PITCH | FF_SWISS, "Arial");
  SelectObject(hdc, fontBoton);
  DrawText(hdc, "COMPRAR", -1, &menu->botonComprar,
           DT_CENTER | DT_VCENTER | DT_SINGLELINE);

  DeleteObject(penBoton);
  DeleteObject(brushBotonComprar);

  // Dibujar botón CERRAR
  HBRUSH brushBotonCerrar = CreateSolidBrush(RGB(120, 60, 60));
  SelectObject(hdc, brushBotonCerrar);
  HPEN penBotonCerrar = CreatePen(PS_SOLID, 2, RGB(200, 100, 100));
  SelectObject(hdc, penBotonCerrar);
  RoundRect(hdc, menu->botonCerrar.left, menu->botonCerrar.top,
            menu->botonCerrar.right, menu->botonCerrar.bottom, 10, 10);

  DrawText(hdc, "CERRAR", -1, &menu->botonCerrar,
           DT_CENTER | DT_VCENTER | DT_SINGLELINE);

  DeleteObject(penBotonCerrar);
  DeleteObject(brushBotonCerrar);
  DeleteObject(fontBoton);

  // Mostrar mensaje de error si existe
  if (menu->tiempoError > 0) {
    SetTextColor(hdc, RGB(255, 100, 100));
    HFONT fontError =
        CreateFont(16, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                   OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                   DEFAULT_PITCH | FF_SWISS, "Arial");
    SelectObject(hdc, fontError);

    RECT rectError = {menu->pantallaX + 20, menu->pantallaY + 250,
                      menu->pantallaX + menu->ancho - 20,
                      menu->pantallaY + 280};
    DrawText(hdc, menu->mensajeError, -1, &rectError,
             DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    DeleteObject(fontError);
  }

  SelectObject(hdc, oldBrush);
  DeleteObject(brushFondo);
}

bool menuCompraClick(MenuCompra *menu, struct Jugador *jugador, int pantallaX,
                     int pantallaY) {
  if (!menu->abierto)
    return false;

  // Verificar si el click está dentro del menú
  if (pantallaX < menu->pantallaX ||
      pantallaX > menu->pantallaX + menu->ancho ||
      pantallaY < menu->pantallaY || pantallaY > menu->pantallaY + menu->alto) {
    return false; // Click fuera del menú
  }

  // Verificar click en botón COMPRAR
  if (pantallaX >= menu->botonComprar.left &&
      pantallaX <= menu->botonComprar.right &&
      pantallaY >= menu->botonComprar.top &&
      pantallaY <= menu->botonComprar.bottom) {

    // Verificar si el jugador tiene suficientes recursos
    if (jugador->Oro >= COSTO_CULTIVO_ORO &&
        jugador->Piedra >= COSTO_CULTIVO_PIEDRA) {
      // Realizar compra
      jugador->Oro -= COSTO_CULTIVO_ORO;
      jugador->Piedra -= COSTO_CULTIVO_PIEDRA;
      jugador->Comida += GANANCIA_CULTIVO_COMIDA;

      // Cerrar menú automáticamente después de compra exitosa
      menuCompraCerrar(menu);
    } else {
      // Mostrar error
      strcpy(menu->mensajeError, "¡Recursos insuficientes!");
      menu->tiempoError = 120; // Mostrar por ~2 segundos (120 frames a 60fps)
    }
    return true;
  }

  // Verificar click en botón CERRAR
  if (pantallaX >= menu->botonCerrar.left &&
      pantallaX <= menu->botonCerrar.right &&
      pantallaY >= menu->botonCerrar.top &&
      pantallaY <= menu->botonCerrar.bottom) {
    menuCompraCerrar(menu);
    return true;
  }

  return true; // Click dentro del menú pero no en botones
}

void menuCompraActualizar(MenuCompra *menu) {
  if (menu->tiempoError > 0) {
    menu->tiempoError--;
  }
}
