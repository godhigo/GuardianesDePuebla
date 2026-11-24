#pragma once

#include <memory>
#include <vector>

#include "Entidad.h"

class Enemigo;

// Los tres tipos de torre que puede construir el jugador
enum class TipoTorre {
    Talavera,
    Canon,
    Agua
};

// Torre: clase base ABSTRACTA de las defensas.
// La base sabe buscar enemigos en su alcance, girar y esperar su cadencia.
// Cada hija decide A QUIEN atacar (ElegirObjetivo) y COMO dispara (Disparar).
class Torre : public Entidad {
public:
    Torre(Vector2 posicion, float alcance, float danio, float cadencia, int costo);

    // "final": las hijas ya no pueden cambiar este metodo; el ciclo de disparo es igual para todas
    void Actualizar(float dt, Partida& partida) override final;

    virtual bool PuedeAtacar(const Enemigo& enemigo) const;  // por defecto, a cualquiera
    virtual Enemigo* ElegirObjetivo(const std::vector<Enemigo*>& candidatos) const = 0;
    virtual void Disparar(Enemigo& objetivo, Partida& partida) = 0;
    virtual const char* GetNombre() const = 0;
    virtual const char* GetEstrategia() const = 0;  // texto corto que se muestra al seleccionarla

    bool PuedeMejorar() const;
    int GetCostoMejora() const;
    void Mejorar();               // mas alcance, mas danio y dispara mas rapido
    int GetValorVenta() const;    // la mitad de lo que se ha invertido

    float GetAlcance() const;
    float GetDanio() const;
    int GetNivel() const;

    // Vida de la torre: el Coloso de Lava las golpea y las puede destruir
    void RecibirDanio(float cantidad);
    float GetVida() const;
    float GetVidaMax() const;
    void DibujarVida() const;  // barra de vida (solo si ya la golpearon)

    // Fabrica: crea la torre del tipo pedido (regresa un puntero a la base, pero el objeto es la hija)
    static std::unique_ptr<Torre> Crear(TipoTorre tipo, Vector2 posicion);
    static int CostoDe(TipoTorre tipo);
    static const char* NombreDe(TipoTorre tipo);

    static constexpr int NIVEL_MAXIMO = 3;

protected:
    float GetAngulo() const;
    float GetRetroceso() const;                // 1 justo al disparar, baja a 0 (el canon "patea")
    Vector2 PuntaDelCanon(float largo) const;  // de donde sale el proyectil
    void DibujarSombra(bool redonda) const;
    void DibujarNivel() const;                 // puntitos amarillos debajo: uno por nivel

private:
    float alcance_;
    float danio_;
    float cadencia_;  // segundos entre disparos
    float espera_;    // segundos que faltan para el siguiente disparo
    float angulo_;
    float retroceso_;
    int nivel_;
    int costo_;       // lo que costo construirla
    int invertido_;   // costo + mejoras
    float vida_;
    float vidaMax_;
    float tiempoGolpe_;  // para que parpadee en rojo al recibir un golpe
};
