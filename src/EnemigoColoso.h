#pragma once

#include "EnemigoJefe.h"

// EnemigoColoso: jefe del nivel 2, hecho de piedra y lava. Enorme, lento y con muchisima vida.
// Le lanza rocas a las torres que tiene cerca.
class EnemigoColoso : public EnemigoJefe {
public:
    explicit EnemigoColoso(const Nivel& nivel);

    void Dibujar() const override;
    const char* GetNombre() const override;

protected:
    void LanzarAtaque(Vector2 destino, Partida& partida) override;  // avienta una roca
};
