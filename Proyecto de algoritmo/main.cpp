#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <unordered_map>
#include <map>
#include <set>
#include <queue>
#include <algorithm>
#include <climits>
#include <chrono>  // agregado para medir tiempo
#include "ArbolBinarioAVL.h"
#include "NodoArbolAVL.h"

using namespace std;

struct Venta {
    int id;
    string fecha;
    string pais;
    string ciudad;
    string cliente;
    string producto;
    string categoria;
    int cantidad;
    float precio_unitario;
    float monto_total;
    string medio_envio;
    string estado_envio;
};

vector<Venta> ventas;

vector<string> dividirCSVConComillas(const string& linea) {
    vector<string> resultado;
    bool entreComillas = false;
    string campo;

    for (size_t i = 0; i < linea.size(); ++i) {
        char c = linea[i];
        if (c == '"') {
            entreComillas = !entreComillas;
        } else if (c == ',' && !entreComillas) {
            resultado.push_back(campo);
            campo.clear();
        } else {
            campo += c;
        }
    }
    resultado.push_back(campo);
    return resultado;
}

void procesarEstadisticas() {
    cout << "\n=== Procesando estadísticas ===\n";

    int contadorIf = 0; // contador de condicionales if
    auto start = chrono::high_resolution_clock::now(); // inicio tiempo

    map<string, unordered_map<string, float>> ventasPorCiudadPais;
    map<string, unordered_map<string, float>> ventasProductoPais;
    map<string, unordered_map<string, pair<float, int>>> montoYCantPorCategoriaPais;
    map<string, unordered_map<string, int>> medioEnvioPorPais;
    map<string, unordered_map<string, int>> medioEnvioPorCategoria;
    map<string, unordered_map<string, int>> estadoEnvioPorPais;
    unordered_map<string, int> cantidadPorProducto;

    unordered_map<string, float> montoPorFecha;
    ArbolBinarioAVL<float> arbolFechas;

    for (auto& v : ventas) {
        ventasPorCiudadPais[v.pais][v.ciudad] += v.monto_total;
        ventasProductoPais[v.pais][v.producto] += v.monto_total;
        montoYCantPorCategoriaPais[v.pais][v.categoria].first += v.monto_total;
        montoYCantPorCategoriaPais[v.pais][v.categoria].second++;
        medioEnvioPorPais[v.pais][v.medio_envio]++;
        medioEnvioPorCategoria[v.categoria][v.medio_envio]++;
        montoPorFecha[v.fecha] += v.monto_total;
        estadoEnvioPorPais[v.pais][v.estado_envio]++;
        cantidadPorProducto[v.producto] += v.cantidad;
    }

    for (auto& par : montoPorFecha) {
        try {
            arbolFechas.put(par.second);
        } catch (...) {}
    }

    for (auto& paisData : ventasPorCiudadPais) {
        string pais = paisData.first;
        auto& ciudades = paisData.second;

        priority_queue<pair<float, string>> top;
        for (auto& ciudadData : ciudades) {
            top.push({ciudadData.second, ciudadData.first});
        }

        cout << "\nTop 5 ciudades con mayor monto en " << pais << ":\n";
        for (int i = 0; i < 5 && !top.empty(); ++i) {
            auto par = top.top(); top.pop();
            cout << i + 1 << ". " << par.second << ": $" << par.first << endl;
        }
    }

    string diaMax;
    float montoMax = -1;
    for (auto& par : montoPorFecha) {
        contadorIf++;
        if (par.second > montoMax) {
            montoMax = par.second;
            diaMax = par.first;
        }
    }
    cout << "\nDia con mayor monto de ventas: " << diaMax << " ($" << montoMax << ")" << endl;

    string masVendido, menosVendido;
    int maxCant = -1, minCant = INT_MAX;

    for (auto& par : cantidadPorProducto) {
        contadorIf++;
        if (par.second > maxCant) {
            masVendido = par.first;
            maxCant = par.second;
        }
        contadorIf++;
        if (par.second < minCant) {
            menosVendido = par.first;
            minCant = par.second;
        }
    }

    cout << "\nProducto mas vendido (en unidades): " << masVendido << " (" << maxCant << " unidades)\n";
    cout << "Producto menos vendido (en unidades): " << menosVendido << " (" << minCant << " unidades)\n";

    auto end = chrono::high_resolution_clock::now(); // fin tiempo
    chrono::duration<double> duracion = end - start;

    cout << "\n[Estadísticas] Tiempo de ejecución: " << duracion.count() << " segundos\n";
    cout << "[Estadísticas] Cantidad de condicionales (if) ejecutados: " << contadorIf << endl;
    cout << "[Estadísticas] Proceso: 'procesarEstadisticas', Algoritmo: 'Iterativo con mapas y colas de prioridad', Estructura: 'Mapas, colas de prioridad, árbol AVL'\n";
}

#include <sstream>

string convertirFechaFormato(string fechaOriginal) {
    // Convierte fecha de "DD/MM/YYYY" a "YYYY-MM-DD"
    string dia, mes, anio;
    stringstream ss(fechaOriginal);
    getline(ss, dia, '/');
    getline(ss, mes, '/');
    getline(ss, anio, '/');

    // Añadir ceros a la izquierda si es necesario
    if (dia.length() == 1) dia = "0" + dia;
    if (mes.length() == 1) mes = "0" + mes;

    return anio + "-" + mes + "-" + dia;
}

void cargarVentasDesdeCSV(string nombreArchivo) {
    cout << "Intentando abrir el archivo: " << nombreArchivo << endl;
    ifstream archivo(nombreArchivo);
    if (!archivo.is_open()) {
        cout << "Error: No se pudo abrir el archivo " << nombreArchivo << endl;
        return;
    }
    string linea;
    getline(archivo, linea);
    cout << "Primera línea (cabecera): " << linea << endl;

    int linea_num = 1;
    while (getline(archivo, linea)) {
        cout << "Leyendo línea " << linea_num << ": " << linea << endl;
        vector<string> campos = dividirCSVConComillas(linea);
        cout << "Cantidad de campos parseados: " << campos.size() << endl;
        if (campos.size() != 12) {
            cout << "Línea " << linea_num << " ignorada por cantidad incorrecta de campos." << endl;
            linea_num++;
            continue;
        }

        try {
            Venta v;
            v.id = stoi(campos[0]);
            v.fecha = convertirFechaFormato(campos[1]);
            v.pais = campos[2];
            v.ciudad = campos[3];
            v.cliente = campos[4];
            v.producto = campos[5]; 
            v.categoria = campos[6];
            v.cantidad = stoi(campos[7]);
            v.precio_unitario = stof(campos[8]);
            v.monto_total = stof(campos[9]);
            v.medio_envio = campos[10];
            v.estado_envio = campos[11];

            ventas.push_back(v);
        } catch (...) {
            cout << "Error al parsear la línea " << linea_num << endl;
        }
        linea_num++;
    }

    archivo.close();
    cout << "Ventas cargadas correctamente: " << ventas.size() << endl;

    procesarEstadisticas();
}

#include <limits> // para numeric_limits

void agregarVenta() {
    Venta v;
    cout << "Ingrese ID: "; cin >> v.id;
    cin.ignore(numeric_limits<streamsize>::max(), '\n'); // limpiar buffer completamente
    cout << "Fecha (YYYY-MM-DD): "; getline(cin, v.fecha);
    cout << "Pais: "; getline(cin, v.pais);
    cout << "Ciudad: "; getline(cin, v.ciudad);
    cout << "Cliente: "; getline(cin, v.cliente);
    cout << "Producto: "; getline(cin, v.producto);
    cout << ""<<endl;
    cout << "Categoria: "; getline(cin, v.categoria);
    cout << "Cantidad: "; cin >> v.cantidad;
    cin.ignore(numeric_limits<streamsize>::max(), '\n'); // limpiar buffer completamente
    cout << "Precio unitario: "; cin >> v.precio_unitario;
    cin.ignore(numeric_limits<streamsize>::max(), '\n'); // limpiar buffer completamente
    v.monto_total = v.precio_unitario * v.cantidad;
    cout << "Medio de envio: "; getline(cin, v.medio_envio);
    cout << "Estado del envío: "; getline(cin, v.estado_envio);

    ventas.push_back(v);
    cout << "Venta agregada correctamente.\n";

    procesarEstadisticas();
}

void eliminarVenta() {
    string criterio;
    cout << "¿Desea eliminar por 'pais' o 'ciudad'? ";
    cin >> criterio;
    string valor;
    cout << "Ingrese el valor: ";
    cin >> valor;

    ventas.erase(remove_if(ventas.begin(), ventas.end(), [&](Venta v) {
        return (criterio == "pais" && v.pais == valor) || (criterio == "ciudad" && v.ciudad == valor);
    }), ventas.end());

    cout << "Ventas eliminadas con éxito.\n";
    procesarEstadisticas();
}

void modificarVenta() {
    int id;
    cout << "Ingrese el ID de la venta a modificar: ";
    cin >> id;

    for (auto& v : ventas) {
        if (v.id == id) {
            cout << "Modificar ciudad actual (" << v.ciudad << "): ";
            cin >> v.ciudad;
            cout << "Modificar producto (" << v.producto << "): ";
            cin >> v.producto;
            cout << "Modificar cantidad (" << v.cantidad << "): ";
            cin >> v.cantidad;
            cout << "Modificar precio unitario (" << v.precio_unitario << "): ";
            cin >> v.precio_unitario;
            v.monto_total = v.precio_unitario * v.cantidad;
            cout << "Modificacion realizada.\n";
            procesarEstadisticas();
            return;
        }
    }

    cout << "ID no encontrado.\n";
}

void consultarVentasPorCiudad() {
    string ciudad;
    cout << "Ingrese el nombre de la ciudad: ";
    cin >> ciudad;

    cout << "\nVentas en " << ciudad << ":\n";
    for (auto& v : ventas) {
        if (v.ciudad == ciudad) {
            cout << v.fecha << " | " << v.producto << " | $" << v.monto_total << endl;
        }
    }
}

void mostrarMenu() {
    cout << "\n==== MENU PRINCIPAL ====\n";
    cout << "1. Consultar ventas por ciudad\n";
    cout << "2. Agregar venta\n";
    cout << "3. Eliminar venta por pais/ciudad\n";
    cout << "4. Modificar venta por ID\n";
    cout << "5. Reprocesar estadisticas\n";
    cout << "6. Listar ventas por rango de fechas y pais\n";
    cout << "7. Comparar dos paises\n";
    cout << "8. Comparar dos productos\n";
    cout << "9. Productos con monto promedio > umbral\n";
    cout << "10. Productos con monto promedio < umbral\n";
    cout << "0. Salir\n";
    cout << "=========================\n";
    cout << "Seleccione una opcion: ";
}
void listarVentasPorRangoFechasYPais();
void compararPaises();
void compararProductos();
void buscarProductosPorMontoPromedio(bool mayor);
void agregarVenta(); 
void eliminarVenta(); 
void  modificarVenta(); 
void  procesarEstadisticas();

int main() {
    string nombreArchivo;
    cout << "Ingrese el nombre del archivo CSV de ventas: ";
    cin >> nombreArchivo;
    cargarVentasDesdeCSV(nombreArchivo);

    int opcion;
    do {
        mostrarMenu();
        cin >> opcion;
                switch (opcion) {
            case 1: consultarVentasPorCiudad(); break;
            case 2: agregarVenta(); break;
            case 3: eliminarVenta(); break;
            case 4: modificarVenta(); break;
            case 5: procesarEstadisticas(); break;
            case 6: listarVentasPorRangoFechasYPais(); break;
            case 7: compararPaises(); break;
            case 8: compararProductos(); break;
            case 9: buscarProductosPorMontoPromedio(true); break;
            case 10: buscarProductosPorMontoPromedio(false); break;
            case 0: cout << "Saliendo del sistema...\n"; break;
            default: cout << "Opción inválida.\n"; break;
        }

    } while (opcion != 0);

    return 0;
}
string normalizarFecha(string fecha) {
    // Convierte fechas tipo "2020-9-3" a "2020-09-03"
    string anio, mes, dia;
    stringstream ss(fecha);
    getline(ss, anio, '-');
    getline(ss, mes, '-');
    getline(ss, dia, '-');

    if (mes.length() == 1) mes = "0" + mes;
    if (dia.length() == 1) dia = "0" + dia;

    return anio + "-" + mes + "-" + dia;
}

void listarVentasPorRangoFechasYPais() {
    string pais, desde, hasta;
    cout << "Ingrese pais: ";
    cin >> pais;
    cout << "Fecha desde (YYYY-MM-DD): ";
    cin >> desde;
    cout << "Fecha hasta (YYYY-MM-DD): ";
    cin >> hasta;

    desde = normalizarFecha(desde);
    hasta = normalizarFecha(hasta);

    cout << "\nVentas en " << pais << " entre " << desde << " y " << hasta << ":\n";
    for (auto& v : ventas) {
        if (v.pais == pais && v.fecha >= desde && v.fecha <= hasta) {
            cout << v.fecha << " | " << v.producto << " | $" << v.monto_total << endl;
        }
    }
}

void compararPaises() {
    string p1, p2;
    cout << "Ingrese primer pais: "; cin >> p1;
    cout << "Ingrese segundo pais: "; cin >> p2;

    float total1 = 0, total2 = 0;
    unordered_map<string, int> prod1, prod2;
    unordered_map<string, int> envio1, envio2;

    for (auto& v : ventas) {
        if (v.pais == p1) {
            total1 += v.monto_total;
            prod1[v.producto] += v.cantidad;
            envio1[v.medio_envio]++;
        } else if (v.pais == p2) {
            total2 += v.monto_total;
            prod2[v.producto] += v.cantidad;
            envio2[v.medio_envio]++;
        }
    }

    auto maxProducto = [](unordered_map<string, int>& m) {
        string r; int max = -1;
        for (auto& e : m)
            if (e.second > max) max = e.second, r = e.first;
        return r;
    };

    auto maxEnvio = [](unordered_map<string, int>& m) {
        string r; int max = -1;
        for (auto& e : m)
            if (e.second > max) max = e.second, r = e.first;
        return r;
    };

    cout << "\nComparación entre " << p1 << " y " << p2 << ":\n";
    cout << p1 << ": $" << total1 << " | Producto más vendido: " << maxProducto(prod1)
         << " | Medio de envío más usado: " << maxEnvio(envio1) << endl;
    cout << p2 << ": $" << total2 << " | Producto más vendido: " << maxProducto(prod2)
         << " | Medio de envío más usado: " << maxEnvio(envio2) << endl;
}

void compararProductos() {
    string prod1, prod2;
    cout << "Ingrese nombre del primer producto: ";
    cin >> prod1;
    cout << "Ingrese nombre del segundo producto: ";
    cin >> prod2;

    unordered_map<string, int> cant1, cant2;
    unordered_map<string, float> total1, total2;

    for (auto& v : ventas) {
        if (v.producto == prod1) {
            cant1[v.pais] += v.cantidad;
            total1[v.pais] += v.monto_total;
        } else if (v.producto == prod2) {
            cant2[v.pais] += v.cantidad;
            total2[v.pais] += v.monto_total;
        }
    }

    set<string> paises;
    for (auto& p : cant1) paises.insert(p.first);
    for (auto& p : cant2) paises.insert(p.first);

    cout << "\nComparacion por país:\n";
    for (auto& pais : paises) {
        cout << "- " << pais << ": "
             << prod1 << " (" << cant1[pais] << " u | $" << total1[pais] << ") vs "
             << prod2 << " (" << cant2[pais] << " u | $" << total2[pais] << ")\n";
    }
}

void buscarProductosPorMontoPromedio(bool mayor) {
    float umbral;
    string pais;
    cout << "Ingrese pais: "; cin >> pais;
    cout << "Ingrese umbral ($): "; cin >> umbral;

    unordered_map<string, pair<float, int>> acumulado;

    for (auto& v : ventas) {
        if (v.pais == pais) {
            acumulado[v.producto].first += v.monto_total;
            acumulado[v.producto].second += v.cantidad;
        }
    }

    cout << "\nProductos con promedio por unidad "
         << (mayor ? "mayor" : "menor") << " a $" << umbral << " en " << pais << ":\n";

    for (auto& e : acumulado) {
        float promedio = e.second.first / e.second.second;
        if ((mayor && promedio > umbral) || (!mayor && promedio < umbral)) {
            cout << "- " << e.first << " ($" << promedio << ")\n";
        }
    }
}
