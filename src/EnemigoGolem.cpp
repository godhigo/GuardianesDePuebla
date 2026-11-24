#include "EnemigoGolem.h"

#include <algorithm>
#include <cmath>

#include "Paleta.h"

const int ARMADURA_INICIAL = 8;

// vida 340, velocidad 45 px/s, da $25 al morir, quita 3 vidas si llega
EnemigoGolem::EnemigoGolem(const Nivel& nivel) : Enemigo(nivel, 340.0f, 45.0f, 25, 3), armadura_(ARMADURA_INICIAL) {}

void EnemigoGolem::RecibirDanio(float cantidad, TipoDanio tipo, Partida& partida) {
    // El agua enfria la lava y le quita 2 de armadura
    if (tipo == TipoDanio::Agua) {
        armadura_ = std::max(0, armadura_ - 2);
    }
    // La armadura le resta al golpe, pero siempre le hace al menos 1 de danio
    float danioReal = std::max(1.0f, cantidad - armadura_);
    Enemigo::RecibirDanio(danioReal, tipo, partida);  // el resto (bajar vida, morir) lo hace la clase base
}

void EnemigoGolem::Dibujar() const {
    Vector2 p = GetPosicion();
    float grados = GetAngulo() * RAD2DEG;

    // Sombra y cuerpo de roca (un poligono de 7 lados)
    DrawEllipse((int)p.x + 3, (int)p.y + 9, 19, 8, Fade(BLACK, 0.3f));
    DrawPoly(p, 7, 19.0f, grados, FueGolpeadoRecien() ? Color{122, 98, 92, 255} : Color{59, 48, 46, 255});
    DrawPolyLinesEx(p, 7, 19.0f, grados, 2.0f, Color{33, 25, 24, 255});

    // Grietas de lava: brillan mas conforme pierde armadura
    float brillo = 0.45f + 0.55f * (1.0f - armadura_ / (float)ARMADURA_INICIAL);
    for (int k = 0; k < 3; k++) {
        float a = GetAngulo() + k * 2.1f;
        Vector2 inicio{p.x + std::cos(a) * 3, p.y + std::sin(a) * 3};
        Vector2 fin{p.x + std::cos(a + 0.3f) * 13, p.y + std::sin(a + 0.3f) * 13};
        DrawLineEx(inicio, fin, 2.2f, Fade(Color{255, 122, 46, 255}, brillo));
    }

    // Placas de armadura en la espalda: una placa por cada 2 puntos de armadura
    int placas = (armadura_ + 1) / 2;
    for (int i = 0; i < placas; i++) {
        float a = GetAngulo() + PI + (i - 1.5f) * 0.6f;
        DrawCircleV(Vector2{p.x + std::cos(a) * 12, p.y + std::sin(a) * 12}, 4.0f, Color{141, 135, 129, 255});
    }

    // Ojos de lava
    for (int lado = -1; lado <= 1; lado += 2) {
        float a = GetAngulo() + 0.45f * lado;
        DrawCircleV(Vector2{p.x + std::cos(a) * 10, p.y + std::sin(a) * 10}, 2.2f, Color{255, 179, 71, 255});
    }

    DibujarEfectoLento(24.0f);
    DibujarBarraVida(38.0f, 30.0f);
}

const char* EnemigoGolem::GetNombre() const {
    return "Gólem";
}

int EnemigoGolem::GetArmadura() const {
    return armadura_;
}
