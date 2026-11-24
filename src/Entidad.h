#pragma once

#include "raylib.h"

class Partida;  // aviso: "Partida existe", sin incluir todo su .h (evita que los .h se incluyan en circulo)

// Entidad: clase base ABSTRACTA de todo lo que vive en el mapa (enemigos, torres, proyectiles, particulas).
// No se pueden crear objetos Entidad directamente: Actualizar y Dibujar son virtuales puras (= 0).
class Entidad {
public:
    explicit Entidad(Vector2 posicion);
    virtual ~Entidad() = default;  // destructor virtual: al borrar un hijo por un puntero a Entidad, se borra completo

    virtual void Actualizar(float dt, Partida& partida) = 0;  // cada hijo decide como se mueve
    virtual void Dibujar() const = 0;                         // cada hijo decide como se ve

    Vector2 GetPosicion() const;
    bool EstaActivo() const;  // false = ya se debe borrar al final del cuadro
    void Desactivar();

protected:
    void SetPosicion(Vector2 posicion);  // solo los hijos pueden moverse

private:
    Vector2 posicion_;
    bool activo_;
};
