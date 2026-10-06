#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>

using namespace std;

struct InfoHuella {
    string ruta;
    int totalMinucias;
};

// Cuenta minucias asegurando que la línea tenga las 4 columnas (x, y, theta, calidad)
int contarMinucias(const string& ruta) {
    ifstream archivo(ruta.c_str());
    if (!archivo.is_open()) return -1;

    int x, y, t, calidad;
    int contador = 0;
    while (archivo >> x >> y >> t >> calidad) {
        contador++;
    }
    return contador;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cout << "Uso: " << argv[0] << " <lista_archivos_xyt.txt>" << endl;
        return 1;
    }

    string rutaLista = argv[1];
    ifstream lista(rutaLista.c_str());
    if (!lista.is_open()) {
        cerr << "Error al abrir la lista: " << rutaLista << endl;
        return 1;
    }

    vector<InfoHuella> huellas;
    string rutaArchivo;
    long long sumaMinucias = 0;

    cout << "Leyendo y analizando archivos de minucias..." << endl;
    while (lista >> rutaArchivo) {
        int nMin = contarMinucias(rutaArchivo);
        if (nMin > 0) {
            huellas.push_back({rutaArchivo, nMin});
            sumaMinucias += nMin;
        } else if (nMin == 0) {
            cerr << "Aviso: Archivo sin minucias validas: " << rutaArchivo << endl;
        } else {
            cerr << "Aviso: No se pudo abrir: " << rutaArchivo << endl;
        }
    }
    lista.close();

    if (huellas.empty()) {
        cerr << "Error: No se encontraron huellas validas." << endl;
        return 1;
    }

    // Ordenar de menor a mayor cantidad de minucias
    sort(huellas.begin(), huellas.end(), [](const InfoHuella& a, const InfoHuella& b) {
        return a.totalMinucias < b.totalMinucias;
    });

    const InfoHuella& minimo = huellas.front();
    const InfoHuella& maximo = huellas.back();

    // 1. Punto medio aritmético entre los extremos (Max + Min) / 2
    double centroRango = (minimo.totalMinucias + maximo.totalMinucias) / 2.0;

    // 2. Mediana por posición de la lista ordenada
    const InfoHuella& medianaPosicion = huellas[huellas.size() / 2];

    // 3. Buscar el archivo con la cantidad de minucias más cercana a (Max + Min) / 2
    size_t indiceMasCercanoAlCentro = 0;
    double menorDiferencia = abs(huellas[0].totalMinucias - centroRango);

    for (size_t i = 1; i < huellas.size(); i++) {
        double dif = abs(huellas[i].totalMinucias - centroRango);
        if (dif < menorDiferencia) {
            menorDiferencia = dif;
            indiceMasCercanoAlCentro = i;
        }
    }
    const InfoHuella& medioAritmetico = huellas[indiceMasCercanoAlCentro];

    double promedioPoblacion = (double)sumaMinucias / huellas.size();

    // ========================================================
    // RESULTADOS
    // ========================================================
    cout << "\n========================================================" << endl;
    cout << "ANALISIS DE MINUCIAS (" << huellas.size() << " huellas analizadas)" << endl;
    cout << "========================================================" << endl;
    cout << "Promedio general de la base : " << promedioPoblacion << " minucias" << endl;
    cout << "Punto medio teorico (Min+Max)/2: " << centroRango << " minucias" << endl;
    cout << "--------------------------------------------------------" << endl;
    cout << "1. CASO MINIMO:" << endl;
    cout << "   Minucias : " << minimo.totalMinucias << endl;
    cout << "   Archivo  : " << minimo.ruta << endl;
    cout << "--------------------------------------------------------" << endl;
    cout << "2. CASO MEDIO (Mas cercano a Min+Max/2):" << endl;
    cout << "   Minucias : " << medioAritmetico.totalMinucias << endl;
    cout << "   Archivo  : " << medioAritmetico.ruta << endl;
    cout << "--------------------------------------------------------" << endl;
    cout << "   CASO MEDIANA (Posicion central de la lista):" << endl;
    cout << "   Minucias : " << medianaPosicion.totalMinucias << endl;
    cout << "   Archivo  : " << medianaPosicion.ruta << endl;
    cout << "--------------------------------------------------------" << endl;
    cout << "3. CASO MAXIMO:" << endl;
    cout << "   Minucias : " << maximo.totalMinucias << endl;
    cout << "   Archivo  : " << maximo.ruta << endl;
    cout << "========================================================" << endl;

    return 0;
}