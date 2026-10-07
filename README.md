# MCC - Parallel Fingerprint Matching (OpenMP)

Implementación y pruebas de matching paralelo de huellas dactilares basadas en el descriptor Minutia Cylinder-Code (MCC) utilizando OpenMP.

> **Nota:** Este trabajo se basa en el framework paralelo para identificación biométrica desarrollado por D. Peralta et al. (*Pattern Recognition*, 2014).

---

## Contenido de esta rama

Esta rama (`experimental/mcc_prueba_match_paralelo`) contiene las implementaciones experimentales para evaluar el rendimiento y aceleración del matching MCC en entornos multinúcleo con OpenMP:

- `main_omp.cpp`: Algoritmo de matching paralelo contra base de datos de huellas.
- `main_omp_13k.cpp`: Prueba de estrés / benchmark sobre conjunto ampliado (~13k comparaciones).
- `Makefile`: Reglas de compilación para los ejecutables y dependencias comunes (`../commons`).
- `huella10.xyt`: Archivo de muestra en formato `.xyt` para pruebas de entrada.
- `a-LEEME`: Notas internas de ejecución y parámetros experimentales.
