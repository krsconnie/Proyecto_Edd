#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <queue>
#include <bitset>
#include <chrono>

using namespace std;
using namespace std::chrono;

//representar un nodo del árbol de Huffman
struct Nodo {
    char simbolo;
    int frecuencia;
    Nodo* izquierda;
    Nodo* derecha;
};

// comparador para la cola de prioridad
struct Comparador {
    bool operator()(Nodo* a, Nodo* b) {
        return a->frecuencia > b->frecuencia;
    }
};

//construir el árbol de Huffman
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

// generar la tabla de códigos
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

//codificar el texto 
string codificar(const string& texto, Nodo* raiz, unordered_map<char, string>& tablaCodigos) {
    generarTablaCodigos(raiz, "", tablaCodigos);

    string codificado;
    for (char c : texto) {
        codificado += tablaCodigos[c];
    }

    return codificado;
}

// decodificar 
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

//obtener el tamaño de un archivo #include <fstream>
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

int main() {
    // leer el archivo de texto original
    //se mide el tiempo desde que lee el archivo
    auto startCodificar = high_resolution_clock::now();
    ifstream archivoOriginal("PRUEBA.txt");
    if (!archivoOriginal.is_open()) {
        cerr << "No se pudo abrir el archivo de texto original." << endl;
        return 1;
    }

    string texto((istreambuf_iterator<char>(archivoOriginal)), istreambuf_iterator<char>());
    archivoOriginal.close();

    
    
    Nodo* raiz = construirArbol(texto);
    unordered_map<char, string> tablaCodigos;
    string codificado = codificar(texto, raiz, tablaCodigos);

    //hasta que termina de construir el arbol
    auto endCodificar = high_resolution_clock::now();
    auto duracionCodificar = duration_cast<duration<double>>(endCodificar - startCodificar);
    
    
    cout << "Tabla de códigos de Huffman:" << endl;
    for (const auto& par : tablaCodigos) {
        cout << par.first << ": " << par.second << endl;
    }

    
    escribirBinario(codificado, "texto_codificado.bin");


   //se mide el tiempo desde que recibe el texto codificado
   auto startDecodificar = high_resolution_clock::now();
    string textoCodificado = leerBinario("texto_codificado.bin", codificado.size());
    string decodificado = decodificar(textoCodificado, raiz);
     auto endDecodificar = high_resolution_clock::now();
     auto duracionDecodificar = duration_cast<duration<double>>(endDecodificar - startDecodificar);
  //se mide el tiempo hasta que lee el archivo binario y lo decodifica

    streampos tamanoOriginal = obtenerTamanoArchivo("PRUEBA.txt");
    streampos tamanoCodificado = obtenerTamanoArchivo("texto_codificado.bin");

    
    cout << "Texto original: " << texto << endl;
    cout << "Codificado: " << codificado <<endl;
    cout << "Decodificado: " << decodificado << endl;
    cout << "Tamaño del archivo original: " <<tamanoOriginal << " bytes" << endl;
    cout << "Tamaño del archivo codificado: " << tamanoCodificado<< "bytes" << endl;
    cout << "Tiempo para codificar: "<<duracionCodificar.count() << "s" << endl;
    cout << "Tiempo para Decodificar: "<< duracionDecodificar.count() << " s" << endl;

    return 0;
}
