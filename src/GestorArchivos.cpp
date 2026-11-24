#include "GestorArchivos.h"

#include <filesystem>
#include <fstream>
#include <sstream>

const char* GestorArchivos::RUTA_PUNTAJES = "datos/puntajes.txt";

bool GestorArchivos::LeerNivel(const std::string& ruta, DatosNivel& datos, std::string& error) {
    std::ifstream archivo(ruta);
    if (!archivo.is_open()) {
        error = "No se encontro el archivo " + ruta;
        return false;
    }

    datos = DatosNivel{};
    std::string linea;
    while (std::getline(archivo, linea)) {
        std::istringstream partes(linea);  // nos deja leer la linea palabra por palabra
        std::string clave;
        partes >> clave;

        if (clave.empty() || clave[0] == '#') {
            continue;  // linea vacia o comentario
        } else if (clave == "NOMBRE") {
            std::getline(partes >> std::ws, datos.nombre);  // el resto de la linea (puede llevar espacios)
        } else if (clave == "NUMERO") {
            partes >> datos.numero;
        } else if (clave == "SIGUIENTE") {
            partes >> datos.siguiente;
        } else if (clave == "LADRONES") {
            partes >> datos.ladrones;
        } else if (clave == "DINERO") {
            partes >> datos.dinero;
        } else if (clave == "VIDAS") {
            partes >> datos.vidas;
        } else if (clave == "FONDO") {
            partes >> datos.fondo;
        } else if (clave == "MAPA") {
            std::string fila;
            partes >> fila;
            datos.mapa.push_back(fila);
        } else if (clave == "CAMINO" || clave == "PIRAMIDE" || clave == "CRATER") {
            // Cada linea CAMINO es una ruta nueva. Cada punto viene como x,y
            if (clave == "CAMINO") datos.caminos.push_back({});
            std::string punto;
            while (partes >> punto) {
                size_t coma = punto.find(',');
                if (coma == std::string::npos) continue;
                float x = std::stof(punto.substr(0, coma));
                float y = std::stof(punto.substr(coma + 1));
                if (clave == "CAMINO") datos.caminos.back().push_back(Vector2{x, y});
                else if (clave == "PIRAMIDE") datos.piramide = Vector2{x, y};
                else datos.crater = Vector2{x, y};
            }
        } else if (clave == "OLEADA") {
            // Grupos de 3 en 3: TIPO CANTIDAD INTERVALO
            std::vector<GrupoOleada> oleada;
            GrupoOleada grupo;
            while (partes >> grupo.tipo >> grupo.cantidad >> grupo.intervalo) {
                oleada.push_back(grupo);
            }
            datos.oleadas.push_back(oleada);
        }
    }

    if (datos.caminos.empty()) {
        error = "El nivel no tiene ningun CAMINO";
        return false;
    }
    for (const std::vector<Vector2>& camino : datos.caminos) {
        if (camino.size() < 2) {
            error = "Cada CAMINO necesita al menos 2 puntos";
            return false;
        }
    }
    if (datos.oleadas.empty()) {
        error = "El nivel no tiene ninguna OLEADA";
        return false;
    }
    return true;
}

int GestorArchivos::LeerMejorPuntaje(const std::string& nivel) {
    std::ifstream archivo(RUTA_PUNTAJES);
    std::string linea;
    while (std::getline(archivo, linea)) {
        // Formato de cada linea: nivel;jugador;puntaje
        std::istringstream partes(linea);
        std::string nombreNivel, jugador, puntaje;
        std::getline(partes, nombreNivel, ';');
        std::getline(partes, jugador, ';');
        std::getline(partes, puntaje, ';');
        if (nombreNivel == nivel && !puntaje.empty()) {
            return std::stoi(puntaje);
        }
    }
    return 0;
}

int GestorArchivos::GuardarPuntaje(const std::string& nivel, const std::string& jugador, int puntaje) {
    // 1. Leer todas las lineas que ya existen
    std::vector<std::string> lineas;
    int mejorAnterior = 0;
    bool existia = false;
    {
        std::ifstream archivo(RUTA_PUNTAJES);
        std::string linea;
        while (std::getline(archivo, linea)) {
            std::istringstream partes(linea);
            std::string nombreNivel, nombreJugador, valor;
            std::getline(partes, nombreNivel, ';');
            std::getline(partes, nombreJugador, ';');
            std::getline(partes, valor, ';');
            if (nombreNivel == nivel && !valor.empty()) {
                existia = true;
                mejorAnterior = std::stoi(valor);
                if (puntaje > mejorAnterior) {
                    linea = nivel + ";" + jugador + ";" + std::to_string(puntaje);  // lo reemplazamos
                }
            }
            lineas.push_back(linea);
        }
    }
    if (!existia) {
        lineas.push_back(nivel + ";" + jugador + ";" + std::to_string(puntaje));
    }

    // 2. Volver a escribir el archivo completo (si la carpeta datos/ no existe, se crea)
    std::filesystem::create_directories("datos");
    std::ofstream salida(RUTA_PUNTAJES);
    for (const std::string& linea : lineas) {
        salida << linea << "\n";
    }

    return puntaje > mejorAnterior ? puntaje : mejorAnterior;
}
