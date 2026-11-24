#pragma once

#include "Entidad.h"

// Que lanza el jefe
enum class TipoAtaque {
    BolaDeFuego,  // la Llamarada (nivel 1)
    Roca          // el Coloso (nivel 2)
};

// AtaqueJefe: lo que un jefe le avienta a las torres. Vuela en curva desde el jefe hasta donde
// estaba la torre y al caer dania a las torres de esa zona.
// Guarda el PUNTO de destino (no un puntero a la torre): si la venden mientras vuela, no pasa nada malo.
class AtaqueJefe : public Entidad {
public:
    AtaqueJefe(Vector2 inicio, Vector2 destino, float duracion, float danio, TipoAtaque tipo);

    void Actualizar(float dt, Partida& partida) override;
    void Dibujar() const override;

private:
    Vector2 inicio_;
    Vector2 destino_;
    float duracion_;  // segundos que tarda en llegar
    float tiempo_;    // segundos que lleva volando
    float danio_;
    TipoAtaque tipo_;
};
