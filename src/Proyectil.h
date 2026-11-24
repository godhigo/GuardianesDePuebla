#pragma once

#include "Entidad.h"
#include "Enemigo.h"

// Proyectil: viaja hacia su enemigo y le hace danio al llegar.
// Si tiene radio de explosion, dania a todos los enemigos de tierra cercanos.
class Proyectil : public Entidad {
public:
    Proyectil(Vector2 inicio, Enemigo* objetivo, float velocidad, float danio, TipoDanio tipo, float radioExplosion);

    void Actualizar(float dt, Partida& partida) override;
    void Dibujar() const override;

    // Si su objetivo ya murio, suelta el puntero y sigue hacia el ultimo lugar donde lo vio.
    // Partida lo llama ANTES de borrar a los enemigos muertos, asi nunca apunta a memoria liberada.
    void RevisarObjetivo();

private:
    void Impactar(Partida& partida);

    Enemigo* objetivo_;  // apuntador al enemigo (no es dueno de el: el dueno es Partida)
    Vector2 destino_;
    float velocidad_;
    float danio_;
    TipoDanio tipo_;
    float radioExplosion_;
    float giro_;  // el azulejo gira en el aire
};
