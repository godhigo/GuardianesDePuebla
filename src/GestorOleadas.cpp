#include "GestorOleadas.h"

#include "EnemigoChispa.h"
#include "EnemigoColoso.h"
#include "EnemigoGolem.h"
#include "EnemigoLlamarada.h"
#include "Nivel.h"
#include "Partida.h"

GestorOleadas::GestorOleadas() : lanzadas_(0), siguienteRuta_(0), espera_(0.0f), activa_(false) {}

void GestorOleadas::Configurar(const std::vector<std::vector<GrupoOleada>>& oleadas) {
    oleadas_ = oleadas;
    cola_.clear();
    lanzadas_ = 0;
    siguienteRuta_ = 0;
    espera_ = 0.0f;
    activa_ = false;
}

bool GestorOleadas::PuedeLanzar() const {
    return !activa_ && lanzadas_ < (int)oleadas_.size();
}

bool GestorOleadas::Lanzar() {
    if (!PuedeLanzar()) return false;

    // Formamos en la cola a cada enemigo de cada grupo de la oleada
    for (const GrupoOleada& grupo : oleadas_[lanzadas_]) {
        for (int i = 0; i < grupo.cantidad; i++) {
            cola_.push_back(grupo);
        }
    }
    lanzadas_++;
    activa_ = true;
    espera_ = 0.0f;
    return true;
}

void GestorOleadas::Actualizar(float dt, Partida& partida) {
    if (!activa_) return;

    if (!cola_.empty()) {
        espera_ -= dt;
        if (espera_ <= 0.0f) {
            GrupoOleada siguiente = cola_.front();
            cola_.pop_front();
            std::unique_ptr<Enemigo> enemigo = CrearEnemigo(siguiente.tipo, partida.GetNivel());

            // Si el nivel tiene varios caminos, los enemigos se turnan: uno por cada camino
            int totalRutas = partida.GetNivel().GetTotalRutas();
            enemigo->SetRuta(siguienteRuta_);
            siguienteRuta_ = (siguienteRuta_ + 1) % totalRutas;

            partida.AgregarEnemigo(std::move(enemigo));
            espera_ = siguiente.intervalo;
        }
    } else if (partida.GetEnemigos().empty()) {
        // Ya salieron todos y no queda ninguno vivo: la oleada termino
        activa_ = false;
        partida.OleadaTerminada();
    }
}

int GestorOleadas::GetNumero() const { return lanzadas_; }
int GestorOleadas::GetTotal() const { return (int)oleadas_.size(); }
bool GestorOleadas::EstaActiva() const { return activa_; }

std::string GestorOleadas::Describir(int indice) const {
    if (indice < 0 || indice >= (int)oleadas_.size()) return "";
    std::string texto;
    for (const GrupoOleada& grupo : oleadas_[indice]) {
        if (!texto.empty()) texto += ", ";
        std::string nombre = "Chispa";
        if (grupo.tipo == "GOLEM") nombre = "Gólem";
        bool esJefe = grupo.tipo == "COLOSO" || grupo.tipo == "LLAMARADA";
        if (esJefe) nombre = "JEFE: " + std::string(grupo.tipo == "COLOSO" ? "Coloso" : "Llamarada");
        texto += std::to_string(grupo.cantidad) + " " + nombre + (grupo.cantidad == 1 || esJefe ? "" : "s");
    }
    return texto;
}

std::unique_ptr<Enemigo> GestorOleadas::CrearEnemigo(const std::string& tipo, const Nivel& nivel) {
    if (tipo == "GOLEM") return std::make_unique<EnemigoGolem>(nivel);
    if (tipo == "COLOSO") return std::make_unique<EnemigoColoso>(nivel);
    if (tipo == "LLAMARADA") return std::make_unique<EnemigoLlamarada>(nivel);
    return std::make_unique<EnemigoChispa>(nivel);  // CHISPA (o cualquier tipo desconocido)
}
