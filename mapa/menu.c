#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <conio.h>
#include <windows.h>
#include <windowsx.h>
#include "../batallas/guardado/guardado.h"

// --- CONSTANTES DE DIMENSIONES Y POSICIONAMIENTO ---
#define ANCHO_CONSOLA 80
#define ALTO_CONSOLA 25

// --- CONSTANTES DE COLOR (Basadas en la API de Windows) ---
#define COLOR_NEGRO 0
#define COLOR_AZUL_MARINO 1
#define COLOR_VERDE_OSCURO 2
#define COLOR_ROJO 4

#define COLOR_AMARILLO 14
#define COLOR_VERDE_CLARO 10
#define COLOR_MARRON 6
#define COLOR_AMARILLO_CLARO 14
#define COLOR_GRIS_OSCURO 8

#define COLOR_BLANCO 7
#define COLOR_GRIS_CLARO 8

// Colores del Cielo y Océano
#define COLOR_AZUL_CIELO 11
#define COLOR_AZUL_OCEANO 1
#define COLOR_AZUL_CLARO 9

// Colores de la Arena
#define COLOR_AMARILLO_ARENA 4
#define COLOR_MARRON_GRANOS 6

#define COLOR_PURPURA_CIELO 13
#define COLOR_AZUL_PROFUNDO_MAR 9
#define COLOR_ARENA_CLARA 14
#define COLOR_AMARILLO_ARENA_OSCURO 6
#define COLOR_GRIS_OSCURO_ROCA 8
#define COLOR_GRIS_CLARO_MOAI 7
#define COLOR_VERDE_OSCURO 2
#define COLOR_MARRON_TRONCO 6
#define COLOR_VERDE_PALMERA 10

// --- CONSTANTES DE TECLA ---
#define TECLA_ENTER 13
#define TECLA_ESC 27
#define TECLA_ESPECIAL 224
#define FLECHA_ARRIBA 72
#define FLECHA_ABAJO 80
#define TECLA_W 'w'
#define TECLA_S 's'

// --- RECURSOS ---
// Fondo general del menú y fondo específico de instrucciones
#define RUTA_FONDO "..\\assets\\menu\\menu_bg.bmp"
#define RUTA_FONDO_INSTRUC "..\\assets\\menu\\menu_bg_instruc.bmp"

#define RUTA_ISLA1 "..\\assets\\islas\\isla1.bmp"
#define RUTA_ISLA1_ALT "assets/islas/isla1.bmp"
#define RUTA_ISLA2 "..\\assets\\islas\\isla2.bmp"
#define RUTA_FONDO_ISLAS "..\\assets\\menu\\menu_bg_islas.bmp"
#define RUTA_FONDO_ISLAS_ALT "assets/menu/menu_bg_islas.bmp"
#define RUTA_ISLA2_ALT "assets/islas/isla2.bmp"
#define RUTA_ISLA3 "..\\assets\\islas\\isla3.bmp"
#define RUTA_ISLA3_ALT "assets/islas/isla3.bmp"
#define RUTA_ISLA4 "..\\assets\\islas\\isla4.bmp"
#define RUTA_ISLA4_ALT "assets/islas/isla4.bmp"
#define RUTA_ISLA5 "..\\assets\\islas\\isla5.bmp"
#define RUTA_ISLA5_ALT "assets/islas/isla5.bmp"
#define RUTA_MAPA_COMPLETO "..\\assets\\mapa_islas_guerra.bmp"
#define RUTA_MAPA_COMPLETO_ALT "assets/mapa_islas_guerra.bmp"
#define RUTA_MAPA2 "..\\assets\\mapa2.bmp"
#define RUTA_MAPA2_ALT "assets/mapa2.bmp"

static HBITMAP fondoBmp = NULL;
static BITMAP infoFondo;
static bool fondoListo = false;

static HBITMAP fondoInstrucBmp = NULL;
static BITMAP infoFondoInstruc;
static bool fondoInstrucListo = false;

static HBITMAP fondoIslasBmp = NULL;
static BITMAP infoFondoIslas;
static bool fondoIslasListo = false;

// --- Estado global del menú ---
static int gSeleccion = 0;
static HFONT gFontTitulo = NULL;
static HFONT gFontOpciones = NULL;
// Opciones fijas
static const char *OPCIONES_CON_CARGAR[] = { "Iniciar partida", "Cargar partida", "Instrucciones", "Salir" };
static const char *OPCIONES_SIN_CARGAR[] = { "Iniciar partida", "Instrucciones", "Salir" };
// Opciones dinámicas
static const char **OPCIONES_MENU = NULL;
static int OPCIONES_TOTAL = 0;
static bool gHayPartidasGuardadas = false;
static bool gMostrandoInstrucciones = false;
static HWND gMenuHwnd = NULL;
static int gMenuAccion = 0; // 0 = nueva partida, 1 = cargar partida
static bool gMostrandoSeleccionIsla = false;
static int gSeleccionIsla = 0;
static int gIslaSeleccionada = 1;
static const char *OPCIONES_ISLAS[] = { 
    "Mapa 3 Islas",        // 0 - Preview del mapa completo original
    "Isla 1",              // 1 - Seleccionable
    "Isla 2",              // 2 - Seleccionable
    "Isla 3",              // 3 - Seleccionable
    "Mapa 2 Islas",        // 4 - Preview del nuevo mapa (mapa2.bmp)
    "Isla 4 [Bloqueada]",  // 5 - Visible pero NO seleccionable
    "Isla 5 [Bloqueada]",  // 6 - Visible pero NO seleccionable
    "Volver"               // 7
};
static const int OPCIONES_ISLAS_TOTAL = 8;

// Array de flags para indicar qué opciones están bloqueadas
static const bool ISLAS_BLOQUEADAS[] = { 
    false, // Mapa 3 Islas
    false, // Isla 1
    false, // Isla 2
    false, // Isla 3
    false, // Mapa 2 Islas (solo preview)
    true,  // Isla 4 - BLOQUEADA
    true,  // Isla 5 - BLOQUEADA
    false  // Volver
};

static HBITMAP hIslaBmp[7] = {NULL};  // 0=mapa3, 1-3=isla1-3, 4=mapa2, 5-6=isla4-5
static BITMAP infoIsla[7];
static bool islasCargadas = false;

// --- Estado del menú de carga de partidas ---
static bool gMostrandoCargaPartidas = false;
static InfoPartida gPartidasDisponibles[MAX_PARTIDAS];
static int gNumPartidasDisponibles = 0;
static int gSeleccionPartida = 0;
static char gNombrePartidaSeleccionada[MAX_NOMBRE_JUGADOR] = {0};
static HFONT gFontPequena = NULL;

// --- Sistema de Paginación de Instrucciones ---
#define MAX_PAGINAS 20
#define MAX_LINEAS_PAGINA 18
#define MAX_CHARS_LINEA 100

typedef struct {
    char lineas[MAX_LINEAS_PAGINA][MAX_CHARS_LINEA];
    int count;
} PaginaInstrucciones;

static PaginaInstrucciones gPaginas[MAX_PAGINAS];
static int gTotalPaginas = 0;
static int gPaginaActual = 0;


// --- Prototipos necesarios ---
void mostrarInstrucciones();
static void dibujarPreviewIsla(HDC hdc, RECT rc);


static bool cargarFondo(void) {
    if (fondoListo) {
        return true;
    }

    fondoBmp = (HBITMAP)LoadImageA(NULL, RUTA_FONDO, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
    if (fondoBmp == NULL) {
        return false;
    }

    GetObject(fondoBmp, sizeof(infoFondo), &infoFondo);
    fondoListo = true;
    return true;
}

static bool cargarFondoInstruc(void) {
    if (fondoInstrucListo) {
        return true;
    }

    fondoInstrucBmp = (HBITMAP)LoadImageA(NULL, RUTA_FONDO_INSTRUC, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
    if (fondoInstrucBmp == NULL) {
        return false;
    }

    GetObject(fondoInstrucBmp, sizeof(infoFondoInstruc), &infoFondoInstruc);
    fondoInstrucListo = true;
    return true;
}

static bool cargarFondoIslas(void) {
    if (fondoIslasListo) {
        return true;
    }

    fondoIslasBmp = (HBITMAP)LoadImageA(NULL, RUTA_FONDO_ISLAS, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
    if (fondoIslasBmp == NULL) {
        fondoIslasBmp = (HBITMAP)LoadImageA(NULL, RUTA_FONDO_ISLAS_ALT, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
    }
    if (fondoIslasBmp == NULL) {
        return false;
    }

    GetObject(fondoIslasBmp, sizeof(infoFondoIslas), &infoFondoIslas);
    fondoIslasListo = true;
    return true;
}

static bool cargarIslas(void) {
    if (islasCargadas) {
        return true;
    }

    const char *rutas[] = { 
        RUTA_MAPA_COMPLETO, RUTA_ISLA1, RUTA_ISLA2, RUTA_ISLA3,
        RUTA_MAPA2, RUTA_ISLA4, RUTA_ISLA5
    };
    const char *rutasAlt[] = { 
        RUTA_MAPA_COMPLETO_ALT, RUTA_ISLA1_ALT, RUTA_ISLA2_ALT, RUTA_ISLA3_ALT,
        RUTA_MAPA2_ALT, RUTA_ISLA4_ALT, RUTA_ISLA5_ALT
    };

    for (int i = 0; i < 7; i++) {
        hIslaBmp[i] = (HBITMAP)LoadImageA(NULL, rutas[i], IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
        if (!hIslaBmp[i]) {
            hIslaBmp[i] = (HBITMAP)LoadImageA(NULL, rutasAlt[i], IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
        }
        if (hIslaBmp[i]) {
            GetObject(hIslaBmp[i], sizeof(BITMAP), &infoIsla[i]);
        }
    }

    // Considerar cargado si al menos las primeras 4 están OK (las originales)
    islasCargadas = (hIslaBmp[0] && hIslaBmp[1] && hIslaBmp[2] && hIslaBmp[3]);
    return islasCargadas;
}

static void dibujarFondoBmp(HDC hdc, RECT rc) {
    if (!cargarFondo()) {
        HBRUSH brush = CreateSolidBrush(RGB(0, 0, 0));
        FillRect(hdc, &rc, brush);
        DeleteObject(brush);
        return;
    }

    HDC mem = CreateCompatibleDC(hdc);
    HBITMAP old = (HBITMAP)SelectObject(mem, fondoBmp);
    StretchBlt(
        hdc,
        0,
        0,
        rc.right - rc.left,
        rc.bottom - rc.top,
        mem,
        0,
        0,
        infoFondo.bmWidth,
        infoFondo.bmHeight,
        SRCCOPY);
    SelectObject(mem, old);
    DeleteDC(mem);
}

static void dibujarFondoBmpIslas(HDC hdc, RECT rc) {
    if (!cargarFondoIslas()) {
        HBRUSH brush = CreateSolidBrush(RGB(0, 0, 0));
        FillRect(hdc, &rc, brush);
        DeleteObject(brush);
        return;
    }

    HDC mem = CreateCompatibleDC(hdc);
    HBITMAP old = (HBITMAP)SelectObject(mem, fondoIslasBmp);
    StretchBlt(
        hdc,
        0,
        0,
        rc.right - rc.left,
        rc.bottom - rc.top,
        mem,
        0,
        0,
        infoFondoIslas.bmWidth,
        infoFondoIslas.bmHeight,
        SRCCOPY);
    SelectObject(mem, old);
    DeleteDC(mem);
}

static void dibujarFondoBmpInstrucciones(HDC hdc, RECT rc) {
    if (!cargarFondoInstruc()) {
        HBRUSH brush = CreateSolidBrush(RGB(0, 0, 0));
        FillRect(hdc, &rc, brush);
        DeleteObject(brush);
        return;
    }

    HDC mem = CreateCompatibleDC(hdc);
    HBITMAP old = (HBITMAP)SelectObject(mem, fondoInstrucBmp);
    StretchBlt(
        hdc,
        0,
        0,
        rc.right - rc.left,
        rc.bottom - rc.top,
        mem,
        0,
        0,
        infoFondoInstruc.bmWidth,
        infoFondoInstruc.bmHeight,
        SRCCOPY);
    SelectObject(mem, old);
    DeleteDC(mem);
}

static SIZE medirTexto(HDC hdc, HFONT font, const char *texto) {
    SIZE size = {0, 0};
    HFONT old = (HFONT)SelectObject(hdc, font);
    GetTextExtentPoint32A(hdc, texto, (int)strlen(texto), &size);
    SelectObject(hdc, old);
    return size;
}

static void dibujarTextoCentrado(HDC hdc, HFONT font, const char *texto, int xCentro, int y, COLORREF color) {
    HFONT oldFont = (HFONT)SelectObject(hdc, font);
    SIZE size;
    GetTextExtentPoint32A(hdc, texto, (int)strlen(texto), &size);

    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, color);
    TextOutA(hdc, xCentro - size.cx / 2, y, texto, (int)strlen(texto));

    SelectObject(hdc, oldFont);
}

static void dibujarOpciones(HDC hdc, HFONT font, const char *opciones[], int total, int seleccion, RECT rc) {
    int ancho = rc.right - rc.left;
    int alto = rc.bottom - rc.top;
    int centroX = ancho / 2;

    SIZE size = medirTexto(hdc, font, "Iniciar partida");
    int alturaLinea = size.cy + 12;
    int bloqueAlto = alturaLinea * total;
    int inicioY = (alto - bloqueAlto) / 2;

    for (int i = 0; i < total; i++) {
        const char *texto = opciones[i];
        SIZE medida = medirTexto(hdc, font, texto);
        COLORREF color = (i == seleccion) ? RGB(120, 255, 150) : RGB(255, 255, 255);
        dibujarTextoCentrado(hdc, font, texto, centroX, inicioY + i * alturaLinea, color);

        if (i == seleccion) {
            int highlightPadding = 18;
            RECT highlight = {
                centroX - medida.cx / 2 - highlightPadding,
                inicioY + i * alturaLinea - 4,
                centroX + medida.cx / 2 + highlightPadding,
                inicioY + i * alturaLinea + medida.cy + 6};
            HBRUSH brush = CreateSolidBrush(RGB(20, 80, 30));
            FrameRect(hdc, &highlight, brush);
            DeleteObject(brush);
        }
    }
}

// Función especializada para dibujar opciones de islas con soporte para bloqueadas
static void dibujarOpcionesIslas(HDC hdc, HFONT font, RECT rc) {
    int ancho = rc.right - rc.left;
    int alto = rc.bottom - rc.top;
    int centroX = ancho / 4; // Opciones a la izquierda

    SIZE size = medirTexto(hdc, font, "Isla 1");
    int alturaLinea = size.cy + 12;
    int bloqueAlto = alturaLinea * OPCIONES_ISLAS_TOTAL;
    int inicioY = (alto - bloqueAlto) / 2;

    for (int i = 0; i < OPCIONES_ISLAS_TOTAL; i++) {
        const char *texto = OPCIONES_ISLAS[i];
        SIZE medida = medirTexto(hdc, font, texto);
        
        COLORREF color;
        if (ISLAS_BLOQUEADAS[i]) {
            // Islas bloqueadas: gris oscuro
            color = RGB(120, 120, 120);
        } else if (i == gSeleccionIsla) {
            // Seleccionado: verde brillante
            color = RGB(120, 255, 150);
        } else {
            // Normal: blanco
            color = RGB(255, 255, 255);
        }
        
        dibujarTextoCentrado(hdc, font, texto, centroX, inicioY + i * alturaLinea, color);

        // Marco de selección (incluso para bloqueadas, para indicar posición)
        if (i == gSeleccionIsla) {
            int highlightPadding = 18;
            RECT highlight = {
                centroX - medida.cx / 2 - highlightPadding,
                inicioY + i * alturaLinea - 4,
                centroX + medida.cx / 2 + highlightPadding,
                inicioY + i * alturaLinea + medida.cy + 6
            };
            
            // Color del borde: rojo si bloqueada, verde si normal
            COLORREF bordeColor = ISLAS_BLOQUEADAS[i] ? RGB(100, 50, 50) : RGB(20, 80, 30);
            HBRUSH brush = CreateSolidBrush(bordeColor);
            FrameRect(hdc, &highlight, brush);
            DeleteObject(brush);
        }
    }
}

// Render del menú en la ventana GDI (usado desde WM_PAINT)
static void renderMenu(HWND hwnd) {
    RECT rc;
    GetClientRect(hwnd, &rc);

    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwnd, &ps);
    int ancho = rc.right - rc.left;
    int alto = rc.bottom - rc.top;
    int centroX = ancho / 2;
    int tituloY = alto / 5;

    if (gMostrandoInstrucciones) {
        dibujarFondoBmpInstrucciones(hdc, rc);
        // Título de instrucciones (con indicador de página)
        char tituloBuf[64];
        if (gTotalPaginas > 0) {
            snprintf(tituloBuf, sizeof(tituloBuf), "INSTRUCCIONES (%d/%d)", gPaginaActual + 1, gTotalPaginas);
        } else {
            snprintf(tituloBuf, sizeof(tituloBuf), "INSTRUCCIONES");
        }
        dibujarTextoCentrado(hdc, gFontTitulo, tituloBuf, centroX, tituloY, RGB(240, 240, 240));

        // Calcular área para el cuerpo del texto
        SIZE m = medirTexto(hdc, gFontOpciones, "Ay");
        int alturaLinea = m.cy + 8;
        
        if (gTotalPaginas > 0) {
            // Mostrar líneas de la página actual
            PaginaInstrucciones *pag = &gPaginas[gPaginaActual];
            int n = pag->count;
            int bloqueAlto = n * alturaLinea;
            int inicioY = (alto - bloqueAlto) / 2 + 20; // Un poco más abajo del título

            for (int i = 0; i < n; i++) {
                // Color blanco por defecto
                COLORREF color = RGB(255, 255, 255);
                // Si la línea parece un subtítulo (emezaba con #), pintarla amarilla
                // (Ya limpiamos los caracteres '#' pero podemos inferir por contexto o simplemente todo blanco)
                // Para diferenciar, podemos detectar mayúsculas al inicio
                dibujarTextoCentrado(hdc, gFontOpciones, pag->lineas[i], centroX, inicioY + i * alturaLinea, color);
            }
            
            // Pie de página con controles
            int pieY = alto - 50;
            dibujarTextoCentrado(hdc, gFontPequena, "< ANTERIOR   |   SIGUIENTE >", centroX, pieY, RGB(100, 200, 255));
            dibujarTextoCentrado(hdc, gFontPequena, "ESC: Volver al menú", centroX, pieY + 20, RGB(255, 230, 120));
        } else {
             // Fallback por si falla la carga (no debería ocurrir con el manejo de error)
             dibujarTextoCentrado(hdc, gFontOpciones, "Cargando instrucciones...", centroX, alto/2, RGB(255, 255, 255));
        }
    } else if (gMostrandoSeleccionIsla) {
        dibujarFondoBmpIslas(hdc, rc);
        RECT rcOpc = rc;
        int ancho = rc.right - rc.left;
        rcOpc.left += ancho / 14;// padding izquierdo
        rcOpc.right = rc.left + (ancho / 2) - ancho / 20; // dejar aire con el preview
        dibujarOpcionesIslas(hdc, gFontOpciones, rcOpc);
        dibujarPreviewIsla(hdc, rc);
    } else if (gMostrandoCargaPartidas) {
        // Menú de carga de partidas con el mismo fondo que el menú principal
        dibujarFondoBmp(hdc, rc);
        dibujarTextoCentrado(hdc, gFontTitulo, "CARGAR PARTIDA", centroX, tituloY, RGB(240, 240, 240));
        
        if (gNumPartidasDisponibles == 0) {
            // No hay partidas guardadas
            dibujarTextoCentrado(hdc, gFontOpciones, "No hay partidas guardadas", centroX, alto / 2, RGB(200, 200, 200));
            dibujarTextoCentrado(hdc, gFontPequena, "Presiona ESC para volver", centroX, alto / 2 + 60, RGB(255, 230, 120));
        } else {
            // Mostrar lista de partidas
            SIZE medida = medirTexto(hdc, gFontOpciones, "Texto");
            int alturaLinea = medida.cy + 20;
            int bloqueAlto = alturaLinea * gNumPartidasDisponibles;
            int inicioY = (alto - bloqueAlto) / 2;
            
            for (int i = 0; i < gNumPartidasDisponibles; i++) {
                char lineaPartida[256];
                snprintf(lineaPartida, sizeof(lineaPartida), "%s - Isla %d",
                         gPartidasDisponibles[i].nombreJugador,
                         gPartidasDisponibles[i].islaActual);
                
                COLORREF color = (i == gSeleccionPartida) ? RGB(120, 255, 150) : RGB(255, 255, 255);
                int yPos = inicioY + i * alturaLinea;
                
                // Dibujar nombre de partida
                dibujarTextoCentrado(hdc, gFontOpciones, lineaPartida, centroX, yPos, color);
                
                // Dibujar fecha (más pequeña, debajo)
                if (i == gSeleccionPartida) {
                    char fechaTexto[128];
                    snprintf(fechaTexto, sizeof(fechaTexto), "Guardado: %s", gPartidasDisponibles[i].timestamp);
                    dibujarTextoCentrado(hdc, gFontPequena, fechaTexto, centroX, yPos + 30, RGB(180, 180, 180));
                    
                    // Marco de selección
                    SIZE medidaLinea = medirTexto(hdc, gFontOpciones, lineaPartida);
                    int highlightPadding = 20;
                    RECT highlight = {
                        centroX - medidaLinea.cx / 2 - highlightPadding,
                        yPos - 6,
                        centroX + medidaLinea.cx / 2 + highlightPadding,
                        yPos + medidaLinea.cy + 40};
                    HBRUSH brush = CreateSolidBrush(RGB(20, 80, 30));
                    FrameRect(hdc, &highlight, brush);
                    DeleteObject(brush);
                }
            }
            
            // Instrucciones
            dibujarTextoCentrado(hdc, gFontPequena, "ENTER: Cargar | ESC: Volver", centroX, alto - 80, RGB(255, 230, 120));
        }
    } else {
        dibujarFondoBmp(hdc, rc);
        dibujarOpciones(hdc, gFontOpciones, OPCIONES_MENU, OPCIONES_TOTAL, gSeleccion, rc);
    }

    EndPaint(hwnd, &ps);
}

static LRESULT CALLBACK MenuWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            // Crear fuentes para título, opciones y texto pequeño
            gFontTitulo = CreateFontA(48, 0, 0, 0, FW_HEAVY, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                                    OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, FF_DONTCARE, "Segoe UI");
            gFontOpciones = CreateFontA(28, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                                        OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, FF_DONTCARE, "Segoe UI");
            gFontPequena = CreateFontA(18, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                                       OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, FF_DONTCARE, "Segoe UI");
            return 0;
        }
        case WM_PAINT:
            renderMenu(hwnd);
            return 0;
        case WM_KEYDOWN: {
            switch (wParam) {
                case VK_ESCAPE:
                    if (gMostrandoInstrucciones) {
                        gMostrandoInstrucciones = false;
                        gPaginaActual = 0; // Resetear página al salir
                        InvalidateRect(hwnd, NULL, FALSE);
                    } else if (gMostrandoCargaPartidas) {
                        // Volver del menú de carga al menú principal
                        gMostrandoCargaPartidas = false;
                        InvalidateRect(hwnd, NULL, FALSE);
                    } else if (gMostrandoSeleccionIsla) {
                        gMostrandoSeleccionIsla = false;
                        InvalidateRect(hwnd, NULL, FALSE);
                    } else {
                        PostMessage(hwnd, WM_CLOSE, 0, 0);
                    }
                    break;
                case VK_UP:
                case 'W':
                case 'w':
                    if (gMostrandoCargaPartidas && gNumPartidasDisponibles > 0) {
                        // Navegar en el menú de carga
                        gSeleccionPartida = (gSeleccionPartida - 1 + gNumPartidasDisponibles) % gNumPartidasDisponibles;
                        InvalidateRect(hwnd, NULL, FALSE);
                    } else if (gMostrandoSeleccionIsla) {
                        gSeleccionIsla = (gSeleccionIsla - 1 + OPCIONES_ISLAS_TOTAL) % OPCIONES_ISLAS_TOTAL;
                        InvalidateRect(hwnd, NULL, FALSE);
                    } else if (!gMostrandoInstrucciones) {
                        gSeleccion = (gSeleccion - 1 + OPCIONES_TOTAL) % OPCIONES_TOTAL;
                        InvalidateRect(hwnd, NULL, FALSE);
                    }
                    break;
                case VK_DOWN:
                case 'S':
                case 's':
                    if (gMostrandoCargaPartidas && gNumPartidasDisponibles > 0) {
                        // Navegar en el menú de carga
                        gSeleccionPartida = (gSeleccionPartida + 1) % gNumPartidasDisponibles;
                        InvalidateRect(hwnd, NULL, FALSE);
                    } else if (gMostrandoSeleccionIsla) {
                        gSeleccionIsla = (gSeleccionIsla + 1) % OPCIONES_ISLAS_TOTAL;
                        InvalidateRect(hwnd, NULL, FALSE);
                    } else if (!gMostrandoInstrucciones) {
                        gSeleccion = (gSeleccion + 1) % OPCIONES_TOTAL;
                        InvalidateRect(hwnd, NULL, FALSE);
                    }
                    break;
                case VK_RETURN:
                    if (gMostrandoCargaPartidas) {
                        // Cargar la partida seleccionada
                        if (gNumPartidasDisponibles > 0) {
                            strncpy(gNombrePartidaSeleccionada, gPartidasDisponibles[gSeleccionPartida].nombreJugador, MAX_NOMBRE_JUGADOR - 1);
                            gNombrePartidaSeleccionada[MAX_NOMBRE_JUGADOR - 1] = '\0';
                            gMenuAccion = 1;  // Indica que queremos cargar partida
                            PostMessage(hwnd, WM_CLOSE, 0, 0);
                        }
                    } else if (gMostrandoSeleccionIsla) {
                        if (gSeleccionIsla >= 1 && gSeleccionIsla <= 3) {
                            gIslaSeleccionada = gSeleccionIsla;
                            gMenuAccion = 0;
                            PostMessage(hwnd, WM_CLOSE, 0, 0);
                        } else if (gSeleccionIsla == OPCIONES_ISLAS_TOTAL - 1) {
                            gMostrandoSeleccionIsla = false;
                            InvalidateRect(hwnd, NULL, FALSE);
                        } else {
                            // "Mapa completo" solo muestra la imagen.
                        }
                    } else if (!gMostrandoInstrucciones) {
                        if (gSeleccion == 0) {
                            // Iniciar partida
                            gMostrandoSeleccionIsla = true;
                            gSeleccionIsla = gIslaSeleccionada;
                            InvalidateRect(hwnd, NULL, FALSE);
                        } else if (gHayPartidasGuardadas && gSeleccion == 1) {
                            // Mostrar menú de carga de partidas
                            gMostrandoCargaPartidas = true;
                            gSeleccionPartida = 0;
                            InvalidateRect(hwnd, NULL, FALSE);
                        } else if ((gHayPartidasGuardadas && gSeleccion == 2) || (!gHayPartidasGuardadas && gSeleccion == 1)) {
                            // Instrucciones
                            mostrarInstrucciones();
                        } else if ((gHayPartidasGuardadas && gSeleccion == 3) || (!gHayPartidasGuardadas && gSeleccion == 2)) {
                            // Salir
                            ExitProcess(0);
                        }
                    }
                    break;
                case VK_LEFT:
                case VK_BACK:
                    if (gMostrandoInstrucciones) {
                        if (gPaginaActual > 0) gPaginaActual--;
                        else gPaginaActual = gTotalPaginas - 1; // Loop al final
                        InvalidateRect(hwnd, NULL, FALSE);
                    }
                    break;
                case VK_RIGHT:
                case VK_SPACE:
                    if (gMostrandoInstrucciones) {
                        if (gPaginaActual < gTotalPaginas - 1) gPaginaActual++;
                        else gPaginaActual = 0; // Loop al inicio
                        InvalidateRect(hwnd, NULL, FALSE);
                    }
                    break;
                default:
                    break;
            }
            return 0;
        }
        case WM_DESTROY:
            if (gFontTitulo) { DeleteObject(gFontTitulo); gFontTitulo = NULL; }
            if (gFontOpciones) { DeleteObject(gFontOpciones); gFontOpciones = NULL; }
            if (gFontPequena) { DeleteObject(gFontPequena); gFontPequena = NULL; }
            if (fondoIslasBmp) { DeleteObject(fondoIslasBmp); fondoIslasBmp = NULL; }
            fondoIslasListo = false;  // CRÍTICO: Resetear flag para que se recargue
            for (int i = 0; i < 7; i++) {
                if (hIslaBmp[i]) { DeleteObject(hIslaBmp[i]); hIslaBmp[i] = NULL; }
            }
            islasCargadas = false;  // CRÍTICO: Resetear flag para que se recarguen las islas
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

// --- FUNCIONES DE CONSOLA (Implementaciones) ---

void ocultarCursor() {
    CONSOLE_CURSOR_INFO cursorInfo;
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hConsole == INVALID_HANDLE_VALUE) {
        return;
    }
    cursorInfo.dwSize = 1;
    cursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(hConsole, &cursorInfo);
}

// --- PANTALLAS DEL JUEGO ---

// Carga instrucciones desde archivo markdwon
static void cargarInstruccionesInterno() {
    if (gTotalPaginas > 0) return; // Ya cargado

    gTotalPaginas = 0;
    
    // Pagina 1: Objetivo
    strcpy(gPaginas[0].lineas[0], "--- ISLAS EN GUERRA ---");
    strcpy(gPaginas[0].lineas[1], "Bienvenido a este RTS de estrategia y exploracion.");
    strcpy(gPaginas[0].lineas[2], "");
    strcpy(gPaginas[0].lineas[3], "OBJETIVO PRINCIPAL:");
    strcpy(gPaginas[0].lineas[4], "Conquistar las 5 islas del archipielago.");
    strcpy(gPaginas[0].lineas[5], "");
    strcpy(gPaginas[0].lineas[6], "PASOS PARA LA VICTORIA:");
    strcpy(gPaginas[0].lineas[7], "1. Recolectar recursos basicos.");
    strcpy(gPaginas[0].lineas[8], "2. Entrenar ejercito en el Cuartel.");
    strcpy(gPaginas[0].lineas[9], "3. Reparar el Barco y viajar.");
    strcpy(gPaginas[0].lineas[10], "4. Eliminar enemigos de cada isla.");
    gPaginas[0].count = 11;

    // Pagina 2: Controles
    strcpy(gPaginas[1].lineas[0], "CONTROLES - MOUSE");
    strcpy(gPaginas[1].lineas[1], "Clic Izq: Seleccionar unidad / Interactuar");
    strcpy(gPaginas[1].lineas[2], "Clic Der: Mover / Atacar / Talar / Cazar");
    strcpy(gPaginas[1].lineas[3], "Arrastrar: Mover camara");
    strcpy(gPaginas[1].lineas[4], "Rueda: Zoom In/Out");
    strcpy(gPaginas[1].lineas[5], "");
    strcpy(gPaginas[1].lineas[6], "CONTROLES - TECLADO");
    strcpy(gPaginas[1].lineas[7], "C: Centrar camara en Ayuntamiento");
    strcpy(gPaginas[1].lineas[8], "H: Curar unidades seleccionadas (100 Comida)");
    strcpy(gPaginas[1].lineas[9], "ESC: Pausar / Menu");
    gPaginas[1].count = 10;

    // Pagina 3: Recursos
    strcpy(gPaginas[2].lineas[0], "RECURSOS");
    strcpy(gPaginas[2].lineas[1], "COMIDA (Vacas): Para entrenar y curar tropas.");
    strcpy(gPaginas[2].lineas[2], "MADERA (Arboles): Barcos y mejoras.");
    strcpy(gPaginas[2].lineas[3], "ORO (Mina): Compra de tropas.");
    strcpy(gPaginas[2].lineas[4], "PIEDRA/HIERRO (Mina): Construccion avanzada.");
    strcpy(gPaginas[2].lineas[5], "");
    strcpy(gPaginas[2].lineas[6], "COMO RECOLECTAR:");
    strcpy(gPaginas[2].lineas[7], "- Mina: Enviar Obrero al centro.");
    strcpy(gPaginas[2].lineas[8], "- Arboles: Clic derecho con Obrero/Guerrero.");
    strcpy(gPaginas[2].lineas[9], "- Vacas: Clic derecho para cazar.");
    gPaginas[2].count = 10;

    // Pagina 4: Unidades
    strcpy(gPaginas[3].lineas[0], "UNIDADES (Entrenar en Cuartel)");
    strcpy(gPaginas[3].lineas[1], "OBRERO: Recolecta, repara barco. Muy debil.");
    strcpy(gPaginas[3].lineas[2], "");
    strcpy(gPaginas[3].lineas[3], "CABALLERO (ESCUDO):");
    strcpy(gPaginas[3].lineas[4], "Tanque defensivo. Vida alta, ataque medio.");
    strcpy(gPaginas[3].lineas[5], "");
    strcpy(gPaginas[3].lineas[6], "CABALLERO (SIN ESCUDO):");
    strcpy(gPaginas[3].lineas[7], "Ofensivo rapido. Critico alto, defensa media.");
    strcpy(gPaginas[3].lineas[8], "");
    strcpy(gPaginas[3].lineas[9], "GUERRERO:");
    strcpy(gPaginas[3].lineas[10], "Dano masivo, poca vida. Puede talar arboles.");
    gPaginas[3].count = 11;

    // Pagina 5: Navegacion
    strcpy(gPaginas[4].lineas[0], "NAVEGACION Y CONQUISTA");
    strcpy(gPaginas[4].lineas[1], "1. Envia un Obrero a reparar el barco.");
    strcpy(gPaginas[4].lineas[2], "   (Cuesta Oro, Madera, Piedra, Hierro)");
    strcpy(gPaginas[4].lineas[3], "2. Sube tropas (Clic derecho en barco).");
    strcpy(gPaginas[4].lineas[4], "3. Clic izquierdo en barco para Mapa de Viaje.");
    strcpy(gPaginas[4].lineas[5], "4. Mejora el barco para llevar mas tropas.");
    strcpy(gPaginas[4].lineas[6], "");
    strcpy(gPaginas[4].lineas[7], "CONSEJO:");
    strcpy(gPaginas[4].lineas[8], "No viajes sin un ejercito fuerte.");
    strcpy(gPaginas[4].lineas[9], "Las islas avanzadas son peligrosas.");
    gPaginas[4].count = 10;
     
    gTotalPaginas = 5;
}

void mostrarInstrucciones() {
    // Asegurar carga
    cargarInstruccionesInterno();
    // Cambiar a vista de instrucciones y refrescar
    gMostrandoInstrucciones = true;
    if (gMenuHwnd) InvalidateRect(gMenuHwnd, NULL, TRUE);
}

int menuObtenerAccion() {
    return gMenuAccion;
}

int menuObtenerIsla() {
    return gIslaSeleccionada;
}

const char* menuObtenerNombrePartida() {
    return gNombrePartidaSeleccionada;
}

void mostrarMenu() {
    // Crear una ventana GDI dedicada para el menú a pantalla completa
    ocultarCursor();

    // Verificar si hay partidas guardadas y copiarlas
    gNumPartidasDisponibles = obtenerPartidasGuardadas(gPartidasDisponibles);
    gHayPartidasGuardadas = (gNumPartidasDisponibles > 0);
    
    // Configurar opciones del menú según si hay partidas guardadas
    if (gHayPartidasGuardadas) {
        OPCIONES_MENU = OPCIONES_CON_CARGAR;
        OPCIONES_TOTAL = 4;
    } else {
        OPCIONES_MENU = OPCIONES_SIN_CARGAR;
        OPCIONES_TOTAL = 3;
    }

    // CRÍTICO: Resetear estado del menú para comenzar desde el menú principal
    // Esto asegura que al volver del juego se muestre el menú principal, no la selección de islas
    gSeleccion = 0;
    gMostrandoInstrucciones = false;
    gMostrandoSeleccionIsla = false;
    gMostrandoCargaPartidas = false;
    gSeleccionIsla = 0;
    gSeleccionPartida = 0;
    gMenuAccion = 0;
    gNombrePartidaSeleccionada[0] = '\0';

    HINSTANCE hInst = GetModuleHandle(NULL);
    WNDCLASSA wc = {0};
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = MenuWndProc;
    wc.hInstance = hInst;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpszClassName = "MenuWndClass";
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    RegisterClassA(&wc);

    int screenW = GetSystemMetrics(SM_CXSCREEN);
    int screenH = GetSystemMetrics(SM_CYSCREEN);

    // Ocultar consola mientras se muestra el menú
    HWND hConsole = GetConsoleWindow();
    if (hConsole) ShowWindow(hConsole, SW_HIDE);

    HWND hwnd = CreateWindowExA(
        WS_EX_TOPMOST,
        wc.lpszClassName,
        "Islas en guerra - Menú",
        WS_POPUP,
        0, 0, screenW, screenH,
        NULL, NULL, hInst, NULL);

    if (!hwnd) {
        // Fallback: mostrar instrucciones de error
        MessageBoxA(NULL, "No se pudo crear la ventana de menú.", "Error", MB_OK | MB_ICONERROR);
        return;
    }

    gMenuHwnd = hwnd;
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    // Bucle de mensajes: se cerrará cuando el usuario elija Iniciar o Escape
    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }

    // NOTA: No volver a mostrar la consola - debe permanecer oculta
    if (hConsole) ShowWindow(hConsole, SW_SHOW);
}

static void dibujarPreviewIsla(HDC hdc, RECT rc) {
    if (!cargarIslas()) {
        return;
    }

    if (gSeleccionIsla == OPCIONES_ISLAS_TOTAL - 1) {
        return; // no preview for "Volver"
    }

    int idx = gSeleccionIsla;
    // Permitir índices 0-6 (7 imagenes)
    if (idx < 0 || idx > 6 || !hIslaBmp[idx]) {
        return;
    }

    HDC mem = CreateCompatibleDC(hdc);
    HBITMAP old = (HBITMAP)SelectObject(mem, hIslaBmp[idx]);

    int ancho = rc.right - rc.left;
    int alto = rc.bottom - rc.top;

    int previewW = ancho / 3;
    int previewH = alto / 2;

    if (infoIsla[idx].bmWidth && infoIsla[idx].bmHeight) {
        double ratioSrc = (double)infoIsla[idx].bmWidth / (double)infoIsla[idx].bmHeight;
        double ratioDst = (double)previewW / (double)previewH;
        if (ratioSrc > ratioDst) {
            previewH = (int)(previewW / ratioSrc);
        } else {
            previewW = (int)(previewH * ratioSrc);
        }
    }

    int destX = rc.left + (ancho / 2) - (previewW / 2); // centrar en el área media
    int destY = rc.top + (alto / 2) - (previewH / 2);

    StretchBlt(hdc, destX, destY, previewW, previewH, mem, 0, 0, infoIsla[idx].bmWidth, infoIsla[idx].bmHeight, SRCCOPY);

    // Si está bloqueada, dibujar texto "BLOQUEADA" sobre la imagen
    if (ISLAS_BLOQUEADAS[gSeleccionIsla]) {
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, RGB(255, 80, 80)); // Rojo brillante
        HFONT oldFont = (HFONT)SelectObject(hdc, gFontTitulo);
        SIZE textSize;
        const char *textoBloqueo = "BLOQUEADA";
        GetTextExtentPoint32A(hdc, textoBloqueo, (int)strlen(textoBloqueo), &textSize);
        // Centrar texto en el preview
        int textX = destX + (previewW - textSize.cx) / 2;
        int textY = destY + (previewH - textSize.cy) / 2;
        TextOutA(hdc, textX, textY, textoBloqueo, (int)strlen(textoBloqueo));
        SelectObject(hdc, oldFont);
    }

    SelectObject(mem, old);
    DeleteDC(mem);
}