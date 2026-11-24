#include "Paleta.h"

#include <vector>

namespace {
Font fuente;               // la tipografia del juego
bool fuenteCargada = false;

// Espacio extra entre letras: un poquito, proporcional al tamano
float Espaciado(int tamano) {
    return tamano / 20.0f;
}
}  // namespace

void CargarFuente(const char* ruta) {
    if (!FileExists(ruta)) {
        TraceLog(LOG_WARNING, "No se encontro la fuente %s, se usa la de raylib", ruta);
        return;
    }
    // Letras que vamos a usar: las normales (32 a 126) y las de espanol (a partir de 160: ¡ ¿ á é í ó ú ñ ü...)
    std::vector<int> letras;
    for (int c = 32; c <= 126; c++) letras.push_back(c);
    for (int c = 160; c <= 255; c++) letras.push_back(c);

    // Se carga grande (64 px) para que se vea bien al hacerla mas chica
    fuente = LoadFontEx(ruta, 64, letras.data(), (int)letras.size());
    SetTextureFilter(fuente.texture, TEXTURE_FILTER_BILINEAR);
    fuenteCargada = true;
}

void DescargarFuente() {
    if (fuenteCargada) {
        UnloadFont(fuente);
        fuenteCargada = false;
    }
}

void Texto(const char* texto, int x, int y, int tamano, Color color) {
    if (!fuenteCargada) {
        DrawText(texto, x, y, tamano, color);  // sin archivo de fuente: la de raylib
        return;
    }
    DrawTextEx(fuente, texto, Vector2{(float)x, (float)y}, (float)tamano, Espaciado(tamano), color);
}

int MedirTexto(const char* texto, int tamano) {
    if (!fuenteCargada) return MeasureText(texto, tamano);
    return (int)MeasureTextEx(fuente, texto, (float)tamano, Espaciado(tamano)).x;
}

void DibujarTextoContorno(const char* texto, int x, int y, int tamano, Color color) {
    int grosor = tamano / 16 + 1;  // texto mas grande, contorno mas grueso

    // Sombra abajo a la derecha
    Texto(texto, x + grosor * 2, y + grosor * 2, tamano, Fade(BLACK, 0.6f));
    // Contorno: el mismo texto en oscuro, movido arriba, abajo, izquierda y derecha
    Texto(texto, x - grosor, y, tamano, AZUL_OSCURO);
    Texto(texto, x + grosor, y, tamano, AZUL_OSCURO);
    Texto(texto, x, y - grosor, tamano, AZUL_OSCURO);
    Texto(texto, x, y + grosor, tamano, AZUL_OSCURO);
    // El texto encima, en su color
    Texto(texto, x, y, tamano, color);
}
