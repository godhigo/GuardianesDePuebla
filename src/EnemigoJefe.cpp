#include "EnemigoJefe.h"

#include <cmath>

#include "Paleta.h"
#include "Partida.h"
#include "Torre.h"

EnemigoJefe::EnemigoJefe(const Nivel& nivel, float vida, float velocidad, int recompensa, int danioCiudad,
                         float alcanceAtaque, float cadenciaAtaque)
    : Enemigo(nivel, vida, velocidad, recompensa, danioCiudad), alcanceAtaque_(alcanceAtaque),
      cadenciaAtaque_(cadenciaAtaque), esperaAtaque_(cadenciaAtaque), tiempo_(0.0f) {}

void EnemigoJefe::Actualizar(float dt, Partida& partida) {
    Enemigo::Actualizar(dt, partida);  // primero lo de todos los enemigos: avanzar por el camino
    if (!EstaActivo()) return;
    tiempo_ += dt;

    // Despues lo que solo hacen los jefes: atacar a distancia
    esperaAtaque_ -= dt;
    if (esperaAtaque_ > 0.0f) return;

    Torre* objetivo = BuscarTorreCercana(partida);
    if (objetivo == nullptr) return;  // no hay torres cerca: sigue caminando

    LanzarAtaque(objetivo->GetPosicion(), partida);  // cada jefe a su manera (polimorfismo)
    esperaAtaque_ = cadenciaAtaque_;
}

bool EnemigoJefe::EsJefe() const {
    return true;
}

Torre* EnemigoJefe::BuscarTorreCercana(Partida& partida) const {
    Torre* masCercana = nullptr;
    float mejorDistancia = alcanceAtaque_;
    for (const auto& torre : partida.GetTorres()) {
        if (!torre->EstaActivo()) continue;
        Vector2 t = torre->GetPosicion();
        float distancia = std::hypot(t.x - GetPosicion().x, t.y - GetPosicion().y);
        if (distancia <= mejorDistancia) {
            mejorDistancia = distancia;
            masCercana = torre.get();
        }
    }
    return masCercana;
}

void EnemigoJefe::DibujarAlcanceAtaque() const {
    DrawCircleV(GetPosicion(), alcanceAtaque_, Fade(ROJO, 0.05f));
    DrawCircleLinesV(GetPosicion(), alcanceAtaque_, Fade(ROJO, 0.25f));
}

float EnemigoJefe::GetTiempo() const {
    return tiempo_;
}
