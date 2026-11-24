#include "TorreCanon.h"

#include <cmath>

#include "Enemigo.h"
#include "Paleta.h"
#include "Partida.h"
#include "Proyectil.h"

const float RADIO_EXPLOSION = 65.0f;

// alcance 170 px, danio 40, dispara cada 1.4 s
TorreCanon::TorreCanon(Vector2 posicion) : Torre(posicion, 170.0f, 40.0f, 1.4f, COSTO) {}

bool TorreCanon::PuedeAtacar(const Enemigo& enemigo) const {
    return !enemigo.EsVolador();
}

Enemigo* TorreCanon::ElegirObjetivo(const std::vector<Enemigo*>& candidatos) const {
    // Cuenta cuantos enemigos tiene cerca cada candidato y elige al del grupo mas grande
    Enemigo* elegido = candidatos[0];
    int mejorGrupo = -1;
    for (Enemigo* e : candidatos) {
        int grupo = 0;
        for (Enemigo* otro : candidatos) {
            Vector2 a = e->GetPosicion(), b = otro->GetPosicion();
            if (std::hypot(a.x - b.x, a.y - b.y) < 70.0f) grupo++;
        }
        if (grupo > mejorGrupo) {
            mejorGrupo = grupo;
            elegido = e;
        }
    }
    return elegido;
}

void TorreCanon::Disparar(Enemigo& objetivo, Partida& partida) {
    partida.AgregarProyectil(Proyectil(PuntaDelCanon(24.0f), &objetivo, 380.0f, GetDanio(), TipoDanio::Explosion, RADIO_EXPLOSION));
}

void TorreCanon::Dibujar() const {
    Vector2 p = GetPosicion();
    DibujarSombra(true);

    // Base de cantera con lineas de piedra
    DrawCircleV(p, 19.0f, Color{207, 179, 160, 255});
    DrawRing(p, 17.5f, 20.0f, 0, 360, 32, Color{142, 110, 92, 255});
    for (int i = 0; i < 8; i++) {
        float a = i * PI / 4.0f;
        DrawLineEx(Vector2{p.x + std::cos(a) * 11, p.y + std::sin(a) * 11}, Vector2{p.x + std::cos(a) * 18, p.y + std::sin(a) * 18},
                   1.5f, Color{168, 138, 119, 255});
    }
    DrawCircleV(p, 11.0f, Color{184, 152, 127, 255});

    // Canon de hierro
    float r = -GetRetroceso() * 5.0f;
    float grados = GetAngulo() * RAD2DEG;
    DrawRectanglePro(Rectangle{p.x, p.y, 26, 12}, Vector2{4 - r, 6}, grados, Color{52, 51, 59, 255});
    DrawRectanglePro(Rectangle{p.x, p.y, 5, 14}, Vector2{-20 - r, 7}, grados, Color{29, 28, 34, 255});
    DrawCircleV(p, 6.0f, Color{74, 72, 80, 255});

    DibujarNivel();
}

const char* TorreCanon::GetNombre() const { return "Cañón de Cantera"; }
const char* TorreCanon::GetEstrategia() const { return "Explota sobre el grupo mas grande."; }
