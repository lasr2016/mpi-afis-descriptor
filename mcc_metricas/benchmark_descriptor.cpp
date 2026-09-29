#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdio>
#include <chrono>
#include "MCC.h"

using namespace std;

bool cargarMinucias(const string& rutaArchivo, Matrix<int>& M_xyt) {
    ifstream archivo(rutaArchivo.c_str());
    if (!archivo.is_open()) return false;

    vector<vector<int>> filas;
    int x, y, t, calidad;

    while (archivo >> x >> y >> t >> calidad) {
        filas.push_back({x, y, t});
    }

    if (filas.empty()) return false;

    M_xyt.resize(filas.size(), 3);
    for (size_t i = 0; i < filas.size(); i++) {
        M_xyt[i][0] = filas[i][0];
        M_xyt[i][1] = filas[i][1];
        M_xyt[i][2] = filas[i][2];
    }

    return true;
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        cout << "Uso: " << argv[0] << " <huella.xyt> <salida.bin> -N {8|16} -C {LSS|LSSR|LSA|LSAR|LGS|NHS}" << endl;
        return 1;
    }

    string ruta_entrada = argv[1];
    string ruta_salida = argv[2];

    MCC::configureAlgorithm(argc, argv);

    // ---- 1. Medir LECTURA desde disco ----
    Matrix<int> matrizMinucias;

    auto inicioLectura = chrono::high_resolution_clock::now();
    if (!cargarMinucias(ruta_entrada, matrizMinucias)) {
        cerr << "Error al cargar: " << ruta_entrada << endl;
        return 1;
    }
    auto finLectura = chrono::high_resolution_clock::now();

    chrono::duration<double, milli> tLectura = finLectura - inicioLectura;

    // ---- 2. Medir GENERACION del descriptor ----
    MCC huella(matrizMinucias);

    auto inicioDescriptor = chrono::high_resolution_clock::now();
    huella.initialize();
    auto finDescriptor = chrono::high_resolution_clock::now();

    chrono::duration<double, milli> tDescriptor = finDescriptor - inicioDescriptor;

    // ---- 3. Medir ESCRITURA del .bin ----
    remove(ruta_salida.c_str());

    auto inicioEscritura = chrono::high_resolution_clock::now();
    huella.writeCylinder(ruta_salida);
    auto finEscritura = chrono::high_resolution_clock::now();

    chrono::duration<double, milli> tEscritura = finEscritura - inicioEscritura;

    cout << "------------------------------------------" << endl;
    cout << "Huella: " << ruta_entrada << " (" << matrizMinucias.rows() << " minucias)" << endl;
    cout << "Tiempo LECTURA (disco):      " << tLectura.count() << " ms" << endl;
    cout << "Tiempo DESCRIPTOR (calculo): " << tDescriptor.count() << " ms" << endl;
    cout << "Tiempo ESCRITURA (.bin):     " << tEscritura.count() << " ms" << endl;
    cout << "------------------------------------------" << endl;
    cout << "Tiempo TOTAL:                " << (tLectura.count() + tDescriptor.count() + tEscritura.count()) << " ms" << endl;

    return 0;
}
