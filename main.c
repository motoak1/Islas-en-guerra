#include "recursos/edificios/edificios.h"
#include "batallas/guardado/guardado.h"
#include "mapa/mapa.h"
#include "mapa/menu.h"
#include "recursos/navegacion.h"
#include "recursos/recursos.h"
#include "recursos/ui_compra.h"
#include "recursos/ui_embarque.h"
#include "recursos/ui_entrena.h"
#include "batallas/batallas.h"
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <windowsx.h>

// --- CONFIGURACIÓN ---
#define ZOOM_MAXIMO 6.0f
#define IDT_TIMER_JUEGO 1 // ID para el refresco de lógica (aprox 60fps)


Camara camara = {0, 0, 1.0f};
struct Jugador jugador1;
bool arrastrandoCamara = false;
POINT mouseUltimo;
MenuCompra menuCompra;
MenuEmbarque menuEmbarque;
MenuEntrenamiento menuEntrenamiento;
Edificio ayuntamiento;
Edificio mina;

// (-1 = ninguna)
int mouseFilaHover = -1;
int mouseColHover = -1;
Edificio cuartel;

MenuPausa menuPausa;
bool partidaCargada = false;

// --- MOTOR DE VALIDACIÓN DE CÁMARA ---
// Esta función asegura que la cámara nunca muestre zonas fuera del mapa.
// Calcula el zoom mínimo para cubrir la pantalla y ajusta (x,y)
// para mantenerse dentro de los límites [0, MAPA_SIZE].
void corregirLimitesCamara(RECT rect) {
  int w = rect.right - rect.left, h = rect.bottom - rect.top;
  float sX = (float)w / MAPA_SIZE, sY = (float)h / MAPA_SIZE;
  float zMin = (sX > sY) ? sX : sY;
  if (camara.zoom < zMin) camara.zoom = zMin;
  if (camara.zoom > ZOOM_MAXIMO) camara.zoom = ZOOM_MAXIMO;
  
  int maxW = MAPA_SIZE - (int)(w / camara.zoom);
  int maxH = MAPA_SIZE - (int)(h / camara.zoom);
  if (camara.x < 0) camara.x = 0; else if (camara.x > maxW) camara.x = maxW;
  if (camara.y < 0) camara.y = 0; else if (camara.y > maxH) camara.y = maxH;
}

// Función auxiliar en línea para colisiones
// Verifica si el punto (mx, my) está dentro del cuadro de 64x64 de la unidad.
static inline bool enAreaContacto(Unidad *u, float mx, float my) {
    return (mx >= u->x && mx < u->x + 64.0f && my >= u->y && my < u->y + 64.0f);
}

// Procesa la selección de un grupo de unidades
// Recorre un array de unidades y verifica si el clic cayó sobre alguna.
// toggle: Si es true, invierte la selección (para shift-click).
void procesarGrupo(Unidad *u, int cant, float mx, float my, bool toggle) {
    for (Unidad *top = u + cant; u < top; u++) {
        if (u->x < 0 || u->vida <= 0) { u->seleccionado = false; continue; }
        bool hit = enAreaContacto(u, mx, my);
        u->seleccionado = hit ? (toggle ? !u->seleccionado : true) : false;
    }
}

void seleccionarPersonaje(float mx, float my) {
    struct Jugador *j = &jugador1;
    procesarGrupo(j->obreros, MAX_OBREROS, mx, my, false); // Obreros: selección directa
    procesarGrupo(j->caballeros, MAX_CABALLEROS, mx, my, true); // Militares: toggle
    procesarGrupo(j->caballerosSinEscudo, MAX_CABALLEROS_SIN_ESCUDO, mx, my, true);
    procesarGrupo(j->guerreros, MAX_GUERREROS, mx, my, true);
}

void comandarMovimiento(float mundoX, float mundoY) {
  // Movimiento RTS con pathfinding (en recursos.c)
  rtsComandarMovimiento(&jugador1, mundoX, mundoY);
}

// Procedimiento principal de ventana (Callback de Windows)
// Maneja todos los eventos del sistema operativo (input, timers, pintado).
LRESULT CALLBACK ProcedimientoVentana(HWND hwnd, UINT uMsg, WPARAM wParam,
                            LPARAM lParam) {
  static RECT rect;

  switch (uMsg) {
  case WM_CREATE:
    // Guardado: reiniciar navegación persistente para nueva sesión
    navegacionReiniciarEstado();
    // Inicializar recursos del jugador y obreros
    IniciacionRecursos(&jugador1, "Jugador 1");

    // Guardar isla inicial seleccionada
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


    // Registrar barco en mapaObjetos
    mapaRegistrarObjeto(jugador1.barco.x, jugador1.barco.y, SIMBOLO_BARCO);

    // Inicializar cuartel en la parte inferior del mapa (lado opuesto a la
    // mina) Coordenadas: (960, 1600) - centrado horizontalmente, zona inferior
    edificioInicializar(&cuartel, EDIFICIO_CUARTEL, 1024.0f - 100.0f, 1400.0f);
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
    // cargar directamente la partida seleccionada
    if (partidaCargada) {
      const char* nombrePartida = menuObtenerNombrePartida();
      if (nombrePartida && nombrePartida[0] != '\0') {
        cargarPartidaPorNombre(nombrePartida, &jugador1, &camara, &ayuntamiento, &mina, &cuartel);
      }
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
      // Actualizaciones de juego cuando no hay menú de pausa
      actualizarPersonajes(&jugador1);
      mapaActualizarVacas();
      mapaActualizarArboles();
      menuCompraActualizar(&menuCompra);

      // Actualizar mina si existe
      if (jugador1.mina != NULL) {
        edificioActualizar((Edificio *)jugador1.mina);
      }

      // Actualizar menú de entrenamiento
      menuEntrenamientoActualizar(&menuEntrenamiento);

      // Actualizar batallas (enemigos y combates al viajar)
      simularBatalla(&jugador1);

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


      if (barcoContienePunto(&jugador1.barco, mundoX, mundoY)) {
        // Solo abrir menú de embarque si el barco está construido
        if (jugador1.barco.construido) {
          GetClientRect(hwnd, &rect);
          menuEmbarqueAbrir(&menuEmbarque, rect.right - rect.left,
                            rect.bottom - rect.top);
        }
        // Si está destruido, no hacer nada (el jugador debe construirlo primero)
      }
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
      else if (edificioContienePunto(&cuartel, mundoX, mundoY)) {
        // RESTRICCIÓN: Solo OBREROS pueden interactuar con el cuartel
        // (igual que el ayuntamiento - los soldados no entrenan tropas)
        if (recursosObreroCercaDePunto(&jugador1, cuartel.x + 64.0f,
                                       cuartel.y + 64.0f, 200.0f)) {
          // Abrir menú de entrenamiento
          GetClientRect(hwnd, &rect);
          menuEntrenamientoAbrir(&menuEntrenamiento, rect.right - rect.left,
                                 rect.bottom - rect.top);
        } else {
          // Mandar obreros seleccionados al cuartel
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
        batallasReiniciarEstado();
        // Guardado: limpiar snapshots antes de volver al menú principal
        navegacionReiniciarEstado();
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
    // Tecla 'H' para CURAR unidades seleccionadas (Cuesta Comida)
    if (wParam == 'H') {
      bool curado = false;
      struct Jugador *j = &jugador1;
      Unidad *arrs[] = {j->obreros, j->caballeros, j->caballerosSinEscudo, j->guerreros};
      int maxs[] = {MAX_OBREROS, MAX_CABALLEROS, MAX_CABALLEROS_SIN_ESCUDO, MAX_GUERREROS};

      for (int k = 0; k < 4; k++) {
        Unidad *u = arrs[k], *end = u + maxs[k];
        for (; u < end; u++) {
            if (u->seleccionado && u->x >= 0 && u->vida < u->vidaMax && j->Comida >= COSTO_CURACION) {
                j->Comida -= COSTO_CURACION;
                u->vida += (int)(u->vidaMax * CANTIDAD_CURACION / 100.0f);
                if (u->vida > u->vidaMax) u->vida = u->vidaMax;
                curado = true;
            }
        }
      }
      if (curado) InvalidateRect(hwnd, NULL, FALSE);
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

    // Dibujar siempre el mundo (sin escena independiente)
    // El menú de pausa se dibuja dentro del buffer para evitar parpadeo
    dibujarMundo(hdc, rect, camara, &jugador1, &menuCompra, &menuEmbarque,
                 mouseFilaHover, mouseColHover, &menuPausa);

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
  wc.lpfnWndProc = ProcedimientoVentana;
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