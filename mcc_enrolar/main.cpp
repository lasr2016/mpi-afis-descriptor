#include <iostream>
#include <fstream>
#include <string>
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
    if (!cargarMinucias(ruta_entrada, matrizMinucias)) {
        return 1;
    }

    cout << "Calculando descriptor MCC para: " << ruta_entrada << "..." << endl;

    // 2. Instanciar y transformar a cilindros 3D
    MCC huella(matrizMinucias);
    huella.initialize();

    // 3. Guardar el descriptor precalculado en archivo binario
    huella.writeCylinder(ruta_salida);

    cout << "Descriptor guardado exitosamente en: " << ruta_salida << endl;

    return 0;
}