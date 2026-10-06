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

struct ItemGaleria {
    string ruta;
    MCC descriptor;
};

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
    if (argc < 3) {
        cout << "Uso: " << argv[0] << " <huella_consulta.xyt> <lista_descriptores.txt> -N {8|16} -C {LSS|LSSR|LSA|LSAR}" << endl;
        return 1;
    }

    string ruta_consulta = argv[1];
    string ruta_lista = argv[2];

    // Extraer nombre del archivo como etiqueta
    string etiqueta_caso = ruta_consulta;
    size_t barra = etiqueta_caso.find_last_of("/\\");
    if (barra != string::npos) etiqueta_caso = etiqueta_caso.substr(barra + 1);

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
    // 2. CARGA A RAM (I/O DE DISCO EN FRÍO)
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
    for (size_t i = 0; i < galeria.size(); i++) {
        volatile double dummy = probe.match(galeria[i].descriptor);
        (void)dummy;
    }

    // ========================================================
    // 4. BENCHMARK: 10 REPETICIONES DE MATCHING 1:N
    // ========================================================
    const int NUM_REPETICIONES = 10;
    vector<double> tiempos_runs_ms(NUM_REPETICIONES);
    vector<double> scores_runs(NUM_REPETICIONES);
    vector<string> matches_runs(NUM_REPETICIONES);

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
        scores_runs[rep] = max_score_local;
        matches_runs[rep] = mejor_ruta_local;
    }

    // ========================================================
    // 5. CÁLCULO ESTADÍSTICO Y REPORTES (DESPUÉS DEL CRONÓMETRO)
    // ========================================================
    Metricas stats = calcularMetricas(tiempos_runs_ms);
    double unitario_ms = stats.media / galeria.size();

    cout << "\n========================================================" << endl;
    cout << "DESGLOSE DE LAS 10 REPETICIONES (1:N en memoria RAM)" << endl;
    cout << "========================================================" << endl;
    cout << left << setw(8) << "Run #" 
         << setw(18) << "Tiempo 1:N (ms)" 
         << setw(20) << "Unitario (ms/huella)" 
         << setw(14) << "Max Score" 
         << "Coincidencia" << endl;
    cout << "--------------------------------------------------------" << endl;

    for (int rep = 0; rep < NUM_REPETICIONES; rep++) {
        double unitario_rep = tiempos_runs_ms[rep] / galeria.size();
        cout << left << setw(8) << (rep + 1)
             << setw(18) << fixed << setprecision(2) << tiempos_runs_ms[rep]
             << setw(20) << fixed << setprecision(3) << unitario_rep
             << setw(14) << fixed << setprecision(4) << scores_runs[rep]
             << matches_runs[rep] << endl;
    }

    cout << "========================================================" << endl;
    cout << "RESUMEN GLOBAL" << endl;
    cout << "========================================================" << endl;
    cout << "Huella Entrada       : " << etiqueta_caso << " (" << numMinuciasProbe << " minucias)" << endl;
    cout << "Galeria evaluada     : " << galeria.size() << " descriptores" << endl;
    cout << "Tiempo I/O Disco     : " << fixed << setprecision(2) << tiempo_io_ms << " ms" << endl;
    cout << "Matching Promedio    : " << fixed << setprecision(2) << stats.media << " ± " << stats.desviacion << " ms" << endl;
    cout << "Matching Min / Max   : " << stats.min_val << " ms / " << stats.max_val << " ms" << endl;
    cout << "Throughput Promedio  : " << fixed << setprecision(1) << (1000.0 / unitario_ms) << " matches/segundo" << endl;
    cout << "========================================================" << endl;

    // Fila Markdown
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