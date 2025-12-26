#include <windows.h>
#include <windowsx.h>
#include <stdbool.h>
#include <math.h>
#include "mapa/mapa.h"
#include "mapa/menu.h"
#include "recursos/recursos.h"

// --- CONFIGURACIÓN ---
#define ZOOM_MAXIMO 6.0f
#define IDT_TIMER_JUEGO 1  // ID para el refresco de lógica (aprox 60fps)

// Variables Globales
Camara camara = {0, 0, 1.0f};
struct Jugador jugador1; 
bool arrastrandoCamara = false;
POINT mouseUltimo;

// --- MOTOR DE VALIDACIÓN DE CÁMARA ---
void corregirLimitesCamara(RECT rect) {
    int anchoV = rect.right - rect.left;
    int altoV = rect.bottom - rect.top;
    
    float scaleX = (float)anchoV / (float)MAPA_SIZE;
    float scaleY = (float)altoV / (float)MAPA_SIZE;
    float zMinimo = (scaleX > scaleY) ? scaleX : scaleY;

    if (camara.zoom < zMinimo) camara.zoom = zMinimo;
    if (camara.zoom > ZOOM_MAXIMO) camara.zoom = ZOOM_MAXIMO;

    int maxW = MAPA_SIZE - (int)(anchoV / camara.zoom);
    int maxH = MAPA_SIZE - (int)(altoV / camara.zoom);

    if (camara.x < 0) camara.x = 0;
    if (camara.y < 0) camara.y = 0;
    if (camara.x > maxW) camara.x = maxW;
    if (camara.y > maxH) camara.y = maxH;
}

// --- LÓGICA DE SELECCIÓN Y MOVIMIENTO CON PUNTEROS ---
void seleccionarObrero(float mundoX, float mundoY) {
    // Uso de puntero para recorrer la estructura del jugador
    struct Jugador *pJugador = &jugador1;
    for (int i = 0; i < 6; i++) {
        UnidadObrero *o = &(pJugador->obreros[i]);
        float dx = mundoX - o->x;
        float dy = mundoY - o->y;
        float distancia = sqrt(dx*dx + dy*dy);

        // Si el click está dentro del rango del sprite (64px aprox)
        o->seleccionado = (distancia < 32.0f); 
    }
}

void comandarMovimiento(float mundoX, float mundoY) {
    struct Jugador *pJugador = &jugador1;
    for (int i = 0; i < 6; i++) {
        UnidadObrero *o = &(pJugador->obreros[i]);
        if (o->seleccionado) {
            o->destinoX = mundoX;
            o->destinoY = mundoY;
            o->moviendose = true;
        }
    }
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static RECT rect;

    switch (uMsg) {
        case WM_CREATE:
            // Inicializar recursos del jugador y obreros
            IniciacionRecursos(&jugador1, "Jugador 1");
            // Timer para actualizar física a 60 FPS (16ms)
            SetTimer(hwnd, IDT_TIMER_JUEGO, 16, NULL);
            return 0;

        case WM_TIMER:
            if (wParam == IDT_TIMER_JUEGO) {
                actualizarObreros(&jugador1); // Mueve a los obreros
                InvalidateRect(hwnd, NULL, FALSE); // Redibuja la pantalla
            }
            return 0;

        case WM_SIZE:
            GetClientRect(hwnd, &rect);
            corregirLimitesCamara(rect);
            return 0;

        case WM_MOUSEWHEEL: {
            int delta = GET_WHEEL_DELTA_WPARAM(wParam);
            POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
            ScreenToClient(hwnd, &pt);
            
            float zoomViejo = camara.zoom;
            camara.zoom *= (delta > 0) ? 1.1f : 0.9f;
            corregirLimitesCamara(rect);

            camara.x = (int)((camara.x + (pt.x / zoomViejo)) - (pt.x / camara.zoom));
            camara.y = (int)((camara.y + (pt.y / zoomViejo)) - (pt.y / camara.zoom));
            
            corregirLimitesCamara(rect);
            InvalidateRect(hwnd, NULL, FALSE);
            return 0;
        }

        case WM_LBUTTONDOWN: {
            // Click Izquierdo: SELECCIONAR
            int px = GET_X_LPARAM(lParam);
            int py = GET_Y_LPARAM(lParam);
            
            // Convertir coordenadas de pantalla a coordenadas del mundo real (0-2048)
            float mundoX = (px / camara.zoom) + camara.x;
            float mundoY = (py / camara.zoom) + camara.y;

            seleccionarObrero(mundoX, mundoY);
            
            // También permitir arrastre si se mantiene presionado (opcional)
            arrastrandoCamara = true; 
            mouseUltimo.x = px;
            mouseUltimo.y = py;
            SetCapture(hwnd);
            return 0;
        }

        case WM_RBUTTONDOWN: {
            // Click Derecho: MOVER seleccionados
            int px = GET_X_LPARAM(lParam);
            int py = GET_Y_LPARAM(lParam);

            float mundoX = (px / camara.zoom) + camara.x;
            float mundoY = (py / camara.zoom) + camara.y;

            comandarMovimiento(mundoX, mundoY);
            return 0;
        }

        case WM_MOUSEMOVE:
            if (arrastrandoCamara && (wParam & MK_LBUTTON)) {
                int curX = GET_X_LPARAM(lParam);
                int curY = GET_Y_LPARAM(lParam);
                
                camara.x -= (int)((curX - mouseUltimo.x) / camara.zoom);
                camara.y -= (int)((curY - mouseUltimo.y) / camara.zoom);
                
                corregirLimitesCamara(rect);
                mouseUltimo.x = curX;
                mouseUltimo.y = curY;
            }
            return 0;

        case WM_LBUTTONUP:
            arrastrandoCamara = false;
            ReleaseCapture();
            return 0;

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            // Ahora pasamos &jugador1 como puntero a la función de dibujo
            dibujarMundo(hdc, rect, camara, &jugador1); 
            
            EndPaint(hwnd, &ps);
            return 0;
        }

        case WM_DESTROY:
            KillTimer(hwnd, IDT_TIMER_JUEGO);
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int main() {
    mostrarMenu(); //
    if (menuObtenerAccion() == 3) return 0; // Salir si el usuario lo pide

    WNDCLASS wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = "ClaseGuerraIslas";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(0, wc.lpszClassName, "Islas en Guerra - Motor de Unidades", 
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 1280, 720, NULL, NULL, wc.hInstance, NULL);

    cargarRecursosGraficos(); // Carga BMPs de mapa, árboles y obreros

    ShowWindow(hwnd, SW_SHOWMAXIMIZED);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}