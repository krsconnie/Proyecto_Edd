#!/bin/bash

# Nombre del archivo CSV de salida
output_csv1="resultadosLZW_DNA.csv"
output_csv2="resultadosLZW_ENG.csv"

# Archivos de entrada a probar
archivos_entrada1=("dna1" "dna2" "dna3")
archivos_entrada2=("e1" "e2" "e3")

# Compilar el archivo fuente
g++ Lzw.cpp -o testLZW

# Verificar si la compilación fue exitosa
if [[ $? -ne 0 ]]; then
    echo "Error: La compilación falló."
    exit 1
fi

echo "Compilación exitosa. Ejecutable generado: test"

# Crear el archivo CSV y añadir los encabezados
echo "Acción,Archivo Original,Tamaño Original (bytes),Tiempo de Proceso (ns),Tamaño Final (bytes)" > $output_csv1
echo "Acción,Archivo Original,Tamaño Original (bytes),Tiempo de Proceso (ns),Tamaño Final (bytes)" > $output_csv2

# Función para ejecutar las pruebas
run_test() {
    local executable=$1
    local archivos_entrada=("${!2}")  
    local output_csv=$3

    # Loop para cada archivo de entrada
    for archivo in "${archivos_entrada[@]}"; do
        # Comprobar si el archivo de entrada existe
        if [[ ! -f $archivo ]]; then
            echo "El archivo $archivo no existe."
            continue
        fi

        # Ejecutar compresión y descompresión varias veces
        for ((i=1; i<=20; i++)); do
            # Nombre temporal para el archivo comprimido y descomprimido
            archivo_comprimido="temp_comprimido_$i.lzw"
            archivo_descomprimido="temp_descomprimido_$i.txt"

            # Ejecutar compresión y capturar la salida
            output_comprimir=$($executable -c "$archivo" "$archivo_comprimido")

            # Extraer los valores de la salida de compresión
            accion="Comprimir"
            tamanoEntrada=$(echo "$output_comprimir" | grep "Tamaño del archivo de entrada:" | awk '{print $3}')
            tiempo_proceso=$(echo "$output_comprimir" | grep "Tiempo de procesamiento:" | awk '{print $4}')
            tamanoSalida=$(echo "$output_comprimir" | grep "Tamaño del archivo de salida:" | awk '{print $3}')

            # Guardar resultados de compresión en CSV
            echo "$accion,$archivo,$tamanoEntrada,$tiempo_proceso,$tamanoSalida" >> $output_csv

            # Ejecutar descompresión y capturar la salida
            output_descomprimir=$($executable -d "$archivo_comprimido" "$archivo_descomprimido")

            # Extraer los valores de la salida de descompresión
            accion="Descomprimir"
            tamanoEntrada=$(echo "$output_descomprimir" | grep "Tamaño del archivo de entrada:" | awk '{print $3}')
            tiempo_proceso=$(echo "$output_descomprimir" | grep "Tiempo de procesamiento:" | awk '{print $4}')
            tamanoSalida=$(echo "$output_descomprimir" | grep "Tamaño del archivo de salida:" | awk '{print $3}')

            # Guardar resultados de descompresión en CSV
            echo "$accion,$archivo_comprimido,$tamanoEntrada,$tiempo_proceso,$tamanoSalida" >> $output_csv

            # Eliminar los archivos temporales
            rm -f "$archivo_comprimido" "$archivo_descomprimido"
        done
    done
}

# Ejecutar las pruebas para cada conjunto de archivos
run_test "./test" archivos_entrada1[@] $output_csv1
run_test "./test" archivos_entrada2[@] $output_csv2

echo "Pruebas completadas. Los resultados están guardados en $output_csv1 y $output_csv2."
