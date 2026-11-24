#include "FondoAnimado.h"

FondoAnimado::FondoAnimado()
    : imagen_{}, textura_{}, totalCuadros_(0), cuadroActual_(0), tiempo_(0.0f), duracionCuadro_(0.0f) {}

void FondoAnimado::Cargar(const char* ruta, float cuadrosPorSegundo) {
    imagen_ = LoadImageAnim(ruta, &totalCuadros_);  // lee el GIF completo y nos dice cuantos cuadros tiene
    textura_ = LoadTextureFromImage(imagen_);       // sube el primer cuadro a la tarjeta de video
    duracionCuadro_ = 1.0f / cuadrosPorSegundo;     // a 12 cuadros por segundo, cada uno dura 1/12 s
}

void FondoAnimado::Descargar() {
    UnloadTexture(textura_);
    UnloadImage(imagen_);
}

void FondoAnimado::Actualizar(float dt) {
    if (totalCuadros_ == 0) return;  // si el GIF no se pudo cargar, no hay nada que animar

    tiempo_ += dt;
    if (tiempo_ >= duracionCuadro_) {
        tiempo_ -= duracionCuadro_;

        cuadroActual_++;
        if (cuadroActual_ >= totalCuadros_) {
            cuadroActual_ = 0;  // al terminar, vuelve a empezar
        }

        // Todos los cuadros estan seguidos en memoria. Cada pixel ocupa 4 bytes (rojo, verde, azul, opacidad).
        // Saltamos desde el inicio hasta donde empieza el cuadro actual y lo copiamos a la textura.
        int bytesPorCuadro = imagen_.width * imagen_.height * 4;
        unsigned char* inicio = (unsigned char*)imagen_.data;
        UpdateTexture(textura_, inicio + cuadroActual_ * bytesPorCuadro);
    }
}

void FondoAnimado::Dibujar(int ancho, int alto) {
    if (totalCuadros_ == 0) return;

    Rectangle origen{0.0f, 0.0f, (float)textura_.width, (float)textura_.height};  // toda la imagen
    Rectangle destino{0.0f, 0.0f, (float)ancho, (float)alto};                     // toda la ventana
    DrawTexturePro(textura_, origen, destino, Vector2{0.0f, 0.0f}, 0.0f, WHITE);
}
