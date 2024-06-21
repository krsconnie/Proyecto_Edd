#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <queue>
#include <bitset>
#include <chrono>

using namespace std;
using namespace std::chrono;

// Representar un nodo del árbol de Huffman
struct Nodo {
    char simbolo;
    int frecuencia;
    Nodo* izquierda;
    Nodo* derecha;
};

// Comparador para la cola de prioridad
struct Comparador {
    bool operator()(Nodo* a, Nodo* b) {
        return a->frecuencia > b->frecuencia;
    }
};

// Construir el árbol de Huffman
Nodo* construirArbol(const string& texto) {
    unordered_map<char, int> frecuencias;
    for (char c : texto) {
        frecuencias[c]++;
    }

    priority_queue<Nodo*, vector<Nodo*>, Comparador> cola;
    for (auto& par : frecuencias) {
        Nodo* nodo = new Nodo();
        nodo->simbolo = par.first;
        nodo->frecuencia = par.second;
        nodo->izquierda = nodo->derecha = nullptr;
        cola.push(nodo);
    }

    while (cola.size() > 1) {
        Nodo* izquierda = cola.top();
        cola.pop();
        Nodo* derecha = cola.top();
        cola.pop();

        Nodo* padre = new Nodo();
        padre->simbolo = '\0';
        padre->frecuencia = izquierda->frecuencia + derecha->frecuencia;
        padre->izquierda = izquierda;
        padre->derecha = derecha;
        cola.push(padre);
    }

    return cola.top();
}

// Generar la tabla de códigos
void generarTablaCodigos(Nodo* nodo, string codigo, unordered_map<char, string>& tabla) {
    if (nodo == nullptr) {
        return;
    }

    if (nodo->simbolo != '\0') {
        tabla[nodo->simbolo] = codigo;
    }

    generarTablaCodigos(nodo->izquierda, codigo + "0", tabla);
    generarTablaCodigos(nodo->derecha, codigo + "1", tabla);
}

// Codificar el texto
string codificar(const string& texto, Nodo* raiz, unordered_map<char, string>& tablaCodigos) {
    generarTablaCodigos(raiz, "", tablaCodigos);

    string codificado;
    for (char c : texto) {
        codificado += tablaCodigos[c];
    }

    return codificado;
}

// Decodificar
string decodificar(const string& codificado, Nodo* raiz) {
    string decodificado;
    Nodo* nodo = raiz;

    for (char c : codificado) {
        if (c == '0') {
            nodo = nodo->izquierda;
        } else {
            nodo = nodo->derecha;
        }

        if (nodo->simbolo != '\0') {
            decodificado += nodo->simbolo;
            nodo = raiz;
        }
    }

    return decodificado;
}

// Obtener el tamaño de un archivo
streampos obtenerTamanoArchivo(const string& nombreArchivo) {
    ifstream archivo(nombreArchivo, ios::binary | ios::ate);
    return archivo.tellg();
}

void escribirBinario(const string& codificado, const string& nombreArchivo) {
    ofstream archivo(nombreArchivo, ios::binary);
    bitset<8> bits;
    int bitIndex = 0;
    
    for (char c : codificado) {
        bits[bitIndex++] = (c == '1');
        if (bitIndex == 8) {
            archivo.put(static_cast<unsigned char>(bits.to_ulong()));
            bitIndex = 0;
            bits.reset();
        }
    }

    if (bitIndex > 0) {
        archivo.put(static_cast<unsigned char>(bits.to_ulong()));
    }

    archivo.close();
}

string leerBinario(const string& nombreArchivo, int bitsTotales) {
    ifstream archivo(nombreArchivo, ios::binary);
    string codificado;
    unsigned char byte;
    
    while (archivo.read(reinterpret_cast<char*>(&byte), 1)) {
        bitset<8> bits(byte);
        for (int i = 0; i < 8 && codificado.size() < bitsTotales; ++i) {
            codificado += (bits[i] ? '1' : '0');
        }
    }

    archivo.close();
    return codificado;
}

int main(int argc, char *argv[]) {

    if (argc != 3) {
        cerr << "Uso: " << argv[0] << " <archivo_entrada> <archivo_salida>" << endl;
        return EXIT_FAILURE;
    }

    string archivoEntrada = argv[1];
    string archivoSalida = argv[2];

    ifstream archivoOriginal(archivoEntrada);
    if (!archivoOriginal.is_open()) {
        cerr << "No se pudo abrir el archivo de texto original: " << archivoEntrada << endl;
        return 1;
    }

    // Leer el archivo de texto original

    auto startCodificar = high_resolution_clock::now();
    string texto((istreambuf_iterator<char>(archivoOriginal)), istreambuf_iterator<char>());
    archivoOriginal.close();

    Nodo* raiz = construirArbol(texto);
    unordered_map<char, string> tablaCodigos;
    string codificado = codificar(texto, raiz, tablaCodigos);

    /*cout << "Tabla de códigos de Huffman:" << endl;
    for (const auto& par : tablaCodigos) {
        cout << par.first << ": " << par.second << endl;
    }
    */
    escribirBinario(codificado, archivoSalida);

    auto endCodificar = high_resolution_clock::now();
    auto duracionCodificar = duration_cast<nanoseconds>(endCodificar - startCodificar);

    // Decodificar el archivo binario
    auto startDecodificar = high_resolution_clock::now();
    string textoCodificado = leerBinario(archivoSalida, codificado.size());
    string decodificado = decodificar(textoCodificado, raiz);
    auto endDecodificar = high_resolution_clock::now();
    auto duracionDecodificar = duration_cast<nanoseconds>(endDecodificar - startDecodificar);

    streampos tamanoOriginal = obtenerTamanoArchivo(archivoEntrada) * (1e-6); // en MB
    streampos tamanoCodificado = obtenerTamanoArchivo(archivoSalida) * (1e-6); // en MB

    double tiempoC = duracionCodificar.count() * 1e-9; // en segundos
    double tiempoD = duracionDecodificar.count() * 1e-9; // en segundos

    cout << "Texto original: " << texto << endl;
    cout << "Tamaño del archivo original: " << tamanoOriginal << " MB" << endl;
    cout << "Tamaño del archivo codificado: " << tamanoCodificado << " MB" << endl;
    cout << "Tiempo para codificar: " << tiempoC << " s" << endl;
    cout << "Tiempo para decodificar: " << tiempoD << " s" << endl;

    return 0;
}
