#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdio>
#include <chrono>   // Necesario para medir el tiempo
#include "MCC.h"

using namespace std;

// Función para leer las coordenadas de minucias desde el archivo de texto
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
    // Uso: ./generar_descriptor <minucias_origen.txt> <descriptor_salida.bin> -N {8|16} -C {LSS|LSSR|...}
    if (argc < 3) {
        cout << "Uso: " << argv[0] << " <entrada_minucias.txt> <salida_descriptor.bin> -N {8|16} -C {LSS|LSSR|LSA|LSAR|LGS|NHS}" << endl;
        return 1;
    }

    string ruta_entrada = argv[1];
    string ruta_salida = argv[2];

    // Configurar los parámetros del descriptor (como el tamaño del cilindro -N 8)
    MCC::configureAlgorithm(argc, argv);

    // 1. Cargar las minucias desde el archivo de texto
    Matrix<int> matrizMinucias;

    // ---- Medición del tiempo de lectura de disco a memoria ----
    auto inicio_lectura = chrono::high_resolution_clock::now();
    bool exitoCarga = cargarMinucias(ruta_entrada, matrizMinucias);
    auto fin_lectura = chrono::high_resolution_clock::now();

    if (!exitoCarga) {
        cerr << "Error: No se pudo leer el archivo o está vacío: " << ruta_entrada << endl;
        return 1;
    }

    chrono::duration<double, milli> tiempoLectura = fin_lectura - inicio_lectura;
    cout << "Tiempo de lectura de disco a memoria: " << tiempoLectura.count() << " ms" << endl;
    // ---- Fin medición lectura ----

    cout << "Calculando descriptor MCC para: " << ruta_entrada << "..." << endl;

    // 2. Instanciar y transformar a cilindros 3D
    auto inicio_init = chrono::high_resolution_clock::now();
    MCC huella(matrizMinucias);

    // ---- Medición del tiempo de initialize() ----
    huella.initialize();
    auto fin_init = chrono::high_resolution_clock::now();

    chrono::duration<double, milli> tiempoInitialize = fin_init - inicio_init;
    cout << "Tiempo de initialize() (cómputo del descriptor): " << tiempoInitialize.count() << " ms" << endl;
    // ---- Fin medición initialize ----

    // Eliminar archivo previo si existe (para evitar que writeCylinder haga append)
    remove(ruta_salida.c_str());

    // 3. Guardar el descriptor precalculado en archivo binario
    huella.writeCylinder(ruta_salida);

    cout << "Descriptor guardado exitosamente en: " << ruta_salida << endl;

    return 0;
}
