@echo off
echo Configurando entorno de compilacion (w64devkit portable)...
set PATH=%~dp0w64devkit\bin;%PATH%

echo Compilando Islas en Guerra...
gcc main.c batallas/batallas.c edificios/edificios.c guardado/guardado.c mapa/mapa.c mapa/menu.c recursos/navegacion.c recursos/recursos.c recursos/ui_compra.c recursos/ui_embarque.c recursos/ui_entrena.c -o IslasEnGuerra.exe -lgdi32 -luser32 -lkernel32 -lm -lmsimg32

if %errorlevel% neq 0 (
    echo Error en la compilacion.
    pause
    exit /b %errorlevel%
)

echo Compilacion exitosa. Ejecutando juego...
IslasEnGuerra.exe
