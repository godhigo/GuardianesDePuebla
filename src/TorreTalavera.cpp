#include "TorreTalavera.h"

#include "Enemigo.h"
#include "Paleta.h"
#include "Partida.h"
#include "Proyectil.h"

// alcance 150 px, danio 12, dispara cada 0.45 s
TorreTalavera::TorreTalavera(Vector2 posicion) : Torre(posicion, 150.0f, 12.0f, 0.45f, COSTO) {}

Enemigo* TorreTalavera::ElegirObjetivo(const std::vector<Enemigo*>& candidatos) const {
    // Al que le falta menos camino = el mas cerca de la ciudad (funciona aunque vayan por rutas distintas)
    Enemigo* elegido = candidatos[0];
    for (Enemigo* e : candidatos) {
        if (e->GetFaltante() < elegido->GetFaltante()) elegido = e;
    }
    return elegido;
}

void TorreTalavera::Disparar(Enemigo& objetivo, Partida& partida) {
    partida.AgregarProyectil(Proyectil(PuntaDelCanon(18.0f), &objetivo, 640.0f, GetDanio(), TipoDanio::Normal, 0.0f));
}

void TorreTalavera::Dibujar() const {
    Vector2 p = GetPosicion();
    DibujarSombra(false);

    // Base de azulejo de Talavera: blanco con borde y dibujo azul
    Rectangle base{p.x - 17, p.y - 17, 34, 34};
    DrawRectangleRounded(base, 0.25f, 4, BLANCO);
    DrawRectangleRoundedLinesEx(base, 0.25f, 4, 2.0f, Color{35, 72, 168, 255});
    DrawRectangleLinesEx(Rectangle{p.x - 12, p.y - 12, 24, 24}, 1.5f, Color{35, 72, 168, 255});
    DrawCircleV(Vector2{p.x + 5, p.y}, 4.0f, AZUL);
    DrawCircleV(Vector2{p.x - 5, p.y}, 4.0f, AZUL);
    DrawCircleV(Vector2{p.x, p.y + 5}, 4.0f, AZUL);
    DrawCircleV(Vector2{p.x, p.y - 5}, 4.0f, AZUL);
    DrawCircleV(p, 3.0f, AMARILLO);

    // Canon que gira hacia el enemigo y retrocede al disparar
    float r = -GetRetroceso() * 4.0f;
    DrawRectanglePro(Rectangle{p.x, p.y, 17, 8}, Vector2{-2 - r, 4}, GetAngulo() * RAD2DEG, Color{35, 72, 168, 255});
    DrawRectanglePro(Rectangle{p.x, p.y, 4, 8}, Vector2{-16 - r, 4}, GetAngulo() * RAD2DEG, AMARILLO);

    DibujarNivel();
}

const char* TorreTalavera::GetNombre() const { return "Torre Talavera"; }
const char* TorreTalavera::GetEstrategia() const { return "Ataca al enemigo mas cerca de la ciudad."; }
