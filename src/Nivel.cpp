#include "Nivel.h"

#include <algorithm>
#include <cmath>

#include "Paleta.h"

namespace {
// Numero al azar con decimales entre a y b
float Azar(float a, float b) {
    return a + (b - a) * (GetRandomValue(0, 10000) / 10000.0f);
}

// Dibuja un maguey (planta) con hojas alrededor de un centro
void DibujarMaguey(Vector2 centro) {
    for (int i = 0; i < 8; i++) {
        float angulo = i * PI / 4.0f;
        Vector2 punta{centro.x + std::cos(angulo) * 16.0f, centro.y + std::sin(angulo) * 16.0f};
        DrawLineEx(centro, punta, 6.0f, i % 2 ? Color{95, 138, 108, 255} : Color{121, 163, 135, 255});
    }
    DrawCircleV(centro, 4.0f, Color{63, 102, 80, 255});
}
}  // namespace

Nivel::Nivel() : fondo_{}, fondoListo_(false), imagenFondo_{}, tieneImagen_(false) {}

void Nivel::Cargar(const DatosNivel& datos) {
    datos_ = datos;
    rutas_.clear();

    // Cada camino del archivo es una ruta. La partimos en tramos rectos y guardamos cuanto mide cada uno
    for (const std::vector<Vector2>& camino : datos_.caminos) {
        Ruta ruta{{}, 0.0f};
        for (size_t i = 0; i + 1 < camino.size(); i++) {
            Vector2 a = camino[i];
            Vector2 b = camino[i + 1];
            float largo = std::hypot(b.x - a.x, b.y - a.y);
            ruta.tramos.push_back(Tramo{a, b, largo, ruta.largo, std::atan2(b.y - a.y, b.x - a.x)});
            ruta.largo += largo;
        }
        rutas_.push_back(ruta);
    }

    // Decidimos de una vez que casillas sirven para construir
    casillasValidas_.assign(COLUMNAS * FILAS, false);
    Vector2 salida = GetSalida();
    Vector2 meta = GetMeta();
    Vector2 piramide = datos_.piramide;
    bool hayMapa = (int)datos_.mapa.size() == FILAS;  // el archivo trae dibujadas las casillas libres

    for (int fila = 0; fila < FILAS; fila++) {
        for (int columna = 0; columna < COLUMNAS; columna++) {
            Vector2 centro{columna * CELDA + CELDA / 2.0f, fila * CELDA + CELDA / 2.0f};
            bool valida = true;

            if (DistanciaAlCamino(centro) < 46.0f) valida = false;                 // muy cerca del camino
            if (centro.y < ALTO_BARRA + 22) valida = false;                        // debajo de la barra de arriba
            if (centro.y + CELDA / 2.0f > Y_PANEL) valida = false;                 // debajo del panel de abajo

            if (hayMapa) {
                // El archivo dice que casillas son pasto libre ('.') y cuales estan ocupadas ('#')
                const std::string& filaMapa = datos_.mapa[fila];
                if (columna >= (int)filaMapa.size() || filaMapa[columna] != '.') valida = false;
            } else {
                // Sin mapa: apartamos las zonas del volcan, la ciudad y la piramide
                if (centro.x < salida.x + 135 && std::fabs(centro.y - salida.y) < 110) valida = false;
                if (centro.x > meta.x - 130 && centro.y > meta.y - 100 && centro.y < meta.y + 90) valida = false;
                if (std::fabs(centro.x - piramide.x) < 100 && std::fabs(centro.y - piramide.y) < 100) valida = false;
            }

            casillasValidas_[fila * COLUMNAS + columna] = valida;
        }
    }
}

PuntoCamino Nivel::PuntoEn(float distancia, int ruta) const {
    const std::vector<Tramo>& tramos = rutas_[ruta].tramos;
    for (const Tramo& t : tramos) {
        if (distancia <= t.inicio + t.largo) {
            float f = (distancia - t.inicio) / t.largo;  // que fraccion del tramo ya recorrio (0 a 1)
            return PuntoCamino{Vector2{t.a.x + (t.b.x - t.a.x) * f, t.a.y + (t.b.y - t.a.y) * f}, t.angulo};
        }
    }
    const Tramo& ultimo = tramos.back();
    return PuntoCamino{ultimo.b, ultimo.angulo};
}

float Nivel::GetLargo(int ruta) const {
    return rutas_[ruta].largo;
}

int Nivel::GetTotalRutas() const {
    return (int)rutas_.size();
}

bool Nivel::CasillaValida(int columna, int fila) const {
    if (columna < 0 || fila < 0 || columna >= COLUMNAS || fila >= FILAS) return false;
    return casillasValidas_[fila * COLUMNAS + columna];
}

Vector2 Nivel::GetSalida() const {
    return datos_.caminos.front().front();  // todas las rutas salen del mismo lugar
}

Vector2 Nivel::GetMeta() const {
    return datos_.caminos.front().back();   // y llegan al mismo lugar
}

Vector2 Nivel::GetCrater() const {
    if (datos_.crater.x > -500.0f) return datos_.crater;           // viene en el archivo
    return Vector2{GetSalida().x + 34.0f, GetSalida().y};           // si no, junto a la salida
}

const DatosNivel& Nivel::GetDatos() const {
    return datos_;
}

float Nivel::DistanciaAlCamino(Vector2 punto) const {
    float minima = 1e9f;
    for (const Ruta& ruta : rutas_) {  // revisamos todas las rutas
        for (const Tramo& t : ruta.tramos) {
            // Punto mas cercano dentro del tramo (proyeccion acotada entre 0 y 1)
            float dx = t.b.x - t.a.x, dy = t.b.y - t.a.y;
            float f = ((punto.x - t.a.x) * dx + (punto.y - t.a.y) * dy) / (dx * dx + dy * dy);
            f = std::clamp(f, 0.0f, 1.0f);
            float cx = t.a.x + dx * f, cy = t.a.y + dy * f;
            minima = std::min(minima, std::hypot(punto.x - cx, punto.y - cy));
        }
    }
    return minima;
}

// ---------- Dibujo ----------

void Nivel::GenerarFondo() {
    // Si el nivel trae imagen de escenario y el archivo existe, usamos la imagen
    if (!datos_.fondo.empty() && FileExists(datos_.fondo.c_str())) {
        imagenFondo_ = LoadTexture(datos_.fondo.c_str());
        SetTextureFilter(imagenFondo_, TEXTURE_FILTER_BILINEAR);  // suaviza si la imagen no mide 1280 x 720
        tieneImagen_ = true;
        return;
    }

    // Si no, pintamos el escenario con codigo, una sola vez
    if (fondoListo_) return;
    fondo_ = LoadRenderTexture(1280, 720);
    BeginTextureMode(fondo_);  // todo lo que se dibuje ahora va a la textura, no a la pantalla
    PintarEscenario();
    EndTextureMode();
    fondoListo_ = true;
}

void Nivel::Descargar() {
    if (tieneImagen_) {
        UnloadTexture(imagenFondo_);
        tieneImagen_ = false;
    }
    if (fondoListo_) {
        UnloadRenderTexture(fondo_);
        fondoListo_ = false;
    }
}

void Nivel::Dibujar(float tiempo) const {
    if (tieneImagen_) {
        // La imagen se estira exactamente a la ventana
        Rectangle origen{0, 0, (float)imagenFondo_.width, (float)imagenFondo_.height};
        DrawTexturePro(imagenFondo_, origen, Rectangle{0, 0, 1280, 720}, Vector2{0, 0}, 0.0f, WHITE);

        // Brillo de la lava que late en el crater
        Vector2 crater = GetCrater();
        float pulso = 0.5f + 0.5f * std::sin(tiempo * 2.2f);
        BeginBlendMode(BLEND_ADDITIVE);
        DrawCircleGradient((int)crater.x, (int)crater.y, 34, Fade(Color{255, 170, 60, 255}, 0.25f + 0.25f * pulso),
                           Fade(Color{255, 90, 30, 255}, 0.0f));
        EndBlendMode();
        return;
    }

    if (fondoListo_) {
        // Las RenderTexture quedan de cabeza: por eso el alto va negativo
        DrawTextureRec(fondo_.texture, Rectangle{0, 0, 1280, -720}, Vector2{0, 0}, WHITE);
    } else {
        ClearBackground(VERDE_PASTO);
    }

    // Lava del crater: brilla mas y menos con el tiempo
    Vector2 crater{GetSalida().x + 34, GetSalida().y};
    float pulso = 0.5f + 0.5f * std::sin(tiempo * 2.2f);
    DrawCircleGradient((int)crater.x, (int)crater.y, 36, Fade(Color{255, 241, 184, 255}, 0.75f + 0.25f * pulso),
                       Fade(Color{255, 107, 44, 255}, 0.0f));
    DrawCircleV(crater, 12.0f + 3.0f * pulso, Color{255, 179, 71, 230});

    // Letreros del escenario
    DibujarTextoContorno("DON GOYO", (int)GetSalida().x + 18, (int)GetSalida().y - 125, 16, BLANCO);
    if (datos_.piramide.x > 0) {
        DibujarTextoContorno(datos_.nombre.c_str(), (int)datos_.piramide.x - 42, (int)datos_.piramide.y - 112, 16, BLANCO);
    }
    DibujarTextoContorno("PUEBLA", (int)GetMeta().x - 108, (int)GetMeta().y - 112, 16, BLANCO);
}

void Nivel::PintarEscenario() const {
    SetRandomSeed(7);  // misma semilla = el escenario siempre sale igual

    // Pasto con manchas de distinto tono
    ClearBackground(VERDE_PASTO);
    for (int i = 0; i < 70; i++) {
        Color tono = GetRandomValue(0, 1) ? Color{98, 108, 58, 45} : Color{160, 142, 92, 35};
        DrawEllipse(GetRandomValue(0, 1280), GetRandomValue(0, 720), Azar(30, 120), Azar(18, 68), tono);
    }

    // Milpa: surcos con plantitas en la esquina de arriba a la derecha
    for (int y = 80; y < 440; y += 22) {
        DrawLineEx(Vector2{704, (float)y + 4}, Vector2{1236, (float)y + 4}, 3, Color{92, 80, 48, 60});
        for (int x = 710; x < 1236; x += 14) {
            Color hoja = GetRandomValue(0, 1) ? Color{74, 96, 40, 220} : Color{118, 136, 58, 220};
            DrawLineEx(Vector2{(float)x - 4, (float)y - 3}, Vector2{(float)x, (float)y + 2}, 2, hoja);
            DrawLineEx(Vector2{(float)x, (float)y + 2}, Vector2{(float)x + 4, (float)y - 3}, 2, hoja);
        }
    }

    // Pastito suelto
    for (int i = 0; i < 1100; i++) {
        float x = Azar(0, 1280), y = Azar(0, 720);
        Color tono = GetRandomValue(0, 1) ? Color{60, 72, 34, 128} : Color{176, 176, 108, 115};
        DrawLineEx(Vector2{x, y}, Vector2{x + Azar(-2, 2), y - Azar(4, 8)}, 1.5f, tono);
    }

    // Volcan: cono oscuro con nieve alrededor del crater
    Vector2 crater{GetSalida().x + 34, GetSalida().y};
    DrawCircleGradient((int)crater.x, (int)crater.y, 112, Color{94, 74, 68, 255}, Fade(Color{111, 90, 76, 255}, 0.0f));
    for (int i = 0; i < 14; i++) {
        float angulo = i * 2.0f * PI / 14.0f + Azar(0, 0.3f);
        float r = Azar(52, 66);
        DrawEllipse((int)(crater.x + std::cos(angulo) * r), (int)(crater.y + std::sin(angulo) * r),
                    Azar(10, 18), Azar(5, 9), Color{238, 241, 246, 215});
    }

    // Camino de cantera: sombra, orilla y centro, con un circulo en cada esquina para que se vea redondeado
    const float anchos[] = {68.0f, 58.0f, 48.0f};
    const Color colores[] = {Color{40, 30, 20, 70}, Color{168, 131, 110, 255}, Color{220, 186, 163, 255}};
    for (int capa = 0; capa < 3; capa++) {
        for (const Ruta& ruta : rutas_) {
            for (const Tramo& t : ruta.tramos) {
                Vector2 a = t.a, b = t.b;
                if (&t == &ruta.tramos.front()) a.x -= 40;  // que salga de la orilla de la pantalla
                if (&t == &ruta.tramos.back()) b.x += 40;
                DrawLineEx(a, b, anchos[capa], colores[capa]);
                DrawCircleV(t.b, anchos[capa] / 2.0f, colores[capa]);
            }
        }
    }
    // Losetas: cuadritos girados a lo largo del camino (solo la primera ruta, para no encimarlas)
    for (float d = 6.0f; d < GetLargo(); d += 13.0f) {
        PuntoCamino p = PuntoEn(d);
        for (int k = -1; k <= 1; k++) {
            float desplazamiento = k * 15.0f + Azar(-2, 2);
            Vector2 centro{p.posicion.x - std::sin(p.angulo) * desplazamiento, p.posicion.y + std::cos(p.angulo) * desplazamiento};
            Color tono = GetRandomValue(0, 1) ? Color{168, 131, 110, 97} : Color{250, 232, 218, 102};
            DrawRectanglePro(Rectangle{centro.x, centro.y, 10, 10}, Vector2{5, 5}, p.angulo * RAD2DEG, tono);
        }
    }

    // Crater encima del camino
    DrawCircleV(crater, 40.0f, Color{42, 32, 31, 255});
    DrawRing(crater, 38.0f, 42.0f, 0, 360, 40, Color{74, 58, 54, 255});

    // Piramide de Cholula con el santuario arriba
    Vector2 p = datos_.piramide;
    if (p.x > 0) {
        const Color pisos[] = {{143, 98, 67, 255}, {168, 116, 79, 255}, {191, 138, 96, 255}, {211, 161, 115, 255}};
        const float lados[] = {184, 144, 104, 66};
        for (int i = 0; i < 4; i++) {
            Rectangle r{p.x - lados[i] / 2, p.y - lados[i] / 2, lados[i], lados[i]};
            DrawRectangleRec(r, pisos[i]);
            DrawRectangleLinesEx(r, 2, Color{60, 35, 20, 90});
        }
        DrawRectangle((int)p.x - 8, (int)p.y + 20, 16, 72, Color{232, 192, 143, 255});  // escalera
        DrawRectangle((int)p.x - 18, (int)p.y - 13, 36, 26, Color{240, 194, 75, 255});  // iglesia
        DrawRectangle((int)p.x - 20, (int)p.y - 15, 8, 8, Color{224, 169, 46, 255});
        DrawRectangle((int)p.x + 12, (int)p.y - 15, 8, 8, Color{224, 169, 46, 255});
        DrawCircleV(p, 8.0f, AZUL);
        DrawCircleV(p, 3.0f, BLANCO);
        DibujarMaguey(Vector2{p.x - 94, p.y + 20});
        DibujarMaguey(Vector2{p.x + 98, p.y + 20});
        DibujarMaguey(Vector2{p.x - 98, p.y - 84});
    }
    DibujarMaguey(Vector2{GetSalida().x + 118, GetSalida().y - 108});

    // Ciudad al final del camino: plaza, catedral con domos de Talavera y casas de colores
    Vector2 m = GetMeta();
    DrawRectangle((int)m.x - 120, (int)m.y - 92, 120, 184, Color{233, 211, 194, 255});
    DrawRectangleLines((int)m.x - 120, (int)m.y - 92, 120, 184, Color{184, 149, 128, 255});
    DrawRectangle((int)m.x - 120, (int)m.y - 24, 120, 48, Color{220, 186, 163, 255});  // calle
    DrawRectangle((int)m.x - 100, (int)m.y - 86, 84, 52, BLANCO);                    // fachada
    for (int x = (int)m.x - 100; x < m.x - 16; x += 7) {
        DrawRectangle(x, (int)m.y - 86, 4, 4, AZUL);  // azulejos
        DrawRectangle(x, (int)m.y - 38, 4, 4, AZUL);
    }
    DrawRectangle((int)m.x - 104, (int)m.y - 88, 18, 18, Color{181, 83, 60, 255});   // torres
    DrawRectangle((int)m.x - 30, (int)m.y - 88, 18, 18, Color{181, 83, 60, 255});
    DrawCircle((int)m.x - 95, (int)m.y - 79, 6, AMARILLO);
    DrawCircle((int)m.x - 21, (int)m.y - 79, 6, AMARILLO);
    DrawCircle((int)m.x - 58, (int)m.y - 58, 14, AZUL);                             // domo
    for (int i = 0; i < 6; i++) {
        float a = i * PI / 3.0f;
        DrawCircle((int)(m.x - 58 + std::cos(a) * 8), (int)(m.y - 58 + std::sin(a) * 8), 2.4f, BLANCO);
    }
    DrawCircle((int)m.x - 58, (int)m.y - 58, 3.5f, AMARILLO);
    const Color casas[] = {{231, 163, 58, 255}, {196, 84, 62, 255}, {61, 114, 240, 255}};
    for (int i = 0; i < 3; i++) {
        int x = (int)m.x - 112 + i * 34;
        DrawRectangle(x, (int)m.y + 40, 30, 30, casas[i]);
        DrawRectangle(x + 11, (int)m.y + 58, 8, 12, Fade(BLANCO, 0.85f));
    }
}
