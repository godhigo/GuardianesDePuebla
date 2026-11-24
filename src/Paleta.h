#pragma once

#include "raylib.h"

// Paleta: los colores y la tipografia del juego, compartidos por todas las clases.
// "inline" permite definir estas constantes en un .h que incluyen muchos archivos.
inline const Color AZUL_OSCURO{16, 23, 58, 255};
inline const Color AZUL       {61, 114, 240, 255};
inline const Color BLANCO     {245, 240, 230, 255};
inline const Color AMARILLO   {242, 179, 61, 255};
inline const Color GRIS       {170, 179, 220, 255};
inline const Color VERDE_PASTO{127, 138, 76, 255};
inline const Color ROJO       {255, 107, 80, 255};
inline const Color VERDE_VIDA {127, 214, 107, 255};
inline const Color NARANJA    {255, 154, 60, 255};

// ---- Tipografia ----
// Se carga UNA vez al abrir la ventana. Para cambiar la letra de todo el juego
// solo hay que reemplazar el archivo recursos/fuentes/juego.ttf
void CargarFuente(const char* ruta);
void DescargarFuente();

// Reemplazan a DrawText y MeasureText de raylib, pero con nuestra tipografia
void Texto(const char* texto, int x, int y, int tamano, Color color);
int MedirTexto(const char* texto, int tamano);

// Escribe un texto con contorno oscuro y sombra, para que se lea sobre cualquier fondo
void DibujarTextoContorno(const char* texto, int x, int y, int tamano, Color color);
