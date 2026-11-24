#pragma once

#include <memory>
#include <string>
#include <vector>

#include "raylib.h"
#include "AtaqueJefe.h"
#include "Enemigo.h"
#include "GestorOleadas.h"
#include "Jugador.h"
#include "Nivel.h"
#include "Particula.h"
#include "Proyectil.h"
#include "Torre.h"

// Estados de una partida (dentro de la pantalla "Jugando")
enum class EstadoPartida {
    Jugando,
    Pausa,
    Victoria,
    Derrota
};

// Partida: un nivel en juego. Es duena de todos los objetos del mapa y aplica las reglas:
// dinero, vidas, oleadas, victoria y derrota. Juego solo le pasa el control cuando estamos jugando.
class Partida {
public:
    Partida();

    bool Iniciar(const std::string& rutaNivel, Jugador& jugador);  // false si no se pudo leer el nivel
    void Descargar();

    void ManejarEntrada();      // mouse y teclado
    void Actualizar(float dt);  // reglas y movimiento de todo
    void Dibujar() const;

    bool QuiereVolverAlMenu() const;
    bool QuiereSiguienteNivel() const;          // gano y presiono ENTER para pasar al siguiente nivel
    std::string GetSiguienteNivel() const;      // archivo del siguiente nivel ("" si este es el ultimo)
    bool QuiereFinal() const;                   // gano el ULTIMO nivel y presiono ENTER: toca la pantalla final
    const std::string& GetError() const;
    EstadoPartida GetEstado() const;

    // ---- Lo que usan enemigos, torres y proyectiles ----
    const Nivel& GetNivel() const;
    std::vector<std::unique_ptr<Enemigo>>& GetEnemigos();
    void AgregarEnemigo(std::unique_ptr<Enemigo> enemigo);
    void AgregarProyectil(const Proyectil& proyectil);
    void AgregarAtaqueJefe(const AtaqueJefe& ataque);
    void DaniarTorres(Vector2 centro, float radio, float danio);  // cuando cae el ataque de un jefe
    void AgregarParticula(const Particula& particula);
    void EnemigoEliminado(const Enemigo& enemigo);
    void OleadaTerminada();
    void DaniarArea(Vector2 centro, float radio, float danio, TipoDanio tipo);
    void Explosion(Vector2 centro, float radio);
    void Salpicar(Vector2 punto, Color color);

    // ---- Acciones del jugador (publicas para poder probarlas sin mouse) ----
    bool ColocarTorre(TipoTorre tipo, int columna, int fila);
    bool LanzarOleada();
    void MejorarSeleccionada();
    void VenderSeleccionada();
    void Seleccionar(int columna, int fila);
    const std::vector<std::unique_ptr<Torre>>& GetTorres() const;
    const Jugador& GetJugador() const;
    const GestorOleadas& GetOleadas() const;

private:
    Torre* TorreEn(int columna, int fila) const;
    void Terminar(bool gano);
    void Avisar(const std::string& texto, float segundos = 1.8f);
    void ClicEnMapa(Vector2 raton);
    void ClicEnPanel(Vector2 raton);
    void EmitirHumoVolcan(float dt);

    // Dibujo
    void DibujarMapa() const;
    void DibujarFantasma() const;  // vista previa de la torre bajo el mouse
    void DibujarBarraSuperior() const;
    void DibujarPanel() const;
    void DibujarAvisos() const;
    void DibujarRobo() const;       // el ladron que se roba parte del bono
    void DibujarBarraJefe() const;  // vida del Coloso arriba de la pantalla
    void DibujarPausa() const;
    void DibujarFinal() const;

    // Zonas del panel de abajo
    static Rectangle CajaCarta(int indice);
    static Rectangle BotonOleada();
    static Rectangle BotonMejorar();
    static Rectangle BotonVender();

    Nivel nivel_;
    Jugador* jugador_;  // apunta al Jugador que vive en Juego
    GestorOleadas oleadas_;
    std::string rutaNivel_;

    // Colecciones de objetos: crecen y se vacian mientras se juega
    std::vector<std::unique_ptr<Enemigo>> enemigos_;  // unique_ptr: cada enemigo puede ser de una clase hija distinta
    std::vector<std::unique_ptr<Torre>> torres_;
    std::vector<Proyectil> proyectiles_;
    std::vector<AtaqueJefe> ataquesJefe_;             // fuego y rocas que los jefes le avientan a las torres
    std::vector<Particula> particulas_;
    std::vector<std::unique_ptr<Torre>> iconos_;      // torres de muestra para el panel

    EstadoPartida estado_;
    TipoTorre tipoElegido_;
    Torre* seleccionada_;
    float velocidad_;     // 1 = normal, 2 = rapido
    float tiempo_;
    std::string aviso_;
    float tiempoAviso_;
    std::string anuncio_;  // "Oleada 2" en grande
    float tiempoAnuncio_;
    int mejorPuntaje_;
    bool volverAlMenu_;
    bool irSiguiente_;
    bool irFinal_;
    std::string ladron_;      // "El Tlacuache" o "La Comadreja"
    std::string textoRobo_;   // "se robo $40 de tu bono"
    float tiempoRobo_;        // segundos que se sigue viendo el aviso del robo
    bool cargada_;
    std::string error_;
};
