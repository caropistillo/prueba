
#include <iostream>
#include "Juego.h"

using namespace std;


Juego* juego = 0;

int main(int argc, char** argv) {

    juego = new Juego();
    juego->iniciar("Tp celulas", 100, 100, 0);
    cout<<"Hola"<<endl;
    juego->correr();

    juego->limpiar();

    delete juego;
    return 0;
}
