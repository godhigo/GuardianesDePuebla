#pragma once

#include "EnemigoJefe.h"

// EnemigoLlamarada: jefe del nivel 1, una Chispa gigante hecha de puro fuego.
// Mas rapida que el Coloso y con menos vida, le lanza bolas de fuego a las torres.
class EnemigoLlamarada : public EnemigoJefe {
public:
    explicit EnemigoLlamarada(const Nivel& nivel);

    void Dibujar() const override;
    const char* GetNombre() const override;

protected:
    void LanzarAtaque(Vector2 destino, Partida& partida) override;  // avienta una bola de fuego
};
