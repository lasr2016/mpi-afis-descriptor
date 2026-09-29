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

    cout << "Numero Minucias: " << filas.size() << endl;

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
        cout << "Uso: " << argv[0] << " <huella1.xyt> <huella2.xyt> -N {8|16} -C {LSS|LSSR|LSA|LSAR|LGS|NHS}" << endl;
        return 1;
    }

    string ruta_huella1 = argv[1];
    string ruta_huella2 = argv[2];

    MCC::configureAlgorithm(argc, argv);

    // 1. Cargar y calcular el descriptor de ambas huellas (fuera del cronometro)
    Matrix<int> matriz1, matriz2;
    if (!cargarMinucias(ruta_huella1, matriz1)) {
        cerr << "Error al cargar: " << ruta_huella1 << endl;
        return 1;
    }
    if (!cargarMinucias(ruta_huella2, matriz2)) {
        cerr << "Error al cargar: " << ruta_huella2 << endl;
        return 1;
    }

    MCC huella1(matriz1);
    huella1.initialize();

    MCC huella2(matriz2);
    huella2.initialize();

    cout << "Descriptores calculados. Iniciando benchmark de matching..." << endl;

    // 2. Correr match() 1000 veces, midiendo el tiempo total
    const int NUM_ITERACIONES = 1000;

    auto inicio = chrono::high_resolution_clock::now();

    float ultimoPuntaje = 0;
    for (int i = 0; i < NUM_ITERACIONES; i++) {
        ultimoPuntaje = huella1.match(huella2);
    }

    auto fin = chrono::high_resolution_clock::now();

    // 3. Calcular el promedio
    chrono::duration<double, milli> tiempoTotalMs = fin - inicio;
    double promedioMs = tiempoTotalMs.count() / NUM_ITERACIONES;

    cout << "------------------------------------------" << endl;
    cout << "Puntaje de coincidencia: " << ultimoPuntaje << endl;
    cout << "Iteraciones: " << NUM_ITERACIONES << endl;
    cout << "Tiempo total: " << tiempoTotalMs.count() << " ms" << endl;
    cout << "Tiempo promedio por match: " << promedioMs << " ms" << endl;

    return 0;
}
