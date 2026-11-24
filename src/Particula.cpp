#include "Particula.h"

Particula::Particula(Vector2 posicion, Vector2 velocidad, float vida, Color color, float tamano,
                     FormaParticula forma, float gravedad)
    : Entidad(posicion), velocidad_(velocidad), vida_(vida), vidaMax_(vida), color_(color),
      tamano_(tamano), forma_(forma), gravedad_(gravedad) {}

void Particula::Actualizar(float dt, Partida&) {
    Vector2 p = GetPosicion();
    SetPosicion(Vector2{p.x + velocidad_.x * dt, p.y + velocidad_.y * dt});
    velocidad_.y += gravedad_ * dt;

    vida_ -= dt;
    if (vida_ <= 0.0f) {
        Desactivar();
    }
}

void Particula::Dibujar() const {
    Vector2 p = GetPosicion();
    float k = vida_ / vidaMax_;  // 1 al nacer, 0 al morir

    switch (forma_) {
    case FormaParticula::Punto:
        DrawCircleV(p, tamano_ * (0.4f + 0.6f * k), Fade(color_, k));
        break;
    case FormaParticula::Chispa:
        BeginBlendMode(BLEND_ADDITIVE);  // suma su color a lo de abajo: se ve como luz
        DrawCircleV(p, tamano_ * (0.4f + 0.6f * k), Fade(color_, k));
        EndBlendMode();
        break;
    case FormaParticula::Humo:
        DrawCircleV(p, tamano_ * (1.8f - k), Fade(color_, k * 0.38f));
        break;
    case FormaParticula::Anillo:
        DrawRing(p, 8.0f + tamano_ * (1.0f - k) - (2.0f + 4.0f * k), 8.0f + tamano_ * (1.0f - k), 0, 360, 36,
                 Fade(color_, k));
        break;
    }
}
