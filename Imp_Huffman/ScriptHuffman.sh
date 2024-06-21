#!/bin/bash

# Nombre del archivo CSV de salida
output_csv1="resultadosH_DNA.csv"
output_csv2="resultadosH_ENG.csv"

# Archivos de entrada a probar
archivos_entrada1=("dna1" "dna2" "dna3")
archivos_entrada2=("e1" "e2" "e3")

# Compilar el archivo fuente
g++ HUFFMAN3000.cpp -o testH

# Verificar si la compilación fue exitosa
if [[ $? -ne 0 ]]; then
    echo "Error: La compilación falló."
    exit 1
fi

echo "Compilación exitosa. Ejecutable generado: testH"

# Crear el archivo CSV y añadir los encabezados
echo "Acción,Archivo Original,Tamaño Original (MB),Tiempo de Proceso (s),Tamaño Final (MB)" > $output_csv1
echo "Acción,Archivo Original,Tamaño Original (MB),Tiempo de Proceso (s),Tamaño Final (MB)" > $output_csv2

# Función para procesar un archivo
procesar_archivo() {
    archivo=$1
    csv_output=$2
    tmp_output="tmp_output.txt"
    
    # Repetir 20 veces
    for i in {1..20}
    do
        echo "Procesando $archivo (Iteración $i)"
        
        # Ejecutar el programa y redirigir la salida a un archivo temporal
        ./testH "$archivo" "${archivo}_codificado.bin" > $tmp_output

        # Extraer los valores necesarios del archivo temporal
        tamano_original=$(grep "Tamaño del archivo original" $tmp_output | awk '{print $5}')
        tamano_codificado=$(grep "Tamaño del archivo codificado" $tmp_output | awk '{print $5}')
        tiempo_codificacion=$(grep "Tiempo para codificar" $tmp_output | awk '{print $4}')
        tiempo_decodificacion=$(grep "Tiempo para decodificar" $tmp_output | awk '{print $4}')

        # Guardar los resultados en el archivo CSV
        echo "Codificar,$archivo,$tamano_original,$tiempo_codificacion,$tamano_codificado" >> $csv_output
        echo "Decodificar,$archivo,$tamano_codificado,$tiempo_decodificacion,$tamano_original" >> $csv_output

        # Limpiar archivos generados
        rm -f "${archivo}_codificado.bin" "${archivo}_decodificado.txt" $tmp_output
    done
}

# Procesar archivos DNA
for archivo in "${archivos_entrada1[@]}"
do
    procesar_archivo "$archivo" "$output_csv1"
done

# Procesar archivos ENG
for archivo in "${archivos_entrada2[@]}"
do
    procesar_archivo "$archivo" "$output_csv2"
done
