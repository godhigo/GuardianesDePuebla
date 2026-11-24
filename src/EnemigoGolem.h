#pragma once

#include "Enemigo.h"

// EnemigoGolem: lento y con mucha vida. Su armadura le resta danio a cada golpe;
// el agua se la va quitando.
class EnemigoGolem : public Enemigo {
public:
    explicit EnemigoGolem(const Nivel& nivel);

    void RecibirDanio(float cantidad, TipoDanio tipo, Partida& partida) override;  // la armadura reduce el golpe
    void Dibujar() const override;
    const char* GetNombre() const override;

    int GetArmadura() const;

private:
    int armadura_;  // cuanto le resta a cada golpe
};
