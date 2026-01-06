#include "batallas/batallas.h"
#include "edificios/edificios.h"
#include "guardado/guardado.h"
#include "mapa/mapa.h"
#include "mapa/menu.h"
#include "recursos/navegacion.h"
#include "recursos/recursos.h"
#include "recursos/ui_compra.h"
#include "recursos/ui_embarque.h"
#include "recursos/ui_entrena.h"
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <windowsx.h>

// --- CONFIGURACIÓN ---
#define ZOOM_MAXIMO 6.0f
#define IDT_TIMER_JUEGO 1 // ID para el refresco de lógica (aprox 60fps)

// Variables Globales
Camara camara = {0, 0, 1.0f};
struct Jugador jugador1;
bool arrastrandoCamara = false;
POINT mouseUltimo;
MenuCompra menuCompra;               // Estado global del menú de compra
MenuEmbarque menuEmbarque;           // Menú de embarque de tropas
MenuEntrenamiento menuEntrenamiento; // Estado global del menú de entrenamiento
Edificio ayuntamiento;               // Edificio del ayuntamiento
Edificio mina;                       // Edificio de la mina

// Variables para resaltar celda bajo el cursor
int mouseFilaHover = -1; // Fila de la celda bajo el cursor (-1 = ninguna)
int mouseColHover = -1;  // Columna de la celda bajo el cursor
Edificio cuartel;        // Edificio del cuartel

// Sistema de guardado/pausa
MenuPausa menuPausa;         // Menú de pausa con opciones de guardar/cargar
bool partidaCargada = false; // Indica si se está cargando una partida guardada

// --- MOTOR DE VALIDACIÓN DE CÁMARA ---
void corregirLimitesCamara(RECT rect) {
  int anchoV = rect.right - rect.left;
  int altoV = rect.bottom - rect.top;

  float scaleX = (float)anchoV / (float)MAPA_SIZE;
  float scaleY = (float)altoV / (float)MAPA_SIZE;
  float zMinimo = (scaleX > scaleY) ? scaleX : scaleY;

  if (camara.zoom < zMinimo)
    camara.zoom = zMinimo;
  if (camara.zoom > ZOOM_MAXIMO)
    camara.zoom = ZOOM_MAXIMO;

  int maxW = MAPA_SIZE - (int)(anchoV / camara.zoom);
  int maxH = MAPA_SIZE - (int)(altoV / camara.zoom);

  if (camara.x < 0)
    camara.x = 0;
  if (camara.y < 0)
    camara.y = 0;
  if (camara.x > maxW)
    camara.x = maxW;
  if (camara.y > maxH)
    camara.y = maxH;
}

// ============================================================================
// LÓGICA DE SELECCIÓN PRIORITARIA CON ARITMÉTICA DE PUNTEROS
// ============================================================================
// La selección es INDEPENDIENTE del orden de dibujo (renderizado).
// Aunque un árbol esté dibujado ENCIMA de un obrero, la lógica de selección
// evalúa directamente la posición del obrero en coordenadas del mundo.
// Esto permite seleccionar unidades "a través" de objetos visuales.
// ============================================================================
void seleccionarPersonaje(float mundoX, float mundoY) {
  // Puntero a la estructura del jugador
  struct Jugador *pJugador = &jugador1;

  // ================================================================
  // CONSTANTE: Tamaño de hitbox de las unidades (64x64px)
  // ================================================================
  const float OBRERO_SIZE = 64.0f;

  // Puntero base al array de obreros (para aritmética de punteros)
  Unidad *base = pJugador->obreros;

  // Solo cambiar el estado del que se clickeó
  for (Unidad *o = base; o < base + 6; o++) {
    // ================================================================
    // PUNTO EN RECTÁNGULO (Hitbox 64x64)
    // ================================================================
    // Verificamos si las coordenadas del mouse (mundoX, mundoY)
    // están dentro del rectángulo del obrero:
    //   - Esquina superior izquierda: (o->x, o->y)
    //   - Esquina inferior derecha: (o->x + 64, o->y + 64)
    //
    // CRÍTICO: El obrero mide 64x64px, NO 32x32px (TILE_SIZE)
    // ================================================================

    // Comparación en X: mundoX >= o->x && mundoX < o->x + 64
    bool dentroX = (mundoX >= o->x) && (mundoX < o->x + OBRERO_SIZE);

    // Comparación en Y: mundoY >= o->y && mundoY < o->y + 64
    bool dentroY = (mundoY >= o->y) && (mundoY < o->y + OBRERO_SIZE);

    // Si ambas condiciones son verdaderas, el punto está dentro del hitbox
    // Esto funciona INDEPENDIENTEMENTE de qué se haya dibujado encima
    o->seleccionado = (dentroX && dentroY);
  }

  // SELECCIONAR CABALLEROS CON ESCUDO
  Unidad *baseCaballeros = pJugador->caballeros;
  for (Unidad *c = baseCaballeros; c < baseCaballeros + 4; c++) {
    if (c->x < 0)
      continue;
    float mundoXUnit = c->x;
    float mundoYUnit = c->y;

    bool dentro = (mundoX >= mundoXUnit && mundoX < mundoXUnit + OBRERO_SIZE &&
                   mundoY >= mundoYUnit && mundoY < mundoYUnit + OBRERO_SIZE);

    if (dentro) {
      c->seleccionado = !c->seleccionado;
    } else {
      c->seleccionado = false;
    }
  }

  // SELECCIONAR CABALLEROS SIN ESCUDO
  Unidad *baseCSE = pJugador->caballerosSinEscudo;
  for (Unidad *c = baseCSE; c < baseCSE + 4; c++) {
    if (c->x < 0)
      continue;
    float mundoXUnit = c->x;
    float mundoYUnit = c->y;

    bool dentro = (mundoX >= mundoXUnit && mundoX < mundoXUnit + OBRERO_SIZE &&
                   mundoY >= mundoYUnit && mundoY < mundoYUnit + OBRERO_SIZE);

    if (dentro) {
      c->seleccionado = !c->seleccionado;
    } else {
      c->seleccionado = false;
    }
  }

  // SELECCIONAR GUERREROS
  Unidad *baseGuerreros = pJugador->guerreros;
  for (Unidad *g = baseGuerreros; g < baseGuerreros + 4; g++) {
    if (g->x < 0)
      continue;
    float mundoXUnit = g->x;
    float mundoYUnit = g->y;
    bool dentro = (mundoX >= mundoXUnit && mundoX < mundoXUnit + OBRERO_SIZE &&
                   mundoY >= mundoYUnit && mundoY < mundoYUnit + OBRERO_SIZE);
    if (dentro) {
      g->seleccionado = !g->seleccionado;
    } else {
      g->seleccionado = false;
    }
  }
}

void comandarMovimiento(float mundoX, float mundoY) {
  // Movimiento RTS con pathfinding (en recursos.c)
  rtsComandarMovimiento(&jugador1, mundoX, mundoY);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam,
                            LPARAM lParam) {
  static RECT rect;

  switch (uMsg) {
  case WM_CREATE:
    // Inicializar recursos del jugador y obreros
    IniciacionRecursos(&jugador1, "Jugador 1");
    batallasInicializar();

    // NUEVO: Guardar isla inicial seleccionada
    jugador1.islaActual = menuObtenerIsla(); // 1, 2, o 3
    navegacionRegistrarIslaInicial(jugador1.islaActual);

    // Inicializar ayuntamiento en el centro del mapa (1024, 1024)
    // Tamaño 128x128, centrado en el mapa de 2048x2048
    edificioInicializar(&ayuntamiento, EDIFICIO_AYUNTAMIENTO, 1024.0f - 64.0f,
                        1024.0f - 64.0f);
    jugador1.ayuntamiento = &ayuntamiento;

    // Marcar el ayuntamiento en el mapa de colisiones
    mapaMarcarEdificio(ayuntamiento.x, ayuntamiento.y, ayuntamiento.ancho,
                       ayuntamiento.alto);

    // Registrar en mapaObjetos
    mapaRegistrarObjeto(ayuntamiento.x, ayuntamiento.y, SIMBOLO_EDIFICIO);

    // Inicializar mina en la parte superior de la isla (zona verde)
    // Coordenadas ajustables: (x, y)
    // - x: 960 = centrado horizontalmente en el mapa (2048/2 - 64)
    // - y: 600 = zona superior pero no extrema (ajusta según necesites)
    edificioInicializar(&mina, EDIFICIO_MINA, 1024.0f - 64.0f, 450.0f);
    jugador1.mina = &mina;

    // Marcar la mina en el mapa de colisiones
    mapaMarcarEdificio(mina.x, mina.y, mina.ancho, mina.alto);

    // Registrar en mapaObjetos
    mapaRegistrarObjeto(mina.x, mina.y, SIMBOLO_MINA);

    // Inicializar menú de compra
    menuCompraInicializar(&menuCompra);

    // Inicializar menú de embarque
    menuEmbarqueInicializar(&menuEmbarque);

    // Obtener posición fija del barco según la isla actual
    float barcoX, barcoY;
    int barcoDir;
    navegacionObtenerPosicionBarcoIsla(jugador1.islaActual, &barcoX, &barcoY,
                                       &barcoDir);
    jugador1.barco.x = barcoX;
    jugador1.barco.y = barcoY;
    jugador1.barco.dir = (Direccion)barcoDir;
    jugador1.barco.activo = true;
    jugador1.barco.numTropas = 0;

    printf("[DEBUG] Barco colocado en isla %d: (%.1f, %.1f), dir=%d\n",
           jugador1.islaActual, barcoX, barcoY, barcoDir);

    // Registrar barco en mapaObjetos
    mapaRegistrarObjeto(jugador1.barco.x, jugador1.barco.y, SIMBOLO_BARCO);

    // Inicializar cuartel en la parte inferior del mapa (lado opuesto a la
    // mina) Coordenadas: (960, 1600) - centrado horizontalmente, zona inferior
    edificioInicializar(&cuartel, EDIFICIO_CUARTEL, 1024.0f - 64.0f, 1600.0f);
    jugador1.cuartel = &cuartel;

    // Marcar el cuartel en el mapa de colisiones
    mapaMarcarEdificio(cuartel.x, cuartel.y, cuartel.ancho, cuartel.alto);

    // Registrar cuartel en mapaObjetos
    mapaRegistrarObjeto(cuartel.x, cuartel.y, SIMBOLO_CUARTEL);

    // Inicializar menú de entrenamiento
    menuEntrenamientoInicializar(&menuEntrenamiento);

    // Inicializar menú de pausa/guardado
    menuPausaInicializar(&menuPausa);

    // Si el usuario eligió cargar partida desde el menú principal,
    // abrir automáticamente el menú de pausa en modo carga
    if (partidaCargada) {
      menuPausaAbrir(&menuPausa);
      menuPausa.modo = MODO_CARGAR;
      menuPausa.partidaSeleccionada = 0;
      menuPausa.numPartidas = obtenerPartidasGuardadas(menuPausa.partidas);
    }

    // Timer para actualizar física a 60 FPS (16ms)
    SetTimer(hwnd, IDT_TIMER_JUEGO, 16, NULL);
    return 0;

  case WM_TIMER:
    if (wParam == IDT_TIMER_JUEGO) {
      // Actualizar menú de pausa (timers de mensajes)
      menuPausaActualizar(&menuPausa);

      // Si el menú de pausa está activo, solo actualizar ese menú
      if (menuPausa.activo) {
        InvalidateRect(hwnd, NULL, FALSE);
        return 0;
      }

      if (batallasEnCurso()) {
        batallasActualizar(0.016f);
      } else {
        actualizarPersonajes(&jugador1);
        mapaActualizarVacas(); // NUEVO: Actualizar vacas (movimiento
                               // automático)
        menuCompraActualizar(&menuCompra);

        // Actualizar mina si existe
        if (jugador1.mina != NULL) {
          edificioActualizar((Edificio *)jugador1.mina);
        }

        // Actualizar menú de entrenamiento
        menuEntrenamientoActualizar(&menuEntrenamiento);
      }

      BatallaResultado res;
      int islaRes;
      if (batallasObtenerResultado(&res, &islaRes)) {
        navegacionProcesarResultadoBatalla(&jugador1, res, islaRes);
      }

      InvalidateRect(hwnd, NULL, FALSE);
    }
    return 0;

  case WM_RBUTTONDOWN: {
    int px = GET_X_LPARAM(lParam);
    int py = GET_Y_LPARAM(lParam);

    // Solo procesar en vista local
    if (jugador1.vistaActual == VISTA_LOCAL) {
      float mundoX = (px / camara.zoom) + camara.x;
      float mundoY = (py / camara.zoom) + camara.y;

      // Verificar si se hizo click sobre el barco
      if (barcoContienePunto(&jugador1.barco, mundoX, mundoY)) {
        // Abrir menú de embarque
        GetClientRect(hwnd, &rect);
        menuEmbarqueAbrir(&menuEmbarque, rect.right - rect.left,
                          rect.bottom - rect.top);
      }
      // Verificar si se hizo click sobre el ayuntamiento
      else if (edificioContienePunto(&ayuntamiento, mundoX, mundoY)) {
        // RESTRICCIÓN: Solo obreros pueden interactuar con el ayuntamiento
        // Usamos el centro del ayuntamiento para medir proximidad
        if (recursosObreroCercaDePunto(&jugador1, ayuntamiento.x + 64.0f,
                                       ayuntamiento.y + 64.0f, 200.0f)) {
          // Abrir menú de compra
          GetClientRect(hwnd, &rect);
          menuCompraAbrir(&menuCompra, rect.right - rect.left,
                          rect.bottom - rect.top);
        } else {
          // No hay obrero cerca, mandarlos al centro
          rtsComandarMovimiento(&jugador1, ayuntamiento.x + 64.0f,
                                ayuntamiento.y + 64.0f);
        }
      }
      // Verificar si se hizo click sobre el cuartel
      else if (edificioContienePunto(&cuartel, mundoX, mundoY)) {
        // RESTRICCIÓN: Todos pueden interactuar con el cuartel
        // Usamos el centro del cuartel para medir proximidad
        if (recursosCualquierTropaCercaDePunto(&jugador1, cuartel.x + 64.0f,
                                               cuartel.y + 64.0f, 200.0f)) {
          // Abrir menú de entrenamiento
          GetClientRect(hwnd, &rect);
          menuEntrenamientoAbrir(&menuEntrenamiento, rect.right - rect.left,
                                 rect.bottom - rect.top);
        } else {
          // Mandar al centro
          rtsComandarMovimiento(&jugador1, cuartel.x + 64.0f,
                                cuartel.y + 64.0f);
        }
      }
      // Intentar recoger recursos de la mina
      else if (recursosIntentarRecogerMina(&jugador1, mundoX, mundoY)) {
        // Interacción con mina ya manejada
      } else if (recursosIntentarCazar(&jugador1, mundoX, mundoY)) {
        // Interacción con vaca ya manejada
      } else {
        // 1. Intentar acción de talar (si es árbol y hay obrero cerca)
        if (!recursosIntentarTalar(&jugador1, mundoX, mundoY)) {
          // 2. Comandar movimiento normal
          rtsComandarMovimiento(&jugador1, mundoX, mundoY);
        }
      }
    }
    return 0;
  }

  case WM_KEYDOWN:
    // Primero procesar teclas del menú de pausa (incluye ESC para abrir)
    if (menuPausaProcesarTecla(&menuPausa, wParam, &jugador1, &camara,
                               &ayuntamiento, &mina, &cuartel)) {
      // Verificar si el usuario quiere volver al menú principal
      if (menuPausa.volverAlMenu) {
        // Cerrar la ventana de juego para volver al menú
        DestroyWindow(hwnd);
      }
      InvalidateRect(hwnd, NULL, FALSE);
      return 0;
    }

    if (wParam == 'C') {
      // Centrar cámara en el Ayuntamiento (1024, 1024)
      // Restamos la mitad de la pantalla (aprox 640x360) para que quede al
      // centro
      camara.x = 1024 - (1280 / 2 / camara.zoom);
      camara.y = 1024 - (720 / 2 / camara.zoom);
      corregirLimitesCamara(rect);
      InvalidateRect(hwnd, NULL, FALSE);
    }
    // Tecla 'M' para mostrar matriz en consola (debug)
    if (wParam == 'M') {
      mostrarMapa(mapaObjetos);
    }
    // Tecla 'H' para CURAR unidades seleccionadas (Cuesta Comida)
    if (wParam == 'H') {
      bool alguienCurado = false;
      struct Jugador *j = &jugador1;

      // Lista de punteros a arrays de unidades para iterar
      Unidad *arrays[] = {j->obreros, j->caballeros, j->caballerosSinEscudo,
                          j->guerreros};
      int tamanos[] = {6, 4, 4, 4};

      for (int a = 0; a < 4; a++) {
        for (int i = 0; i < tamanos[a]; i++) {
          Unidad *u = &arrays[a][i];
          if (u->seleccionado && u->x >= 0 && u->vida < u->vidaMax) {
            if (j->Comida >= COSTO_CURACION) {
              j->Comida -= COSTO_CURACION;
              u->vida += CANTIDAD_CURACION;
              if (u->vida > u->vidaMax)
                u->vida = u->vidaMax;
              alguienCurado = true;
            }
          }
        }
      }

      if (alguienCurado) {
        InvalidateRect(hwnd, NULL, FALSE);
      }
    }
    break;

  case WM_CHAR:
    // Procesar entrada de caracteres para el nombre del jugador
    if (menuPausaProcesarCaracter(&menuPausa, wParam)) {
      InvalidateRect(hwnd, NULL, FALSE);
      return 0;
    }
    break;

  case WM_SIZE:
    GetClientRect(hwnd, &rect);
    corregirLimitesCamara(rect);
    return 0;

  case WM_MOUSEWHEEL: {
    int delta = GET_WHEEL_DELTA_WPARAM(wParam);
    POINT pt = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
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
    // Click Izquierdo: SELECCIONAR o interactuar con menú
    int px = GET_X_LPARAM(lParam);
    int py = GET_Y_LPARAM(lParam);

    // Si el menú de embarque está abierto, procesar click
    if (menuEmbarqueClick(&menuEmbarque, &jugador1, px, py)) {
      InvalidateRect(hwnd, NULL, FALSE);
      return 0; // Click procesado por el menú de embarque
    }

    // Si el menú de compra está abierto, procesar click en el menú
    if (menuCompraClick(&menuCompra, &jugador1, px, py)) {
      return 0; // Click procesado por el menú
    }

    // Si el menú de entrenamiento está abierto, procesar click
    if (menuEntrenamientoClick(&menuEntrenamiento, &jugador1, px, py)) {
      return 0; // Click procesado por el menú de entrenamiento
    }

    // Vista local: selección normal
    // Convertir coordenadas de pantalla a coordenadas del mundo real (0-2048)
    float mundoX = (px / camara.zoom) + camara.x;
    float mundoY = (py / camara.zoom) + camara.y;

    seleccionarPersonaje(mundoX, mundoY);

    // También permitir arrastre si se mantiene presionado (opcional)
    arrastrandoCamara = true;
    mouseUltimo.x = px;
    mouseUltimo.y = py;
    SetCapture(hwnd);
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

    // NUEVO: Actualizar celda bajo el cursor
    if (jugador1.vistaActual == VISTA_LOCAL) {
      int px = GET_X_LPARAM(lParam);
      int py = GET_Y_LPARAM(lParam);

      // Convertir coordenadas de pantalla a mundo
      float mundoX = (px / camara.zoom) + camara.x;
      float mundoY = (py / camara.zoom) + camara.y;

      // Convertir a celda de la matriz
      mouseFilaHover = (int)(mundoY / TILE_SIZE);
      mouseColHover = (int)(mundoX / TILE_SIZE);

      // Validar límites
      if (mouseFilaHover < 0 || mouseFilaHover >= GRID_SIZE ||
          mouseColHover < 0 || mouseColHover >= GRID_SIZE) {
        mouseFilaHover = -1;
        mouseColHover = -1;
      }

      InvalidateRect(hwnd, NULL, FALSE); // Redibujar para mostrar highlight
    }
    return 0;

  case WM_LBUTTONUP:
    arrastrandoCamara = false;
    ReleaseCapture();
    return 0;

  case WM_ERASEBKGND:
    return 1; // Indicar que nosotros manejamos el fondo para evitar parpadeo

  case WM_PAINT: {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwnd, &ps);

    // Asegurar que el rect esté actualizado para el culling de los edificios
    GetClientRect(hwnd, &rect);

    if (batallasEnCurso()) {
      batallasRender(hdc, rect, camara);
    } else {
      dibujarMundo(hdc, rect, camara, &jugador1, &menuCompra, &menuEmbarque,
                   mouseFilaHover, mouseColHover);
    }

    // Dibujar menú de pausa como overlay (si está activo)
    if (menuPausa.activo) {
      menuPausaDibujar(hdc, rect, &menuPausa);
    }

    EndPaint(hwnd, &ps);
    return 0;
  }

  case WM_DESTROY:
    KillTimer(hwnd, IDT_TIMER_JUEGO);
    // Limpieza de memoria dinámica (rutas + collisionMap)
    rtsLiberarMovimientoJugador(&jugador1);
    mapaLiberarCollisionMap();
    edificiosLiberarSprites();
    PostQuitMessage(0);
    return 0;
  }
  return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int main() {
  // Registrar clase de ventana una sola vez
  WNDCLASSA wc = {0};
  wc.lpfnWndProc = WindowProc;
  wc.hInstance = GetModuleHandle(NULL);
  wc.lpszClassName = "ClaseGuerraIslas";
  wc.hCursor = LoadCursor(NULL, IDC_ARROW);
  wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
  RegisterClassA(&wc);

  // Loop principal del juego (permite volver al menú)
  bool continuar = true;
  while (continuar) {
    // Mostrar menú principal
    mostrarMenu();

    int accion = menuObtenerAccion();
    if (accion == 3) {
      // Salir del juego
      break;
    }

    // Reiniciar estado global para nueva partida
    partidaCargada = false;
    menuPausaInicializar(&menuPausa);

    // CRITICO: Reiniciar TODOS los globals para que el juego comience desde
    // cero
    memset(&jugador1, 0, sizeof(jugador1));
    camara.x = 0;
    camara.y = 0;
    camara.zoom = 1.0f;
    memset(&ayuntamiento, 0, sizeof(ayuntamiento));
    memset(&mina, 0, sizeof(mina));
    memset(&cuartel, 0, sizeof(cuartel));
    menuCompraInicializar(&menuCompra);
    menuEmbarqueInicializar(&menuEmbarque);
    menuEntrenamientoInicializar(&menuEntrenamiento);
    arrastrandoCamara = false;
    mouseFilaHover = -1;
    mouseColHover = -1;

    // Verificar si el usuario eligió cargar partida
    if (accion == 1) {
      // Cargar partida: establecer flag global
      partidaCargada = true;
      // Seleccionar isla 1 por defecto, se actualizará al cargar
      mapaSeleccionarIsla(1);
    } else {
      // Nueva partida: seleccionar isla normalmente
      mapaSeleccionarIsla(menuObtenerIsla());
    }

    // IMPORTANTE: Cargar sprites ANTES de crear la ventana
    // para que estén disponibles en WM_CREATE
    cargarRecursosGraficos(); // Carga BMPs de mapa, árboles y obreros
    edificiosCargarSprites(); // Carga sprites de edificios

    HWND hwnd = CreateWindowEx(
        0, wc.lpszClassName, "Islas en Guerra - Motor de Unidades",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 1280, 720, NULL,
        NULL, wc.hInstance, NULL);

    if (!hwnd) {
      MessageBoxA(NULL, "Error al crear la ventana del juego", "Error",
                  MB_OK | MB_ICONERROR);
      continue;
    }

    ShowWindow(hwnd, SW_SHOWMAXIMIZED);
    UpdateWindow(hwnd);

    // Loop de mensajes del juego
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }

    // Si se llegó aquí, se cerró la ventana del juego
    // El loop continuará mostrando el menú de nuevo
  }

  return 0;
}