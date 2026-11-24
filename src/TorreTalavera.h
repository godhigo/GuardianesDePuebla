#pragma once

#include "Torre.h"

// TorreTalavera: dispara azulejos rapidos a un solo enemigo: el que va mas cerca de la ciudad.
class TorreTalavera : public Torre {
public:
    static constexpr int COSTO = 100;

    explicit TorreTalavera(Vector2 posicion);

    Enemigo* ElegirObjetivo(const std::vector<Enemigo*>& candidatos) const override;
    void Disparar(Enemigo& objetivo, Partida& partida) override;
    void Dibujar() const override;
    const char* GetNombre() const override;
    const char* GetEstrategia() const override;
};
