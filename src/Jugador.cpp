#include "Jugador.h"

#include <algorithm>

Jugador::Jugador() : nombre_(""), dinero_(0), vidas_(0), puntaje_(0), eliminados_(0) {}

void Jugador::SetNombre(const std::string& nombre) {
    nombre_ = nombre;
}

const std::string& Jugador::GetNombre() const {
    return nombre_;
}

void Jugador::Reiniciar(int dinero, int vidas) {
    dinero_ = dinero;
    vidas_ = vidas;
    puntaje_ = 0;
    eliminados_ = 0;
}

int Jugador::GetDinero() const { return dinero_; }
int Jugador::GetVidas() const { return vidas_; }
int Jugador::GetPuntaje() const { return puntaje_; }
int Jugador::GetEliminados() const { return eliminados_; }

bool Jugador::PuedePagar(int costo) const {
    return dinero_ >= costo;
}

bool Jugador::Gastar(int costo) {
    if (!PuedePagar(costo)) return false;
    dinero_ -= costo;
    return true;
}

void Jugador::Ganar(int cantidad) {
    dinero_ += cantidad;
}

void Jugador::PerderVidas(int cantidad) {
    vidas_ = std::max(0, vidas_ - cantidad);
}

void Jugador::RegistrarEliminacion(int puntos) {
    eliminados_++;
    puntaje_ += puntos;
}

void Jugador::SumarPuntaje(int puntos) {
    puntaje_ += puntos;
}
