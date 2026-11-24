#pragma once

#include "Torre.h"

// TorreCanon (Canon de Cantera): bala lenta que explota y dania a todos los cercanos.
// Apunta al grupo mas grande. No alcanza a los voladores.
class TorreCanon : public Torre {
public:
    static constexpr int COSTO = 150;

    explicit TorreCanon(Vector2 posicion);

    bool PuedeAtacar(const Enemigo& enemigo) const override;  // solo enemigos de tierra
    Enemigo* ElegirObjetivo(const std::vector<Enemigo*>& candidatos) const override;
    void Disparar(Enemigo& objetivo, Partida& partida) override;
    void Dibujar() const override;
    const char* GetNombre() const override;
    const char* GetEstrategia() const override;
};
