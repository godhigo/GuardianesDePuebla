#include "AtaqueJefe.h"

#include <cmath>

#include "Paleta.h"
#include "Partida.h"

AtaqueJefe::AtaqueJefe(Vector2 inicio, Vector2 destino, float duracion, float danio, TipoAtaque tipo)
    : Entidad(inicio), inicio_(inicio), destino_(destino), duracion_(duracion), tiempo_(0.0f), danio_(danio), tipo_(tipo) {}

void AtaqueJefe::Actualizar(float dt, Partida& partida) {
    tiempo_ += dt;
    float f = tiempo_ / duracion_;  // 0 = recien lanzado, 1 = llego

    if (f >= 1.0f) {
        // Cae: dania a las torres que esten ahi y hace una explosion
        partida.DaniarTorres(destino_, 26.0f, danio_);
        partida.Explosion(destino_, 30.0f);
        Desactivar();
        return;
    }

    // Vuela en linea recta pero subiendo y bajando (una parabola): la altura es mayor a la mitad del camino
    float altura = std::sin(f * PI) * (tipo_ == TipoAtaque::Roca ? 90.0f : 50.0f);
    SetPosicion(Vector2{inicio_.x + (destino_.x - inicio_.x) * f, inicio_.y + (destino_.y - inicio_.y) * f - altura});

    // Estela: el fuego deja chispas, la roca deja polvito
    if (GetRandomValue(0, 1) == 0) {
        Color color = tipo_ == TipoAtaque::BolaDeFuego ? NARANJA : Color{120, 104, 96, 255};
        FormaParticula forma = tipo_ == TipoAtaque::BolaDeFuego ? FormaParticula::Chispa : FormaParticula::Punto;
        partida.AgregarParticula(Particula(GetPosicion(), Vector2{0, -8}, 0.35f, color, 3.0f, forma));
    }
}

void AtaqueJefe::Dibujar() const {
    Vector2 p = GetPosicion();

    // Sombra en el suelo: marca donde va a caer
    float f = tiempo_ / duracion_;
    Vector2 sombra{inicio_.x + (destino_.x - inicio_.x) * f, inicio_.y + (destino_.y - inicio_.y) * f};
    DrawEllipse((int)sombra.x, (int)sombra.y, 9, 4, Fade(BLACK, 0.3f));

    if (tipo_ == TipoAtaque::BolaDeFuego) {
        DrawCircleV(p, 13.0f, Fade(Color{255, 110, 40, 255}, 0.35f));
        DrawCircleV(p, 8.0f, NARANJA);
        DrawCircleV(p, 4.0f, Color{255, 240, 170, 255});
    } else {
        float giro = tiempo_ * 400.0f;  // la roca da vueltas en el aire
        DrawPoly(p, 6, 10.0f, giro, Color{74, 62, 58, 255});
        DrawPolyLinesEx(p, 6, 10.0f, giro, 2.0f, Color{30, 24, 22, 255});
        DrawCircleV(Vector2{p.x + 2, p.y - 2}, 3.0f, Fade(NARANJA, 0.8f));  // un poco de lava pegada
    }
}
