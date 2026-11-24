#include "EnemigoLlamarada.h"

#include <cmath>

#include "AtaqueJefe.h"
#include "Paleta.h"
#include "Partida.h"

// vida 1600, velocidad 38 px/s, da $200 al morir, quita 10 vidas si llega a la ciudad.
// Lanza una bola de fuego cada 1.5 segundos a las torres que esten a 170 px o menos
EnemigoLlamarada::EnemigoLlamarada(const Nivel& nivel) : EnemigoJefe(nivel, 1600.0f, 38.0f, 200, 10, 170.0f, 1.5f) {}

void EnemigoLlamarada::LanzarAtaque(Vector2 destino, Partida& partida) {
    // Bola de fuego rapida (0.55 s en llegar) que quita 35 de vida
    partida.AgregarAtaqueJefe(AtaqueJefe(GetPosicion(), destino, 0.55f, 35.0f, TipoAtaque::BolaDeFuego));
}

void EnemigoLlamarada::Dibujar() const {
    Vector2 p = GetPosicion();
    float t = GetTiempo();
    float pulso = 0.5f + 0.5f * std::sin(t * 6.0f);

    DibujarAlcanceAtaque();

    // Resplandor grande
    BeginBlendMode(BLEND_ADDITIVE);
    DrawCircleGradient((int)p.x, (int)p.y, 52, Fade(Color{255, 120, 40, 255}, 0.35f + 0.2f * pulso),
                       Fade(Color{255, 60, 20, 255}, 0.0f));
    EndBlendMode();

    // Lenguas de fuego que giran alrededor (triangulos que apuntan hacia afuera)
    for (int i = 0; i < 9; i++) {
        float a = t * 2.0f + i * (2.0f * PI / 9.0f);
        float largo = 30.0f + 7.0f * std::sin(t * 8.0f + i);
        Vector2 punta{p.x + std::cos(a) * largo, p.y + std::sin(a) * largo};
        Vector2 izq{p.x + std::cos(a - 0.35f) * 16, p.y + std::sin(a - 0.35f) * 16};
        Vector2 der{p.x + std::cos(a + 0.35f) * 16, p.y + std::sin(a + 0.35f) * 16};
        DrawTriangle(punta, izq, der, i % 2 ? ROJO : NARANJA);  // raylib pide los puntos en orden contrario al reloj
    }

    // Cuerpo: capas de fuego de afuera hacia adentro
    DrawCircleV(p, 21.0f, FueGolpeadoRecien() ? BLANCO : Color{232, 80, 40, 255});
    DrawCircleV(p, 15.0f, NARANJA);
    DrawCircleV(p, 9.0f + 2.0f * pulso, Color{255, 230, 140, 255});

    // Ojos enojados
    for (int lado = -1; lado <= 1; lado += 2) {
        float a = GetAngulo() + 0.5f * lado;
        Vector2 ojo{p.x + std::cos(a) * 9, p.y + std::sin(a) * 9};
        DrawCircleV(ojo, 3.2f, Color{60, 20, 10, 255});
    }

    DibujarEfectoLento(34.0f);
}

const char* EnemigoLlamarada::GetNombre() const {
    return "Llamarada";
}
