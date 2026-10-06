#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <chrono>
#include <cmath>
#include <numeric>
#include <iomanip>
#include "MCC.h"

using namespace std;
using namespace std::chrono;

// Estructura para almacenar cada huella en la memoria RAM
struct ItemGaleria {
    string ruta;
    MCC descriptor;
};

// Carga de minucias desde archivo de texto
bool cargarMinucias(const string& rutaArchivo, Matrix<int>& M_xyt, int& totalMinucias) {
    ifstream archivo(rutaArchivo.c_str());
    if (!archivo.is_open()) return false;

    vector<int> datos;
    int x, y, t, calidad;

    while (archivo >> x >> y >> t >> calidad) {
        datos.push_back(x);
        datos.push_back(y);
        datos.push_back(t);
    }

    if (datos.empty()) return false;

    totalMinucias = datos.size() / 3;
    M_xyt.resize(totalMinucias, 3);
    for (int i = 0; i < totalMinucias; i++) {
        M_xyt[i][0] = datos[i * 3];
        M_xyt[i][1] = datos[i * 3 + 1];
        M_xyt[i][2] = datos[i * 3 + 2];
    }

    return true;
}

// Estructura para métricas estadísticas
struct Metricas {
    double media;
    double desviacion;
    double min_val;
    double max_val;
};

Metricas calcularMetricas(const vector<double>& datos) {
    double suma = accumulate(datos.begin(), datos.end(), 0.0);
    double media = suma / datos.size();

    double varianza = 0.0;
    double min_v = datos[0];
    double max_v = datos[0];

    for (double v : datos) {
        varianza += (v - media) * (v - media);
        if (v < min_v) min_v = v;
        if (v > max_v) max_v = v;
    }

    double desv = (datos.size() > 1) ? sqrt(varianza / (datos.size() - 1)) : 0.0;
    return {media, desv, min_v, max_v};
}

int main(int argc, char* argv[]) {
    if (argc < 4) {
        cout << "Uso: " << argv[0] << " <etiqueta_caso> <huella_consulta.txt> <lista_descriptores.txt> -N {8|16} -C {LSS|LSSR|LSA|LSAR}" << endl;
        cout << "Ejemplo: " << argv[0] << " \"Media Densidad\" probe_35.txt lista_1000.txt -N 8 -C LSSR" << endl;
        return 1;
    }

    string etiqueta_caso = argv[1];
    string ruta_consulta = argv[2];
    string ruta_lista = argv[3];

    // Ajustar argv para configureAlgorithm saltando la etiqueta personalizada
    MCC::configureAlgorithm(argc, argv);

    // ========================================================
    // 1. CARGA DE CONSULTA (PROBE)
    // ========================================================
    Matrix<int> matrizProbe;
    int numMinuciasProbe = 0;
    if (!cargarMinucias(ruta_consulta, matrizProbe, numMinuciasProbe)) {
        cerr << "Error al cargar archivo de consulta: " << ruta_consulta << endl;
        return 1;
    }

    MCC probe(matrizProbe);
    probe.initialize();

    ifstream lista(ruta_lista.c_str());
    if (!lista.is_open()) {
        cerr << "Error al abrir la lista de descriptores: " << ruta_lista << endl;
        return 1;
    }

    int total_huellas;
    if (!(lista >> total_huellas) || total_huellas <= 0) {
        cerr << "Cantidad de huellas invalida en la lista." << endl;
        return 1;
    }

    // ========================================================
    // 2. CARGA A RAM (I/O DE DISCO)
    // ========================================================
    auto inicio_io = high_resolution_clock::now();

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

    auto fin_io = high_resolution_clock::now();
    double tiempo_io_ms = duration_cast<duration<double, milli>>(fin_io - inicio_io).count();

    // ========================================================
    // 3. FASE DE CALENTAMIENTO (WARM-UP)
    // ========================================================
    // Una corrida previa no cronometrada para calentar cachés L1/L2
    for (size_t i = 0; i < galeria.size(); i++) {
        volatile double dummy = probe.match(galeria[i].descriptor);
        (void)dummy;
    }

    // ========================================================
    // 4. BENCHMARK: 10 REPETICIONES DE MATCHING 1:N EN RAM
    // ========================================================
    const int NUM_REPETICIONES = 10;
    vector<double> tiempos_runs_ms(NUM_REPETICIONES);
    double mejor_score_global = -1.0;
    string mejor_ruta_global = "";

    for (int rep = 0; rep < NUM_REPETICIONES; rep++) {
        auto t_ini = high_resolution_clock::now();

        double max_score_local = -1.0;
        string mejor_ruta_local = "";

        for (size_t i = 0; i < galeria.size(); i++) {
            double score = probe.match(galeria[i].descriptor);
            if (score > max_score_local) {
                max_score_local = score;
                mejor_ruta_local = galeria[i].ruta;
            }
        }

        auto t_fin = high_resolution_clock::now();
        tiempos_runs_ms[rep] = duration_cast<duration<double, milli>>(t_fin - t_ini).count();

        mejor_score_global = max_score_local;
        mejor_ruta_global = mejor_ruta_local;
    }

    // ========================================================
    // 5. CÁLCULO ESTADÍSTICO Y FORMATO DE TABLA
    // ========================================================
    Metricas stats = calcularMetricas(tiempos_runs_ms);
    double unitario_ms = stats.media / galeria.size();

    cout << "\n========================================================" << endl;
    cout << "RESUMEN DE EJECUCION (10 CORRIDAS)" << endl;
    cout << "========================================================" << endl;
    cout << "Caso                 : " << etiqueta_caso << endl;
    cout << "Minucias Probe       : " << numMinuciasProbe << endl;
    cout << "Huellas Galeria (N)  : " << galeria.size() << endl;
    cout << "Tiempo I/O Disco     : " << fixed << setprecision(2) << tiempo_io_ms << " ms" << endl;
    cout << "Score maximo         : " << fixed << setprecision(4) << mejor_score_global 
         << " (" << mejor_ruta_global << ")" << endl;
    cout << "Matching 1:N Promedio: " << fixed << setprecision(2) << stats.media 
         << " ± " << stats.desviacion << " ms" << endl;
    cout << "Matching Min / Max   : " << stats.min_val << " ms / " << stats.max_val << " ms" << endl;
    cout << "Tiempo Unitario      : " << fixed << setprecision(3) << unitario_ms << " ms/huella" << endl;
    cout << "========================================================" << endl;

    // Fila formateada lista para Markdown
    cout << "\nCOPIAR A TU TABLA MARKDOWN:" << endl;
    cout << "| **" << etiqueta_caso << "** | " 
         << numMinuciasProbe << " | " 
         << galeria.size() << " | " 
         << NUM_REPETICIONES << " | " 
         << fixed << setprecision(1) << tiempo_io_ms << " ms | " 
         << fixed << setprecision(1) << stats.media << " ± " << stats.desviacion << " ms | " 
         << fixed << setprecision(1) << stats.min_val << " | " 
         << fixed << setprecision(1) << stats.max_val << " | " 
         << fixed << setprecision(3) << unitario_ms << " ms |" << endl;

    return 0;
}