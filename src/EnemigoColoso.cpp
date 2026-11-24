#include "EnemigoColoso.h"

#include <cmath>

#include "AtaqueJefe.h"
#include "Paleta.h"
#include "Partida.h"

// vida 2900, velocidad 24 px/s, da $300 al morir, quita 15 vidas si llega a la ciudad.
// Lanza una roca cada 2 segundos a las torres que esten a 160 px o menos
EnemigoColoso::EnemigoColoso(const Nivel& nivel) : EnemigoJefe(nivel, 2900.0f, 24.0f, 300, 15, 160.0f, 2.0f) {}

void EnemigoColoso::LanzarAtaque(Vector2 destino, Partida& partida) {
    // Roca lenta (0.9 s en llegar) que quita 50 de vida
    partida.AgregarAtaqueJefe(AtaqueJefe(GetPosicion(), destino, 0.9f, 50.0f, TipoAtaque::Roca));
}

void EnemigoColoso::Dibujar() const {
    Vector2 p = GetPosicion();
    float grados = GetAngulo() * RAD2DEG;
    float pulso = 0.5f + 0.5f * std::sin(GetTiempo() * 4.0f);

    DibujarAlcanceAtaque();

    // Sombra y cuerpo: un poligono grande de roca con un brillo de lava alrededor
    DrawEllipse((int)p.x + 5, (int)p.y + 18, 38, 14, Fade(BLACK, 0.35f));
    DrawCircleGradient((int)p.x, (int)p.y, 50, Fade(Color{255, 110, 40, 255}, 0.25f + 0.2f * pulso),
                       Fade(Color{255, 110, 40, 255}, 0.0f));
    DrawPoly(p, 8, 34.0f, grados, FueGolpeadoRecien() ? Color{130, 104, 96, 255} : Color{48, 38, 37, 255});
    DrawPolyLinesEx(p, 8, 34.0f, grados, 3.0f, Color{25, 18, 18, 255});

    // Grietas de lava que laten
    for (int k = 0; k < 5; k++) {
        float a = GetAngulo() + k * 1.26f;
        Vector2 inicio{p.x + std::cos(a) * 6, p.y + std::sin(a) * 6};
        Vector2 medio{p.x + std::cos(a + 0.25f) * 18, p.y + std::sin(a + 0.25f) * 18};
        Vector2 fin{p.x + std::cos(a - 0.1f) * 29, p.y + std::sin(a - 0.1f) * 29};
        Color lava = Fade(Color{255, 140, 50, 255}, 0.6f + 0.4f * pulso);
        DrawLineEx(inicio, medio, 3.0f, lava);
        DrawLineEx(medio, fin, 2.5f, lava);
    }

    // Brazos de roca a los lados
    for (int lado = -1; lado <= 1; lado += 2) {
        float a = GetAngulo() + 1.4f * lado;
        DrawCircleV(Vector2{p.x + std::cos(a) * 36, p.y + std::sin(a) * 36}, 11.0f, Color{62, 50, 48, 255});
        DrawCircleV(Vector2{p.x + std::cos(a) * 40, p.y + std::sin(a) * 40}, 4.0f, Fade(NARANJA, 0.8f));
    }

    // Corona de roca y ojos de fuego
    for (int i = 0; i < 3; i++) {
        float a = GetAngulo() + PI + (i - 1) * 0.5f;
        DrawCircleV(Vector2{p.x + std::cos(a) * 24, p.y + std::sin(a) * 24}, 6.0f, Color{90, 78, 74, 255});
    }
    for (int lado = -1; lado <= 1; lado += 2) {
        float a = GetAngulo() + 0.4f * lado;
        Vector2 ojo{p.x + std::cos(a) * 20, p.y + std::sin(a) * 20};
        DrawCircleV(ojo, 5.0f, Color{255, 200, 80, 255});
        DrawCircleV(ojo, 2.5f, Color{255, 255, 220, 255});
    }

    DibujarEfectoLento(42.0f);
    // Su barra de vida grande se dibuja arriba de la pantalla (Partida), no encima de el
}

const char* EnemigoColoso::GetNombre() const {
    return "Coloso de Lava";
}
