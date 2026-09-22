#include <iostream>
#include <vector>
#include <string>
#include "MCC.h"

using namespace std;

// Umbral de decisión experimental (ajustar según el consolidador: LSS, LSA, etc.)
const double UMBRAL_MATCH = 0.40;

// Función auxiliar para leer una huella desde cin/archivo
bool leerHuella(Matrix<int>& M_xyt) {
    int num_minucias, num;
    if (cin >> num_minucias) {
        if (num_minucias <= 0) return false;
        
        M_xyt.resize(num_minucias, 3);
        for (int i = 0; i < num_minucias; i++) {
            for (int j = 0; j < 4; j++) {
                cin >> num;
                if (j < 3)
                    M_xyt[i][j] = num;
            }
        }
        return true;
    }
    return false;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cout << "Usage: MCC <dummy> -N {8|16} -C {LSS|LSSR|LSA|LSAR|LGS|NHS} [-H] [-B]" << endl;
        return 0;
    }

    MCC::configureAlgorithm(argc, argv);

    // ========================================================
    // 1. CARGAR LA HUELLA CONSULTA (Probe)
    // ========================================================
    Matrix<int> matrizProbe;
    if (!leerHuella(matrizProbe)) {
        cerr << "Error al leer la huella de consulta." << endl;
        return 1;
    }

    MCC probe(matrizProbe);
    probe.initialize();

    // ========================================================
    // 2. CARGAR LA LISTA DE HUELLAS DE LA BASE DE DATOS
    // ========================================================
    int total_bd;
    if (!(cin >> total_bd)) {
        cerr << "Error al leer la cantidad de huellas en la base de datos." << endl;
        return 1;
    }

    vector<MCC> galeria;
    for (int k = 0; k < total_bd; k++) {
        Matrix<int> matrizGaleria;
        if (leerHuella(matrizGaleria)) {
            MCC item(matrizGaleria);
            item.initialize();
            galeria.push_back(item);
        }
    }

    // ========================================================
    // 3. COMPARAR 1:N (MATCHING LINEAL)
    // ========================================================
    double max_score = -1.0;
    int mejor_indice = -1;

    for (size_t i = 0; i < galeria.size(); i++) {
        double score = probe.match(galeria[i]);

        cout << "Comparando con Huella #" << i << " -> Score: " << score << endl;

        if (score > max_score) {
            max_score = score;
            mejor_indice = i;
        }
    }

    // ========================================================
    // 4. VEREDICTO DE VALIDACIÓN
    // ========================================================
    cout << "------------------------------------------" << endl;
    if (max_score >= UMBRAL_MATCH && mejor_indice != -1) {
        cout << "COINCIDENCIA ENCONTRADA!" << endl;
        cout << "La huella coincide con el registro #" << mejor_indice 
             << " con un puntaje de: " << max_score << endl;
    } else {
        cout << "NO IDENTIFICADO." << endl;
        cout << "La huella no esta en la lista (Mejor score: " << max_score 
             << " por debajo del umbral: " << UMBRAL_MATCH << ")." << endl;
    }

    return 0;
}