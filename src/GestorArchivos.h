#pragma once

#include <string>
#include <vector>

#include "raylib.h"

// Un grupo dentro de una oleada: por ejemplo "8 Chispas, una cada 0.8 segundos"
struct GrupoOleada {
    std::string tipo;  // "CHISPA" o "GOLEM"
    int cantidad;
    float intervalo;   // segundos entre un enemigo y el siguiente
};

// Todo lo que se lee del archivo de un nivel
struct DatosNivel {
    std::string nombre;
    int numero = 1;                                  // numero del nivel (1, 2, ...)
    std::string siguiente;                           // archivo del siguiente nivel (vacio = es el ultimo)
    int dinero = 0;
    int ladrones = 0;                                // % del bono que roban los ladrones (0 = no hay ladrones)
    int vidas = 0;
    std::vector<std::vector<Vector2>> caminos;       // cada CAMINO es una ruta completa de la salida a la meta
    Vector2 piramide{-1000.0f, -1000.0f};            // donde se dibuja la piramide (si el nivel la tiene)
    std::string fondo;                               // imagen del escenario (vacio = se dibuja con codigo)
    Vector2 crater{-1000.0f, -1000.0f};              // de donde sale el humo del volcan
    std::vector<std::string> mapa;                   // filas de casillas: '.' se puede construir, '#' no
    std::vector<std::vector<GrupoOleada>> oleadas;   // cada oleada es una lista de grupos
};

// GestorArchivos: lee los niveles y guarda los mejores puntajes en disco
class GestorArchivos {
public:
    // Lee un nivel. Si algo sale mal regresa false y explica el problema en "error"
    static bool LeerNivel(const std::string& ruta, DatosNivel& datos, std::string& error);

    // Mejor puntaje guardado para un nivel (0 si no hay ninguno)
    static int LeerMejorPuntaje(const std::string& nivel);

    // Guarda el puntaje si supera al mejor anterior. Regresa el mejor puntaje despues de guardar
    static int GuardarPuntaje(const std::string& nivel, const std::string& jugador, int puntaje);

private:
    static const char* RUTA_PUNTAJES;
};
