#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <queue>
#include <bitset>

using namespace std;

// Estructura para representar un nodo del árbol de Huffman
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

// Función para construir el árbol de Huffman
Nodo* construirArbol(const string& texto) {
    map<char, int> frecuencias;
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

// Función para generar la tabla de códigos
void generarTablaCodigos(Nodo* nodo, string codigo, map<char, string>& tabla) {
    if (nodo == nullptr) {
        return;
    }

    if (nodo->simbolo != '\0') {
        tabla[nodo->simbolo] = codigo;
    }

    generarTablaCodigos(nodo->izquierda, codigo + "0", tabla);
    generarTablaCodigos(nodo->derecha, codigo + "1", tabla);
}

// Función para codificar un texto usando el árbol de Huffman
string codificar(const string& texto, Nodo* raiz) {
    map<char, string> tablaCodigos;
    generarTablaCodigos(raiz, "", tablaCodigos);

    string codificado;
    for (char c : texto) {
        codificado += tablaCodigos[c];
    }

    return codificado;
}

// Función para decodificar una secuencia de códigos
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

// Función para obtener el tamaño de un archivo
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
    // Leer el archivo de texto original
    ifstream archivoOriginal("PRUEBA.txt.txt");
    if (!archivoOriginal.is_open()) {
        cerr << "No se pudo abrir el archivo de texto original." << endl;
        return 1;
    }

    string texto((istreambuf_iterator<char>(archivoOriginal)), istreambuf_iterator<char>());
    archivoOriginal.close();

    // Construir el árbol de Huffman y codificar el texto
    Nodo* raiz = construirArbol(texto);
    string codificado = codificar(texto, raiz);

    // Crear el archivo binario con el texto codificado
    escribirBinario(codificado, "texto_codificado.bin");

    // Obtener el tamaño de los archivos
    streampos tamanoOriginal = obtenerTamanoArchivo("PRUEBA.txt.txt");
    streampos tamanoCodificado = obtenerTamanoArchivo("texto_codificado.bin");

    // Leer el archivo binario codificado y decodificar el contenido
    string textoCodificado = leerBinario("texto_codificado.bin", codificado.size());

    string decodificado = decodificar(textoCodificado, raiz);

    // Mostrar resultados
    cout << "Texto original: " << texto << endl;
    cout << "Codificado: ";
    for (char c : codificado) {
        cout << bitset<1>(c - '0');
    }
    cout << endl;
    cout << "Decodificado: " << decodificado << endl;
    cout << "Tamaño del archivo original: " << tamanoOriginal << " bytes" << endl;
    cout << "Tamaño del archivo codificado: " << tamanoCodificado << " bytes" << endl;

    return 0;
}
