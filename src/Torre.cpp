#include "Torre.h"

#include <algorithm>
#include <cmath>

#include "Enemigo.h"
#include "Paleta.h"
#include "Partida.h"
#include "TorreAgua.h"
#include "TorreCanon.h"
#include "TorreTalavera.h"

const float VIDA_TORRE = 150.0f;  // todas las torres empiezan con 150 de vida

Torre::Torre(Vector2 posicion, float alcance, float danio, float cadencia, int costo)
    : Entidad(posicion), alcance_(alcance), danio_(danio), cadencia_(cadencia), espera_(0.0f),
      angulo_(-PI / 2.0f), retroceso_(0.0f), nivel_(1), costo_(costo), invertido_(costo),
      vida_(VIDA_TORRE), vidaMax_(VIDA_TORRE), tiempoGolpe_(0.0f) {}

void Torre::Actualizar(float dt, Partida& partida) {
    espera_ -= dt;
    if (tiempoGolpe_ > 0.0f) tiempoGolpe_ -= dt;
    retroceso_ = std::max(0.0f, retroceso_ - dt * 5.0f);

    // 1. Juntar los enemigos que esta torre puede atacar y estan dentro de su alcance
    std::vector<Enemigo*> candidatos;
    for (auto& enemigo : partida.GetEnemigos()) {
        if (!enemigo->EstaActivo() || !PuedeAtacar(*enemigo)) continue;
        Vector2 blanco = enemigo->GetPosicionBlanco();
        Vector2 p = GetPosicion();
        if (std::hypot(blanco.x - p.x, blanco.y - p.y) <= alcance_) {
            candidatos.push_back(enemigo.get());
        }
    }
    if (candidatos.empty()) return;

    // 2. Cada tipo de torre elige a quien atacar a su manera (polimorfismo)
    Enemigo* objetivo = ElegirObjetivo(candidatos);
    Vector2 blanco = objetivo->GetPosicionBlanco();
    angulo_ = std::atan2(blanco.y - GetPosicion().y, blanco.x - GetPosicion().x);

    // 3. Si ya paso su tiempo de recarga, dispara (cada tipo dispara distinto)
    if (espera_ <= 0.0f) {
        Disparar(*objetivo, partida);
        espera_ = cadencia_;
        retroceso_ = 1.0f;
    }
}

bool Torre::PuedeAtacar(const Enemigo&) const {
    return true;
}

bool Torre::PuedeMejorar() const {
    return nivel_ < NIVEL_MAXIMO;
}

int Torre::GetCostoMejora() const {
    return (int)std::round(costo_ * 0.9f * nivel_);  // nivel 2 cuesta 90% del costo, nivel 3 el 180%
}

void Torre::Mejorar() {
    invertido_ += GetCostoMejora();
    nivel_++;
    alcance_ *= 1.12f;
    danio_ *= 1.45f;
    cadencia_ *= 0.85f;
    vidaMax_ *= 1.3f;   // mejorada aguanta mas golpes
    vida_ = vidaMax_;   // y queda reparada
}

void Torre::RecibirDanio(float cantidad) {
    if (!EstaActivo()) return;
    vida_ -= cantidad;
    tiempoGolpe_ = 0.15f;
    if (vida_ <= 0.0f) {
        vida_ = 0.0f;
        Desactivar();  // Partida la borra al final del cuadro
    }
}

float Torre::GetVida() const { return vida_; }
float Torre::GetVidaMax() const { return vidaMax_; }

void Torre::DibujarVida() const {
    Vector2 p = GetPosicion();
    if (tiempoGolpe_ > 0.0f) {
        DrawCircleV(p, 21.0f, Fade(ROJO, 0.45f));  // destello rojo al recibir el golpe
    }
    float k = vida_ / vidaMax_;
    if (k >= 1.0f) return;  // con la vida llena no se dibuja la barra
    DrawRectangle((int)p.x - 18, (int)p.y - 29, 36, 6, Fade(BLACK, 0.7f));
    DrawRectangle((int)p.x - 17, (int)p.y - 28, (int)(34 * k), 4, k > 0.5f ? VERDE_VIDA : (k > 0.25f ? AMARILLO : ROJO));
}

int Torre::GetValorVenta() const { return invertido_ / 2; }
float Torre::GetAlcance() const { return alcance_; }
float Torre::GetDanio() const { return danio_; }
int Torre::GetNivel() const { return nivel_; }
float Torre::GetAngulo() const { return angulo_; }
float Torre::GetRetroceso() const { return retroceso_; }

Vector2 Torre::PuntaDelCanon(float largo) const {
    Vector2 p = GetPosicion();
    return Vector2{p.x + std::cos(angulo_) * largo, p.y + std::sin(angulo_) * largo};
}

void Torre::DibujarSombra(bool redonda) const {
    Vector2 p = GetPosicion();
    if (redonda) {
        DrawCircleV(Vector2{p.x + 3, p.y + 4}, 19.0f, Fade(BLACK, 0.28f));
    } else {
        DrawRectangleRounded(Rectangle{p.x - 14, p.y - 13, 34, 34}, 0.25f, 4, Fade(BLACK, 0.28f));
    }
}

void Torre::DibujarNivel() const {
    Vector2 p = GetPosicion();
    for (int i = 0; i < nivel_; i++) {
        Vector2 punto{p.x - (nivel_ - 1) * 4.0f + i * 8.0f, p.y + 23.0f};
        DrawCircleV(punto, 3.6f, Color{58, 42, 16, 255});
        DrawCircleV(punto, 2.8f, AMARILLO);
    }
}

std::unique_ptr<Torre> Torre::Crear(TipoTorre tipo, Vector2 posicion) {
    switch (tipo) {
    case TipoTorre::Talavera: return std::make_unique<TorreTalavera>(posicion);
    case TipoTorre::Canon:    return std::make_unique<TorreCanon>(posicion);
    case TipoTorre::Agua:     return std::make_unique<TorreAgua>(posicion);
    }
    return nullptr;
}

int Torre::CostoDe(TipoTorre tipo) {
    switch (tipo) {
    case TipoTorre::Talavera: return TorreTalavera::COSTO;
    case TipoTorre::Canon:    return TorreCanon::COSTO;
    case TipoTorre::Agua:     return TorreAgua::COSTO;
    }
    return 0;
}

const char* Torre::NombreDe(TipoTorre tipo) {
    switch (tipo) {
    case TipoTorre::Talavera: return "Torre Talavera";
    case TipoTorre::Canon:    return "Cañón de Cantera";
    case TipoTorre::Agua:     return "Torre de Agua";
    }
    return "";
}
