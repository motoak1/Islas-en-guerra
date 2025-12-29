// edificios/edificios.c
#include "edificios.h"
#include <stdio.h>
#include <string.h>

#define imgAyuntamiento "../assets/ayuntamiento2.bmp"
#define imgMina "../assets/mina.bmp"

// Sprites globales de edificios
HBITMAP g_spriteAyuntamiento = NULL;
HBITMAP g_spriteMina = NULL;

void edificioInicializar(Edificio *e, TipoEdificio tipo, float x, float y) {
  e->tipo = tipo;
  e->x = x;
  e->y = y;
  e->construido = true;

  // Configurar dimensiones según el tipo
  switch (tipo) {
  case EDIFICIO_AYUNTAMIENTO:
    e->ancho = 128; // Volver a 128 para coincidir con el tamaño de la imagen
                    // del usuario
    e->alto = 128;
    e->sprite = g_spriteAyuntamiento;
    break;
  case EDIFICIO_MINA:
    e->ancho = 128;
    e->alto = 128;
    e->sprite = g_spriteMina;
    break;
  case EDIFICIO_CUARTEL:
  case EDIFICIO_GRANJA:
    e->ancho = 128;
    e->alto = 128;
    e->sprite = NULL; // Por ahora solo ayuntamiento y mina
    break;
  }
}

bool edificioContienePunto(const Edificio *e, float mundoX, float mundoY) {
  if (!e->construido)
    return false;

  // Verificar si el punto está dentro del rectángulo del edificio
  return (mundoX >= e->x && mundoX < e->x + e->ancho && mundoY >= e->y &&
          mundoY < e->y + e->alto);
}

void edificiosCargarSprites() {
  char rutaCompleta[512];
  char directorioEXE[512];

  // Obtener la ruta completa del ejecutable
  GetModuleFileNameA(NULL, directorioEXE, sizeof(directorioEXE));

  // Encontrar la última barra invertida para obtener solo el directorio
  char *ultimaBarra = strrchr(directorioEXE, '\\');
  if (ultimaBarra != NULL) {
    *ultimaBarra = '\0'; // Terminar string en la última barra
  }

  // Construir ruta completa: directorio_exe\assets\ayuntamiento.bmp
  sprintf(rutaCompleta, "%s..\\assets\\ayuntamiento.bmp", directorioEXE);

  // Cargar sprite del ayuntamiento
  g_spriteAyuntamiento =
      (HBITMAP)LoadImageA(NULL, imgAyuntamiento, IMAGE_BITMAP, 128, 128,
                          LR_LOADFROMFILE);

  if (!g_spriteAyuntamiento) {
    printf("[ERROR] No se pudo cargar ayuntamiento desde: %s\n", rutaCompleta);
    char mensaje[512];
    sprintf(mensaje,
            "No se pudo cargar la imagen del ayuntamiento.\n"
            "Buscado en: %s\n\n"
            "Asegurate de que la imagen existe en esa ruta.\n"
            "Se mostrara un rectangulo marron.",
            rutaCompleta);
    MessageBox(NULL, mensaje, "Aviso: Sprite Ayuntamiento",
               MB_OK | MB_ICONWARNING);
  } else {
    printf("[OK] Ayuntamiento BMP cargado correctamente desde: %s\n",
           rutaCompleta);
  }

  // Cargar sprite de la mina
  g_spriteMina =
      (HBITMAP)LoadImageA(NULL, imgMina, IMAGE_BITMAP, 128, 128,
                          LR_LOADFROMFILE);

  if (!g_spriteMina) {
    printf("[ERROR] No se pudo cargar mina desde: %s\n", imgMina);
  } else {
    printf("[OK] Mina BMP cargado correctamente.\n");
  }
}

void edificioDibujar(HDC hdcBuffer, const Edificio *e, int camX, int camY,
                     float zoom, int anchoP, int altoP) {
  if (!e->construido)
    return;

  // Convertir coordenadas del mundo a pantalla
  int pantallaX = (int)((e->x - camX) * zoom);
  int pantallaY = (int)((e->y - camY) * zoom);
  int anchoZoom = (int)(e->ancho * zoom);
  int altoZoom = (int)(e->alto * zoom);

  // Culling: no dibujar si está fuera de la pantalla
  if (pantallaX + anchoZoom < 0 || pantallaX > anchoP ||
      pantallaY + altoZoom < 0 || pantallaY > altoP) {
    return;
  }

  // Si no hay sprite, dibujar un rectángulo café (FALLBACK)
  if (!e->sprite) {
    HBRUSH brushEdificio = CreateSolidBrush(RGB(139, 69, 19));
    HPEN penBorde = CreatePen(PS_SOLID, 2, RGB(218, 165, 32));
    HBRUSH oldBrush = (HBRUSH)SelectObject(hdcBuffer, brushEdificio);
    HPEN oldPen = (HPEN)SelectObject(hdcBuffer, penBorde);

    Rectangle(hdcBuffer, pantallaX, pantallaY, pantallaX + anchoZoom,
              pantallaY + altoZoom);

    SelectObject(hdcBuffer, oldPen);
    SelectObject(hdcBuffer, oldBrush);
    DeleteObject(penBorde);
    DeleteObject(brushEdificio);
    return;
  }

  // Crear DC compatible para el sprite
  HDC hdcSprite = CreateCompatibleDC(hdcBuffer);
  HBITMAP oldBmp = (HBITMAP)SelectObject(hdcSprite, e->sprite);

  // Obtener dimensiones reales del bitmap cargado
  BITMAP bm;
  GetObject(e->sprite, sizeof(bm), &bm);

  // Dibujar sprite con transparencia (fondo blanco = RGB(255, 255, 255))
  // Esto hace que el edificio se vea sin recuadro blanco, igual que árboles y unidades
  SetStretchBltMode(hdcBuffer, HALFTONE);
  TransparentBlt(hdcBuffer, pantallaX, pantallaY, anchoZoom, altoZoom, 
                 hdcSprite, 0, 0, bm.bmWidth, bm.bmHeight, RGB(255, 255, 255));

  SelectObject(hdcSprite, oldBmp);
  DeleteDC(hdcSprite);
}

void edificiosLiberarSprites() {
  if (g_spriteAyuntamiento) {
    DeleteObject(g_spriteAyuntamiento);
    g_spriteAyuntamiento = NULL;
  }
  if (g_spriteMina) {
    DeleteObject(g_spriteMina);
    g_spriteMina = NULL;
  }
}
