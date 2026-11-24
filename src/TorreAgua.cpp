#include "TorreAgua.h"

#include <cmath>

#include "Enemigo.h"
#include "Paleta.h"
#include "Partida.h"
#include "Proyectil.h"

// alcance 130 px, danio 5, dispara cada 0.8 s
TorreAgua::TorreAgua(Vector2 posicion) : Torre(posicion, 130.0f, 5.0f, 0.8f, COSTO) {}

Enemigo* TorreAgua::ElegirObjetivo(const std::vector<Enemigo*>& candidatos) const {
    // El que tiene mas vida (casi siempre un Golem)
    Enemigo* elegido = candidatos[0];
    for (Enemigo* e : candidatos) {
        if (e->GetVida() > elegido->GetVida()) elegido = e;
    }
    return elegido;
}

void TorreAgua::Disparar(Enemigo& objetivo, Partida& partida) {
    partida.AgregarProyectil(Proyectil(PuntaDelCanon(18.0f), &objetivo, 460.0f, GetDanio(), TipoDanio::Agua, 0.0f));
}

void TorreAgua::Dibujar() const {
    Vector2 p = GetPosicion();
    DibujarSombra(true);

    // Fuente: aro azul oscuro con agua clara adentro
    DrawCircleV(p, 18.0f, Color{42, 94, 196, 255});
    DrawRing(p, 16.5f, 18.5f, 0, 360, 32, Color{23, 61, 138, 255});
    DrawCircleV(p, 13.0f, Color{111, 183, 255, 255});

    // Onda que se expande en el agua
    float k = std::fmod((float)GetTime() * 0.8f + p.x * 0.013f, 1.0f);
    DrawRing(p, 2.0f + k * 9.0f, 3.5f + k * 9.0f, 0, 360, 24, Fade(Color{232, 246, 255, 255}, (1.0f - k) * 0.85f));

    // Boquilla
    float r = -GetRetroceso() * 3.0f;
    float grados = GetAngulo() * RAD2DEG;
    DrawRectanglePro(Rectangle{p.x, p.y, 15, 7}, Vector2{-4 - r, 3.5f}, grados, Color{233, 244, 255, 255});
    DrawRectanglePro(Rectangle{p.x, p.y, 3, 7}, Vector2{-16 - r, 3.5f}, grados, Color{42, 94, 196, 255});

    DibujarNivel();
}

const char* TorreAgua::GetNombre() const { return "Torre de Agua"; }
const char* TorreAgua::GetEstrategia() const { return "Frena al de mas vida y le quita armadura."; }
