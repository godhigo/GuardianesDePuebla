#pragma once

#include "Entidad.h"

enum class FormaParticula {
    Punto,   // circulito que se encoge
    Chispa,  // circulito brillante (se suma a los colores de abajo)
    Humo,    // circulo transparente que crece
    Anillo   // aro que se expande (explosiones)
};

// Particula: efecto visual pequeno que vive poco tiempo y desaparece solo
class Particula : public Entidad {
public:
    Particula(Vector2 posicion, Vector2 velocidad, float vida, Color color, float tamano,
              FormaParticula forma, float gravedad = 0.0f);

    void Actualizar(float dt, Partida& partida) override;
    void Dibujar() const override;

private:
    Vector2 velocidad_;
    float vida_;      // segundos que le quedan
    float vidaMax_;   // segundos con los que nacio
    Color color_;
    float tamano_;
    FormaParticula forma_;
    float gravedad_;  // cuanto acelera hacia abajo
};
