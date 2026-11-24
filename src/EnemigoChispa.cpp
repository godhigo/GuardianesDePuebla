#include "EnemigoChispa.h"

#include <cmath>

#include "Paleta.h"
#include "Partida.h"
#include "Particula.h"

// vida 45, velocidad 110 px/s, da $8 al morir, quita 1 vida si llega
EnemigoChispa::EnemigoChispa(const Nivel& nivel)
    : Enemigo(nivel, 45.0f, 110.0f, 8, 1), tiempo_(GetRandomValue(0, 600) / 100.0f) {}

void EnemigoChispa::Actualizar(float dt, Partida& partida) {
    Enemigo::Actualizar(dt, partida);  // primero hace lo de todos los enemigos: avanzar por el camino
    if (!EstaActivo()) return;

    // Luego se mueve de lado a lado, perpendicular a hacia donde camina
    tiempo_ += dt;
    float desplazamiento = std::sin(tiempo_ * 9.0f) * 9.0f;
    Vector2 p = GetPosicion();
    SetPosicion(Vector2{p.x - std::sin(GetAngulo()) * desplazamiento, p.y + std::cos(GetAngulo()) * desplazamiento});

    // Suelta una chispa de vez en cuando
    if (GetRandomValue(0, 100) < 20) {
        Vector2 velocidad{GetRandomValue(-15, 15) * 1.0f, -18.0f - GetRandomValue(0, 26)};
        partida.AgregarParticula(Particula(GetPosicion(), velocidad, 0.4f, NARANJA, 2.2f, FormaParticula::Chispa));
    }
}

void EnemigoChispa::Dibujar() const {
    Vector2 p = GetPosicion();
    float radio = 7.0f + std::sin(tiempo_ * 20.0f) * 1.2f;

    // Brillo alrededor
    BeginBlendMode(BLEND_ADDITIVE);
    DrawCircleGradient((int)p.x, (int)p.y, 20.0f, Fade(Color{255, 200, 90, 255}, 0.8f), Fade(Color{255, 90, 30, 255}, 0.0f));
    EndBlendMode();

    // Cuerpo y centro mas claro
    DrawCircleV(p, radio, FueGolpeadoRecien() ? WHITE : Color{255, 201, 92, 255});
    DrawCircleV(Vector2{p.x - 1.5f, p.y - 1.5f}, radio * 0.45f, Color{255, 246, 216, 255});

    // Ojitos que miran hacia donde camina
    float ax = std::cos(GetAngulo()), ay = std::sin(GetAngulo());
    for (int lado = -1; lado <= 1; lado += 2) {
        DrawCircleV(Vector2{p.x + ax * 3 - ay * 2.6f * lado, p.y + ay * 3 + ax * 2.6f * lado}, 1.4f, Color{90, 30, 14, 255});
    }

    DibujarEfectoLento(12.0f);
    DibujarBarraVida(24.0f, 18.0f);
}

const char* EnemigoChispa::GetNombre() const {
    return "Chispa";
}
