// edificios/edificios.c
#include "edificios.h"
#include <stdio.h>
#include <string.h>

#define imgAyuntamiento "../assets/ayuntamiento2.bmp"
#define imgMina "../assets/mina.bmp"
#define imgCuartel "../assets/cuartel.bmp"

// Sprites globales de edificios
HBITMAP g_spriteAyuntamiento = NULL;
HBITMAP g_spriteMina = NULL;
HBITMAP g_spriteCuartel = NULL;

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
    e->ancho = 128;
    e->alto = 128;
    e->sprite = g_spriteCuartel;
    break;
  case EDIFICIO_GRANJA:
    e->ancho = 128;
    e->alto = 128;
    e->sprite = NULL; // Por ahora no implementado
    break;
  }

  // Inicializar acumuladores de recursos
  e->oroAcumulado = 0;
  e->piedraAcumulada = 0;
  e->hierroAcumulado = 0;
  e->ultimoTickGeneracion = GetTickCount();
}

bool edificioContienePunto(const Edificio *e, float mundoX, float mundoY) {
  if (!e->construido)
    return false;

  // Verificar si el punto está dentro del rectángulo del edificio
  return (mundoX >= e->x && mundoX < e->x + e->ancho && mundoY >= e->y &&
          mundoY < e->y + e->alto);
}

void edificioActualizar(Edificio *e) {
  if (!e->construido || e->tipo != EDIFICIO_MINA)
    return;

  DWORD tickActual = GetTickCount();
  // Generar cada 15 segundos (15000 ms)
  if (tickActual - e->ultimoTickGeneracion >= 15000) {
    // Generar +20 de cada uno, hasta un tope de 100
    if (e->oroAcumulado < 100)
      e->oroAcumulado += 20;
    if (e->piedraAcumulada < 100)
      e->piedraAcumulada += 20;
    if (e->hierroAcumulado < 100)
      e->hierroAcumulado += 10; // Hierro es mas escaso

    e->ultimoTickGeneracion = tickActual;

    // Debug para consola
    printf("[MINA] Recursos generados: Oro=%d, Piedra=%d, Hierro=%d\n",
           e->oroAcumulado, e->piedraAcumulada, e->hierroAcumulado);
  }
}

#define MAX_PATH_LEN 512

void edificiosCargarSprites() {
  char pathExe[MAX_PATH_LEN];
  GetModuleFileNameA(NULL, pathExe, MAX_PATH_LEN);
  char *last = strrchr(pathExe, '\\');
  if (last)
    *last = '\0';

  char fullPath[MAX_PATH_LEN];

  // Lista de intentos para el Ayuntamiento
  const char *attemptsAyunt[] = {
      "\\assets\\ayuntamiento2.bmp", "\\assets\\ayuntamiento.bmp",
      "\\..\\assets\\ayuntamiento2.bmp", "\\ayuntamiento2.bmp"};

  g_spriteAyuntamiento = NULL;
  for (int i = 0; i < 4; i++) {
    sprintf(fullPath, "%s%s", pathExe, attemptsAyunt[i]);
    g_spriteAyuntamiento =
        (HBITMAP)LoadImageA(NULL, fullPath, IMAGE_BITMAP, 128, 128,
                            LR_LOADFROMFILE | LR_CREATEDIBSECTION);
    if (g_spriteAyuntamiento)
      break;
  }

  // Lista de intentos para la Mina
  const char *attemptsMina[] = {"\\assets\\mina.bmp", "\\..\\assets\\mina.bmp",
                                "\\mina.bmp"};

  g_spriteMina = NULL;
  for (int i = 0; i < 3; i++) {
    sprintf(fullPath, "%s%s", pathExe, attemptsMina[i]);
    g_spriteMina = (HBITMAP)LoadImageA(NULL, fullPath, IMAGE_BITMAP, 128, 128,
                                       LR_LOADFROMFILE | LR_CREATEDIBSECTION);
    if (g_spriteMina)
      break;
  }

  // Lista de intentos para el Cuartel
  const char *attemptsCuartel[] = {
      "\\assets\\cuartel.bmp", "\\..\\assets\\cuartel.bmp", "\\cuartel.bmp"};

  g_spriteCuartel = NULL;
  for (int i = 0; i < 3; i++) {
    sprintf(fullPath, "%s%s", pathExe, attemptsCuartel[i]);
    g_spriteCuartel =
        (HBITMAP)LoadImageA(NULL, fullPath, IMAGE_BITMAP, 128, 128,
                            LR_LOADFROMFILE | LR_CREATEDIBSECTION);
    if (g_spriteCuartel) {
      printf("[SISTEMA] Imagen del cuartel cargada con exito.\n");
      break;
    }
  }

  if (g_spriteAyuntamiento && g_spriteMina && g_spriteCuartel) {
    printf("[SISTEMA] Recursos de edificios cargados con exito.\n");
  } else {
    printf("[ERROR] Fallo en carga de sprites:\n");
    if (!g_spriteAyuntamiento)
      printf(" - Fallo Ayuntamiento\n");
    if (!g_spriteMina)
      printf(" - Fallo Mina\n");
    if (!g_spriteCuartel)
      printf(" - Fallo Cuartel (assets/cuartel.bmp)\n");

    char errorMsg[1024];
    sprintf(errorMsg,
            "No se pudieron cargar algunos sprites de edificios.\nDirectorio "
            "del EXE: %s\n\nVerifica que la carpeta 'assets' contenga "
            "'ayuntamiento2.bmp', 'mina.bmp' y 'cuartel.bmp'.\n\nRevisa la "
            "consola negra para mas detalles.",
            pathExe);
    MessageBoxA(NULL, errorMsg, "Aviso de Carga", MB_OK | MB_ICONWARNING);
  }

  // Cargar sprite de la mina
  g_spriteMina = (HBITMAP)LoadImageA(NULL, imgMina, IMAGE_BITMAP, 128, 128,
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

  // SEGURIDAD: Si no tiene sprite asignado, intentar usar el global de carga
  HBITMAP spriteADibujar = e->sprite;
  if (!spriteADibujar) {
    if (e->tipo == EDIFICIO_AYUNTAMIENTO)
      spriteADibujar = g_spriteAyuntamiento;
    else if (e->tipo == EDIFICIO_MINA)
      spriteADibujar = g_spriteMina;
    else if (e->tipo == EDIFICIO_CUARTEL)
      spriteADibujar = g_spriteCuartel;
  }

  // DIBUJAR SPRITE
  if (spriteADibujar) {
    HDC hdcSprite = CreateCompatibleDC(hdcBuffer);
    HBITMAP oldBmp = (HBITMAP)SelectObject(hdcSprite, spriteADibujar);
    BITMAP bm;
    GetObject(spriteADibujar, sizeof(bm), &bm);

    TransparentBlt(hdcBuffer, pantallaX, pantallaY, anchoZoom, altoZoom,
                   hdcSprite, 0, 0, bm.bmWidth, bm.bmHeight,
                   RGB(255, 255, 255));

    SelectObject(hdcSprite, oldBmp);
    DeleteDC(hdcSprite);
  } else {
    // Cuadro cafe si falla todo
    HBRUSH brushEdificio = CreateSolidBrush(RGB(139, 69, 19));
    HBRUSH oldBrush = (HBRUSH)SelectObject(hdcBuffer, brushEdificio);
    Rectangle(hdcBuffer, pantallaX, pantallaY, pantallaX + anchoZoom,
              pantallaY + altoZoom);
    SelectObject(hdcBuffer, oldBrush);
    DeleteObject(brushEdificio);
  }

  // --- SEÑALIZACIÓN DE RECURSOS (Mina) ---
  if (e->tipo == EDIFICIO_MINA &&
      (e->oroAcumulado > 0 || e->piedraAcumulada > 0)) {
    // Dibujar un pequeño círculo indicador (Amarillo para Oro, Gris para
    // Piedra)
    int offsetSignal = (int)(10 * zoom);

    if (e->oroAcumulado > 0) {
      HBRUSH hBrush = CreateSolidBrush(RGB(255, 215, 0)); // Oro
      HBRUSH oldB = (HBRUSH)SelectObject(hdcBuffer, hBrush);
      Ellipse(hdcBuffer, pantallaX + offsetSignal, pantallaY - offsetSignal,
              pantallaX + offsetSignal + (int)(15 * zoom), pantallaY);
      SelectObject(hdcBuffer, oldB);
      DeleteObject(hBrush);
    }

    if (e->piedraAcumulada > 0) {
      HBRUSH hBrush = CreateSolidBrush(RGB(169, 169, 169)); // Piedra
      HBRUSH oldB = (HBRUSH)SelectObject(hdcBuffer, hBrush);
      int offsetPiedra = (int)(25 * zoom);
      Ellipse(hdcBuffer, pantallaX + offsetPiedra, pantallaY - offsetSignal,
              pantallaX + offsetPiedra + (int)(15 * zoom), pantallaY);
      SelectObject(hdcBuffer, oldB);
      DeleteObject(hBrush);
    }

    // Texto flotante opcional
    SetBkMode(hdcBuffer, TRANSPARENT);
    SetTextColor(hdcBuffer, RGB(255, 255, 255));
    char txt[32];
    sprintf(txt, "[!] RECURSOS");
    TextOutA(hdcBuffer, pantallaX + (int)(40 * zoom),
             pantallaY - (int)(20 * zoom), txt, strlen(txt));
  }
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
  if (g_spriteCuartel) {
    DeleteObject(g_spriteCuartel);
    g_spriteCuartel = NULL;
  }
}
