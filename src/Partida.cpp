#include "Partida.h"

#include <algorithm>
#include <cmath>

#include "GestorArchivos.h"
#include "Paleta.h"

namespace {
const int MAX_PARTICULAS = 700;

// Texto con contorno centrado en la ventana
void Centrado(const char* texto, int y, int tamano, Color color) {
    DibujarTextoContorno(texto, (1280 - MedirTexto(texto, tamano)) / 2, y, tamano, color);
}

float Azar(float a, float b) {
    return a + (b - a) * (GetRandomValue(0, 10000) / 10000.0f);
}

const TipoTorre TIPOS[] = {TipoTorre::Talavera, TipoTorre::Canon, TipoTorre::Agua};
}  // namespace

Partida::Partida()
    : jugador_(nullptr), estado_(EstadoPartida::Jugando), tipoElegido_(TipoTorre::Talavera), seleccionada_(nullptr),
      velocidad_(1.0f), tiempo_(0.0f), tiempoAviso_(0.0f), tiempoAnuncio_(0.0f), mejorPuntaje_(0),
      volverAlMenu_(false), irSiguiente_(false), irFinal_(false), tiempoRobo_(0.0f), cargada_(false) {
    // Torres de muestra para dibujar en las tarjetas del panel
    for (int i = 0; i < 3; i++) {
        Rectangle carta = CajaCarta(i);
        iconos_.push_back(Torre::Crear(TIPOS[i], Vector2{carta.x + 40, carta.y + 34}));
    }
}

bool Partida::Iniciar(const std::string& rutaNivel, Jugador& jugador) {
    rutaNivel_ = rutaNivel;
    jugador_ = &jugador;

    DatosNivel datos;
    if (!GestorArchivos::LeerNivel(rutaNivel, datos, error_)) {
        cargada_ = false;
        return false;
    }

    // Limpiamos todo lo de una partida anterior
    enemigos_.clear();
    torres_.clear();
    proyectiles_.clear();
    ataquesJefe_.clear();
    particulas_.clear();
    seleccionada_ = nullptr;

    nivel_.Descargar();
    nivel_.Cargar(datos);
    if (IsWindowReady()) nivel_.GenerarFondo();  // sin ventana (en pruebas) no se puede pintar

    jugador.Reiniciar(datos.dinero, datos.vidas);
    oleadas_.Configurar(datos.oleadas);
    mejorPuntaje_ = GestorArchivos::LeerMejorPuntaje(datos.nombre);

    estado_ = EstadoPartida::Jugando;
    tipoElegido_ = TipoTorre::Talavera;
    velocidad_ = 1.0f;
    tiempo_ = 0.0f;
    aviso_.clear();
    tiempoAviso_ = 0.0f;
    anuncio_ = "Nivel " + std::to_string(datos.numero) + ": " + datos.nombre;
    tiempoAnuncio_ = 2.5f;
    volverAlMenu_ = false;
    irSiguiente_ = false;
    irFinal_ = false;
    tiempoRobo_ = 0.0f;
    cargada_ = true;
    if (nivel_.GetTotalRutas() > 1) {
        Avisar("Cuidado: los enemigos vienen por " + std::to_string(nivel_.GetTotalRutas()) + " caminos", 4.0f);
    } else {
        Avisar("Coloca torres junto al camino y presiona ESPACIO", 4.0f);
    }
    return true;
}

void Partida::Descargar() {
    nivel_.Descargar();
}

bool Partida::QuiereVolverAlMenu() const { return volverAlMenu_; }
bool Partida::QuiereSiguienteNivel() const { return irSiguiente_; }
bool Partida::QuiereFinal() const { return irFinal_; }
std::string Partida::GetSiguienteNivel() const { return nivel_.GetDatos().siguiente; }
const std::string& Partida::GetError() const { return error_; }
EstadoPartida Partida::GetEstado() const { return estado_; }
const Nivel& Partida::GetNivel() const { return nivel_; }
std::vector<std::unique_ptr<Enemigo>>& Partida::GetEnemigos() { return enemigos_; }
const std::vector<std::unique_ptr<Torre>>& Partida::GetTorres() const { return torres_; }
const Jugador& Partida::GetJugador() const { return *jugador_; }
const GestorOleadas& Partida::GetOleadas() const { return oleadas_; }

// ---------- Entrada (mouse y teclado) ----------

void Partida::ManejarEntrada() {
    if (!cargada_) {
        if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_ENTER)) volverAlMenu_ = true;
        return;
    }

    if (estado_ == EstadoPartida::Victoria || estado_ == EstadoPartida::Derrota) {
        bool gano = estado_ == EstadoPartida::Victoria;
        bool haySiguiente = gano && !GetSiguienteNivel().empty();
        bool ganoElUltimo = gano && GetSiguienteNivel().empty();
        if (IsKeyPressed(KEY_R)) {
            Iniciar(rutaNivel_, *jugador_);
        } else if (IsKeyPressed(KEY_ENTER)) {
            if (haySiguiente) irSiguiente_ = true;     // Juego carga el siguiente nivel (despues de P4)
            else if (ganoElUltimo) irFinal_ = true;    // Juego muestra la pantalla final (P5)
            else volverAlMenu_ = true;                 // perdio: regresa al menu
        } else if (gano && IsKeyPressed(KEY_M)) {
            volverAlMenu_ = true;
        }
        return;
    }

    if (estado_ == EstadoPartida::Pausa) {
        if (IsKeyPressed(KEY_P) || IsKeyPressed(KEY_ESCAPE)) estado_ = EstadoPartida::Jugando;
        if (IsKeyPressed(KEY_M)) volverAlMenu_ = true;
        return;
    }

    // Jugando
    if (IsKeyPressed(KEY_P) || IsKeyPressed(KEY_ESCAPE)) {
        estado_ = EstadoPartida::Pausa;
        return;
    }
    if (IsKeyPressed(KEY_ONE)) tipoElegido_ = TipoTorre::Talavera;
    if (IsKeyPressed(KEY_TWO)) tipoElegido_ = TipoTorre::Canon;
    if (IsKeyPressed(KEY_THREE)) tipoElegido_ = TipoTorre::Agua;
    if (IsKeyPressed(KEY_SPACE)) LanzarOleada();
    if (IsKeyPressed(KEY_F)) velocidad_ = velocidad_ == 1.0f ? 2.0f : 1.0f;
    if (IsKeyPressed(KEY_U)) MejorarSeleccionada();
    if (IsKeyPressed(KEY_V)) VenderSeleccionada();

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Vector2 raton = GetMousePosition();
        if (raton.y >= Nivel::Y_PANEL) ClicEnPanel(raton);
        else if (raton.y > Nivel::ALTO_BARRA) ClicEnMapa(raton);
    }
    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) seleccionada_ = nullptr;
}

void Partida::ClicEnMapa(Vector2 raton) {
    int columna = (int)(raton.x / Nivel::CELDA);
    int fila = (int)(raton.y / Nivel::CELDA);

    if (TorreEn(columna, fila) != nullptr) {
        Seleccionar(columna, fila);
    } else if (nivel_.CasillaValida(columna, fila)) {
        ColocarTorre(tipoElegido_, columna, fila);
    } else if (seleccionada_ != nullptr) {
        seleccionada_ = nullptr;
    } else {
        Avisar("Ahi no se puede construir");
    }
}

void Partida::ClicEnPanel(Vector2 raton) {
    for (int i = 0; i < 3; i++) {
        if (CheckCollisionPointRec(raton, CajaCarta(i))) tipoElegido_ = TIPOS[i];
    }
    if (CheckCollisionPointRec(raton, BotonOleada())) LanzarOleada();
    if (seleccionada_ != nullptr) {
        if (CheckCollisionPointRec(raton, BotonMejorar())) MejorarSeleccionada();
        else if (CheckCollisionPointRec(raton, BotonVender())) VenderSeleccionada();
    }
}

// ---------- Acciones del jugador ----------

Torre* Partida::TorreEn(int columna, int fila) const {
    for (const auto& torre : torres_) {
        Vector2 p = torre->GetPosicion();
        if ((int)(p.x / Nivel::CELDA) == columna && (int)(p.y / Nivel::CELDA) == fila) return torre.get();
    }
    return nullptr;
}

bool Partida::ColocarTorre(TipoTorre tipo, int columna, int fila) {
    if (!nivel_.CasillaValida(columna, fila) || TorreEn(columna, fila) != nullptr) return false;

    int costo = Torre::CostoDe(tipo);
    if (!jugador_->Gastar(costo)) {
        Avisar("Te faltan $" + std::to_string(costo - jugador_->GetDinero()) + " para " + Torre::NombreDe(tipo));
        return false;
    }

    Vector2 centro{columna * Nivel::CELDA + Nivel::CELDA / 2.0f, fila * Nivel::CELDA + Nivel::CELDA / 2.0f};
    torres_.push_back(Torre::Crear(tipo, centro));
    seleccionada_ = nullptr;

    // Polvito al construir
    for (int i = 0; i < 10; i++) {
        float a = Azar(0, 2 * PI);
        Vector2 inicio{centro.x + std::cos(a) * 16, centro.y + std::sin(a) * 16};
        AgregarParticula(Particula(inicio, Vector2{std::cos(a) * 40, std::sin(a) * 40}, 0.4f, Color{233, 214, 196, 255}, 3.0f,
                                   FormaParticula::Punto));
    }
    return true;
}

bool Partida::LanzarOleada() {
    if (estado_ != EstadoPartida::Jugando || !oleadas_.Lanzar()) return false;
    anuncio_ = "Oleada " + std::to_string(oleadas_.GetNumero());
    tiempoAnuncio_ = 2.0f;
    aviso_.clear();
    seleccionada_ = nullptr;
    return true;
}

void Partida::Seleccionar(int columna, int fila) {
    Torre* torre = TorreEn(columna, fila);
    seleccionada_ = (torre == seleccionada_) ? nullptr : torre;  // clic otra vez en la misma: se deselecciona
}

void Partida::MejorarSeleccionada() {
    if (seleccionada_ == nullptr) return;
    if (!seleccionada_->PuedeMejorar()) {
        Avisar("Esta torre ya esta al nivel maximo");
        return;
    }
    int costo = seleccionada_->GetCostoMejora();
    if (!jugador_->Gastar(costo)) {
        Avisar("Te faltan $" + std::to_string(costo - jugador_->GetDinero()) + " para mejorar");
        return;
    }
    seleccionada_->Mejorar();
    for (int i = 0; i < 14; i++) {
        float a = Azar(0, 2 * PI);
        AgregarParticula(Particula(seleccionada_->GetPosicion(), Vector2{std::cos(a) * 70, std::sin(a) * 70 - 20}, 0.6f, AMARILLO,
                                   2.6f, FormaParticula::Chispa));
    }
}

void Partida::VenderSeleccionada() {
    if (seleccionada_ == nullptr) return;
    int valor = seleccionada_->GetValorVenta();
    jugador_->Ganar(valor);
    Avisar("Vendida: +$" + std::to_string(valor));

    // Borramos la torre de la coleccion (el unique_ptr libera su memoria)
    Torre* vendida = seleccionada_;
    seleccionada_ = nullptr;
    torres_.erase(std::remove_if(torres_.begin(), torres_.end(),
                                 [vendida](const std::unique_ptr<Torre>& t) { return t.get() == vendida; }),
                  torres_.end());
}

// ---------- Lo que llaman los objetos del mapa ----------

void Partida::AgregarEnemigo(std::unique_ptr<Enemigo> enemigo) {
    enemigos_.push_back(std::move(enemigo));  // move: el unique_ptr pasa a ser de la coleccion
}

void Partida::AgregarProyectil(const Proyectil& proyectil) {
    proyectiles_.push_back(proyectil);
}

void Partida::AgregarAtaqueJefe(const AtaqueJefe& ataque) {
    ataquesJefe_.push_back(ataque);
}

void Partida::DaniarTorres(Vector2 centro, float radio, float danio) {
    for (auto& torre : torres_) {
        Vector2 p = torre->GetPosicion();
        if (std::hypot(p.x - centro.x, p.y - centro.y) <= radio) {
            torre->RecibirDanio(danio);
        }
    }
}

void Partida::AgregarParticula(const Particula& particula) {
    if ((int)particulas_.size() < MAX_PARTICULAS) particulas_.push_back(particula);
}

void Partida::EnemigoEliminado(const Enemigo& enemigo) {
    jugador_->Ganar(enemigo.GetRecompensa());
    jugador_->RegistrarEliminacion(enemigo.GetRecompensa() * 10);

    // Estallido: los grandes sueltan mas pedazos
    bool grande = enemigo.GetDanioCiudad() > 1;
    int pedazos = grande ? 18 : 12;
    for (int i = 0; i < pedazos; i++) {
        float a = Azar(0, 2 * PI), v = Azar(40, 130);
        bool roca = grande && i % 2 == 1;
        AgregarParticula(Particula(enemigo.GetPosicion(), Vector2{std::cos(a) * v, std::sin(a) * v}, 0.55f,
                                   roca ? Color{74, 60, 57, 255} : NARANJA, grande ? 4.0f : 2.6f,
                                   roca ? FormaParticula::Punto : FormaParticula::Chispa, 120.0f));
    }
}

void Partida::OleadaTerminada() {
    int numero = oleadas_.GetNumero();
    if (numero >= oleadas_.GetTotal()) {
        Terminar(true);
        return;
    }
    int bono = 50 + numero * 10;

    // Ladrones: en los niveles que los tienen, se roban una parte del bono.
    // Se turnan: en las oleadas impares llega El Tlacuache y en las pares La Comadreja
    int porcentaje = nivel_.GetDatos().ladrones;
    if (porcentaje > 0) {
        int robo = bono * porcentaje / 100;
        bono -= robo;
        ladron_ = (numero % 2 == 1) ? "El Tlacuache" : "La Comadreja";
        textoRobo_ = "se robo $" + std::to_string(robo) + " de tu bono";
        tiempoRobo_ = 3.0f;
    }

    jugador_->Ganar(bono);
    Avisar("Oleada " + std::to_string(numero) + " superada: +$" + std::to_string(bono), 3.0f);
}

void Partida::DaniarArea(Vector2 centro, float radio, float danio, TipoDanio tipo) {
    for (auto& enemigo : enemigos_) {
        if (!enemigo->EstaActivo() || enemigo->EsVolador()) continue;
        Vector2 p = enemigo->GetPosicion();
        if (std::hypot(p.x - centro.x, p.y - centro.y) <= radio) {
            enemigo->RecibirDanio(danio, tipo, *this);
        }
    }
}

void Partida::Explosion(Vector2 centro, float radio) {
    AgregarParticula(Particula(centro, Vector2{0, 0}, 0.45f, Color{255, 179, 71, 255}, radio, FormaParticula::Anillo));
    for (int i = 0; i < 14; i++) {
        float a = Azar(0, 2 * PI), v = Azar(60, 180);
        bool chispa = i % 3 != 0;
        AgregarParticula(Particula(centro, Vector2{std::cos(a) * v, std::sin(a) * v}, 0.5f,
                                   chispa ? Color{255, 138, 60, 255} : Color{91, 82, 76, 255}, 3.0f,
                                   chispa ? FormaParticula::Chispa : FormaParticula::Punto, 140.0f));
    }
    AgregarParticula(Particula(centro, Vector2{0, -12}, 1.0f, Color{141, 132, 120, 255}, 18.0f, FormaParticula::Humo));
}

void Partida::Salpicar(Vector2 punto, Color color) {
    for (int i = 0; i < 6; i++) {
        float a = Azar(0, 2 * PI);
        Color c = (i % 2 == 1 && color.r == BLANCO.r) ? AZUL : color;  // el azulejo suelta pedacitos blancos y azules
        AgregarParticula(Particula(punto, Vector2{std::cos(a) * 60, std::sin(a) * 60}, 0.3f, c, 2.2f, FormaParticula::Punto));
    }
}

// ---------- Logica de cada cuadro ----------

void Partida::Actualizar(float dt) {
    if (tiempoAviso_ > 0.0f) tiempoAviso_ -= dt;
    if (tiempoRobo_ > 0.0f) tiempoRobo_ -= dt;
    if (!cargada_ || estado_ != EstadoPartida::Jugando) return;
    if (tiempoAnuncio_ > 0.0f) tiempoAnuncio_ -= dt;

    float paso = dt * velocidad_;  // con velocidad x2 todo avanza el doble
    tiempo_ += paso;
    EmitirHumoVolcan(paso);

    // 1. Salen los enemigos de la oleada
    oleadas_.Actualizar(paso, *this);

    // 2. Todo se actualiza. Juego no sabe de que clase es cada uno: cada objeto usa SU version (polimorfismo)
    for (size_t i = 0; i < enemigos_.size(); i++) {
        Enemigo& enemigo = *enemigos_[i];
        enemigo.Actualizar(paso, *this);
        if (enemigo.LlegoALaCiudad()) {
            jugador_->PerderVidas(enemigo.GetDanioCiudad());
            Vector2 meta = nivel_.GetMeta();
            for (int k = 0; k < 8; k++) {
                AgregarParticula(Particula(Vector2{meta.x - 30, meta.y}, Vector2{Azar(-80, -20), Azar(-40, 40)}, 0.5f, ROJO, 3.0f,
                                           FormaParticula::Chispa));
            }
            int danio = enemigo.GetDanioCiudad();
            Avisar("La ciudad perdio " + std::to_string(danio) + (danio == 1 ? " vida" : " vidas"), 1.4f);
        }
    }
    for (auto& torre : torres_) torre->Actualizar(paso, *this);
    for (auto& proyectil : proyectiles_) proyectil.Actualizar(paso, *this);
    for (auto& ataque : ataquesJefe_) ataque.Actualizar(paso, *this);
    for (auto& particula : particulas_) particula.Actualizar(paso, *this);

    // 3. Limpieza al final del cuadro.
    //    Primero los proyectiles sueltan a los enemigos muertos, DESPUES se borran los enemigos:
    //    asi ningun proyectil se queda apuntando a memoria liberada.
    for (auto& proyectil : proyectiles_) proyectil.RevisarObjetivo();

    // Torres destruidas por un jefe: si era la seleccionada, primero la soltamos
    if (seleccionada_ != nullptr && !seleccionada_->EstaActivo()) seleccionada_ = nullptr;
    for (const auto& torre : torres_) {
        if (!torre->EstaActivo()) Avisar("¡Destruyeron tu " + std::string(torre->GetNombre()) + "!", 2.0f);
    }
    torres_.erase(std::remove_if(torres_.begin(), torres_.end(),
                                 [](const std::unique_ptr<Torre>& t) { return !t->EstaActivo(); }),
                  torres_.end());

    enemigos_.erase(std::remove_if(enemigos_.begin(), enemigos_.end(),
                                   [](const std::unique_ptr<Enemigo>& e) { return !e->EstaActivo(); }),
                    enemigos_.end());
    proyectiles_.erase(std::remove_if(proyectiles_.begin(), proyectiles_.end(),
                                      [](const Proyectil& p) { return !p.EstaActivo(); }),
                       proyectiles_.end());
    ataquesJefe_.erase(std::remove_if(ataquesJefe_.begin(), ataquesJefe_.end(),
                                      [](const AtaqueJefe& a) { return !a.EstaActivo(); }),
                       ataquesJefe_.end());
    particulas_.erase(std::remove_if(particulas_.begin(), particulas_.end(),
                                     [](const Particula& p) { return !p.EstaActivo(); }),
                      particulas_.end());

    // 4. Reglas: si se acabaron las vidas, perdiste
    if (jugador_->GetVidas() <= 0) Terminar(false);
}

void Partida::EmitirHumoVolcan(float dt) {
    if (GetRandomValue(0, 1000) < (int)(dt * 5.0f * 1000.0f)) {
        Vector2 c = nivel_.GetCrater();
        AgregarParticula(Particula(Vector2{c.x + Azar(-6, 6), c.y - 10}, Vector2{Azar(8, 18), Azar(-40, -26)}, 2.6f,
                                   Color{142, 136, 144, 255}, Azar(9, 15), FormaParticula::Humo));
    }
}

void Partida::Terminar(bool gano) {
    estado_ = gano ? EstadoPartida::Victoria : EstadoPartida::Derrota;
    if (gano) {
        jugador_->SumarPuntaje(jugador_->GetVidas() * 100 + jugador_->GetDinero());  // bono por vidas y dinero sobrante
    }
    mejorPuntaje_ = GestorArchivos::GuardarPuntaje(nivel_.GetDatos().nombre, jugador_->GetNombre(), jugador_->GetPuntaje());
    seleccionada_ = nullptr;
}

void Partida::Avisar(const std::string& texto, float segundos) {
    aviso_ = texto;
    tiempoAviso_ = segundos;
}

// ---------- Dibujo ----------

void Partida::Dibujar() const {
    if (!cargada_) {
        ClearBackground(AZUL_OSCURO);
        Centrado("No se pudo cargar el nivel", 300, 32, ROJO);
        Centrado(error_.c_str(), 350, 20, BLANCO);
        Centrado("Revisa que exista la carpeta niveles/ junto al juego.  ENTER para volver", 390, 18, GRIS);
        return;
    }

    DibujarMapa();
    DibujarBarraSuperior();
    DibujarPanel();
    DibujarBarraJefe();
    DibujarAvisos();
    DibujarRobo();
    if (estado_ == EstadoPartida::Pausa) DibujarPausa();
    if (estado_ == EstadoPartida::Victoria || estado_ == EstadoPartida::Derrota) DibujarFinal();
}

void Partida::DibujarMapa() const {
    nivel_.Dibujar(tiempo_);

    // Alcance de la torre seleccionada
    if (seleccionada_ != nullptr) {
        Vector2 p = seleccionada_->GetPosicion();
        DrawCircleV(p, seleccionada_->GetAlcance(), Fade(AMARILLO, 0.13f));
        DrawCircleLinesV(p, seleccionada_->GetAlcance(), AMARILLO);
    }

    // Orden de dibujo: lo que esta "abajo" primero
    for (const auto& e : enemigos_) if (!e->EsVolador()) e->Dibujar();
    for (const auto& t : torres_) {
        t->Dibujar();
        t->DibujarVida();
    }
    for (const auto& p : proyectiles_) p.Dibujar();
    for (const auto& a : ataquesJefe_) a.Dibujar();
    for (const auto& e : enemigos_) if (e->EsVolador()) e->Dibujar();
    for (const auto& p : particulas_) p.Dibujar();

    if (estado_ == EstadoPartida::Jugando) DibujarFantasma();
}

void Partida::DibujarFantasma() const {
    Vector2 raton = GetMousePosition();
    if (raton.y <= Nivel::ALTO_BARRA || raton.y >= Nivel::Y_PANEL) return;

    // Casillas libres marcadas muy suave, para que se vea donde se puede construir
    for (int fila = 0; fila < Nivel::FILAS; fila++) {
        for (int columna = 0; columna < Nivel::COLUMNAS; columna++) {
            if (nivel_.CasillaValida(columna, fila) && TorreEn(columna, fila) == nullptr) {
                DrawRectangle(columna * Nivel::CELDA + 2, fila * Nivel::CELDA + 2, Nivel::CELDA - 4, Nivel::CELDA - 4,
                              Fade(BLANCO, 0.045f));
            }
        }
    }

    int columna = (int)(raton.x / Nivel::CELDA);
    int fila = (int)(raton.y / Nivel::CELDA);
    if (TorreEn(columna, fila) != nullptr) return;

    if (!nivel_.CasillaValida(columna, fila)) {
        DrawRectangle(columna * Nivel::CELDA, fila * Nivel::CELDA, Nivel::CELDA, Nivel::CELDA, Fade(ROJO, 0.35f));
        return;
    }

    // Vista previa: el alcance (verde si alcanza el dinero, rojo si no) y la torre semitransparente
    Vector2 centro{columna * Nivel::CELDA + Nivel::CELDA / 2.0f, fila * Nivel::CELDA + Nivel::CELDA / 2.0f};
    std::unique_ptr<Torre> fantasma = Torre::Crear(tipoElegido_, centro);
    bool alcanza = jugador_->PuedePagar(Torre::CostoDe(tipoElegido_));
    Color color = alcanza ? VERDE_VIDA : ROJO;
    DrawCircleV(centro, fantasma->GetAlcance(), Fade(color, 0.14f));
    DrawCircleLinesV(centro, fantasma->GetAlcance(), Fade(color, 0.85f));
    fantasma->Dibujar();
    DrawCircleV(centro, 22.0f, Fade(VERDE_PASTO, 0.45f));
}

void Partida::DibujarBarraSuperior() const {
    DrawRectangle(0, 0, 1280, Nivel::ALTO_BARRA, Fade(AZUL_OSCURO, 0.92f));
    DrawRectangle(0, Nivel::ALTO_BARRA - 2, 1280, 2, Fade(AMARILLO, 0.6f));

    // Vidas con corazon
    int vidas = jugador_->GetVidas();
    DrawCircle(24, 21, 6, ROJO);
    DrawCircle(34, 21, 6, ROJO);
    DrawTriangle(Vector2{18, 23}, Vector2{29, 35}, Vector2{40, 23}, ROJO);
    Texto(TextFormat("%d", vidas), 50, 14, 24, vidas <= 5 ? ROJO : BLANCO);

    // Dinero con moneda
    DrawCircle(132, 24, 11, AMARILLO);
    DrawCircleLines(132, 24, 7, Color{184, 128, 30, 255});
    Texto(TextFormat("$%d", jugador_->GetDinero()), 150, 14, 24, BLANCO);

    Texto(TextFormat("Oleada %d / %d", oleadas_.GetNumero(), oleadas_.GetTotal()), 300, 14, 24, BLANCO);
    Texto(TextFormat("Puntaje %d", jugador_->GetPuntaje()), 520, 14, 24, BLANCO);

    // A la derecha: nombre del jugador y velocidad
    const char* derecha = TextFormat("%s   |   x%d (F)   |   P pausa", jugador_->GetNombre().c_str(), (int)velocidad_);
    Texto(derecha, 1264 - MedirTexto(derecha, 18), 16, 18, velocidad_ > 1.0f ? AMARILLO : GRIS);
}

void Partida::DibujarPanel() const {
    DrawRectangle(0, Nivel::Y_PANEL, 1280, 720 - Nivel::Y_PANEL, Fade(AZUL_OSCURO, 0.96f));
    DrawRectangle(0, Nivel::Y_PANEL, 1280, 2, AMARILLO);

    // Tarjetas de las tres torres
    for (int i = 0; i < 3; i++) {
        Rectangle carta = CajaCarta(i);
        bool elegida = TIPOS[i] == tipoElegido_;
        int costo = Torre::CostoDe(TIPOS[i]);
        DrawRectangleRounded(carta, 0.15f, 6, elegida ? Fade(AZUL, 0.35f) : Fade(BLACK, 0.25f));
        DrawRectangleRoundedLinesEx(carta, 0.15f, 6, elegida ? 2.0f : 1.0f, elegida ? AMARILLO : Fade(GRIS, 0.5f));
        iconos_[i]->Dibujar();
        Texto(Torre::NombreDe(TIPOS[i]), (int)carta.x + 76, (int)carta.y + 10, 16, BLANCO);
        Texto(TextFormat("$%d", costo), (int)carta.x + 76, (int)carta.y + 30, 22,
                 jugador_->PuedePagar(costo) ? AMARILLO : ROJO);
        Texto(TextFormat("Tecla %d", i + 1), (int)carta.x + 76, (int)carta.y + 54, 14, GRIS);
    }

    // Boton de lanzar oleada
    Rectangle boton = BotonOleada();
    int siguiente = oleadas_.GetNumero();
    if (oleadas_.PuedeLanzar()) {
        DrawRectangleRounded(boton, 0.15f, 6, AMARILLO);
        Texto(TextFormat("Lanzar oleada %d", siguiente + 1), (int)boton.x + 14, (int)boton.y + 10, 20, AZUL_OSCURO);
        Texto("[Espacio]", (int)boton.x + 14, (int)boton.y + 34, 14, AZUL_OSCURO);
        Texto(oleadas_.Describir(siguiente).c_str(), (int)boton.x + 14, (int)boton.y + 52, 14, AZUL_OSCURO);
    } else {
        DrawRectangleRounded(boton, 0.15f, 6, Fade(GRIS, 0.18f));
        if (oleadas_.EstaActiva()) {
            Texto(TextFormat("Oleada %d en curso", siguiente), (int)boton.x + 14, (int)boton.y + 12, 20, BLANCO);
            Texto(TextFormat("Enemigos en el mapa: %d", (int)enemigos_.size()), (int)boton.x + 14, (int)boton.y + 42, 14, GRIS);
        } else {
            Texto("Sin oleadas pendientes", (int)boton.x + 14, (int)boton.y + 26, 18, GRIS);
        }
    }

    // Informacion de la torre seleccionada, o ayuda
    int x = 988;
    int y = Nivel::Y_PANEL + 10;
    if (seleccionada_ != nullptr) {
        Texto(seleccionada_->GetNombre(), x, y, 18, BLANCO);
        Texto(TextFormat("Nivel %d/%d   Alcance %d   Danio %d", seleccionada_->GetNivel(), Torre::NIVEL_MAXIMO,
                            (int)seleccionada_->GetAlcance(), (int)seleccionada_->GetDanio()),
                 x, y + 22, 13, GRIS);

        Rectangle mejorar = BotonMejorar();
        bool puedeMejorar = seleccionada_->PuedeMejorar() && jugador_->PuedePagar(seleccionada_->GetCostoMejora());
        DrawRectangleRounded(mejorar, 0.2f, 6, puedeMejorar ? AMARILLO : Fade(GRIS, 0.25f));
        const char* textoMejorar = seleccionada_->PuedeMejorar() ? TextFormat("U Mejorar $%d", seleccionada_->GetCostoMejora())
                                                                 : "Nivel maximo";
        Texto(textoMejorar, (int)mejorar.x + 10, (int)mejorar.y + 8, 16, puedeMejorar ? AZUL_OSCURO : GRIS);

        Rectangle vender = BotonVender();
        DrawRectangleRounded(vender, 0.2f, 6, Fade(ROJO, 0.85f));
        Texto(TextFormat("V Vender +$%d", seleccionada_->GetValorVenta()), (int)vender.x + 10, (int)vender.y + 8, 16,
                 AZUL_OSCURO);
    } else {
        Texto("Clic en el pasto: construir", x, y, 15, GRIS);
        Texto("Clic en una torre: mejorar o vender", x, y + 22, 15, GRIS);
        Texto(TextFormat("Elegida: %s", Torre::NombreDe(tipoElegido_)), x, y + 46, 15, AMARILLO);
    }
}

void Partida::DibujarAvisos() const {
    if (tiempoAviso_ > 0.0f && !aviso_.empty()) {
        // Si esta el Coloso, su barra ocupa la parte de arriba: el aviso baja un poco
        bool hayJefe = false;
        for (const auto& e : enemigos_) {
            if (e->EsJefe()) hayJefe = true;
        }
        int ancho = MedirTexto(aviso_.c_str(), 20) + 36;
        Rectangle caja{(1280 - ancho) / 2.0f, hayJefe ? 112.0f : 62.0f, (float)ancho, 34};
        DrawRectangleRounded(caja, 0.5f, 8, Fade(AZUL_OSCURO, 0.85f));
        Texto(aviso_.c_str(), (int)caja.x + 18, (int)caja.y + 7, 20, BLANCO);
    }
    if (tiempoAnuncio_ > 0.0f) {
        float alfa = std::min(1.0f, tiempoAnuncio_ * 1.6f);
        int ancho = MedirTexto(anuncio_.c_str(), 52);
        DibujarTextoContorno(anuncio_.c_str(), (1280 - ancho) / 2, 120, 52, Fade(AMARILLO, alfa));
    }
}

void Partida::DibujarRobo() const {
    if (tiempoRobo_ <= 0.0f) return;

    // Panel que baja desde arriba y se desvanece al final
    float alfa = std::min(1.0f, tiempoRobo_ * 2.0f);
    Rectangle panel{(1280 - 460) / 2.0f, 104, 460, 84};
    DrawRectangleRounded(panel, 0.25f, 8, Fade(AZUL_OSCURO, 0.92f * alfa));
    DrawRectangleRoundedLinesEx(panel, 0.25f, 8, 2.0f, Fade(NARANJA, alfa));

    // Carita del ladron: gris el tlacuache, cafe la comadreja, los dos con antifaz de bandido
    bool tlacuache = ladron_ == "El Tlacuache";
    Color pelo = tlacuache ? Color{176, 172, 168, 255} : Color{150, 96, 56, 255};
    Vector2 c{panel.x + 46, panel.y + 44};
    DrawCircleV(Vector2{c.x - 18, c.y - 18}, 9, Fade(pelo, alfa));   // orejas
    DrawCircleV(Vector2{c.x + 18, c.y - 18}, 9, Fade(pelo, alfa));
    DrawCircleV(c, 24, Fade(pelo, alfa));                            // cabeza
    DrawRectangleRounded(Rectangle{c.x - 22, c.y - 9, 44, 12}, 0.6f, 6, Fade(Color{30, 26, 30, 255}, alfa));  // antifaz
    DrawCircleV(Vector2{c.x - 9, c.y - 3}, 3, Fade(BLANCO, alfa));   // ojos
    DrawCircleV(Vector2{c.x + 9, c.y - 3}, 3, Fade(BLANCO, alfa));
    DrawCircleV(Vector2{c.x, c.y + 10}, 4, Fade(Color{235, 120, 140, 255}, alfa));  // nariz

    Texto(TextFormat("¡%s!", ladron_.c_str()), (int)panel.x + 90, (int)panel.y + 14, 28, Fade(NARANJA, alfa));
    Texto(textoRobo_.c_str(), (int)panel.x + 90, (int)panel.y + 50, 20, Fade(BLANCO, alfa));
}

void Partida::DibujarBarraJefe() const {
    for (const auto& e : enemigos_) {
        if (!e->EsJefe() || !e->EstaActivo()) continue;

        float k = e->GetVida() / e->GetVidaMax();
        Rectangle barra{340, 58, 600, 18};
        DrawRectangleRec(Rectangle{barra.x - 3, barra.y - 3, barra.width + 6, barra.height + 6}, Fade(AZUL_OSCURO, 0.9f));
        DrawRectangleRec(barra, Fade(ROJO, 0.25f));
        DrawRectangleRec(Rectangle{barra.x, barra.y, barra.width * k, barra.height}, NARANJA);
        DrawRectangleLinesEx(barra, 2.0f, AMARILLO);
        const char* titulo = TextFormat("%s   %d / %d", e->GetNombre(), (int)e->GetVida(), (int)e->GetVidaMax());
        DibujarTextoContorno(titulo, 640 - MedirTexto(titulo, 18) / 2, 84, 18, BLANCO);
        return;  // solo hay un jefe a la vez
    }
}

void Partida::DibujarPausa() const {
    DrawRectangle(0, 0, 1280, 720, Fade(AZUL_OSCURO, 0.6f));
    Centrado("PAUSA", 260, 64, BLANCO);
    Centrado("P para continuar", 350, 24, GRIS);
    Centrado("M para salir al menu", 384, 24, GRIS);
}

void Partida::DibujarFinal() const {
    bool gano = estado_ == EstadoPartida::Victoria;
    DrawRectangle(0, 0, 1280, 720, Fade(AZUL_OSCURO, 0.75f));

    Rectangle panel{320, 170, 640, 360};
    DrawRectangleRounded(panel, 0.06f, 8, Fade(AZUL_OSCURO, 0.95f));
    DrawRectangleRoundedLinesEx(panel, 0.06f, 8, 2.0f, gano ? AMARILLO : ROJO);

    if (gano && GetSiguienteNivel().empty()) {
        // Ultimo nivel: se acabo el juego
        Centrado("¡SALVASTE PUEBLA!", 202, 34, AMARILLO);
        Centrado(TextFormat("Bien hecho, %s. Don Goyo vuelve a dormir.", jugador_->GetNombre().c_str()), 256, 22, BLANCO);
    } else if (gano) {
        Centrado(TextFormat("NIVEL %d SUPERADO", nivel_.GetDatos().numero), 202, 34, AMARILLO);
        Centrado(TextFormat("Bien hecho, %s. %s esta a salvo.", jugador_->GetNombre().c_str(),
                            nivel_.GetDatos().nombre.c_str()),
                 256, 22, BLANCO);
    } else {
        Centrado("DON GOYO LLEGO A LA CIUDAD", 202, 34, ROJO);
        Centrado(TextFormat("Aguantaste hasta la oleada %d de %d.", oleadas_.GetNumero(), oleadas_.GetTotal()), 256, 22, BLANCO);
    }

    Centrado(TextFormat("Enemigos eliminados: %d", jugador_->GetEliminados()), 306, 20, GRIS);
    Centrado(TextFormat("Vidas restantes: %d", jugador_->GetVidas()), 334, 20, GRIS);
    Centrado(TextFormat("Puntaje: %d", jugador_->GetPuntaje()), 380, 30, BLANCO);
    Centrado(TextFormat("Mejor puntaje en %s: %d", nivel_.GetDatos().nombre.c_str(), mejorPuntaje_), 420, 20, AMARILLO);
    if (!gano) Centrado("Tip: la Torre de Agua le quita la armadura al Golem", 452, 16, GRIS);
    if (gano && !GetSiguienteNivel().empty()) {
        Centrado("ENTER siguiente nivel   |   R reintentar   |   M menu", 486, 20, BLANCO);
    } else if (gano) {
        Centrado("ENTER continuar   |   R reintentar   |   M menu", 486, 20, BLANCO);
    } else {
        Centrado("R para reintentar   |   ENTER para ir al menu", 486, 20, BLANCO);
    }
}

// ---------- Zonas del panel ----------

Rectangle Partida::CajaCarta(int indice) {
    return Rectangle{16.0f + indice * 238.0f, (float)Nivel::Y_PANEL + 10, 226, 74};
}

Rectangle Partida::BotonOleada() {
    return Rectangle{736, (float)Nivel::Y_PANEL + 10, 236, 74};
}

Rectangle Partida::BotonMejorar() {
    return Rectangle{988, (float)Nivel::Y_PANEL + 46, 136, 32};
}

Rectangle Partida::BotonVender() {
    return Rectangle{1130, (float)Nivel::Y_PANEL + 46, 136, 32};
}
