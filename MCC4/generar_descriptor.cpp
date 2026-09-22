#include <iostream>
#include <fstream>
#include <string>
#include "MCC.h"

using namespace std;

// Función para leer las coordenadas de minucias desde el archivo de texto
bool cargarMinucias(const string& rutaArchivo, Matrix<int>& M_xyt) {
    ifstream archivo(rutaArchivo.c_str());
    if (!archivo.is_open()) {
        cerr << "Error: No se pudo abrir el archivo de entrada: " << rutaArchivo << endl;
        return false;
    }

    int num_minucias;
    if (!(archivo >> num_minucias) || num_minucias <= 0) {
        cerr << "Error: Cantidad de minucias invalida en: " << rutaArchivo << endl;
        return false;
    }

    M_xyt.resize(num_minucias, 3);
    int num;
    for (int i = 0; i < num_minucias; i++) {
        for (int j = 0; j < 4; j++) {
            archivo >> num;
            if (j < 3)
                M_xyt[i][j] = num;
        }
    }

    archivo.close();
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