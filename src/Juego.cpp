#include "Juego.h"

#include "Paleta.h"  // colores del juego y DibujarTextoContorno

// Lista con los textos del menu principal (se cuentan desde 0)
const char* const OPCIONES_MENU[] = {"Nueva partida", "Controles", "Créditos"};
const int TOTAL_OPCIONES = 3;  // cuantas opciones hay en la lista

// Controles del juego: la tecla y lo que hace (cada fila de una lista va con la misma fila de la otra)
const char* const TECLAS[] = {"Clic izquierdo", "1, 2, 3", "Espacio", "U", "V", "F", "P o Esc"};
const char* const ACCIONES[] = {"Colocar o seleccionar torre", "Elegir tipo de torre", "Lanzar la siguiente oleada",
                                "Mejorar torre seleccionada", "Vender torre seleccionada", "Velocidad x2", "Pausa"};
const int TOTAL_CONTROLES = 7;

const int MAX_LETRAS_NOMBRE = 12;  // el nombre puede tener hasta 12 letras

// Regresa true si la letra se puede usar en un nombre: letras, espacio, acentos y la ñ
bool EsLetraValida(int letra) {
    if (letra >= 'a' && letra <= 'z') return true;
    if (letra >= 'A' && letra <= 'Z') return true;
    if (letra == ' ') return true;
    // Letras con acento, ñ y ü (en mayuscula y minuscula)
    const int especiales[] = {225, 233, 237, 243, 250, 193, 201, 205, 211, 218, 241, 209, 252, 220};
    for (int especial : especiales) {
        if (letra == especial) return true;
    }
    return false;
}

// Borra la ultima letra de un texto. Una letra con acento ocupa 2 bytes, asi que
// borramos los bytes "de continuacion" (los que empiezan con 10xxxxxx) y luego el primero.
void BorrarUltimaLetra(std::string& texto) {
    if (texto.empty()) return;
    while (texto.size() > 1 && (texto.back() & 0xC0) == 0x80) {
        texto.pop_back();
    }
    texto.pop_back();
}

Juego::Juego()
    : ancho_(1280), alto_(720), pantalla_(Pantalla::Menu), salir_(false), opcionSeleccionada_(0),
      imagenP1_{}, imagenP2_{}, imagenP3_{}, imagenP4_{}, imagenP5_{}, siguienteNivel_(""), nombreEscrito_(""), tiempo_(0.0f), tiempoAviso_(0.0f) {
    InitWindow(ancho_, alto_, "Guardianes de Puebla");
    SetTargetFPS(60);
    SetExitKey(KEY_NULL);  // Esc ya no cierra la ventana solo: nosotros decidimos que hace en cada pantalla
    CargarFuente("recursos/fuentes/juego.ttf");  // la tipografia de todo el juego
    fondoMenu_.Cargar("recursos/fondo_menu.gif", 12.0f);  // despues de InitWindow: necesita la ventana
    imagenP1_ = LoadTexture("recursos/dialogos/p1.png");
    imagenP2_ = LoadTexture("recursos/dialogos/p2.png");
    imagenP3_ = LoadTexture("recursos/dialogos/p3.png");
    imagenP4_ = LoadTexture("recursos/dialogos/p4.png");
    imagenP5_ = LoadTexture("recursos/dialogos/p5.png");
}

Juego::~Juego() {
    fondoMenu_.Descargar();  // antes de CloseWindow: la ventana todavia existe
    partida_.Descargar();
    UnloadTexture(imagenP1_);
    UnloadTexture(imagenP2_);
    UnloadTexture(imagenP3_);
    UnloadTexture(imagenP4_);
    UnloadTexture(imagenP5_);
    DescargarFuente();
    CloseWindow();
}

void Juego::Ejecutar() {
    // Sigue mientras nadie pida salir con Esc en el menu ni cierre la ventana con la X
    while (!salir_ && !WindowShouldClose()) {
        Actualizar(GetFrameTime());
        Dibujar();
    }
}

// ---------- Logica ----------

void Juego::Actualizar(float dt) {
    fondoMenu_.Actualizar(dt);
    tiempo_ += dt;
    if (tiempoAviso_ > 0.0f) tiempoAviso_ -= dt;

    switch (pantalla_) {
    case Pantalla::Menu:
        ActualizarMenu();
        break;

    case Pantalla::Nombre:
        ActualizarNombre();
        break;

    case Pantalla::Historia:  // P2: ENTER va a P3, Esc regresa a P1
        ActualizarDialogo(Pantalla::Mision, Pantalla::Nombre);
        break;

    case Pantalla::Mision:    // P3: ENTER empieza el juego, Esc regresa a P2
        ActualizarDialogo(Pantalla::Jugando, Pantalla::Historia);
        if (pantalla_ == Pantalla::Jugando) {
            partida_.Iniciar("niveles/nivel1.txt", jugador_);  // carga el nivel 1 desde su archivo
        }
        break;

    case Pantalla::Jugando:
        // La partida se encarga de todo; Juego solo revisa si hay que volver al menu
        partida_.ManejarEntrada();
        partida_.Actualizar(dt);
        if (partida_.QuiereSiguienteNivel()) {
            siguienteNivel_ = partida_.GetSiguienteNivel();  // lo guardamos para cargarlo despues de P4
            pantalla_ = Pantalla::Felicitacion;
        }
        if (partida_.QuiereFinal()) {
            pantalla_ = Pantalla::Final;  // gano el ultimo nivel
        }
        if (partida_.QuiereVolverAlMenu()) {
            pantalla_ = Pantalla::Menu;
        }
        break;

    case Pantalla::Felicitacion:  // P4: ENTER empieza el siguiente nivel, Esc regresa al menu
        ActualizarDialogo(Pantalla::Jugando, Pantalla::Menu);
        if (pantalla_ == Pantalla::Jugando) {
            partida_.Iniciar(siguienteNivel_, jugador_);
        }
        break;

    case Pantalla::Final:  // P5: ENTER muestra los creditos, Esc regresa al menu
        ActualizarDialogo(Pantalla::Creditos, Pantalla::Menu);
        break;

    case Pantalla::Controles:
    case Pantalla::Creditos:
        // Por ahora estas pantallas solo esperan a que el jugador regrese al menu
        if (IsKeyPressed(KEY_ESCAPE)) {
            pantalla_ = Pantalla::Menu;
        }
        break;
    }
}

void Juego::ActualizarMenu() {
    if (IsKeyPressed(KEY_DOWN)) {
        opcionSeleccionada_++;
        if (opcionSeleccionada_ >= TOTAL_OPCIONES) {
            opcionSeleccionada_ = 0;  // del ultimo regresa al primero
        }
    }
    if (IsKeyPressed(KEY_UP)) {
        opcionSeleccionada_--;
        if (opcionSeleccionada_ < 0) {
            opcionSeleccionada_ = TOTAL_OPCIONES - 1;  // del primero salta al ultimo
        }
    }

    // ENTER abre la pantalla de la opcion marcada
    if (IsKeyPressed(KEY_ENTER)) {
        switch (opcionSeleccionada_) {
        case 0:
            nombreEscrito_ = "";  // partida nueva: el cuadro del nombre empieza vacio
            pantalla_ = Pantalla::Nombre;
            break;
        case 1: pantalla_ = Pantalla::Controles; break;
        case 2: pantalla_ = Pantalla::Creditos; break;
        }
    }

    // Esc en el menu cierra el juego
    if (IsKeyPressed(KEY_ESCAPE)) {
        salir_ = true;
    }
}

void Juego::ActualizarNombre() {
    // GetCharPressed regresa las letras que se presionaron en este cuadro, una por una (0 = ya no hay mas)
    int letra = GetCharPressed();
    while (letra != 0) {
        int letrasActuales = GetCodepointCount(nombreEscrito_.c_str());
        if (EsLetraValida(letra) && letrasActuales < MAX_LETRAS_NOMBRE) {
            // Convertimos la letra a texto (las que llevan acento ocupan 2 bytes) y la pegamos al final
            int bytes = 0;
            const char* letraComoTexto = CodepointToUTF8(letra, &bytes);
            nombreEscrito_.append(letraComoTexto, bytes);
        }
        letra = GetCharPressed();
    }

    // Retroceso borra la ultima letra (si se deja presionado, sigue borrando)
    if (IsKeyPressed(KEY_BACKSPACE) || IsKeyPressedRepeat(KEY_BACKSPACE)) {
        BorrarUltimaLetra(nombreEscrito_);
    }

    // ENTER solo avanza si escribio algo que no sean puros espacios
    if (IsKeyPressed(KEY_ENTER)) {
        bool soloEspacios = nombreEscrito_.find_first_not_of(' ') == std::string::npos;
        if (soloEspacios) {
            tiempoAviso_ = 2.0f;  // muestra el aviso 2 segundos
        } else {
            jugador_.SetNombre(nombreEscrito_);
            pantalla_ = Pantalla::Historia;  // pasa a P2
        }
    }

    // Esc regresa al menu principal
    if (IsKeyPressed(KEY_ESCAPE)) {
        pantalla_ = Pantalla::Menu;
    }
}

// P2 y P3 funcionan igual: ENTER pasa a la pantalla siguiente y Esc regresa a la anterior
void Juego::ActualizarDialogo(Pantalla siguiente, Pantalla anterior) {
    if (IsKeyPressed(KEY_ENTER)) {
        pantalla_ = siguiente;
    }
    if (IsKeyPressed(KEY_ESCAPE)) {
        pantalla_ = anterior;
    }
}

// ---------- Dibujo ----------

void Juego::Dibujar() {
    BeginDrawing();
    ClearBackground(AZUL_OSCURO);

    if (pantalla_ == Pantalla::Jugando) {
        DibujarJugando();  // el juego tiene su propio escenario, sin el video
    } else if (pantalla_ == Pantalla::Nombre) {
        DibujarNombre();     // P1, P2 y P3 tienen su propia imagen, sin el video
    } else if (pantalla_ == Pantalla::Historia) {
        DibujarHistoria();
    } else if (pantalla_ == Pantalla::Mision) {
        DibujarMision();
    } else if (pantalla_ == Pantalla::Felicitacion) {
        DibujarFelicitacion();
    } else if (pantalla_ == Pantalla::Final) {
        DibujarFinal();
    } else {
        fondoMenu_.Dibujar(ancho_, alto_);

        if (pantalla_ == Pantalla::Menu) {
            // Menu: oscurece el lado izquierdo, donde van el titulo y las opciones; el volcan queda libre a la derecha
            DrawRectangleGradientH(0, 0, 760, alto_, Fade(AZUL_OSCURO, 0.92f), Fade(AZUL_OSCURO, 0.0f));
        } else {
            // Otras pantallas: oscurece todo el video para que se lea el panel
            DrawRectangle(0, 0, ancho_, alto_, Fade(AZUL_OSCURO, 0.55f));
        }

        switch (pantalla_) {
        case Pantalla::Menu:          DibujarMenu(); break;
        case Pantalla::Controles:     DibujarControles(); break;
        case Pantalla::Creditos:      DibujarCreditos(); break;
        case Pantalla::Jugando:       break;  // ya se dibujo arriba
        case Pantalla::Nombre:        break;  // ya se dibujo arriba
        case Pantalla::Historia:      break;  // ya se dibujo arriba
        case Pantalla::Mision:        break;  // ya se dibujo arriba
        case Pantalla::Felicitacion:  break;  // ya se dibujo arriba
        case Pantalla::Final:         break;  // ya se dibujo arriba
        }
    }

    EndDrawing();
}

void Juego::DibujarMenu() {
    int margen = 80;  // todo el menu va alineado a 80 pixeles del borde izquierdo

    // Titulo en dos lineas y subtitulo
    TextoConContorno("GUARDIANES", margen, 110, 64, AMARILLO);
    TextoConContorno("DE PUEBLA", margen, 180, 40, AMARILLO);
    TextoConContorno("El ataque de Don Goyo", margen, 232, 22, BLANCO);

    // Linea decorativa debajo del titulo
    DrawRectangle(margen, 272, 120, 3, AMARILLO);

    // Se repite 3 veces, una por opcion: i vale 0, 1 y 2
    for (int i = 0; i < TOTAL_OPCIONES; i++) {
        Rectangle caja = CajaOpcion(i);   // area que ocupa la opcion i
        int xTexto = (int)caja.x + 20;    // el texto va 20 pixeles a la derecha del borde de la caja
        int yTexto = (int)caja.y + 9;     // y 9 pixeles abajo, para quedar centrado de arriba a abajo

        if (i == opcionSeleccionada_) {
            // Opcion marcada: fondo oscuro, borde amarillo, una barrita a la izquierda y texto amarillo
            DrawRectangleRounded(caja, 0.3f, 8, Fade(AZUL_OSCURO, 0.85f));
            DrawRectangleRoundedLinesEx(caja, 0.3f, 8, 2.0f, AMARILLO);
            DrawRectangle((int)caja.x - 14, (int)caja.y + 8, 5, (int)caja.height - 16, AMARILLO);
            TextoConContorno(OPCIONES_MENU[i], xTexto, yTexto, 26, AMARILLO);
        } else {
            // Opcion normal: solo el texto en blanco
            TextoConContorno(OPCIONES_MENU[i], xTexto, yTexto, 26, BLANCO);
        }
    }

    // Ayuda abajo a la izquierda
    Texto("Flechas para moverte   |   ENTER para elegir   |   Esc para salir", margen, alto_ - 50, 16, GRIS);
}

void Juego::DibujarNombre() {
    // Fondo: la plaza con el abuelito y el cuadro de dialogo ya vacio
    DrawTexture(imagenP1_, 0, 0, WHITE);

    // Lo que dice el abuelito
    Texto("Hola, bienvenido.", 348, 462, 28, BLANCO);
    Texto("Antes de comenzar, dime tu nombre.", 348, 498, 28, BLANCO);

    // Dentro del cuadro amarillo: "Nombre:" y lo que va escribiendo el jugador
    int xNombre = 392;
    int yNombre = 572;
    Texto("Nombre:", xNombre, yNombre, 26, AMARILLO);
    int xTexto = xNombre + MedirTexto("Nombre:", 26) + 14;
    Texto(nombreEscrito_.c_str(), xTexto, yNombre, 26, BLANCO);

    // Cursor: una barrita despues del texto que aparece medio segundo y desaparece medio segundo
    bool cursorVisible = ((int)(tiempo_ * 2.0f)) % 2 == 0;
    if (cursorVisible) {
        int xCursor = xTexto + MedirTexto(nombreEscrito_.c_str(), 26) + 4;
        DrawRectangle(xCursor, yNombre - 1, 3, 28, BLANCO);
    }

    // Aviso si intento continuar sin nombre
    if (tiempoAviso_ > 0.0f) {
        Texto("Escribe tu nombre para continuar", 348, 617, 18, ROJO);
    }

    // Ayuda en la esquina de arriba
    Texto("Esc para volver al menu", 24, 20, 16, BLANCO);
}

void Juego::DibujarHistoria() {
    // P2: el nombre del jugador va despues de "Hola,"
    std::vector<std::string> lineas = {
        "Hola, " + jugador_.GetNombre() + ".",
        "Desde anoche, Don Goyo ha vuelto a despertar",
        "después de muchos años de calma.",
        "Su actividad ha aumentado y ahora envía criaturas de",
        "fuego, piedra y ceniza que se dirigen hacia nuestra ciudad.",
    };
    DibujarDialogo(imagenP2_, lineas, 456, 24, 30);
}

void Juego::DibujarMision() {
    // P3: el nombre del jugador va despues de "Necesitamos tu ayuda,"
    std::vector<std::string> lineas = {
        "Necesitamos tu ayuda, " + jugador_.GetNombre() + ".",
        "Debes defender a la ciudad del fuego,",
        "construyendo defensas y deteniendo las",
        "oleadas de criaturas que envía Don Goyo.",
    };
    DibujarDialogo(imagenP3_, lineas, 440, 26, 36);
}

void Juego::DibujarFelicitacion() {
    // P4: el texto va a la derecha del abuelito (su mano llega hasta x = 400)
    std::vector<std::string> lineas = {
        "¡Muy bien, " + jugador_.GetNombre() + "! Salvaste Cholula.",
        "Pero Don Goyo no se rinde: ahora sus criaturas",
        "van hacia Los Fuertes por dos caminos.",
        "Y se acerca la oleada final... ¡prepárate!",
    };
    DibujarDialogo(imagenP4_, lineas, 482, 26, 40, 420);
    // Ayuda a la izquierda de la flechita amarilla del cuadro (termina en x = 1150)
    const char* ayuda = "ENTER para continuar";
    Texto(ayuda, 1150 - MedirTexto(ayuda, 16), 626, 16, GRIS);
}

void Juego::DibujarFinal() {
    // P5: misma posicion de texto que P4 (a la derecha del abuelito)
    std::vector<std::string> lineas = {
        "¡Gracias, " + jugador_.GetNombre() + "! Lo lograste.",
        "Detuviste a las criaturas y a los ladrones,",
        "y parece que Don Goyo volvió a dormir.",
        "Puebla está a salvo gracias a ti.",
    };
    DibujarDialogo(imagenP5_, lineas, 482, 26, 40, 420);
    const char* ayuda = "ENTER para ver los créditos";
    Texto(ayuda, 1150 - MedirTexto(ayuda, 16), 626, 16, GRIS);
}

void Juego::DibujarJugando() {
    partida_.Dibujar();
}

void Juego::DibujarControles() {
    DibujarPanel("CONTROLES", 390.0f);

    // Dos columnas: la tecla a la izquierda en amarillo, lo que hace a la derecha en blanco
    int xTecla = ancho_ / 2 - 280;
    int xAccion = ancho_ / 2 - 60;
    for (int i = 0; i < TOTAL_CONTROLES; i++) {
        int y = 240 + i * 42;
        Texto(TECLAS[i], xTecla, y, 22, AMARILLO);
        Texto(ACCIONES[i], xAccion, y, 22, BLANCO);
    }
    DibujarAyuda("Esc para volver al menu");
}

void Juego::DibujarCreditos() {
    DibujarPanel("CRÉDITOS", 420.0f);
    TextoCentrado("Guardianes de Puebla: El ataque de Don Goyo", 245, 24, AMARILLO);
    TextoCentrado("Desarrollado por", 300, 18, GRIS);

    // Integrantes del equipo: uno debajo de otro, cada 30 pixeles
    const char* const EQUIPO[] = {"Diego Navarro Sánchez", "Marco Antonio Beltrán Rosales", "Jonathan Hernández Lazcano"};
    for (int i = 0; i < 3; i++) {
        TextoCentrado(EQUIPO[i], 328 + i * 30, 24, BLANCO);
    }

    TextoCentrado("Programación Avanzada y Algoritmos", 445, 18, GRIS);
    TextoCentrado("IBERO Puebla, 2026", 473, 18, GRIS);
    TextoCentrado("Hecho en C++ con raylib", 515, 18, GRIS);
    DibujarAyuda("Esc para volver al menu");
}

// ---------- Ayudas para dibujar ----------

// Regresa el rectangulo (x, y, ancho, alto) donde va la opcion numero "indice"
Rectangle Juego::CajaOpcion(int indice) {
    float anchoCaja = 320.0f;
    float altoCaja = 44.0f;
    float x = 80.0f;                    // alineada a la izquierda, con el titulo
    float y = 340.0f + indice * 58.0f;  // cada opcion 58 pixeles mas abajo que la anterior
    return Rectangle{x, y, anchoCaja, altoCaja};
}

// Escribe un texto centrado horizontalmente en la ventana, con contorno
void Juego::TextoCentrado(const char* texto, int y, int tamano, Color color) {
    int anchoTexto = MedirTexto(texto, tamano);
    TextoConContorno(texto, (ancho_ - anchoTexto) / 2, y, tamano, color);
}

// Escribe un texto en (x, y) con contorno oscuro y sombra, para que se lea sobre el video
void Juego::TextoConContorno(const char* texto, int x, int y, int tamano, Color color) {
    DibujarTextoContorno(texto, x, y, tamano, color);  // la funcion compartida vive en Paleta.cpp
}

// Panel oscuro centrado, de 720 de ancho, con un titulo arriba
void Juego::DibujarPanel(const char* titulo, float alto) {
    Rectangle panel{(ancho_ - 720.0f) / 2.0f, 150.0f, 720.0f, alto};
    DrawRectangleRounded(panel, 0.06f, 8, Fade(AZUL_OSCURO, 0.92f));
    DrawRectangleRoundedLinesEx(panel, 0.06f, 8, 2.0f, AMARILLO);
    TextoCentrado(titulo, 175, 36, AMARILLO);
}

// Texto gris de ayuda, centrado hasta abajo de la ventana
void Juego::DibujarAyuda(const char* texto) {
    int tamano = 16;
    Texto(texto, (ancho_ - MedirTexto(texto, tamano)) / 2, alto_ - 34, tamano, GRIS);
}

// Dibuja una pantalla de dialogo: la imagen de fondo y las lineas del abuelito, una debajo de otra
void Juego::DibujarDialogo(Texture2D imagen, const std::vector<std::string>& lineas, int yInicio, int tamano, int separacion,
                           int x) {
    DrawTexture(imagen, 0, 0, WHITE);

    int y = yInicio;
    for (const std::string& linea : lineas) {
        DibujarLineaDialogo(linea, x, y, tamano);
        y += separacion;
    }

    if (pantalla_ == Pantalla::Felicitacion || pantalla_ == Pantalla::Final) {
        Texto("Esc para volver al menu", 24, 20, 16, BLANCO);
    } else {
        Texto("Esc para regresar", 24, 20, 16, BLANCO);
    }
}

// Escribe una linea de dialogo en blanco, pero si encuentra "Don Goyo" lo pinta en amarillo
void Juego::DibujarLineaDialogo(const std::string& linea, int x, int y, int tamano) {
    const std::string resaltado = "Don Goyo";
    size_t posicion = linea.find(resaltado);

    if (posicion == std::string::npos) {
        Texto(linea.c_str(), x, y, tamano, BLANCO);  // no aparece "Don Goyo": toda la linea en blanco
        return;
    }

    // Partimos la linea en 3 pedazos: antes, "Don Goyo" y despues
    std::string antes = linea.substr(0, posicion);
    std::string despues = linea.substr(posicion + resaltado.size());

    Texto(antes.c_str(), x, y, tamano, BLANCO);
    x += MedirTexto(antes.c_str(), tamano) + tamano / 10;      // avanzamos lo que midio el pedazo, mas el espacio entre letras
    Texto(resaltado.c_str(), x, y, tamano, AMARILLO);
    x += MedirTexto(resaltado.c_str(), tamano) + tamano / 10;
    Texto(despues.c_str(), x, y, tamano, BLANCO);
}
