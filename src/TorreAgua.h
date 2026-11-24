#pragma once

#include "Torre.h"

// TorreAgua: chorro de poco danio que ralentiza y le quita armadura al Golem.
// Apunta al enemigo con mas vida.
class TorreAgua : public Torre {
public:
    static constexpr int COSTO = 120;

    explicit TorreAgua(Vector2 posicion);

    Enemigo* ElegirObjetivo(const std::vector<Enemigo*>& candidatos) const override;
    void Disparar(Enemigo& objetivo, Partida& partida) override;
    void Dibujar() const override;
    const char* GetNombre() const override;
    const char* GetEstrategia() const override;
};
