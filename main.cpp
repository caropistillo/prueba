
#include <iostream>
#include "Juego.h"

using namespace std;


Juego* juego = 0;

int main(int argc, char** argv) {

    juego = new Juego();
    cout<<"Hola"<<endl;

    juego->correr();

    juego->limpiar();

    delete juego;
    return 0;
}
