///
/// @mainpage LZW
/// Este archivo ha sido modificado por propósitos académicos.
/// Las siguientes modificaciones han sido realizadas sobre el código original:
/// - Renombrado de variables para mayor claridad.
/// - Añadido de comentarios en español.
/// - Se agrega la librería Chrono.
/// - Se mide el tiempo de compresión y descompresión.
/// - Se mide el tamaño antes y pos procesar archivos.
/// 
/// Todo esto respetando las restricciones de la Licencia MIT/Expat.
/// Extraído de la página linkeada a continuación.
/// @see https://cplusplus.com/articles/iL18T05o/ 
///
/// 
/// @date Junio del 2024
///
/// @file
/// @author Julius Pettersson
/// @copyright MIT/Expat License.
/// @brief LZW file compressor
/// @version 3
///
/// This is the C++11 implementation of a Lempel-Ziv-Welch single-file command-line compressor.
/// It uses the simpler fixed-width code compression method.
/// It was written with Doxygen comments.
///
/// @see http://en.wikipedia.org/wiki/Lempel%E2%80%93Ziv%E2%80%93Welch
/// @see http://marknelson.us/2011/11/08/lzw-revisited/
/// @see http://www.cs.duke.edu/csed/curious/compression/lzw.html
/// @see http://warp.povusers.org/EfficientLZW/index.html
/// @see http://en.cppreference.com/
/// @see http://www.doxygen.org/
///

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <exception>
#include <fstream>
#include <ios>
#include <iostream>
#include <istream>
#include <limits>
#include <map>
#include <memory>
#include <ostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>
#include <chrono>
#include <filesystem> // Necesario para medir el tamaño del archivo

// Tipo utilizado para almacenar y recuperar códigos.
using TipoCodigo = std::uint16_t;

namespace globales {

    // Tamaño máximo del diccionario (cuando se alcanza, el diccionario se reinicia).
    const TipoCodigo tamanoMaximoDiccionario {std::numeric_limits<TipoCodigo>::max()};

} // namespace globales

///
/// @brief Comprime el contenido de `entrada` y escribe el resultado en `salida`.
/// @param [in] entrada  flujo de entrada
/// @param [out] salida  flujo de salida
///
void comprimir(std::istream &entrada, std::ostream &salida)
{
    // Diccionario para almacenar las secuencias y sus códigos.
    std::map<std::pair<TipoCodigo, char>, TipoCodigo> diccionario;

    // Función lambda para reiniciar el diccionario a su contenido inicial.
    const auto reiniciar_diccionario = [&diccionario] {
        diccionario.clear();

        const long int minimo = std::numeric_limits<char>::min();
        const long int maximo = std::numeric_limits<char>::max();

        for (long int c = minimo; c <= maximo; ++c)
        {
            // Para evitar el comportamiento indefinido al leer y modificar el diccionario al mismo tiempo.
            const TipoCodigo tamanoDiccionario = diccionario.size();
            diccionario[{globales::tamanoMaximoDiccionario, static_cast<char>(c)}] = tamanoDiccionario;
        }
    };

    reiniciar_diccionario();

    TipoCodigo indice {globales::tamanoMaximoDiccionario}; // Índice
    char simbolo;

    while (entrada.get(simbolo))
    {
        // Si se alcanzó el tamaño máximo del diccionario.
        if (diccionario.size() == globales::tamanoMaximoDiccionario)
            reiniciar_diccionario();

        if (diccionario.count({indice, simbolo}) == 0)
        {
            // Para evitar el comportamiento indefinido al leer y modificar el diccionario al mismo tiempo.
            const TipoCodigo tamanoDiccionario = diccionario.size();
            diccionario[{indice, simbolo}] = tamanoDiccionario;

            // Escribir el código de la secuencia actual en la salida.
            salida.write(reinterpret_cast<const char *>(&indice), sizeof(TipoCodigo));

            // Actualizar el índice al valor del símbolo actual en el diccionario.
            indice = diccionario.at({globales::tamanoMaximoDiccionario, simbolo});
        }
        else
        {
            // Actualizar el índice para incluir el símbolo actual.
            indice = diccionario.at({indice, simbolo});
        }
    }

    // Escribir el código de la última secuencia en la salida.
    if (indice != globales::tamanoMaximoDiccionario)
        salida.write(reinterpret_cast<const char *>(&indice), sizeof(TipoCodigo));
}

///
/// @brief Descomprime el contenido de `entrada` y escribe el resultado en `salida`.
/// @param [in] entrada  flujo de entrada
/// @param [out] salida  flujo de salida
///
void descomprimir(std::istream &entrada, std::ostream &salida)
{
    // Diccionario para almacenar las secuencias de códigos y caracteres.
    std::vector<std::pair<TipoCodigo, char>> diccionario;

    // Función lambda para reiniciar el diccionario a su contenido inicial.
    const auto reiniciar_diccionario = [&diccionario] {
        diccionario.clear();
        diccionario.reserve(globales::tamanoMaximoDiccionario);

        const long int minimo = std::numeric_limits<char>::min();
        const long int maximo = std::numeric_limits<char>::max();

        for (long int c = minimo; c <= maximo; ++c)
            diccionario.push_back({globales::tamanoMaximoDiccionario, static_cast<char>(c)});
    };

    // Función lambda para reconstruir una secuencia de caracteres a partir de un código.
    const auto reconstruir_secuencia = [&diccionario](TipoCodigo k) -> std::vector<char> {
        std::vector<char> secuencia; // Secuencia de caracteres

        while (k != globales::tamanoMaximoDiccionario)
        {
            secuencia.push_back(diccionario.at(k).second);
            k = diccionario.at(k).first;
        }

        std::reverse(secuencia.begin(), secuencia.end());
        return secuencia;
    };

    reiniciar_diccionario();

    TipoCodigo indice {globales::tamanoMaximoDiccionario}; // Índice
    TipoCodigo clave; // Clave

    while (entrada.read(reinterpret_cast<char *>(&clave), sizeof(TipoCodigo)))
    {
        // Si se alcanzó el tamaño máximo del diccionario.
        if (diccionario.size() == globales::tamanoMaximoDiccionario)
            reiniciar_diccionario();

        if (clave > diccionario.size())
            throw std::runtime_error("código de compresión inválido");

        std::vector<char> secuencia; // Secuencia de caracteres

        if (clave == diccionario.size())
        {
            // Agregar una nueva entrada al diccionario.
            diccionario.push_back({indice, reconstruir_secuencia(indice).front()});
            secuencia = reconstruir_secuencia(clave);
        }
        else
        {
            secuencia = reconstruir_secuencia(clave);

            if (indice != globales::tamanoMaximoDiccionario)
                diccionario.push_back({indice, secuencia.front()});
        }

        // Escribir la secuencia descomprimida en la salida.
        salida.write(&secuencia.front(), secuencia.size());
        indice = clave;
    }

    if (!entrada.eof() || entrada.gcount() != 0)
        throw std::runtime_error("archivo comprimido corrupto");
}

///
/// @brief Calcula el tamaño de un archivo.
/// @param nombreArchivo Ruta al archivo cuyo tamaño se desea medir.
/// @return Tamaño del archivo en bytes.
///
std::uintmax_t obtenerTamanoArchivo(const std::string& nombreArchivo) {
    std::filesystem::path ruta(nombreArchivo);
    return std::filesystem::file_size(ruta);
}

///
/// @brief Punto de entrada principal del programa.
/// @param argc             número de argumentos de línea de comandos
/// @param [in] argv        array de argumentos de línea de comandos
/// @retval EXIT_FAILURE    operación fallida
/// @retval EXIT_SUCCESS    operación exitosa
///
int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        std::cerr << "Uso: " << argv[0] << " [-c|-d] archivo_entrada archivo_salida\n";
        return EXIT_FAILURE;
    }

    enum class Modo {
        Comprimir,
        Descomprimir
    };

    Modo modo;
    if (std::string_view(argv[1]) == "-c")
        modo = Modo::Comprimir;
    else if (std::string_view(argv[1]) == "-d")
        modo = Modo::Descomprimir;
    else
    {
        std::cerr << "Error: bandera no reconocida. Use '-c' para comprimir o '-d' para descomprimir.\n";
        return EXIT_FAILURE;
    }

    const std::size_t tamanoBuffer {1024 * 1024};

    try
    {
        std::ifstream archivoEntrada;
        std::ofstream archivoSalida;

        // Abrir archivo de entrada y salida
        archivoEntrada.rdbuf()->pubsetbuf(new char[tamanoBuffer], tamanoBuffer);
        archivoEntrada.open(argv[2], std::ios::binary);
        archivoSalida.rdbuf()->pubsetbuf(new char[tamanoBuffer], tamanoBuffer);
        archivoSalida.open(argv[3], std::ios::binary);

        if (!archivoEntrada.is_open()) {
            std::cerr << "No se pudo abrir el archivo de entrada.\n";
            return EXIT_FAILURE;
        }
        if (!archivoSalida.is_open()) {
            std::cerr << "No se pudo abrir el archivo de salida.\n";
            return EXIT_FAILURE;
        }

        // Medir el tamaño del archivo de entrada
        std::uintmax_t tamanoEntrada = obtenerTamanoArchivo(argv[2]);

        double tiempo = 0;
        if (modo == Modo::Comprimir)
        {
            auto start = std::chrono::high_resolution_clock::now();

            comprimir(archivoEntrada, archivoSalida);

            auto end = std::chrono::high_resolution_clock::now();
            tiempo = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
        }
        else if (modo == Modo::Descomprimir)
        {
            auto start = std::chrono::high_resolution_clock::now();

            descomprimir(archivoEntrada, archivoSalida);

            auto end = std::chrono::high_resolution_clock::now();
            tiempo = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
        }

        // Medir el tamaño del archivo de salida
        std::uintmax_t tamanoSalida = obtenerTamanoArchivo(argv[3]);
        tiempo*=(1e-9);

        tamanoSalida *=(1e-6); 
        tamanoEntrada*=(1e-6);

        // Imprimir el tamaño de los archivos y el tiempo de procesamiento
        std::cout << "Tamaño del archivo de entrada: " << tamanoEntrada << " MB\n";
        std::cout << "Tamaño del archivo de salida: " << tamanoSalida << " MB\n";
        std::cout << "Tiempo de procesamiento: " << tiempo << " segundos\n";
    }
    catch (const std::ios_base::failure &fallo)
    {
        std::cerr << "Fallo de entrada/salida de archivos: " << fallo.what() << '\n';
        return EXIT_FAILURE;
    }
    catch (const std::exception &excepcion)
    {
        std::cerr << "Excepción capturada: " << excepcion.what() << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
