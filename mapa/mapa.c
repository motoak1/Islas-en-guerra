// mapa.c - Integración de BMP
#include <windows.h>
#include <stdio.h>
#include "mapa.h"


#define rutaImagenMapa "..//assets//mapa_islas_guerra.bmp"

static HBITMAP hMapaBmp = NULL;

void cargarRecursosGraficos() {
    // Cargamos el mapa que generamos de 1024x1024
    hMapaBmp = (HBITMAP)LoadImageA(NULL, rutaImagenMapa, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    if (!hMapaBmp) {
        printf("Error: No se encontro el archivo 'mapa_islas_guerra.bmp'\n");
    }
}

// mapa.c - Versión de Pantalla Completa
void dibujarMundo(HDC hdc, RECT rect, Camara cam) {
    if (!hMapaBmp) return;

    // 1. Crear el Buffer de Memoria (Pantalla invisible)
    HDC hdcBuffer = CreateCompatibleDC(hdc);
    HBITMAP hbmBuffer = CreateCompatibleBitmap(hdc, rect.right, rect.bottom);
    HBITMAP hOldBuffer = (HBITMAP)SelectObject(hdcBuffer, hbmBuffer);

    // 2. Preparar el mapa original
    HDC hdcMapa = CreateCompatibleDC(hdc);
    HBITMAP hOldMapa = (HBITMAP)SelectObject(hdcMapa, hMapaBmp);

    // 3. Limpiar el buffer (Evita lineas negras residuales)
    BitBlt(hdcBuffer, 0, 0, rect.right, rect.bottom, NULL, 0, 0, BLACKNESS);

    // 4. Calcular el area visible (Zoom)
    int anchoVista = (int)(rect.right / cam.zoom);
    int altoVista = (int)(rect.bottom / cam.zoom);

    // 5. Dibujar del mapa al Buffer
    SetStretchBltMode(hdcBuffer, COLORONCOLOR); // Suavizado de pixeles
    StretchBlt(
        hdcBuffer, 0, 0, rect.right, rect.bottom, 
        hdcMapa, cam.x, cam.y, anchoVista, altoVista, 
        SRCCOPY
    );

    // 6. VOLCADO FINAL: Copia todo el buffer de una sola vez a la pantalla
    // Esto es lo que elimina el parpadeo
    BitBlt(hdc, 0, 0, rect.right, rect.bottom, hdcBuffer, 0, 0, SRCCOPY);

    // 7. Limpieza de memoria (Vital para que el PC no se ponga lento)
    SelectObject(hdcBuffer, hOldBuffer);
    SelectObject(hdcMapa, hOldMapa);
    DeleteObject(hbmBuffer);
    DeleteDC(hdcMapa);
    DeleteDC(hdcBuffer);
}
void explorarMapaGrafico(Camara *cam, int dx, int dy) {
    // [cite: 120] Uso de punteros para modificar el mapa/cámara dinámicamente
    cam->x += dx;
    cam->y += dy;

    // Validación de límites (Tarea específica del Rol 1) [cite: 120, 174]
    if (cam->x < 0) cam->x = 0;
    if (cam->y < 0) cam->y = 0;
    if (cam->x > 1024 - 400) cam->x = 1024 - 400; // 400 es un margen de seguridad
}