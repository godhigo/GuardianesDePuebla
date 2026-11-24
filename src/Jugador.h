#pragma once

#include <string>

// Jugador: guarda los datos de quien esta jugando: nombre, dinero, vidas y puntaje
class Jugador {
public:
    Jugador();  // constructor: empieza sin nombre

    void SetNombre(const std::string& nombre);  // guarda el nombre
    const std::string& GetNombre() const;       // regresa el nombre guardado

    void Reiniciar(int dinero, int vidas);  // al empezar un nivel (el nombre se conserva)

    int GetDinero() const;
    int GetVidas() const;
    int GetPuntaje() const;
    int GetEliminados() const;

    bool PuedePagar(int costo) const;
    bool Gastar(int costo);          // regresa false si no le alcanza
    void Ganar(int cantidad);
    void PerderVidas(int cantidad);
    void RegistrarEliminacion(int puntos);
    void SumarPuntaje(int puntos);

private:
    std::string nombre_;
    int dinero_;
    int vidas_;
    int puntaje_;
    int eliminados_;  // enemigos eliminados en el nivel
};
