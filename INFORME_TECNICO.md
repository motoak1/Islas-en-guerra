# Informe de Métricas del Proyecto "Islas en Guerra"

## Resumen por Carpeta

### 📂 Raíz (Carpeta Principal)

| Archivo (.c/.h) | Líneas de Código (LOC) | Funciones (Est.) | Descripción Breve                                              |
| --------------- | ---------------------- | ---------------- | -------------------------------------------------------------- |
| main.c          | 468                    | 8                | Punto de entrada, bucle principal y manejo de ventana Windows. |

### 📂 batallas

| Archivo (.c/.h) | Líneas de Código (LOC) | Funciones (Est.) | Descripción Breve                                       |
| --------------- | ---------------------- | ---------------- | ------------------------------------------------------- |
| batallas.c      | 148                    | 5                | Lógica de combate y simulación de batallas automáticas. |
| batallas.h      | 7                      | -                | Cabecera para funciones de batalla.                     |

### 📂 batallas/guardado

| Archivo (.c/.h) | Líneas de Código (LOC) | Funciones (Est.) | Descripción Breve                                     |
| --------------- | ---------------------- | ---------------- | ----------------------------------------------------- |
| guardado.c      | 1098                   | 20               | Sistema de serialización binaria y menús de guardado. |
| guardado.h      | 220                    | -                | Definiciones de estructuras serializables.            |

### 📂 mapa

| Archivo (.c/.h) | Líneas de Código (LOC) | Funciones (Est.) | Descripción Breve                                       |
| --------------- | ---------------------- | ---------------- | ------------------------------------------------------- |
| mapa.c          | 1835                   | 25               | Gestión de matrices, renderizado de tiles y colisiones. |
| mapa.h          | 129                    | -                | Cabecera principal del motor de mapas.                  |
| menu.c          | 341                    | 10               | Lógica del menú principal y flujo de pantallas.         |
| menu.h          | 44                     | -                | Definiciones del menú principal.                        |

### 📂 recursos

| Archivo (.c/.h) | Líneas de Código (LOC) | Funciones (Est.) | Descripción Breve                                      |
| --------------- | ---------------------- | ---------------- | ------------------------------------------------------ |
| recursos.c      | 1007                   | 15               | Lógica de unidades, pathfinding y recolección.         |
| recursos.h      | 189                    | -                | Definición de estructuras Unidad y Animacion.          |
| navegacion.c    | 1229                   | 22               | Lógica de viajes entre islas y persistencia de estado. |
| navegacion.h    | 94                     | -                | API de navegación global.                              |
| ui_compra.c     | 254                    | 6                | UI para compra de edificios/mejoras.                   |
| ui_embarque.c   | 561                    | 8                | UI y lógica para embarcar tropas.                      |
| ui_entrena.c    | 396                    | 7                | UI para entrenamiento de unidades.                     |

### 📂 recursos/edificios

| Archivo (.c/.h) | Líneas de Código (LOC) | Funciones (Est.) | Descripción Breve                              |
| --------------- | ---------------------- | ---------------- | ---------------------------------------------- |
| edificios.c     | 279                    | 10               | Lógica base de edificios (Mina, Ayuntamiento). |
| edificios.h     | 62                     | -                | Definición de tipos de edificios.              |

## Totales Globales

- **Total de Archivos:** 19 (analizados)
- **Total de Líneas de Código:** 8450 aprox.
- **Estado de la Traducción:** Completado al 100% (Variables críticas refactorizadas a Español).

## Notas de Normalización

- Se identificaron y refactorizaron términos clave: `damage` -> `dano`, `Animation` -> `Animacion`.
- Se tradujeron comentarios en archivos clave (`main.c`, `recursos.c`, `mapa.c`).
- Se agregó documentación interpretativa explicando el uso de matrices y punteros.
