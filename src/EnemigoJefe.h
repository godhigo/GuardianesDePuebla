#pragma once

#include "Enemigo.h"

class Torre;

// EnemigoJefe: clase base ABSTRACTA de los jefes (hija de Enemigo, madre de Coloso y Llamarada).
// Camina como cualquier enemigo, pero ademas cada cierto tiempo le lanza un ataque A DISTANCIA
// a la torre mas cercana. Cada jefe decide COMO es su ataque (LanzarAtaque): fuego, rocas...
class EnemigoJefe : public Enemigo {
public:
    EnemigoJefe(const Nivel& nivel, float vida, float velocidad, int recompensa, int danioCiudad,
                float alcanceAtaque, float cadenciaAtaque);

    void Actualizar(float dt, Partida& partida) override;  // camina Y ataca torres
    bool EsJefe() const override;                          // todos los jefes tienen barra grande arriba

protected:
    // Virtual pura: cada jefe lanza su propio ataque hacia "destino"
    virtual void LanzarAtaque(Vector2 destino, Partida& partida) = 0;

    void DibujarAlcanceAtaque() const;  // circulo rojo: las torres dentro estan en peligro
    float GetTiempo() const;            // segundos de vida (para animar)

private:
    Torre* BuscarTorreCercana(Partida& partida) const;

    float alcanceAtaque_;
    float cadenciaAtaque_;  // segundos entre ataque y ataque
    float esperaAtaque_;
    float tiempo_;
};
