#!/bin/bash

# Comprobar argumentos de entrada
DIRECTORIO="${1:-.}"
SALIDA="${2:-lista_huellas_xyt.txt}"

if [ ! -d "$DIRECTORIO" ]; then
    echo "Error: El directorio '$DIRECTORIO' no existe."
    exit 1
fi

echo "Buscando archivos .xyt en: $DIRECTORIO"

# Obtener rutas absolutas ordenadas alfabéticamente
find "$(cd "$DIRECTORIO" && pwd)" -maxdepth 1 -type f -name "*.xyt" | sort > "$SALIDA"

TOTAL=$(wc -l < "$SALIDA")

if [ "$TOTAL" -eq 0 ]; then
    echo "Aviso: No se encontraron archivos .xyt en '$DIRECTORIO'."
    rm -f "$SALIDA"
    exit 1
fi

echo "Se generó exitosamente '$SALIDA' con $TOTAL huellas registradas."

#Permisos para ejecutar 
#chmod +x generar_lista.sh

#./encontrar_extremos lista_huellas_xyt.txt
## Sintaxis: ./generar_lista.sh <carpeta_con_archivos_xyt> [nombre_salida.txt]

#sin poner directorio
#./generar_lista.sh ./huellasxyt lista_xyt.txt