#include "Entidad.h"

Entidad::Entidad(Vector2 posicion) : posicion_(posicion), activo_(true) {}

Vector2 Entidad::GetPosicion() const {
    return posicion_;
}

bool Entidad::EstaActivo() const {
    return activo_;
}

void Entidad::Desactivar() {
    activo_ = false;
}

void Entidad::SetPosicion(Vector2 posicion) {
    posicion_ = posicion;
}
