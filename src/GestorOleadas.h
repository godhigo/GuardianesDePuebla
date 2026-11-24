#pragma once

#include <deque>
#include <memory>
#include <string>
#include <vector>

#include "GestorArchivos.h"

class Enemigo;
class Nivel;
class Partida;

// GestorOleadas: suelta los enemigos de cada oleada, uno por uno, con su tiempo de espera
class GestorOleadas {
public:
    GestorOleadas();

    void Configurar(const std::vector<std::vector<GrupoOleada>>& oleadas);
    bool PuedeLanzar() const;  // no hay oleada en curso y todavia quedan oleadas
    bool Lanzar();             // empieza la siguiente oleada
    void Actualizar(float dt, Partida& partida);

    int GetNumero() const;     // cuantas oleadas se han lanzado
    int GetTotal() const;
    bool EstaActiva() const;
    std::string Describir(int indice) const;  // "8 Chispas, 2 Gólems"

    // Fabrica: convierte el texto del archivo ("CHISPA") en el enemigo correcto
    static std::unique_ptr<Enemigo> CrearEnemigo(const std::string& tipo, const Nivel& nivel);

private:
    std::vector<std::vector<GrupoOleada>> oleadas_;
    std::deque<GrupoOleada> cola_;  // enemigos que faltan por salir en la oleada actual
    int lanzadas_;
    int siguienteRuta_;  // a que camino mandamos al proximo enemigo (se van turnando)
    float espera_;
    bool activa_;
};
