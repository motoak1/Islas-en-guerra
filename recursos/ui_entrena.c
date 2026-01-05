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
  menu->ancho = 540;
  menu->alto = 620; // Más alto para 4 botones y stats
  menu->pantallaX = (anchoPantalla - menu->ancho) / 2;
  menu->pantallaY = (altoPantalla - menu->alto) / 2;

  int xCentro = menu->pantallaX + (menu->ancho / 2);

  // Configurar botones
  int btnAncho = 460;
  int btnAlto = 90;

  // Botón Entrenar Obrero
  menu->botonObrero.left = xCentro - (btnAncho / 2);
  menu->botonObrero.top = menu->pantallaY + 110;
  menu->botonObrero.right = xCentro + (btnAncho / 2);
  menu->botonObrero.bottom = menu->botonObrero.top + btnAlto;

  // Botón Entrenar Caballero con Escudo
  menu->botonCaballero.left = xCentro - (btnAncho / 2);
  menu->botonCaballero.top = menu->botonObrero.bottom + 15;
  menu->botonCaballero.right = xCentro + (btnAncho / 2);
  menu->botonCaballero.bottom = menu->botonCaballero.top + btnAlto;

  // Botón Entrenar Caballero sin Escudo
  menu->botonCaballeroSinEscudo.left = xCentro - (btnAncho / 2);
  menu->botonCaballeroSinEscudo.top = menu->botonCaballero.bottom + 15;
  menu->botonCaballeroSinEscudo.right = xCentro + (btnAncho / 2);
  menu->botonCaballeroSinEscudo.bottom =
      menu->botonCaballeroSinEscudo.top + btnAlto;

  // Botón Entrenar Guerrero
  menu->botonGuerrero.left = xCentro - (btnAncho / 2);
  menu->botonGuerrero.top = menu->botonCaballeroSinEscudo.bottom + 15;
  menu->botonGuerrero.right = xCentro + (btnAncho / 2);
  menu->botonGuerrero.bottom = menu->botonGuerrero.top + btnAlto;

  // Botón Cerrar
  menu->botonCerrar.left = xCentro - 80;
  menu->botonCerrar.top = menu->pantallaY + menu->alto - 55;
  menu->botonCerrar.right = xCentro + 80;
  menu->botonCerrar.bottom = menu->botonCerrar.top + 45;

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
                               const char *costo, const char *stats) {
  HBRUSH brush = CreateSolidBrush(RGB(80, 40, 40)); // Fondo rojo oscuro
  SelectObject(hdc, brush);
  HPEN pen = CreatePen(PS_SOLID, 2, RGB(200, 100, 100));
  SelectObject(hdc, pen);
  RoundRect(hdc, rect.left, rect.top, rect.right, rect.bottom, 8, 8);

  DeleteObject(brush);
  DeleteObject(pen);

  SetBkMode(hdc, TRANSPARENT);

  // Título (Arriba)
  SetTextColor(hdc, RGB(255, 200, 100));
  HFONT fontTitulo =
      CreateFont(20, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                 OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                 DEFAULT_PITCH | FF_SWISS, "Arial");
  HFONT oldFont = (HFONT)SelectObject(hdc, fontTitulo);
  TextOut(hdc, rect.left + 15, rect.top + 8, titulo, strlen(titulo));
  SelectObject(hdc, oldFont);
  DeleteObject(fontTitulo);

  // Costo (segunda línea)
  SetTextColor(hdc, RGB(220, 220, 220));
  HFONT fontCosto =
      CreateFont(13, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                 OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                 DEFAULT_PITCH | FF_SWISS, "Arial");
  SelectObject(hdc, fontCosto);
  TextOut(hdc, rect.left + 15, rect.top + 32, costo, strlen(costo));

  // Stats (si están presentes)
  if (stats != NULL) {
    SetTextColor(hdc, RGB(150, 255, 150));
    TextOut(hdc, rect.left + 15, rect.top + 52, stats, strlen(stats));
  }

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
  char stats[150];

  // Botón Entrenar Obrero
  sprintf(costo, "Costo: %d Oro, %d Comida", COSTO_OBRERO_ORO,
          COSTO_OBRERO_COMIDA);
  dibujarBotonEntrenamiento(
      hdc, menu->botonObrero, "Entrenar Obrero", costo,
      "Unidad b\xE1sica para recolecci\xF3n de recursos.");

  // Botón Entrenar Caballero con Escudo
  sprintf(costo, "Costo: %d Oro, %d Comida, %d Madera", COSTO_CABALLERO_ORO,
          COSTO_CABALLERO_COMIDA, COSTO_CABALLERO_MADERA);
  sprintf(stats,
          "Vida:%d | Da\xF1o:%d | Cr\xEDtico:%d%% | Def:%d | Alcance:Cuerpo a "
          "cuerpo",
          CABALLERO_VIDA, CABALLERO_DANO, CABALLERO_CRITICO, CABALLERO_DEFENSA);
  dibujarBotonEntrenamiento(hdc, menu->botonCaballero,
                            "Entrenar Caballero con Escudo", costo, stats);

  // Botón Entrenar Caballero sin Escudo
  sprintf(costo, "Costo: %d Oro, %d Comida, %d Madera",
          COSTO_CABALLERO_SIN_ESCUDO_ORO, COSTO_CABALLERO_SIN_ESCUDO_COMIDA,
          COSTO_CABALLERO_SIN_ESCUDO_MADERA);
  sprintf(stats,
          "Vida:%d | Da\xF1o:%d | Cr\xEDtico:%d%% | Def:%d | Alcance:Cuerpo a "
          "cuerpo",
          CABALLERO_SIN_ESCUDO_VIDA, CABALLERO_SIN_ESCUDO_DANO,
          CABALLERO_SIN_ESCUDO_CRITICO, CABALLERO_SIN_ESCUDO_DEFENSA);
  dibujarBotonEntrenamiento(hdc, menu->botonCaballeroSinEscudo,
                            "Entrenar Caballero sin Escudo", costo, stats);

  // Botón Entrenar Guerrero
  sprintf(costo, "Costo: %d Oro, %d Comida, %d Madera", COSTO_GUERRERO_ORO,
          COSTO_GUERRERO_COMIDA, COSTO_GUERRERO_MADERA);
  sprintf(stats,
          "Vida:%d | Da\xF1o:%d | Cr\xEDtico:%d%% | Def:%d | Alcance:Cuerpo a "
          "cuerpo",
          GUERRERO_VIDA, GUERRERO_DANO, GUERRERO_CRITICO, GUERRERO_DEFENSA);
  dibujarBotonEntrenamiento(hdc, menu->botonGuerrero, "Entrenar Guerrero",
                            costo, stats);

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

  // --- ENTRENAR CABALLERO CON ESCUDO ---
  if (pantallaX >= menu->botonCaballero.left &&
      pantallaX <= menu->botonCaballero.right &&
      pantallaY >= menu->botonCaballero.top &&
      pantallaY <= menu->botonCaballero.bottom) {
    if (jugador->Oro >= COSTO_CABALLERO_ORO &&
        jugador->Comida >= COSTO_CABALLERO_COMIDA &&
        jugador->Madera >= COSTO_CABALLERO_MADERA &&
        jugador->Hierro >= COSTO_CABALLERO_HIERRO) {
      extern bool entrenarCaballero(struct Jugador * j, float x, float y);
      if (entrenarCaballero(jugador, 0, 0)) {
        jugador->Oro -= COSTO_CABALLERO_ORO;
        jugador->Comida -= COSTO_CABALLERO_COMIDA;
        jugador->Madera -= COSTO_CABALLERO_MADERA;
        jugador->Hierro -= COSTO_CABALLERO_HIERRO;
      } else {
        strcpy(menu->mensajeError,
               "\xA1No hay espacio para m\xE1s caballeros!");
        menu->tiempoError = 60;
      }
    } else {
      strcpy(menu->mensajeError, "\xA1Recursos insuficientes!");
      menu->tiempoError = 60;
    }
    return true;
  }

  // --- ENTRENAR CABALLERO SIN ESCUDO ---
  if (pantallaX >= menu->botonCaballeroSinEscudo.left &&
      pantallaX <= menu->botonCaballeroSinEscudo.right &&
      pantallaY >= menu->botonCaballeroSinEscudo.top &&
      pantallaY <= menu->botonCaballeroSinEscudo.bottom) {
    if (jugador->Oro >= COSTO_CABALLERO_SIN_ESCUDO_ORO &&
        jugador->Comida >= COSTO_CABALLERO_SIN_ESCUDO_COMIDA &&
        jugador->Madera >= COSTO_CABALLERO_SIN_ESCUDO_MADERA &&
        jugador->Hierro >= COSTO_CABALLERO_SIN_ESCUDO_HIERRO) {
      extern bool entrenarCaballeroSinEscudo(struct Jugador * j, float x,
                                             float y);
      if (entrenarCaballeroSinEscudo(jugador, 0, 0)) {
        jugador->Oro -= COSTO_CABALLERO_SIN_ESCUDO_ORO;
        jugador->Comida -= COSTO_CABALLERO_SIN_ESCUDO_COMIDA;
        jugador->Madera -= COSTO_CABALLERO_SIN_ESCUDO_MADERA;
        jugador->Hierro -= COSTO_CABALLERO_SIN_ESCUDO_HIERRO;
      } else {
        strcpy(menu->mensajeError,
               "\xA1No hay espacio para m\xE1s caballeros!");
        menu->tiempoError = 60;
      }
    } else {
      strcpy(menu->mensajeError, "\xA1Recursos insuficientes!");
      menu->tiempoError = 60;
    }
    return true;
  }

  // --- ENTRENAR GUERRERO ---
  if (pantallaX >= menu->botonGuerrero.left &&
      pantallaX <= menu->botonGuerrero.right &&
      pantallaY >= menu->botonGuerrero.top &&
      pantallaY <= menu->botonGuerrero.bottom) {
    if (jugador->Oro >= COSTO_GUERRERO_ORO &&
        jugador->Comida >= COSTO_GUERRERO_COMIDA &&
        jugador->Madera >= COSTO_GUERRERO_MADERA) {
      extern bool entrenarGuerrero(struct Jugador * j, float x, float y);
      if (entrenarGuerrero(jugador, 0, 0)) {
        jugador->Oro -= COSTO_GUERRERO_ORO;
        jugador->Comida -= COSTO_GUERRERO_COMIDA;
        jugador->Madera -= COSTO_GUERRERO_MADERA;
      } else {
        strcpy(menu->mensajeError, "No hay espacio para mas guerreros!");
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
