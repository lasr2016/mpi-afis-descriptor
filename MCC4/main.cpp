#include <iostream>
#include <fstream>
#include <string>
#include "MCC.h"

using namespace std;

// Función para leer minucias de texto de la huella a consultar
bool cargarMinucias(const string& rutaArchivo, Matrix<int>& M_xyt) {
    ifstream archivo(rutaArchivo.c_str());
    if (!archivo.is_open()) return false;

    int num_minucias;
    if (!(archivo >> num_minucias) || num_minucias <= 0) return false;
       cout << "CANTIDAD DE MINUCIAS!" << num_minucias << endl;

    M_xyt.resize(num_minucias, 3);
    int num;
    for (int i = 0; i < num_minucias; i++) {
        for (int j = 0; j < 4; j++) {
            archivo >> num;
            if (j < 3)
                M_xyt[i][j] = num;
        }
    }
    return true;
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        cout << "Uso: " << argv[0] << " <huella_consulta.txt> <lista_descriptores.txt> -N {8|16} -C {LSS|LSSR|LSA|LSAR|LGS|NHS}" << endl;
        return 1;
    }

    string ruta_consulta = argv[1];
    string ruta_lista = argv[2];

    MCC::configureAlgorithm(argc, argv);

    // ========================================================
    // 1. CARGAR E INICIALIZAR HUELLA CONSULTA (Probe)
    // ========================================================
    Matrix<int> matrizProbe;
    if (!cargarMinucias(ruta_consulta, matrizProbe)) {
        cerr << "Error al cargar archivo de consulta: " << ruta_consulta << endl;
        return 1;
    }

    MCC probe(matrizProbe);
    probe.initialize(); // Se calcula en el momento

    // ========================================================
    // 2. LEER LISTA DE DESCRIPTORES BINARIOS PRECALCULADOS
    // ========================================================
    ifstream lista(ruta_lista.c_str());
    if (!lista.is_open()) {
        cerr << "Error al abrir la lista de la base de datos." << endl;
        return 1;
    }

    int total_huellas;
    if (!(lista >> total_huellas) || total_huellas <= 0) {
        cerr << "Cantidad de huellas invalida en la lista." << endl;
        return 1;
    }

    double max_score = -1.0;
    string mejor_coincidencia = "";
    double umbral = 0.35;

    cout << "Comparando huella de entrada contra " << total_huellas << " descriptores..." << endl;

    string ruta_bin;
    for (int i = 0; i < total_huellas; i++) {
        if (!(lista >> ruta_bin)) break;

        // Instancia vacía
        MCC candidata;
        
        // CARGA DIRECTA DEL DESCRIPTOR (No se llama a initialize)
        candidata.loadCylinder(ruta_bin);

        // Matching inmediato
        double score = probe.match(candidata);
        cout << "Evaluando [" << ruta_bin << "] -> Score: " << score << endl;

        if (score > max_score) {
            max_score = score;
            mejor_coincidencia = ruta_bin;
        }
    }
    lista.close();

    // ========================================================
    // 3. RESULTADO
    // ========================================================
    cout << "------------------------------------------" << endl;
    if (max_score >= umbral && !mejor_coincidencia.empty()) {
        cout << "MATCH ENCONTRADO!" << endl;
        cout << "Coincide con: " << mejor_coincidencia << endl;
        cout << "Score maximo: " << max_score << endl;
    } else {
        cout << "NO MATCH (Huella no encontrada en la base de datos)" << endl;
        cout << "Mejor score obtenido: " << max_score << endl;
    }

    return 0;
}