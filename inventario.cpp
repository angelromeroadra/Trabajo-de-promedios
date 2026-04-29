#include <iostream>
#include <cmath>
using namespace std;

// ── 1. Captura de datos ───────────────────────────────────────
void pedirDatos(double &stock, double &demanda, double &desviacion,
                double &leadtime, double &costoP, double &costoA,
                double &nivelServicio) {
    cout << "=== OPTIMIZACION DE INVENTARIO (EOQ) ===" << endl << endl;
    cout << "Stock actual (unidades): ";              cin >> stock;
    cout << "Demanda promedio diaria (unidades/dia): "; cin >> demanda;
    cout << "Desviacion estandar de la demanda: ";    cin >> desviacion;
    cout << "Lead time del proveedor (dias): ";       cin >> leadtime;
    cout << "Costo de pedido (USD): ";                cin >> costoP;
    cout << "Costo de almacenamiento (USD/ud/dia): "; cin >> costoA;
    cout << "Nivel de servicio objetivo (ej: 95): ";  cin >> nivelServicio;
}

// ── 2. Factor Z segun nivel de servicio ──────────────────────
double obtenerZ(double nivelServicio) {
    if      (nivelServicio >= 99) return 2.33;
    else if (nivelServicio >= 98) return 2.05;
    else if (nivelServicio >= 97) return 1.88;
    else if (nivelServicio >= 96) return 1.75;
    else if (nivelServicio >= 95) return 1.65;
    else if (nivelServicio >= 90) return 1.28;
    else if (nivelServicio >= 85) return 1.04;
    else if (nivelServicio >= 80) return 0.84;
    else                          return 0.52;
}

// ── 3. Calculos EOQ ───────────────────────────────────────────
double calcularEOQ(double demanda, double costoP, double costoA) {
    double D_anual = demanda * 365;
    double H_anual = costoA  * 365;
    return sqrt((2 * D_anual * costoP) / H_anual);
}

double calcularStockSeguridad(double Z, double desviacion, double leadtime) {
    return Z * desviacion * sqrt(leadtime);
}

double calcularROP(double demanda, double leadtime, double SS) {
    return demanda * leadtime + SS;
}

double calcularCostoTotal(double demanda, double EOQ, double costoP,
                           double costoA, double SS) {
    double D_anual = demanda * 365;
    double H_anual = costoA  * 365;
    return (D_anual / EOQ) * costoP + (EOQ / 2 + SS) * H_anual;
}

// ── 4. Ingenieria de features ML ─────────────────────────────
void mostrarFeatures(double stock, double demanda, double desviacion) {
    double cobertura_dias    = stock / demanda;
    double variabilidad_norm = (desviacion / demanda) * 100;

    cout << endl << "--- Features ML ---" << endl;
    cout << "Cobertura de stock: " << (int)cobertura_dias    << " dias" << endl;
    cout << "Variabilidad (CV):  " << variabilidad_norm       << "%"    << endl;
}

// ── 5. Fecha sugerida de orden ────────────────────────────────
void mostrarFechaOrden(double stock, double ROP, double demanda) {
    double diasHastaROP = (stock - ROP) / demanda;

    cout << endl << "--- Fecha sugerida de orden ---" << endl;
    if (diasHastaROP <= 0)
        cout << ">> EMITIR ORDEN HOY (stock ya bajo el ROP)" << endl;
    else
        cout << ">> Emitir orden en " << (int)diasHastaROP << " dias" << endl;
}

// ── 6. Alertas (base de conocimiento SI/ENTONCES) ────────────
void mostrarAlertas(double stock, double ROP, double stockMax,
                    double desviacion, double demanda) {
    double variabilidad_norm = desviacion / demanda;

    cout << endl << "--- Alertas ---" << endl;

    if (stock < ROP)
        cout << "[!] ALERTA: stock < punto de reorden -> GENERAR PEDIDO YA" << endl;
    if (stock > stockMax)
        cout << "[!] ALERTA: stock > stock maximo -> SUSPENDER PEDIDOS"     << endl;
    if (variabilidad_norm > 0.25)
        cout << "[!] AVISO: alta variabilidad -> revisar stock de seguridad" << endl;
    if (stock >= ROP && stock <= stockMax)
        cout << "[OK] Stock dentro del rango optimo"                         << endl;
}

// ── 7. Mostrar resultados finales ─────────────────────────────
void mostrarResultados(double EOQ, double ROP, double SS,
                       double stockMax, double costoTotal) {
    cout << endl;
    cout << "========================================" << endl;
    cout << "           RESULTADOS EOQ               " << endl;
    cout << "========================================" << endl;
    cout << "Cantidad optima de pedido (EOQ): " << (int)EOQ        << " unidades" << endl;
    cout << "Punto de reorden (ROP):          " << (int)ROP        << " unidades" << endl;
    cout << "Stock de seguridad:              " << (int)SS         << " unidades" << endl;
    cout << "Stock maximo recomendado:        " << (int)stockMax   << " unidades" << endl;
    cout << "Costo total anual estimado:      $" << (int)costoTotal << " USD"     << endl;
}

// ── MAIN ──────────────────────────────────────────────────────
int main() {
    double stock, demanda, desviacion, leadtime, costoP, costoA, nivelServicio;

    pedirDatos(stock, demanda, desviacion, leadtime, costoP, costoA, nivelServicio);

    double Z        = obtenerZ(nivelServicio);
    double EOQ      = calcularEOQ(demanda, costoP, costoA);
    double SS       = calcularStockSeguridad(Z, desviacion, leadtime);
    double ROP      = calcularROP(demanda, leadtime, SS);
    double stockMax = ROP + EOQ;
    double costoTotal = calcularCostoTotal(demanda, EOQ, costoP, costoA, SS);

    mostrarResultados(EOQ, ROP, SS, stockMax, costoTotal);
    mostrarFeatures(stock, demanda, desviacion);
    mostrarFechaOrden(stock, ROP, demanda);
    mostrarAlertas(stock, ROP, stockMax, desviacion, demanda);

    cout << "========================================" << endl;
    return 0;
}
