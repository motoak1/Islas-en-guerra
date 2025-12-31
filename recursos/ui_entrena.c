// recursos/ui_entrena.c
#include "ui_entrena.h"
#include <stdio.h>
#include <string.h>

void menuEntrenamientoInicializar(MenuEntrenamiento *menu) {
  menu->abierto = false;
  menu->ancho = 500;
  menu->alto = 450;
  menu->mensajeError[0] = '\0';
  menu->tiempoError = 0;
}

void menuEntrenamientoAbrir(MenuEntrenamiento *menu, int anchoPantalla,
                            int altoPantalla) {
  menu->abierto = true;
  menu->pantallaX = (anchoPantalla - menu->ancho) / 2;
  menu->pantallaY = (altoPantalla - menu->alto) / 2;

  int xCentro = menu->pantallaX + (menu->ancho / 2);

  // Configurar botones (Centrados)
  // Botón Entrenar Obrero
  menu->botonObrero.left = xCentro - 180;
  menu->botonObrero.top = menu->pantallaY + 160;
  menu->botonObrero.right = xCentro + 180;
  menu->botonObrero.bottom = menu->botonObrero.top + 60;

  // Botón Entrenar Caballero
  menu->botonCaballero.left = xCentro - 180;
  menu->botonCaballero.top = menu->botonObrero.bottom + 30;
  menu->botonCaballero.right = xCentro + 180;
  menu->botonCaballero.bottom = menu->botonCaballero.top + 60;

  // Botón Cerrar
  menu->botonCerrar.left = xCentro - 70;
  menu->botonCerrar.top = menu->pantallaY + 380;
  menu->botonCerrar.right = xCentro + 70;
  menu->botonCerrar.bottom = menu->botonCerrar.top + 40;

  menu->mensajeError[0] = '\0';
  menu->tiempoError = 0;
}

void menuEntrenamientoCerrar(MenuEntrenamiento *menu) {
  menu->abierto = false;
  menu->mensajeError[0] = '\0';
  menu->tiempoError = 0;
}

// Helper para dibujar botones de opción
void dibujarBotonEntrenamiento(HDC hdc, RECT rect, const char *titulo,
                               const char *costo) {
  HBRUSH brush = CreateSolidBrush(RGB(80, 40, 40)); // Fondo rojo oscuro
  SelectObject(hdc, brush);
  HPEN pen = CreatePen(PS_SOLID, 2, RGB(200, 100, 100));
  SelectObject(hdc, pen);
  RoundRect(hdc, rect.left, rect.top, rect.right, rect.bottom, 8, 8);

  DeleteObject(brush);
  DeleteObject(pen);

  SetBkMode(hdc, TRANSPARENT);

  // Título (Izq)
  SetTextColor(hdc, RGB(255, 200, 100));
  HFONT fontTitulo =
      CreateFont(20, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                 OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                 DEFAULT_PITCH | FF_SWISS, "Arial");
  HFONT oldFont = (HFONT)SelectObject(hdc, fontTitulo);
  TextOut(hdc, rect.left + 15, rect.top + 10, titulo, strlen(titulo));
  SelectObject(hdc, oldFont);
  DeleteObject(fontTitulo);

  // Costo (abajo)
  SetTextColor(hdc, RGB(220, 220, 220));
  HFONT fontCosto =
      CreateFont(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                 OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                 DEFAULT_PITCH | FF_SWISS, "Arial");
  SelectObject(hdc, fontCosto);
  TextOut(hdc, rect.left + 15, rect.top + 35, costo, strlen(costo));
  SelectObject(hdc, oldFont);
  DeleteObject(fontCosto);
}

void menuEntrenamientoDibujar(HDC hdc, MenuEntrenamiento *menu,
                              struct Jugador *jugador) {
  if (!menu->abierto)
    return;

  // Fondo del menú
  HBRUSH brushFondo = CreateSolidBrush(RGB(50, 30, 30));
  HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, brushFondo);
  HPEN penBorde = CreatePen(PS_SOLID, 3, RGB(200, 50, 50));
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
  DrawText(hdc, "CUARTEL DE ENTRENAMIENTO", -1, &rectTitulo,
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
  sprintf(buffer, "Oro: %d  |  Comida: %d  |  Madera: %d", jugador->Oro,
          jugador->Comida, jugador->Madera);
  TextOut(hdc, menu->pantallaX + 40, menu->pantallaY + 70, buffer,
          strlen(buffer));

  // Separador
  MoveToEx(hdc, menu->pantallaX + 20, menu->pantallaY + 110, NULL);
  LineTo(hdc, menu->pantallaX + menu->ancho - 20, menu->pantallaY + 110);

  // DIBUJAR BOTONES
  char costo[100];

  // Botón Entrenar Obrero
  sprintf(costo, "Costo: %d Oro + %d Comida", COSTO_OBRERO_ORO,
          COSTO_OBRERO_COMIDA);
  dibujarBotonEntrenamiento(hdc, menu->botonObrero, "Entrenar Obrero", costo);

  // Botón Entrenar Caballero
  sprintf(costo, "Costo: %d Oro + %d Comida + %d Madera", COSTO_CABALLERO_ORO,
          COSTO_CABALLERO_COMIDA, COSTO_CABALLERO_MADERA);
  dibujarBotonEntrenamiento(hdc, menu->botonCaballero, "Entrenar Caballero",
                            costo);

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
  if (menu->tiempoError > 0) {
    SetTextColor(hdc, RGB(255, 100, 100));
    RECT rErr = {menu->pantallaX, menu->pantallaY + 420,
                 menu->pantallaX + menu->ancho, menu->pantallaY + 450};
    DrawText(hdc, menu->mensajeError, -1, &rErr,
             DT_CENTER | DT_VCENTER | DT_SINGLELINE);
  }

  SelectObject(hdc, oldBrush);
  DeleteObject(brushFondo);
}

bool menuEntrenamientoClick(MenuEntrenamiento *menu, struct Jugador *jugador,
                            int pantallaX, int pantallaY) {
  if (!menu->abierto)
    return false;

  // Fuera del menú?
  if (pantallaX < menu->pantallaX ||
      pantallaX > menu->pantallaX + menu->ancho ||
      pantallaY < menu->pantallaY || pantallaY > menu->pantallaY + menu->alto) {
    return false;
  }

  // --- ENTRENAR OBRERO ---
  if (pantallaX >= menu->botonObrero.left &&
      pantallaX <= menu->botonObrero.right &&
      pantallaY >= menu->botonObrero.top &&
      pantallaY <= menu->botonObrero.bottom) {
    // Aquí se llamará a la función de entrenamiento
    // Por ahora solo validamos recursos
    if (jugador->Oro >= COSTO_OBRERO_ORO &&
        jugador->Comida >= COSTO_OBRERO_COMIDA) {
      // La función entrenarObrero() se implementará en recursos.c
      extern bool entrenarObrero(struct Jugador * j, float x, float y);
      if (entrenarObrero(jugador, 0,
                         0)) { // x,y se calculan dentro de la función
        jugador->Oro -= COSTO_OBRERO_ORO;
        jugador->Comida -= COSTO_OBRERO_COMIDA;
      } else {
        strcpy(menu->mensajeError, "No hay espacio para mas obreros!");
        menu->tiempoError = 60;
      }
    } else {
      strcpy(menu->mensajeError, "Recursos insuficientes!");
      menu->tiempoError = 60;
    }
    return true;
  }

  // --- ENTRENAR CABALLERO ---
  if (pantallaX >= menu->botonCaballero.left &&
      pantallaX <= menu->botonCaballero.right &&
      pantallaY >= menu->botonCaballero.top &&
      pantallaY <= menu->botonCaballero.bottom) {
    if (jugador->Oro >= COSTO_CABALLERO_ORO &&
        jugador->Comida >= COSTO_CABALLERO_COMIDA &&
        jugador->Madera >= COSTO_CABALLERO_MADERA) {
      extern bool entrenarCaballero(struct Jugador * j, float x, float y);
      if (entrenarCaballero(jugador, 0, 0)) {
        jugador->Oro -= COSTO_CABALLERO_ORO;
        jugador->Comida -= COSTO_CABALLERO_COMIDA;
        jugador->Madera -= COSTO_CABALLERO_MADERA;
      } else {
        strcpy(menu->mensajeError, "No hay espacio para mas caballeros!");
        menu->tiempoError = 60;
      }
    } else {
      strcpy(menu->mensajeError, "Recursos insuficientes!");
      menu->tiempoError = 60;
    }
    return true;
  }

  // Cerrar
  if (pantallaX >= menu->botonCerrar.left &&
      pantallaX <= menu->botonCerrar.right &&
      pantallaY >= menu->botonCerrar.top &&
      pantallaY <= menu->botonCerrar.bottom) {
    menuEntrenamientoCerrar(menu);
    return true;
  }

  return true; // Click dentro del menú
}

void menuEntrenamientoActualizar(MenuEntrenamiento *menu) {
  if (menu->tiempoError > 0) {
    menu->tiempoError--;
  }
}
