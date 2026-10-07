#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <omp.h>
#include "MCC.h"
#include <chrono>
#include <iomanip>

using namespace std;

struct ItemGaleria {
    string ruta;
    MCC descriptor;
};

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
        cout << "Uso: " << argv[0] << " <huella_consulta.xyt> <lista_descriptores.txt> -N {8|16} -C {LSS|LSSR|LSA|LSAR}" << endl;
        return 1;
    }

    string ruta_consulta = argv[1];
    string ruta_lista = argv[2];

    MCC::configureAlgorithm(argc, argv);

    // ========================================================
    // 1. CARGA DE CONSULTA (PROBE)
    // ========================================================
    Matrix<int> matrizProbe;
    if (!cargarMinucias(ruta_consulta, matrizProbe)) {
        cerr << "Error al cargar archivo de consulta: " << ruta_consulta << endl;
        return 1;
    }

    MCC probe(matrizProbe);
    probe.initialize();

    // ========================================================
    // 2. CARGA DE DESCRIPTORES A MEMORIA
    // ========================================================
    ifstream lista(ruta_lista.c_str());
    if (!lista.is_open()) {
        cerr << "Error al abrir lista de descriptores: " << ruta_lista << endl;
        return 1;
    }

    int total_huellas;
    if (!(lista >> total_huellas) || total_huellas <= 0) {
        cerr << "Cantidad invalida en la lista." << endl;
        return 1;
    }

    vector<ItemGaleria> galeria;
    galeria.reserve(total_huellas);

    string ruta_bin;
    for (int i = 0; i < total_huellas; i++) {
        if (!(lista >> ruta_bin)) break;
        ItemGaleria item;
        item.ruta = ruta_bin;
        item.descriptor.loadCylinder(ruta_bin);
        galeria.push_back(item);
    }
    lista.close();

    // ========================================================
    // 3. MATCHING PARALELO CON OPENMP (Sin parallel for)
    // ========================================================
    double max_score_global = -1.0;
    string mejor_ruta_global = "";
    int n_total = static_cast<int>(galeria.size());

    // Región paralela usando `#pragma omp parallel`
    
    
    // INICIO TIEMPO
    auto t_ini = std::chrono::high_resolution_clock::now();
    
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        int n_threads = omp_get_num_threads();

        // Cálculo de rango contiguo por hilo
        int bloque = n_total / n_threads;
        int inicio = tid * bloque;
        int fin = (tid == n_threads - 1) ? n_total : inicio + bloque;

        // Variables locales para que ningún hilo compita durante el loop
        double max_score_local = -1.0;
        string mejor_ruta_local = "";

        for (int i = inicio; i < fin; i++) {
            double score = probe.match(galeria[i].descriptor);
            if (score > max_score_local) {
                max_score_local = score;
                mejor_ruta_local = galeria[i].ruta;
            }
        }

        // Reducción final única: solo se bloquea una vez por hilo al terminar
        #pragma omp critical
        {
            if (max_score_local > max_score_global) {
                max_score_global = max_score_local;
                mejor_ruta_global = mejor_ruta_local;
            }
        }
    }
    
    auto t_fin = std::chrono::high_resolution_clock::now();
    // FIN TIEMPO
    
    // CALCULO E IMPRIME TIEMPO    
    double tiempo_matching_ms = std::chrono::duration<double, std::milli>(t_fin - t_ini).count();
    double tiempo_unitario_ms = tiempo_matching_ms / galeria.size();
    double estimado_13k_ms = tiempo_unitario_ms * 13000.0;
    double estimado_13k_s  = estimado_13k_ms / 1000.0;

    std::cout << "Tiempo puro de matching: " << tiempo_matching_ms << " ms" << std::endl;
    std::cout << "tiempo por huella 1:1 = " << fixed << setprecision(4) 
              << tiempo_unitario_ms << " ms" << std::endl;
    std::cout << "Tiempo estimado 1:13.000 = " << fixed << setprecision(2) 
              << estimado_13k_ms << " ms (" << setprecision(3) << estimado_13k_s << " s)" << std::endl;
    
    // ========================================================
    
    
    
    // 4. RESULTADO
    // ========================================================
    double umbral = 0.35;
    cout << "------------------------------------------" << endl;
    if (max_score_global >= umbral && !mejor_ruta_global.empty()) {
        cout << "MATCH ENCONTRADO!" << endl;
        cout << "Coincide con: " << mejor_ruta_global << endl;
        cout << "Score maximo: " << max_score_global << endl;
    } else {
        cout << "NO MATCH (Huella no encontrada)" << endl;
        cout << "Mejor score obtenido: " << max_score_global << endl;
    }

    return 0;
}
