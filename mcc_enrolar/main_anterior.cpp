#include <iostream>
#include <fstream>
#include <string>
#include "MCC.h"

using namespace std;

// Función para leer las minucias desde un archivo dado por su ruta
bool cargarMinucias(const string& rutaArchivo, Matrix<int>& M_xyt) {
    ifstream archivo(rutaArchivo.c_str());
    if (!archivo.is_open()) {
        cerr << "Error: No se pudo abrir el archivo: " << rutaArchivo << endl;
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
    // Se esperan al menos: ./MCC <huella1> <huella2> -N 8 -C LSS ...
    if (argc < 3) {
        cout << "Uso: " << argv[0] << " <huella1.txt> <huella2.txt> -N {8|16} -C {LSS|LSSR|LSA|LSAR|LGS|NHS} [-H] [-B]" << endl;
        return 1;
    }

    string ruta_h1 = argv[1];
    string ruta_h2 = argv[2];

    // Desplazamos los argumentos para que configureAlgorithm lea los flags correctamente
    // argv[0] se mantiene, pero omitimos argv[2] moviendo los flags hacia la izquierda
    // o simplemente pasando argc y argv originales si tu librería tolera dos archivos
    MCC::configureAlgorithm(argc, argv);

    // 1. Cargar la primera huella
    Matrix<int> M_xyt1;
    if (!cargarMinucias(ruta_h1, M_xyt1)) return 1;

    // 2. Cargar la segunda huella
    Matrix<int> M_xyt2;
    if (!cargarMinucias(ruta_h2, M_xyt2)) return 1;

    // 3. Crear los cilindros MCC e inicializar
    MCC a1(M_xyt1);
    MCC a2(M_xyt2);

    a1.initialize();
    a2.initialize();

    // 4. Comparar
    double score = a1.match(a2);

    cout << "Comparando: " << ruta_h1 << " vs " << ruta_h2 << endl;
    cout << "Score de similitud: " << score << endl;

    double umbral = 0.40;
    if (score >= umbral) {
        cout << "Resultado: MATCH (Misma huella)" << endl;
    } else {
        cout << "Resultado: NO MATCH (Huellas distintas)" << endl;
    }

    return 0;
}