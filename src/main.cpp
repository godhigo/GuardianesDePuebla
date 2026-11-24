#include "Juego.h"

int main() {
    Juego juego;       // se crea el juego: el constructor abre la ventana
    juego.Ejecutar();  // corre el ciclo del juego hasta que se cierre la ventana
    return 0;          // al terminar main, el destructor cierra la ventana
}
