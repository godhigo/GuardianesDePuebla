#pragma once

#include <string>
#include <vector>

#include "raylib.h"
#include "GestorArchivos.h"

// Un punto del camino con la direccion hacia donde va
struct PuntoCamino {
    Vector2 posicion;
    float angulo;  // en radianes
};

// Nivel: el camino, las casillas donde se puede construir y el dibujo del escenario
class Nivel {
public:
    static constexpr int CELDA = 40;                 // cada casilla mide 40 x 40 pixeles
    static constexpr int COLUMNAS = 1280 / CELDA;    // 32
    static constexpr int FILAS = 720 / CELDA;        // 18
    static constexpr int ALTO_BARRA = 48;            // barra de arriba (vidas, dinero...)
    static constexpr int Y_PANEL = 628;              // donde empieza el panel de abajo

    Nivel();

    void Cargar(const DatosNivel& datos);  // arma el camino y las casillas a partir del archivo
    void GenerarFondo();                    // pinta el escenario una sola vez (necesita la ventana)
    void Descargar();

    // Un nivel puede tener varias rutas (caminos). "ruta" dice cual: 0 = la primera
    PuntoCamino PuntoEn(float distancia, int ruta = 0) const;  // en que punto queda alguien que ya camino "distancia" pixeles
    float GetLargo(int ruta = 0) const;                        // largo total de esa ruta
    int GetTotalRutas() const;                                 // cuantas rutas tiene el nivel
    bool CasillaValida(int columna, int fila) const;
    Vector2 GetSalida() const;
    Vector2 GetMeta() const;
    Vector2 GetCrater() const;  // de donde sale el humo del volcan
    const DatosNivel& GetDatos() const;

    void Dibujar(float tiempo) const;  // fondo + lava del crater (animada)

private:
    struct Tramo {  // cada linea recta del camino
        Vector2 a, b;
        float largo;
        float inicio;  // distancia acumulada hasta donde empieza este tramo
        float angulo;
    };
    struct Ruta {  // un camino completo: sus tramos y cuanto mide
        std::vector<Tramo> tramos;
        float largo;
    };

    float DistanciaAlCamino(Vector2 punto) const;
    void PintarEscenario() const;

    DatosNivel datos_;
    std::vector<Ruta> rutas_;
    std::vector<bool> casillasValidas_;  // COLUMNAS x FILAS
    RenderTexture2D fondo_;              // escenario pintado con codigo (si el nivel no trae imagen)
    bool fondoListo_;
    Texture2D imagenFondo_;              // escenario desde una imagen (FONDO en el archivo del nivel)
    bool tieneImagen_;
};
