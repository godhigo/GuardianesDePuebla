#include "Enemigo.h"

#include "Nivel.h"
#include "Paleta.h"
#include "Partida.h"

Enemigo::Enemigo(const Nivel& nivel, float vida, float velocidad, int recompensa, int danioCiudad)
    : Entidad(nivel.PuntoEn(0.0f).posicion), nivel_(nivel), vida_(vida), vidaMax_(vida), velocidad_(velocidad),
      recompensa_(recompensa), danioCiudad_(danioCiudad), ruta_(0), recorrido_(0.0f), angulo_(0.0f),
      tiempoLento_(0.0f), factorLento_(1.0f), tiempoGolpe_(0.0f), llego_(false) {}

void Enemigo::Actualizar(float dt, Partida&) {
    float velocidad = velocidad_;
    if (tiempoLento_ > 0.0f) {
        velocidad *= factorLento_;
        tiempoLento_ -= dt;
    }
    if (tiempoGolpe_ > 0.0f) tiempoGolpe_ -= dt;

    recorrido_ += velocidad * dt;
    if (recorrido_ >= nivel_.GetLargo(ruta_)) {
        llego_ = true;  // llego a la ciudad: Partida le quita vidas al jugador
        Desactivar();
        return;
    }

    PuntoCamino punto = nivel_.PuntoEn(recorrido_, ruta_);
    SetPosicion(punto.posicion);
    angulo_ = punto.angulo;
}

void Enemigo::RecibirDanio(float cantidad, TipoDanio, Partida& partida) {
    if (!EstaActivo()) return;

    vida_ -= cantidad;
    tiempoGolpe_ = 0.1f;
    if (vida_ <= 0.0f) {
        vida_ = 0.0f;
        Desactivar();
        partida.EnemigoEliminado(*this);  // Partida le da el dinero y los puntos al jugador
    }
}

bool Enemigo::EsVolador() const {
    return false;
}

bool Enemigo::EsJefe() const {
    return false;
}

Vector2 Enemigo::GetPosicionBlanco() const {
    return GetPosicion();
}

void Enemigo::Ralentizar(float segundos, float factor) {
    tiempoLento_ = segundos;
    factorLento_ = factor;
}

void Enemigo::SetRuta(int ruta) {
    if (ruta >= 0 && ruta < nivel_.GetTotalRutas()) ruta_ = ruta;  // si no existe esa ruta, se queda en la 0
}

float Enemigo::GetVida() const { return vida_; }
float Enemigo::GetVidaMax() const { return vidaMax_; }
float Enemigo::GetRecorrido() const { return recorrido_; }
float Enemigo::GetFaltante() const { return nivel_.GetLargo(ruta_) - recorrido_; }
int Enemigo::GetRecompensa() const { return recompensa_; }
int Enemigo::GetDanioCiudad() const { return danioCiudad_; }
bool Enemigo::LlegoALaCiudad() const { return llego_; }
float Enemigo::GetAngulo() const { return angulo_; }
bool Enemigo::FueGolpeadoRecien() const { return tiempoGolpe_ > 0.0f; }
bool Enemigo::EstaLento() const { return tiempoLento_ > 0.0f; }

void Enemigo::DibujarBarraVida(float ancho, float arriba) const {
    float k = vida_ / vidaMax_;
    if (k >= 1.0f) return;  // con la vida llena no se dibuja

    Vector2 p = GetPosicion();
    DrawRectangle((int)(p.x - ancho / 2 - 1), (int)(p.y - arriba - 1), (int)ancho + 2, 6, Fade(BLACK, 0.65f));
    Color color = k > 0.5f ? VERDE_VIDA : (k > 0.25f ? AMARILLO : ROJO);
    DrawRectangle((int)(p.x - ancho / 2), (int)(p.y - arriba), (int)(ancho * k), 4, color);
}

void Enemigo::DibujarEfectoLento(float radio) const {
    if (!EstaLento()) return;
    Vector2 p = GetPosicionBlanco();
    DrawRing(p, radio - 1.5f, radio, 0, 360, 24, Fade(Color{150, 210, 255, 255}, 0.9f));
}
