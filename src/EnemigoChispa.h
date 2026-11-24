#pragma once

#include "Enemigo.h"

// EnemigoChispa: rapida y fragil. Avanza en zigzag soltando chispas.
class EnemigoChispa : public Enemigo {
public:
    explicit EnemigoChispa(const Nivel& nivel);

    void Actualizar(float dt, Partida& partida) override;  // camina como todos y ademas zigzaguea
    void Dibujar() const override;
    const char* GetNombre() const override;

private:
    float tiempo_;  // para el zigzag y el brillo
};
