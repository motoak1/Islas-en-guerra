// guardado/guardado.c
// Implementación del sistema de guardado en archivos binarios
// Guardado único por nombre de jugador - Sin parpadeo
#include "guardado.h"
#include "../mapa/mapa.h"
#include "../recursos/recursos.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <direct.h>
#include <ctype.h>

// ============================================================================
// FUNCIONES AUXILIARES INTERNAS
// ============================================================================

// Construye la ruta completa del archivo de guardado por nombre
void obtenerRutaGuardado(const char *nombreJugador, char *ruta, int maxLen) {
    // Sanitizar nombre (quitar caracteres inválidos para archivos)
    char nombreSanitizado[MAX_NOMBRE_JUGADOR];
    int j = 0;
    for (int i = 0; nombreJugador[i] != '\0' && i < MAX_NOMBRE_JUGADOR - 1; i++) {
        char c = nombreJugador[i];
        // Solo permitir letras, números, guiones y guiones bajos
        if (isalnum((unsigned char)c) || c == '-' || c == '_') {
            nombreSanitizado[j++] = c;
        } else if (c == ' ') {
            nombreSanitizado[j++] = '_'; // Reemplazar espacios por guiones bajos
        }
    }
    nombreSanitizado[j] = '\0';
    
    if (j == 0) {
        strcpy(nombreSanitizado, "partida");
    }
    
    snprintf(ruta, maxLen, "%s\\%s%s", SAVE_FOLDER, nombreSanitizado, SAVE_EXTENSION);
}

// Obtiene la fecha y hora actual en formato legible
static void obtenerTimestamp(char *buffer, int maxLen) {
    time_t ahora = time(NULL);
    struct tm *local = localtime(&ahora);
    strftime(buffer, maxLen, "%d/%m/%Y %H:%M", local);
}

// Convierte una Unidad a UnidadGuardada (elimina punteros)
static void unidadAGuardada(const Unidad *src, UnidadGuardada *dst) {
    dst->x = src->x;
    dst->y = src->y;
    dst->destinoX = src->destinoX;
    dst->destinoY = src->destinoY;
    dst->moviendose = src->moviendose;
    dst->seleccionado = src->seleccionado;
    dst->dir = (int)src->dir;
    dst->frame = src->frame;
    dst->objetivoFila = src->objetivoFila;
    dst->objetivoCol = src->objetivoCol;
    dst->celdaFila = src->celdaFila;
    dst->celdaCol = src->celdaCol;
    dst->tipo = (int)src->tipo;
    dst->vida = src->vida;
    dst->vidaMax = src->vidaMax;
    dst->damage = src->damage;
    dst->critico = src->critico;
    dst->defensa = src->defensa;
    dst->alcance = src->alcance;
}

// Convierte UnidadGuardada a Unidad (restaura estructura)
static void guardadaAUnidad(const UnidadGuardada *src, Unidad *dst) {
    dst->x = src->x;
    dst->y = src->y;
    dst->destinoX = src->destinoX;
    dst->destinoY = src->destinoY;
    dst->moviendose = src->moviendose;
    dst->seleccionado = src->seleccionado;
    dst->dir = (Direccion)src->dir;
    dst->frame = src->frame;
    dst->objetivoFila = src->objetivoFila;
    dst->objetivoCol = src->objetivoCol;
    // Liberar ruta anterior si existe
    if (dst->rutaCeldas != NULL) {
        free(dst->rutaCeldas);
        dst->rutaCeldas = NULL;
    }
    dst->rutaLen = 0;
    dst->rutaIdx = 0;
    dst->celdaFila = src->celdaFila;
    dst->celdaCol = src->celdaCol;
    dst->animActual = NULL;
    dst->animTick = 0;
    dst->tipo = (TipoUnidad)src->tipo;
    dst->vida = src->vida;
    dst->vidaMax = src->vidaMax;
    dst->damage = src->damage;
    dst->critico = src->critico;
    dst->defensa = src->defensa;
    dst->alcance = src->alcance;
}

// Convierte un Edificio a EdificioGuardado
static void edificioAGuardado(const Edificio *src, EdificioGuardado *dst) {
    dst->tipo = (int)src->tipo;
    dst->x = src->x;
    dst->y = src->y;
    dst->ancho = src->ancho;
    dst->alto = src->alto;
    dst->construido = src->construido;
    dst->oroAcumulado = src->oroAcumulado;
    dst->piedraAcumulada = src->piedraAcumulada;
    dst->hierroAcumulado = src->hierroAcumulado;
}

// Convierte EdificioGuardado a Edificio
static void guardadoAEdificio(const EdificioGuardado *src, Edificio *dst) {
    dst->tipo = (TipoEdificio)src->tipo;
    dst->x = src->x;
    dst->y = src->y;
    dst->ancho = src->ancho;
    dst->alto = src->alto;
    dst->construido = src->construido;
    dst->oroAcumulado = src->oroAcumulado;
    dst->piedraAcumulada = src->piedraAcumulada;
    dst->hierroAcumulado = src->hierroAcumulado;
    dst->sprite = NULL;
    dst->ultimoTickGeneracion = GetTickCount();
}

// ============================================================================
// FUNCIONES PÚBLICAS DE GUARDADO/CARGA
// ============================================================================

bool existePartida(const char *nombreJugador) {
    char ruta[256];
    obtenerRutaGuardado(nombreJugador, ruta, sizeof(ruta));
    
    FILE *f = fopen(ruta, "rb");
    if (f) {
        fclose(f);
        return true;
    }
    return false;
}

bool guardarPartidaPorNombre(const char *nombreJugador, struct Jugador *j, Camara *cam) {
    if (nombreJugador == NULL || nombreJugador[0] == '\0') {
        printf("[ERROR] Nombre de jugador vacío\n");
        return false;
    }
    
    // Crear carpeta de guardados si no existe
    _mkdir(SAVE_FOLDER);
    
    char ruta[256];
    obtenerRutaGuardado(nombreJugador, ruta, sizeof(ruta));
    
    FILE *f = fopen(ruta, "wb");
    if (!f) {
        printf("[ERROR] No se pudo crear archivo de guardado: %s\n", ruta);
        return false;
    }
    
    DatosGuardado datos;
    memset(&datos, 0, sizeof(datos));
    
    // --- Header ---
    datos.header.magic = SAVE_MAGIC;
    datos.header.version = SAVE_VERSION;
    obtenerTimestamp(datos.header.timestamp, sizeof(datos.header.timestamp));
    strncpy(datos.header.nombreJugador, nombreJugador, sizeof(datos.header.nombreJugador) - 1);
    datos.header.islaActual = j->islaActual;
    
    // --- Recursos ---
    datos.Comida = j->Comida;
    datos.Oro = j->Oro;
    datos.Madera = j->Madera;
    datos.Piedra = j->Piedra;
    datos.Hierro = j->Hierro;
    
    // --- Unidades (con aritmética de punteros) ---
    for (int i = 0; i < 6; i++) {
        unidadAGuardada(j->obreros + i, datos.obreros + i);
    }
    for (int i = 0; i < 4; i++) {
        unidadAGuardada(j->caballeros + i, datos.caballeros + i);
    }
    for (int i = 0; i < 4; i++) {
        unidadAGuardada(j->caballerosSinEscudo + i, datos.caballerosSinEscudo + i);
    }
    for (int i = 0; i < 4; i++) {
        unidadAGuardada(j->guerreros + i, datos.guerreros + i);
    }
    
    // --- Barco ---
    datos.barco.x = j->barco.x;
    datos.barco.y = j->barco.y;
    datos.barco.dir = (int)j->barco.dir;
    datos.barco.activo = j->barco.activo;
    datos.barco.numTropas = j->barco.numTropas;
    
    for (int i = 0; i < 6; i++) {
        datos.barco.indiceTropas[i] = -1;
        datos.barco.tipoTropas[i] = -1;
        
        if (i < j->barco.numTropas && j->barco.tropas[i] != NULL) {
            Unidad *tropa = j->barco.tropas[i];
            datos.barco.tipoTropas[i] = (int)tropa->tipo;
            
            switch (tropa->tipo) {
                case TIPO_OBRERO:
                    for (int k = 0; k < 6; k++) {
                        if (tropa == j->obreros + k) {
                            datos.barco.indiceTropas[i] = k;
                            break;
                        }
                    }
                    break;
                case TIPO_CABALLERO:
                    for (int k = 0; k < 4; k++) {
                        if (tropa == j->caballeros + k) {
                            datos.barco.indiceTropas[i] = k;
                            break;
                        }
                    }
                    break;
                case TIPO_CABALLERO_SIN_ESCUDO:
                    for (int k = 0; k < 4; k++) {
                        if (tropa == j->caballerosSinEscudo + k) {
                            datos.barco.indiceTropas[i] = k;
                            break;
                        }
                    }
                    break;
                case TIPO_GUERRERO:
                    for (int k = 0; k < 4; k++) {
                        if (tropa == j->guerreros + k) {
                            datos.barco.indiceTropas[i] = k;
                            break;
                        }
                    }
                    break;
                default:
                    break;
            }
        }
    }
    
    // --- Edificios ---
    datos.tieneAyuntamiento = (j->ayuntamiento != NULL);
    if (datos.tieneAyuntamiento) {
        edificioAGuardado((Edificio *)j->ayuntamiento, &datos.ayuntamiento);
    }
    
    datos.tieneMina = (j->mina != NULL);
    if (datos.tieneMina) {
        edificioAGuardado((Edificio *)j->mina, &datos.mina);
    }
    
    datos.tieneCuartel = (j->cuartel != NULL);
    if (datos.tieneCuartel) {
        edificioAGuardado((Edificio *)j->cuartel, &datos.cuartel);
    }
    
    // --- Estado de vista ---
    datos.vistaActual = (int)j->vistaActual;
    datos.islaActual = j->islaActual;
    
    // --- Mapa de objetos ---
    memcpy(datos.mapaObjetosGuardado, mapaObjetos, sizeof(mapaObjetos));
    
    // --- Vacas ---
    Vaca *vacas = mapaObtenerVacas(&datos.numVacas);
    if (datos.numVacas > 10) datos.numVacas = 10;
    for (int i = 0; i < datos.numVacas; i++) {
        datos.vacas[i].x = (vacas + i)->x;
        datos.vacas[i].y = (vacas + i)->y;
        datos.vacas[i].dir = (int)(vacas + i)->dir;
        datos.vacas[i].timerMovimiento = (vacas + i)->timerMovimiento;
    }
    
    // --- Cámara ---
    datos.camaraX = cam->x;
    datos.camaraY = cam->y;
    datos.camaraZoom = cam->zoom;
    
    // Escribir todo en binario
    size_t escritos = fwrite(&datos, sizeof(DatosGuardado), 1, f);
    fclose(f);
    
    if (escritos != 1) {
        printf("[ERROR] Error al escribir datos de guardado\n");
        return false;
    }
    
    printf("[GUARDADO] Partida guardada: %s\n", ruta);
    return true;
}

bool cargarPartidaPorNombre(const char *nombreJugador, struct Jugador *j, Camara *cam,
                            Edificio *ayuntamiento, Edificio *mina, Edificio *cuartel) {
    char ruta[256];
    obtenerRutaGuardado(nombreJugador, ruta, sizeof(ruta));
    
    FILE *f = fopen(ruta, "rb");
    if (!f) {
        printf("[ERROR] No se pudo abrir archivo de guardado: %s\n", ruta);
        return false;
    }
    
    DatosGuardado datos;
    size_t leidos = fread(&datos, sizeof(DatosGuardado), 1, f);
    fclose(f);
    
    if (leidos != 1) {
        printf("[ERROR] Error al leer datos de guardado\n");
        return false;
    }
    
    if (datos.header.magic != SAVE_MAGIC) {
        printf("[ERROR] Archivo de guardado corrupto o inválido\n");
        return false;
    }
    
    // --- Recursos ---
    j->Comida = datos.Comida;
    j->Oro = datos.Oro;
    j->Madera = datos.Madera;
    j->Piedra = datos.Piedra;
    j->Hierro = datos.Hierro;
    strncpy(j->Nombre, datos.header.nombreJugador, sizeof(j->Nombre) - 1);
    
    // --- Unidades ---
    for (int i = 0; i < 6; i++) {
        guardadaAUnidad(datos.obreros + i, j->obreros + i);
    }
    for (int i = 0; i < 4; i++) {
        guardadaAUnidad(datos.caballeros + i, j->caballeros + i);
    }
    for (int i = 0; i < 4; i++) {
        guardadaAUnidad(datos.caballerosSinEscudo + i, j->caballerosSinEscudo + i);
    }
    for (int i = 0; i < 4; i++) {
        guardadaAUnidad(datos.guerreros + i, j->guerreros + i);
    }
    
    // --- Barco ---
    j->barco.x = datos.barco.x;
    j->barco.y = datos.barco.y;
    j->barco.dir = (Direccion)datos.barco.dir;
    j->barco.activo = datos.barco.activo;
    j->barco.numTropas = datos.barco.numTropas;
    
    for (int i = 0; i < 6; i++) {
        j->barco.tropas[i] = NULL;
        
        if (i < datos.barco.numTropas && datos.barco.indiceTropas[i] >= 0) {
            int idx = datos.barco.indiceTropas[i];
            switch (datos.barco.tipoTropas[i]) {
                case TIPO_OBRERO:
                    if (idx < 6) j->barco.tropas[i] = j->obreros + idx;
                    break;
                case TIPO_CABALLERO:
                    if (idx < 4) j->barco.tropas[i] = j->caballeros + idx;
                    break;
                case TIPO_CABALLERO_SIN_ESCUDO:
                    if (idx < 4) j->barco.tropas[i] = j->caballerosSinEscudo + idx;
                    break;
                case TIPO_GUERRERO:
                    if (idx < 4) j->barco.tropas[i] = j->guerreros + idx;
                    break;
                default:
                    break;
            }
        }
    }
    
    // --- Edificios ---
    if (datos.tieneAyuntamiento && ayuntamiento != NULL) {
        guardadoAEdificio(&datos.ayuntamiento, ayuntamiento);
        ayuntamiento->sprite = g_spriteAyuntamiento;
        j->ayuntamiento = ayuntamiento;
    } else {
        j->ayuntamiento = NULL;
    }
    
    if (datos.tieneMina && mina != NULL) {
        guardadoAEdificio(&datos.mina, mina);
        mina->sprite = g_spriteMina;
        j->mina = mina;
    } else {
        j->mina = NULL;
    }
    
    if (datos.tieneCuartel && cuartel != NULL) {
        guardadoAEdificio(&datos.cuartel, cuartel);
        cuartel->sprite = g_spriteCuartel;
        j->cuartel = cuartel;
    } else {
        j->cuartel = NULL;
    }
    
    // --- Estado de vista ---
    j->vistaActual = (EstadoVista)datos.vistaActual;
    j->islaActual = datos.islaActual;
    
    // --- Seleccionar isla y recargar mapa ---
    mapaSeleccionarIsla(datos.islaActual);
    mapaSetGenerarRecursos(false);
    cargarRecursosGraficos();
    mapaSetGenerarRecursos(true);
    
    // --- Restaurar mapa de objetos ---
    memcpy(mapaObjetos, datos.mapaObjetosGuardado, sizeof(mapaObjetos));
    
    // --- Reconstruir mapa de colisiones ---
    mapaReconstruirCollisionMap();
    
    if (j->ayuntamiento != NULL) {
        Edificio *e = (Edificio *)j->ayuntamiento;
        mapaMarcarEdificio(e->x, e->y, e->ancho, e->alto);
    }
    if (j->mina != NULL) {
        Edificio *e = (Edificio *)j->mina;
        mapaMarcarEdificio(e->x, e->y, e->ancho, e->alto);
    }
    if (j->cuartel != NULL) {
        Edificio *e = (Edificio *)j->cuartel;
        mapaMarcarEdificio(e->x, e->y, e->ancho, e->alto);
    }
    
    // --- Cámara ---
    cam->x = datos.camaraX;
    cam->y = datos.camaraY;
    cam->zoom = datos.camaraZoom;
    
    printf("[CARGA] Partida cargada: %s (Isla %d)\n", nombreJugador, j->islaActual);
    return true;
}

int obtenerPartidasGuardadas(InfoPartida partidas[MAX_PARTIDAS]) {
    int count = 0;
    WIN32_FIND_DATAA findData;
    char busqueda[256];
    
    // Inicializar array
    for (int i = 0; i < MAX_PARTIDAS; i++) {
        partidas[i].existe = false;
    }
    
    snprintf(busqueda, sizeof(busqueda), "%s\\*%s", SAVE_FOLDER, SAVE_EXTENSION);
    
    HANDLE hFind = FindFirstFileA(busqueda, &findData);
    if (hFind == INVALID_HANDLE_VALUE) {
        return 0;
    }
    
    do {
        if (count >= MAX_PARTIDAS) break;
        
        char rutaCompleta[256];
        snprintf(rutaCompleta, sizeof(rutaCompleta), "%s\\%s", SAVE_FOLDER, findData.cFileName);
        
        FILE *f = fopen(rutaCompleta, "rb");
        if (f) {
            SaveHeader header;
            if (fread(&header, sizeof(SaveHeader), 1, f) == 1) {
                if (header.magic == SAVE_MAGIC) {
                    partidas[count].existe = true;
                    strncpy(partidas[count].nombreJugador, header.nombreJugador, 
                            sizeof(partidas[count].nombreJugador) - 1);
                    strncpy(partidas[count].timestamp, header.timestamp,
                            sizeof(partidas[count].timestamp) - 1);
                    partidas[count].islaActual = header.islaActual;
                    strncpy(partidas[count].rutaArchivo, rutaCompleta,
                            sizeof(partidas[count].rutaArchivo) - 1);
                    count++;
                }
            }
            fclose(f);
        }
    } while (FindNextFileA(hFind, &findData) != 0);
    
    FindClose(hFind);
    return count;
}

bool eliminarPartida(const char *nombreJugador) {
    char ruta[256];
    obtenerRutaGuardado(nombreJugador, ruta, sizeof(ruta));
    return (remove(ruta) == 0);
}

// ============================================================================
// MENÚ DE PAUSA EN PANTALLA (GDI) - Sin parpadeo
// ============================================================================

static const char *OPCIONES_PAUSA[] = {
    "Continuar",
    "Guardar partida",
    "Cargar partida",
    "Salir al menu"
};
static const int OPCIONES_PAUSA_TOTAL = 4;

void menuPausaInicializar(MenuPausa *menu) {
    menu->activo = false;
    menu->seleccion = 0;
    menu->modo = MODO_PRINCIPAL;
    menu->nombreInput[0] = '\0';
    menu->cursorPos = 0;
    menu->nombreExiste = false;
    menu->numPartidas = 0;
    menu->partidaSeleccionada = 0;
    menu->mensaje[0] = '\0';
    menu->timerMensaje = 0;
    menu->rutaGuardado[0] = '\0';
    menu->volverAlMenu = false;
    menu->partidaGuardada = false;
}

void menuPausaAbrir(MenuPausa *menu) {
    menu->activo = true;
    menu->seleccion = 0;
    menu->modo = MODO_PRINCIPAL;
    menu->nombreInput[0] = '\0';
    menu->cursorPos = 0;
    menu->nombreExiste = false;
    menu->volverAlMenu = false;
}

void menuPausaCerrar(MenuPausa *menu) {
    menu->activo = false;
    menu->modo = MODO_PRINCIPAL;
}

void menuPausaActualizar(MenuPausa *menu) {
    if (menu->timerMensaje > 0) {
        menu->timerMensaje--;
        if (menu->timerMensaje == 0) {
            menu->mensaje[0] = '\0';
            menu->rutaGuardado[0] = '\0';
        }
    }
}

// Dibuja un panel con fondo sólido (sin transparencia para evitar parpadeo)
static void dibujarPanelSolido(HDC hdc, int x, int y, int ancho, int alto) {
    // Fondo sólido oscuro
    HBRUSH brushFondo = CreateSolidBrush(RGB(25, 25, 35));
    RECT rectFondo = {x, y, x + ancho, y + alto};
    FillRect(hdc, &rectFondo, brushFondo);
    DeleteObject(brushFondo);
    
    // Borde dorado
    HPEN penBorde = CreatePen(PS_SOLID, 3, RGB(180, 140, 60));
    HPEN oldPen = (HPEN)SelectObject(hdc, penBorde);
    HBRUSH nullBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
    HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, nullBrush);
    
    RoundRect(hdc, x, y, x + ancho, y + alto, 15, 15);
    
    SelectObject(hdc, oldPen);
    SelectObject(hdc, oldBrush);
    DeleteObject(penBorde);
}

void menuPausaDibujar(HDC hdcBuffer, RECT rect, MenuPausa *menu) {
    if (!menu->activo) return;
    
    int anchoV = rect.right - rect.left;
    int altoV = rect.bottom - rect.top;
    
    // NO dibujar overlay oscuro - permitir que el mapa se vea detras del panel
    
    // Determinar tamaño del panel según el modo
    int panelAncho = 580;  // Aumentado de 450 a 580 para que quepa todo el texto
    int panelAlto = 340;   // Aumentado de 320 a 340
    
    if (menu->modo == MODO_GUARDAR || menu->modo == MODO_PRINCIPAL + 10) {
        panelAlto = 340;   // Aumentado de 310 a 340 para evitar superposición
    } else if (menu->modo == MODO_CARGAR) {
        panelAlto = 380;   // Aumentado de 360 a 380
    } else if (menu->modo == MODO_CONFIRMAR_SALIR) {
        panelAlto = 220;   // Aumentado de 200 a 220
    }
    
    int panelX = (anchoV - panelAncho) / 2;
    int panelY = (altoV - panelAlto) / 2;
    
    dibujarPanelSolido(hdcBuffer, panelX, panelY, panelAncho, panelAlto);
    
    // Fuentes
    HFONT fontTitulo = CreateFontA(32, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                                   DEFAULT_CHARSET, OUT_OUTLINE_PRECIS,
                                   CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                   FF_DONTCARE, "Segoe UI");
    HFONT fontOpciones = CreateFontA(22, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                     DEFAULT_CHARSET, OUT_OUTLINE_PRECIS,
                                     CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                     FF_DONTCARE, "Segoe UI");
    HFONT fontPequena = CreateFontA(17, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                    DEFAULT_CHARSET, OUT_OUTLINE_PRECIS,
                                    CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                    FF_DONTCARE, "Segoe UI");
    
    HFONT oldFont = (HFONT)SelectObject(hdcBuffer, fontTitulo);
    SetBkMode(hdcBuffer, TRANSPARENT);
    SetTextColor(hdcBuffer, RGB(220, 180, 80));
    
    const char *titulo = "";
    switch (menu->modo) {
        case MODO_PRINCIPAL: titulo = "PAUSA"; break;
        case MODO_GUARDAR: titulo = "GUARDAR PARTIDA"; break;
        case MODO_CARGAR: titulo = "CARGAR PARTIDA"; break;
        case MODO_CONFIRMAR_SALIR: titulo = "SALIR AL MENU"; break;
        default:
            if (menu->modo == MODO_PRINCIPAL + 10) {
                titulo = "NUEVA PARTIDA";
            }
            break;
    }
    
    SIZE tituloSize;
    GetTextExtentPoint32A(hdcBuffer, titulo, strlen(titulo), &tituloSize);
    TextOutA(hdcBuffer, panelX + (panelAncho - tituloSize.cx) / 2, panelY + 20, 
             titulo, strlen(titulo));
    
    SelectObject(hdcBuffer, fontOpciones);
    int inicioY = panelY + 70;
    
    if (menu->modo == MODO_PRINCIPAL) {
        // Menú principal de pausa
        for (int i = 0; i < OPCIONES_PAUSA_TOTAL; i++) {
            COLORREF color = (i == menu->seleccion) ? RGB(100, 255, 120) : RGB(200, 200, 200);
            SetTextColor(hdcBuffer, color);
            
            const char *texto = OPCIONES_PAUSA[i];
            SIZE textoSize;
            GetTextExtentPoint32A(hdcBuffer, texto, strlen(texto), &textoSize);
            
            int textoX = panelX + (panelAncho - textoSize.cx) / 2;
            int textoY = inicioY + i * 45;
            
            if (i == menu->seleccion) {
                HPEN penHighlight = CreatePen(PS_SOLID, 2, RGB(80, 200, 100));
                HPEN oldP = (HPEN)SelectObject(hdcBuffer, penHighlight);
                HBRUSH nullB = (HBRUSH)GetStockObject(NULL_BRUSH);
                HBRUSH oldB = (HBRUSH)SelectObject(hdcBuffer, nullB);
                
                RoundRect(hdcBuffer, textoX - 15, textoY - 5,
                          textoX + textoSize.cx + 15, textoY + textoSize.cy + 5, 8, 8);
                
                SelectObject(hdcBuffer, oldP);
                SelectObject(hdcBuffer, oldB);
                DeleteObject(penHighlight);
            }
            
            TextOutA(hdcBuffer, textoX, textoY, texto, strlen(texto));
        }
    } else if (menu->modo == MODO_GUARDAR) {
        // Pantalla de guardar - mostrar lista de partidas guardadas
        if (menu->numPartidas == 0) {
            SetTextColor(hdcBuffer, RGB(180, 180, 180));
            const char *noHay = "No hay partidas guardadas";
            SIZE nhSize;
            GetTextExtentPoint32A(hdcBuffer, noHay, strlen(noHay), &nhSize);
            TextOutA(hdcBuffer, panelX + (panelAncho - nhSize.cx) / 2, inicioY, noHay, strlen(noHay));
        } else {
            // Mostrar partidas existentes
            for (int i = 0; i < menu->numPartidas && i < 5; i++) {
                COLORREF color = (i == menu->partidaSeleccionada) ? RGB(100, 255, 120) : RGB(200, 200, 200);
                SetTextColor(hdcBuffer, color);
                
                char linea[128];
                snprintf(linea, sizeof(linea), "%s - Isla %d (%s)", 
                         menu->partidas[i].nombreJugador,
                         menu->partidas[i].islaActual,
                         menu->partidas[i].timestamp);
                
                int textoY = inicioY + i * 40;
                
                if (i == menu->partidaSeleccionada) {
                    HPEN penHighlight = CreatePen(PS_SOLID, 2, RGB(80, 200, 100));
                    HPEN oldP = (HPEN)SelectObject(hdcBuffer, penHighlight);
                    HBRUSH nullB = (HBRUSH)GetStockObject(NULL_BRUSH);
                    HBRUSH oldB = (HBRUSH)SelectObject(hdcBuffer, nullB);
                    
                    RoundRect(hdcBuffer, panelX + 20, textoY - 5,
                              panelX + panelAncho - 20, textoY + 30, 5, 5);
                    
                    SelectObject(hdcBuffer, oldP);
                    SelectObject(hdcBuffer, oldB);
                    DeleteObject(penHighlight);
                }
                
                TextOutA(hdcBuffer, panelX + 30, textoY, linea, strlen(linea));
            }
        }
        
        // Mostrar opción de nueva partida SIEMPRE (haya o no partidas)
        SelectObject(hdcBuffer, fontPequena);
        SetTextColor(hdcBuffer, RGB(100, 255, 120));
        const char *nueva = menu->numPartidas > 0 
            ? "[N] Nueva partida | [ENTER] Sobrescribir seleccionada | [ESC] Cancelar" 
            : "[N] Crear nueva partida | [ESC] Cancelar";
        SIZE nSize;
        GetTextExtentPoint32A(hdcBuffer, nueva, strlen(nueva), &nSize);
        int ayudaY = menu->numPartidas > 0 ? inicioY + 230 : inicioY + 150;
        TextOutA(hdcBuffer, panelX + (panelAncho - nSize.cx) / 2, ayudaY, nueva, strlen(nueva));
        SelectObject(hdcBuffer, fontOpciones);
        
       
        
    } else if (menu->modo == MODO_CARGAR) {
        // Pantalla de cargar - lista de partidas
        if (menu->numPartidas == 0) {
            SetTextColor(hdcBuffer, RGB(180, 180, 180));
            const char *noHay = "No hay partidas guardadas";
            SIZE nhSize;
            GetTextExtentPoint32A(hdcBuffer, noHay, strlen(noHay), &nhSize);
            TextOutA(hdcBuffer, panelX + (panelAncho - nhSize.cx) / 2, inicioY + 50, noHay, strlen(noHay));
        } else {
            for (int i = 0; i < menu->numPartidas && i < 5; i++) {
                COLORREF color = (i == menu->partidaSeleccionada) ? RGB(100, 255, 120) : RGB(200, 200, 200);
                SetTextColor(hdcBuffer, color);
                
                char linea[128];
                snprintf(linea, sizeof(linea), "%s - Isla %d (%s)", 
                         menu->partidas[i].nombreJugador,
                         menu->partidas[i].islaActual,
                         menu->partidas[i].timestamp);
                
                int textoY = inicioY + i * 40;
                
                if (i == menu->partidaSeleccionada) {
                    HPEN penHighlight = CreatePen(PS_SOLID, 2, RGB(80, 200, 100));
                    HPEN oldP = (HPEN)SelectObject(hdcBuffer, penHighlight);
                    HBRUSH nullB = (HBRUSH)GetStockObject(NULL_BRUSH);
                    HBRUSH oldB = (HBRUSH)SelectObject(hdcBuffer, nullB);
                    
                    RoundRect(hdcBuffer, panelX + 20, textoY - 5,
                              panelX + panelAncho - 20, textoY + 30, 5, 5);
                    
                    SelectObject(hdcBuffer, oldP);
                    SelectObject(hdcBuffer, oldB);
                    DeleteObject(penHighlight);
                }
                
                TextOutA(hdcBuffer, panelX + 30, textoY, linea, strlen(linea));
            }
        }
        
        // Botón volver
        int btnY = panelY + panelAlto - 50;
        SetTextColor(hdcBuffer, RGB(255, 200, 100));
        const char *volver = "[ESC] Volver";
        SIZE vSize;
        GetTextExtentPoint32A(hdcBuffer, volver, strlen(volver), &vSize);
        TextOutA(hdcBuffer, panelX + (panelAncho - vSize.cx) / 2, btnY, volver, strlen(volver));
        
    } else if (menu->modo == MODO_CONFIRMAR_SALIR) {
        // Confirmación antes de salir
        SetTextColor(hdcBuffer, RGB(255, 220, 100));
        const char *advertencia = "No has guardado la partida.";
        SIZE aSize;
        GetTextExtentPoint32A(hdcBuffer, advertencia, strlen(advertencia), &aSize);
        TextOutA(hdcBuffer, panelX + (panelAncho - aSize.cx) / 2, inicioY, advertencia, strlen(advertencia));
        
        SetTextColor(hdcBuffer, RGB(200, 200, 200));
        const char *pregunta = "Seguro que deseas salir?";
        SIZE pSize;
        GetTextExtentPoint32A(hdcBuffer, pregunta, strlen(pregunta), &pSize);
        TextOutA(hdcBuffer, panelX + (panelAncho - pSize.cx) / 2, inicioY + 30, pregunta, strlen(pregunta));
        
        // Opciones
        int btnY = inicioY + 80;
        const char *opSi = menu->seleccion == 0 ? "> Salir sin guardar <" : "Salir sin guardar";
        const char *opNo = menu->seleccion == 1 ? "> Volver y guardar <" : "Volver y guardar";
        
        SetTextColor(hdcBuffer, menu->seleccion == 0 ? RGB(255, 100, 100) : RGB(180, 180, 180));
        SIZE siSize;
        GetTextExtentPoint32A(hdcBuffer, opSi, strlen(opSi), &siSize);
        TextOutA(hdcBuffer, panelX + (panelAncho - siSize.cx) / 2, btnY, opSi, strlen(opSi));
        
        SetTextColor(hdcBuffer, menu->seleccion == 1 ? RGB(100, 255, 120) : RGB(180, 180, 180));
        SIZE noSize;
        GetTextExtentPoint32A(hdcBuffer, opNo, strlen(opNo), &noSize);
        TextOutA(hdcBuffer, panelX + (panelAncho - noSize.cx) / 2, btnY + 35, opNo, strlen(opNo));
    } else if (menu->modo == MODO_PRINCIPAL + 10) {
        // Pantalla de ingresar nombre para nueva partida
        SetTextColor(hdcBuffer, RGB(200, 200, 200));
        const char *instruc = "Ingresa el nombre de la nueva partida:";
        SIZE iSize;
        GetTextExtentPoint32A(hdcBuffer, instruc, strlen(instruc), &iSize);
        TextOutA(hdcBuffer, panelX + (panelAncho - iSize.cx) / 2, inicioY, instruc, strlen(instruc));
        
        // Campo de texto
        int campoX = panelX + 40;
        int campoY = inicioY + 40;
        int campoAncho = panelAncho - 80;
        int campoAlto = 35;
        
        HBRUSH campoFondo = CreateSolidBrush(RGB(40, 40, 50));
        RECT campoRect = {campoX, campoY, campoX + campoAncho, campoY + campoAlto};
        FillRect(hdcBuffer, &campoRect, campoFondo);
        DeleteObject(campoFondo);
        
        HPEN campoBorde = CreatePen(PS_SOLID, 2, menu->nombreExiste ? RGB(255, 100, 100) : RGB(100, 150, 200));
        HPEN oldP = (HPEN)SelectObject(hdcBuffer, campoBorde);
        HBRUSH nullB = (HBRUSH)GetStockObject(NULL_BRUSH);
        HBRUSH oldB = (HBRUSH)SelectObject(hdcBuffer, nullB);
        Rectangle(hdcBuffer, campoX, campoY, campoX + campoAncho, campoY + campoAlto);
        SelectObject(hdcBuffer, oldP);
        SelectObject(hdcBuffer, oldB);
        DeleteObject(campoBorde);
        
        // Texto ingresado
        SetTextColor(hdcBuffer, RGB(255, 255, 255));
        char textoConCursor[MAX_NOMBRE_JUGADOR + 2];
        snprintf(textoConCursor, sizeof(textoConCursor), "%s_", menu->nombreInput);
        TextOutA(hdcBuffer, campoX + 10, campoY + 8, textoConCursor, strlen(textoConCursor));
        
        // Mensaje de error si existe
        if (menu->nombreExiste) {
            SelectObject(hdcBuffer, fontPequena);
            SetTextColor(hdcBuffer, RGB(255, 100, 100));
            const char *err = "Ya existe una partida con este nombre";
            SIZE errSize;
            GetTextExtentPoint32A(hdcBuffer, err, strlen(err), &errSize);
            TextOutA(hdcBuffer, panelX + (panelAncho - errSize.cx) / 2, campoY + 45, err, strlen(err));
            SelectObject(hdcBuffer, fontOpciones);
        }
        
        // Botones
        int btnY = campoY + 80;
        const char *btnGuardar = "[ENTER] Guardar";
        const char *btnVolver = "[ESC] Volver";
        
        SetTextColor(hdcBuffer, RGB(100, 255, 120));
        SIZE gSize;
        GetTextExtentPoint32A(hdcBuffer, btnGuardar, strlen(btnGuardar), &gSize);
        TextOutA(hdcBuffer, panelX + panelAncho/4 - gSize.cx/2, btnY, btnGuardar, strlen(btnGuardar));
        
        SetTextColor(hdcBuffer, RGB(255, 200, 100));
        SIZE vSize;
        GetTextExtentPoint32A(hdcBuffer, btnVolver, strlen(btnVolver), &vSize);
        TextOutA(hdcBuffer, panelX + 3*panelAncho/4 - vSize.cx/2, btnY, btnVolver, strlen(btnVolver));
    }
    
    // Mensaje de estado y ruta de guardado
    if (menu->timerMensaje > 0 && menu->mensaje[0] != '\0') {
        SelectObject(hdcBuffer, fontPequena);
        SetTextColor(hdcBuffer, RGB(100, 255, 150));
        SIZE msgSize;
        GetTextExtentPoint32A(hdcBuffer, menu->mensaje, strlen(menu->mensaje), &msgSize);
        TextOutA(hdcBuffer, panelX + (panelAncho - msgSize.cx) / 2,
                 panelY + panelAlto - 55, menu->mensaje, strlen(menu->mensaje));
        
        // Mostrar ruta del archivo
        if (menu->rutaGuardado[0] != '\0') {
            SetTextColor(hdcBuffer, RGB(150, 150, 200));
            char rutaTexto[280];
            snprintf(rutaTexto, sizeof(rutaTexto), "Archivo: %s", menu->rutaGuardado);
            SIZE rutaSize;
            GetTextExtentPoint32A(hdcBuffer, rutaTexto, strlen(rutaTexto), &rutaSize);
            TextOutA(hdcBuffer, panelX + (panelAncho - rutaSize.cx) / 2,
                     panelY + panelAlto - 35, rutaTexto, strlen(rutaTexto));
        }
    }
    
    // Instrucciones en la parte inferior (solo para modos PRINCIPAL, CARGAR y CONFIRMAR_SALIR)
    // No mostrar en MODO_GUARDAR ni modo de ingreso de nombre para evitar superposición
    if (menu->modo != MODO_GUARDAR && menu->modo != MODO_PRINCIPAL + 10) {
        SelectObject(hdcBuffer, fontPequena);
        SetTextColor(hdcBuffer, RGB(120, 120, 120));
        const char *instruc = "Flechas: navegar | Enter: confirmar | ESC: volver";
        SIZE instrucSize;
        GetTextExtentPoint32A(hdcBuffer, instruc, strlen(instruc), &instrucSize);
        TextOutA(hdcBuffer, panelX + (panelAncho - instrucSize.cx) / 2,
                 panelY + panelAlto - 18, instruc, strlen(instruc));
    }
    
    SelectObject(hdcBuffer, oldFont);
    DeleteObject(fontTitulo);
    DeleteObject(fontOpciones);
    DeleteObject(fontPequena);
}

bool menuPausaProcesarCaracter(MenuPausa *menu, WPARAM caracter) {
    if (!menu->activo || (menu->modo != MODO_GUARDAR && menu->modo != MODO_PRINCIPAL + 10)) {
        return false;
    }
    
    // Solo procesar caracteres imprimibles en el modo de ingreso de nombre
    if (menu->modo == MODO_PRINCIPAL + 10) {
        if (caracter >= 32 && caracter < 127) {
            if (menu->cursorPos < MAX_NOMBRE_JUGADOR - 1) {
                menu->nombreInput[menu->cursorPos] = (char)caracter;
                menu->cursorPos++;
                menu->nombreInput[menu->cursorPos] = '\0';
                menu->nombreExiste = existePartida(menu->nombreInput);
            }
            return true;
        }
    }
    
    return false;
}

bool menuPausaProcesarTecla(MenuPausa *menu, WPARAM tecla, struct Jugador *j,
                            Camara *cam, Edificio *ayuntamiento,
                            Edificio *mina, Edificio *cuartel) {
    if (!menu->activo) {
        if (tecla == VK_ESCAPE) {
            menuPausaAbrir(menu);
            return true;
        }
        return false;
    }
    
    // ESC para cerrar o volver
    if (tecla == VK_ESCAPE) {
        if (menu->modo == MODO_PRINCIPAL + 10) {
            // Si está en modo de ingreso de nombre, volver a la lista de guardado
            menu->modo = MODO_GUARDAR;
            menu->nombreInput[0] = '\0';
            menu->cursorPos = 0;
        } else if (menu->modo != MODO_PRINCIPAL) {
            menu->modo = MODO_PRINCIPAL;
            menu->seleccion = 0;
        } else {
            menuPausaCerrar(menu);
        }
        return true;
    }
    
    // En modo guardar, manejar navegación de lista y creación de nueva partida
    if (menu->modo == MODO_GUARDAR) {
        // Tecla N para crear nueva partida
        if (tecla == 'N' || tecla == 'n') {
            menu->modo = MODO_PRINCIPAL + 10;  // Modo temporal para pedir nombre
            menu->nombreInput[0] = '\0';
            menu->cursorPos = 0;
            menu->nombreExiste = false;
            return true;
        }
        
        // Enter para sobrescribir partida seleccionada
        if (tecla == VK_RETURN && menu->numPartidas > 0) {
            const char *nombreExistente = menu->partidas[menu->partidaSeleccionada].nombreJugador;
            if (guardarPartidaPorNombre(nombreExistente, j, cam)) {
                strncpy(j->Nombre, nombreExistente, sizeof(j->Nombre) - 1);
                j->Nombre[sizeof(j->Nombre) - 1] = '\0';
                
                snprintf(menu->mensaje, sizeof(menu->mensaje), "Partida sobrescrita correctamente!");
                obtenerRutaGuardado(nombreExistente, menu->rutaGuardado, sizeof(menu->rutaGuardado));
                menu->timerMensaje = 180;
                menu->partidaGuardada = true;
                menu->modo = MODO_PRINCIPAL;
            } else {
                snprintf(menu->mensaje, sizeof(menu->mensaje), "Error al guardar");
                menu->timerMensaje = 120;
            }
            return true;
        }
        // NO consumir otras teclas - permitir que se procesen las flechas abajo
    }
    
    // Modo especial para pedir nombre de nueva partida
    if (menu->modo == MODO_PRINCIPAL + 10) {
        if (tecla == VK_BACK && menu->cursorPos > 0) {
            menu->cursorPos--;
            menu->nombreInput[menu->cursorPos] = '\0';
            menu->nombreExiste = existePartida(menu->nombreInput);
            return true;
        }
        
        if (tecla == VK_RETURN) {
            if (menu->cursorPos > 0) {
                // Guardar la partida con el nuevo nombre
                if (guardarPartidaPorNombre(menu->nombreInput, j, cam)) {
                    strncpy(j->Nombre, menu->nombreInput, sizeof(j->Nombre) - 1);
                    j->Nombre[sizeof(j->Nombre) - 1] = '\0';
                    
                    snprintf(menu->mensaje, sizeof(menu->mensaje), "Partida guardada correctamente!");
                    obtenerRutaGuardado(menu->nombreInput, menu->rutaGuardado, sizeof(menu->rutaGuardado));
                    menu->timerMensaje = 180;
                    menu->partidaGuardada = true;
                    menu->modo = MODO_PRINCIPAL;
                    menu->nombreInput[0] = '\0';
                    menu->cursorPos = 0;
                } else {
                    snprintf(menu->mensaje, sizeof(menu->mensaje), "Error al guardar");
                    menu->timerMensaje = 120;
                }
            }
            return true;
        }
        return true;  // Consumir todas las teclas
    }
    
    // Navegación con flechas
    if (tecla == VK_UP || tecla == 'W' || tecla == 'w') {
        if (menu->modo == MODO_PRINCIPAL) {
            menu->seleccion = (menu->seleccion - 1 + OPCIONES_PAUSA_TOTAL) % OPCIONES_PAUSA_TOTAL;
        } else if ((menu->modo == MODO_CARGAR || menu->modo == MODO_GUARDAR) && menu->numPartidas > 0) {
            menu->partidaSeleccionada = (menu->partidaSeleccionada - 1 + menu->numPartidas) % menu->numPartidas;
        } else if (menu->modo == MODO_CONFIRMAR_SALIR) {
            menu->seleccion = (menu->seleccion + 1) % 2;
        }
        return true;
    }
    
    if (tecla == VK_DOWN || tecla == 'S' || tecla == 's') {
        if (menu->modo == MODO_PRINCIPAL) {
            menu->seleccion = (menu->seleccion + 1) % OPCIONES_PAUSA_TOTAL;
        } else if ((menu->modo == MODO_CARGAR || menu->modo == MODO_GUARDAR) && menu->numPartidas > 0) {
            menu->partidaSeleccionada = (menu->partidaSeleccionada + 1) % menu->numPartidas;
        } else if (menu->modo == MODO_CONFIRMAR_SALIR) {
            menu->seleccion = (menu->seleccion + 1) % 2;
        }
        return true;
    }
    
    // Selección con Enter
    if (tecla == VK_RETURN) {
        if (menu->modo == MODO_PRINCIPAL) {
            switch (menu->seleccion) {
                case 0:  // Continuar
                    menuPausaCerrar(menu);
                    break;
                case 1:  // Guardar
                    menu->modo = MODO_GUARDAR;
                    // Obtener lista de partidas guardadas (igual que en cargar)
                    menu->numPartidas = obtenerPartidasGuardadas(menu->partidas);
                    menu->partidaSeleccionada = 0;
                    break;
                case 2:  // Cargar
                    menu->modo = MODO_CARGAR;
                    menu->numPartidas = obtenerPartidasGuardadas(menu->partidas);
                    menu->partidaSeleccionada = 0;
                    break;
                case 3:  // Salir al menú
                    // Si ya guardó, salir directamente sin preguntar
                    if (menu->partidaGuardada) {
                        menu->volverAlMenu = true;
                    } else {
                        menu->modo = MODO_CONFIRMAR_SALIR;
                        menu->seleccion = 1;  // Por defecto en "Volver y guardar"
                    }
                    break;
            }
        } else if (menu->modo == MODO_CARGAR) {
            if (menu->numPartidas > 0) {
                const char *nombre = menu->partidas[menu->partidaSeleccionada].nombreJugador;
                if (cargarPartidaPorNombre(nombre, j, cam, ayuntamiento, mina, cuartel)) {
                    snprintf(menu->mensaje, sizeof(menu->mensaje), "Partida cargada!");
                    menu->timerMensaje = 90;
                    menuPausaCerrar(menu);
                } else {
                    snprintf(menu->mensaje, sizeof(menu->mensaje), "Error al cargar");
                    menu->timerMensaje = 90;
                }
            }
        } else if (menu->modo == MODO_CONFIRMAR_SALIR) {
            if (menu->seleccion == 0) {
                // Salir sin guardar - volver al menú principal
                menu->volverAlMenu = true;
            } else {
                // Volver para guardar primero
                menu->modo = MODO_GUARDAR;
                menu->nombreInput[0] = '\0';
                menu->cursorPos = 0;
            }
        }
        return true;
    }
    
    return true;
}
