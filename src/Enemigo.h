#pragma once

#include "Entidad.h"

class Nivel;

// Tipo de danio que recibe un enemigo (algunos enemigos reaccionan distinto a cada uno)
enum class TipoDanio {
    Normal,     // azulejos de la Torre Talavera
    Explosion,  // balas del Canon de Cantera
    Agua        // chorro de la Torre de Agua: quita armadura
};

// Enemigo: clase base ABSTRACTA de todas las criaturas de Don Goyo.
// Sabe seguir el camino, recibir danio y morir. Cada hijo redefine lo que lo hace distinto.
class Enemigo : public Entidad {
public:
    Enemigo(const Nivel& nivel, float vida, float velocidad, int recompensa, int danioCiudad);

    void Actualizar(float dt, Partida& partida) override;  // avanza por el camino

    virtual void RecibirDanio(float cantidad, TipoDanio tipo, Partida& partida);
    virtual bool EsVolador() const;            // las torres de tierra no le pueden pegar a los voladores
    virtual bool EsJefe() const;               // el jefe tiene su barra de vida grande arriba de la pantalla
    virtual Vector2 GetPosicionBlanco() const;  // a donde apuntan las torres
    virtual const char* GetNombre() const = 0;  // virtual pura: cada enemigo dice su nombre

    void Ralentizar(float segundos, float factor);  // el agua los hace mas lentos
    void SetRuta(int ruta);                         // por cual camino va (0 = el primero)

    float GetVida() const;
    float GetVidaMax() const;
    float GetRecorrido() const;   // cuantos pixeles del camino lleva
    float GetFaltante() const;    // cuantos pixeles le faltan para llegar (para saber quien va mas adelante)
    int GetRecompensa() const;
    int GetDanioCiudad() const;   // cuantas vidas quita si llega a la ciudad
    bool LlegoALaCiudad() const;

protected:
    float GetAngulo() const;            // hacia donde camina
    bool FueGolpeadoRecien() const;     // para que parpadee en blanco al recibir un golpe
    bool EstaLento() const;
    void DibujarBarraVida(float ancho, float arriba) const;
    void DibujarEfectoLento(float radio) const;

private:
    const Nivel& nivel_;
    float vida_;
    float vidaMax_;
    float velocidad_;   // pixeles por segundo
    int recompensa_;    // dinero que da al morir
    int danioCiudad_;
    int ruta_;          // indice del camino que sigue
    float recorrido_;
    float angulo_;
    float tiempoLento_;
    float factorLento_;
    float tiempoGolpe_;
    bool llego_;
};
