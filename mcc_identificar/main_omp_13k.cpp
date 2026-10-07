#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <chrono>
#include <iomanip>
#include <omp.h>
#include "MCC.h"

using namespace std;
using namespace std::chrono;

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
    // 2. CARGA DE DESCRIPTORES A MEMORIA RAM
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

    int n_total = static_cast<int>(galeria.size());
    cout << "Descriptores cargados en RAM: " << n_total << endl;

    // ========================================================
    // 3. FASE DE CALENTAMIENTO (WARM-UP)
    // ========================================================
    // Se ejecuta una pasada en paralelo descartando tiempos para
    // asentar caches L1/L2, levantar frecuencia de CPU y evitar page-faults.
    cout << "Ejecutando fase de Warm-up..." << flush;
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        int n_threads = omp_get_num_threads();

        int bloque = n_total / n_threads;
        int inicio = tid * bloque;
        int fin = (tid == n_threads - 1) ? n_total : inicio + bloque;

        for (int i = inicio; i < fin; i++) {
            volatile double dummy = probe.match(galeria[i].descriptor);
            (void)dummy;
        }
    }
    cout << " Listo.\n" << endl;

    // ========================================================
    // 4. BENCHMARK: 13 REPETICIONES (SIMULACIÓN 13.000 COMPARACIONES)
    // ========================================================
    const int REPETICIONES = 13;
    vector<double> tiempos_ms(REPETICIONES);
    double suma_total_ms = 0.0;

    double max_score_global = -1.0;
    string mejor_ruta_global = "";

    cout << "Iniciando medicion de " << REPETICIONES << " corridas (total " 
         << (REPETICIONES * n_total) << " comparaciones en RAM)..." << endl;

    for (int rep = 0; rep < REPETICIONES; rep++) {
        double max_score_rep = -1.0;
        string mejor_ruta_rep = "";

        auto t_ini = high_resolution_clock::now();

        #pragma omp parallel
        {
            int tid = omp_get_thread_num();
            int n_threads = omp_get_num_threads();

            int bloque = n_total / n_threads;
            int inicio = tid * bloque;
            int fin = (tid == n_threads - 1) ? n_total : inicio + bloque;

            double max_score_local = -1.0;
            string mejor_ruta_local = "";

            for (int i = inicio; i < fin; i++) {
                double score = probe.match(galeria[i].descriptor);
                if (score > max_score_local) {
                    max_score_local = score;
                    mejor_ruta_local = galeria[i].ruta;
                }
            }

            #pragma omp critical
            {
                if (max_score_local > max_score_rep) {
                    max_score_rep = max_score_local;
                    mejor_ruta_rep = mejor_ruta_local;
                }
            }
        }

        auto t_fin = high_resolution_clock::now();
        double duracion_rep = duration_cast<duration<double, milli>>(t_fin - t_ini).count();

        tiempos_ms[rep] = duracion_rep;
        suma_total_ms += duracion_rep;

        if (max_score_rep > max_score_global) {
            max_score_global = max_score_rep;
            mejor_ruta_global = mejor_ruta_rep;
        }
    }

    // ========================================================
    // 5. REPORTE Y MÉTRICAS
    // ========================================================
    double promedio_rep_ms = suma_total_ms / REPETICIONES;
    double tiempo_unitario_ms = promedio_rep_ms / n_total;

    cout << "\n========================================================" << endl;
    cout << "DESGLOSE DE LAS " << REPETICIONES << " REPETICIONES (1:" << n_total << ")" << endl;
    cout << "========================================================" << endl;
    for (int rep = 0; rep < REPETICIONES; rep++) {
        cout << "Pasada #" << setw(2) << (rep + 1) << " : " 
             << fixed << setprecision(2) << setw(7) << tiempos_ms[rep] << " ms" << endl;
    }

    cout << "--------------------------------------------------------" << endl;
    cout << "Tiempo promedio por pasada (1:" << n_total << ") : " 
         << fixed << setprecision(2) << promedio_rep_ms << " ms" << endl;
    cout << "Tiempo promedio por huella (1:1)       : " 
         << fixed << setprecision(4) << tiempo_unitario_ms << " ms" << endl;
    cout << "Tiempo TOTAL acumulado (13 pasadas)   : " 
         << fixed << setprecision(2) << suma_total_ms << " ms (" 
         << setprecision(3) << (suma_total_ms / 1000.0) << " s)" << endl;
    cout << "========================================================" << endl;

    // Validación biométrica
    double umbral = 0.35;
    if (max_score_global >= umbral && !mejor_ruta_global.empty()) {
        cout << "MATCH ENCONTRADO!" << endl;
        cout << "Coincide con : " << mejor_ruta_global << endl;
        cout << "Score maximo : " << fixed << setprecision(4) << max_score_global << endl;
    } else {
        cout << "NO MATCH (Huella no encontrada)" << endl;
        cout << "Mejor score  : " << fixed << setprecision(4) << max_score_global << endl;
    }

    return 0;
}
