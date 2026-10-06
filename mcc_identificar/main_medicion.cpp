#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <chrono>
#include "MCC.h"

using namespace std;

// Función para cargar minucias de la huella a consultar
bool cargarMinucias(const string& rutaArchivo, Matrix<int>& M_xyt) {
    ifstream archivo(rutaArchivo.c_str());
    if (!archivo.is_open()) return false;

    vector<vector<int>> filas;
    int x, y, t, calidad;

    while (archivo >> x >> y >> t >> calidad) {
        filas.push_back({x, y, t});
    }

    if (filas.empty()) return false;

    cout << "Numero Minucias en consulta: " << filas.size() << endl;

    M_xyt.resize(filas.size(), 3);
    for (size_t i = 0; i < filas.size(); i++) {
        M_xyt[i][0] = filas[i][0];
        M_xyt[i][1] = filas[i][1];
        M_xyt[i][2] = filas[i][2];
    }

    return true;
}

// Estructura para almacenar cada huella en la memoria RAM
struct ItemGaleria {
    string ruta;
    MCC descriptor;
};

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
    probe.initialize(); // Se calcula solo la de consulta

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

    // ========================================================
    // 2. FASE 1: CARGAR TODOS LOS DESCRIPTORES A RAM (I/O)
    // ========================================================
    auto inicio_io = chrono::high_resolution_clock::now();

    vector<ItemGaleria> galeria;
    galeria.reserve(total_huellas);

    string ruta_bin;
    for (int i = 0; i < total_huellas; i++) {
        if (!(lista >> ruta_bin)) break;

        ItemGaleria item;
        item.ruta = ruta_bin;
        item.descriptor.loadCylinder(ruta_bin); // Carga directa del binario a memoria
        galeria.push_back(item);
    }
    lista.close();

    auto fin_io = chrono::high_resolution_clock::now();

    // ========================================================
    // 3. FASE 2: MATCHING 1:N PURO EN MEMORIA RAM (CPU pura)
    // ========================================================
    auto inicio_matching = chrono::high_resolution_clock::now();

    double max_score = -1.0;
    string mejor_coincidencia = "";
    double umbral = 0.35; // Umbral con -C LSSR

    for (size_t i = 0; i < galeria.size(); i++) {
        double score = probe.match(galeria[i].descriptor);

        if (score > max_score) {
            max_score = score;
            mejor_coincidencia = galeria[i].ruta;
        }
    }

    auto fin_matching = chrono::high_resolution_clock::now();

    // ========================================================
    // 4. RESULTADO BIOMÉTRICO
    // ========================================================
    cout << "------------------------------------------" << endl;
    if (max_score >= umbral && !mejor_coincidencia.empty()) {
        cout << "MATCH ENCONTRADO!" << endl;
        cout << "Coincide con: " << mejor_coincidencia << endl;
        cout << "Score maximo: " << max_score << endl;
    } else {
        cout << "NO MATCH (Huella no encontrada en la base de datos)" << endl;
        cout << "Mejor score obtenido: " << max_score << " (Umbral: " << umbral << ")" << endl;
    }

    // ========================================================
    // 5. REPORTES DE TIEMPO SEPARADOS
    // ========================================================
    chrono::duration<double, milli> t_io = fin_io - inicio_io;
    chrono::duration<double, milli> t_matching = fin_matching - inicio_matching;
    double t_promedio_match = galeria.empty() ? 0.0 : (t_matching.count() / galeria.size());

    cout << "------------------------------------------" << endl;
    cout << "Huellas evaluadas      : " << galeria.size() << endl;
    cout << "Tiempo de carga (I/O)  : " << t_io.count() << " ms" << endl;
    cout << "Tiempo matching 1:N    : " << t_matching.count() << " ms (CPU puro en RAM)" << endl;
    cout << "Promedio por match     : " << t_promedio_match << " ms/huella" << endl;
    cout << "------------------------------------------" << endl;

    return 0;
}