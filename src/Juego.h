#pragma once

#include <string>
#include <vector>

#include "raylib.h"
#include "FondoAnimado.h"
#include "Jugador.h"
#include "Partida.h"

// Las pantallas por las que se puede mover el jugador
enum class Pantalla {
    Menu,
    Nombre,    // P1: el abuelito pide el nombre
    Historia,  // P2: el abuelito cuenta que Don Goyo desperto
    Mision,    // P3: el abuelito pide ayuda
    Jugando,
    Felicitacion,  // P4: el abuelito te felicita y avisa del siguiente nivel
    Final,         // P5: ganaste el juego, el abuelito te da las gracias
    Controles,
    Creditos
};

// Juego: abre la ventana, corre el ciclo del juego y la cierra al terminar
class Juego {
public:
    Juego();          // constructor: abre la ventana
    ~Juego();         // destructor: cierra la ventana
    void Ejecutar();  // ciclo del juego

private:
    void Actualizar(float dt);  // lee el teclado y cambia lo que haga falta (dt = segundos desde el cuadro anterior)
    void ActualizarMenu();      // flechas y ENTER en el menu principal
    void ActualizarNombre();    // escribir el nombre en P1
    void ActualizarDialogo(Pantalla siguiente, Pantalla anterior);  // P2 y P3: ENTER avanza, Esc regresa
    void Dibujar();             // dibuja un cuadro completo

    // Una funcion de dibujo por pantalla
    void DibujarMenu();
    void DibujarNombre();
    void DibujarHistoria();
    void DibujarMision();
    void DibujarJugando();
    void DibujarFelicitacion();
    void DibujarFinal();
    void DibujarControles();
    void DibujarCreditos();

    // Ayudas para dibujar
    Rectangle CajaOpcion(int indice);                                       // area que ocupa cada opcion del menu
    void TextoCentrado(const char* texto, int y, int tamano, Color color);              // escribe un texto centrado
    void TextoConContorno(const char* texto, int x, int y, int tamano, Color color);    // escribe un texto con contorno en (x, y)
    void DibujarPanel(const char* titulo, float alto);                      // panel oscuro con titulo, al centro
    void DibujarAyuda(const char* texto);                                   // texto gris de ayuda hasta abajo
    void DibujarDialogo(Texture2D imagen, const std::vector<std::string>& lineas, int yInicio, int tamano, int separacion,
                        int x = 348);  // x: donde empieza el texto (P4 lo tiene mas a la derecha)
    void DibujarLineaDialogo(const std::string& linea, int x, int y, int tamano);  // pinta "Don Goyo" en amarillo

    int ancho_;               // ancho de la ventana en pixeles
    int alto_;                // alto de la ventana en pixeles
    Pantalla pantalla_;       // en que pantalla estamos ahorita
    bool salir_;              // se vuelve true cuando el jugador quiere cerrar el juego
    int opcionSeleccionada_;  // opcion marcada del menu (0 a 2)
    FondoAnimado fondoMenu_;  // el video del volcan detras del menu

    Texture2D imagenP1_;         // P1: la plaza con la fuente
    Texture2D imagenP2_;         // P2: el arco con el volcan
    Texture2D imagenP3_;         // P3: el kiosco
    Texture2D imagenP4_;         // P4: el abuelito con la piramide y el volcan de noche
    Texture2D imagenP5_;         // P5: el abuelito en Los Fuertes despues de la batalla
    std::string siguienteNivel_; // archivo del nivel que empieza despues de P4
    std::string nombreEscrito_;  // lo que el jugador va escribiendo en P1
    float tiempo_;               // segundos desde que arranco el juego (para el cursor que parpadea)
    float tiempoAviso_;          // segundos que le quedan al aviso de "escribe tu nombre"
    Jugador jugador_;            // los datos del jugador: nombre, dinero, vidas y puntaje
    Partida partida_;            // el nivel que se esta jugando
};
