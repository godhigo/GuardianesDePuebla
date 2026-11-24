#include "Proyectil.h"

#include <cmath>

#include "Paleta.h"
#include "Partida.h"
#include "Particula.h"

Proyectil::Proyectil(Vector2 inicio, Enemigo* objetivo, float velocidad, float danio, TipoDanio tipo, float radioExplosion)
    : Entidad(inicio), objetivo_(objetivo), destino_(objetivo->GetPosicionBlanco()), velocidad_(velocidad),
      danio_(danio), tipo_(tipo), radioExplosion_(radioExplosion), giro_(0.0f) {}

void Proyectil::RevisarObjetivo() {
    if (objetivo_ != nullptr && !objetivo_->EstaActivo()) {
        objetivo_ = nullptr;
    }
}

void Proyectil::Actualizar(float dt, Partida& partida) {
    // Mientras el enemigo siga vivo, lo persigue
    if (objetivo_ != nullptr && objetivo_->EstaActivo()) {
        destino_ = objetivo_->GetPosicionBlanco();
    }

    Vector2 p = GetPosicion();
    float distancia = std::hypot(destino_.x - p.x, destino_.y - p.y);
    float paso = velocidad_ * dt;
    giro_ += dt * 14.0f;

    if (distancia <= paso) {
        SetPosicion(destino_);
        Impactar(partida);
        return;
    }
    SetPosicion(Vector2{p.x + (destino_.x - p.x) / distancia * paso, p.y + (destino_.y - p.y) / distancia * paso});

    // El agua deja gotitas en el camino
    if (tipo_ == TipoDanio::Agua && GetRandomValue(0, 100) < 40) {
        partida.AgregarParticula(Particula(GetPosicion(), Vector2{0, 0}, 0.25f, Color{159, 212, 255, 255}, 2.0f, FormaParticula::Punto));
    }
}

void Proyectil::Impactar(Partida& partida) {
    Desactivar();

    if (radioExplosion_ > 0.0f) {
        partida.DaniarArea(destino_, radioExplosion_, danio_, tipo_);
        partida.Explosion(destino_, radioExplosion_);
        return;
    }

    if (objetivo_ != nullptr && objetivo_->EstaActivo()) {
        objetivo_->RecibirDanio(danio_, tipo_, partida);
        if (tipo_ == TipoDanio::Agua && objetivo_->EstaActivo()) {
            objetivo_->Ralentizar(1.8f, 0.5f);  // 1.8 segundos a la mitad de velocidad
        }
    }
    Color color = tipo_ == TipoDanio::Agua ? Color{143, 208, 255, 255} : BLANCO;
    partida.Salpicar(destino_, color);
}

void Proyectil::Dibujar() const {
    Vector2 p = GetPosicion();
    switch (tipo_) {
    case TipoDanio::Normal:  // azulejo que gira
        DrawRectanglePro(Rectangle{p.x, p.y, 10, 10}, Vector2{5, 5}, giro_ * RAD2DEG, BLANCO);
        DrawRectanglePro(Rectangle{p.x, p.y, 5, 5}, Vector2{2.5f, 2.5f}, giro_ * RAD2DEG, AZUL);
        break;
    case TipoDanio::Explosion:  // bala de canon con su sombra
        DrawEllipse((int)p.x + 3, (int)p.y + 8, 6, 3, Fade(BLACK, 0.25f));
        DrawCircleV(p, 6.5f, Color{43, 42, 48, 255});
        DrawCircleV(Vector2{p.x - 2, p.y - 2}, 2.0f, Color{106, 104, 114, 255});
        break;
    case TipoDanio::Agua:  // gota
        DrawCircleV(p, 5.0f, Color{124, 194, 255, 255});
        DrawCircleV(Vector2{p.x - 1.5f, p.y - 1.5f}, 2.0f, Color{232, 246, 255, 255});
        break;
    }
}
