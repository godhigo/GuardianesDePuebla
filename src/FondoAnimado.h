#pragma once

#include "raylib.h"

// FondoAnimado: carga un GIF y lo reproduce en bucle, estirado a toda la ventana
class FondoAnimado {
public:
    FondoAnimado();  // constructor: deja todo en cero, todavia no carga nada

    void Cargar(const char* ruta, float cuadrosPorSegundo);  // carga el GIF (la ventana ya debe existir)
    void Descargar();                                        // libera la memoria (antes de cerrar la ventana)

    void Actualizar(float dt);          // avanza al siguiente cuadro cuando toca
    void Dibujar(int ancho, int alto);  // dibuja el cuadro actual del tamano de la ventana

private:
    Image imagen_;          // todos los cuadros del GIF, uno tras otro en memoria
    Texture2D textura_;     // el cuadro que se ve ahorita, ya en la tarjeta de video
    int totalCuadros_;      // cuantos cuadros tiene el GIF
    int cuadroActual_;      // que cuadro se esta mostrando
    float tiempo_;          // segundos desde el ultimo cambio de cuadro
    float duracionCuadro_;  // cuantos segundos dura cada cuadro
};
