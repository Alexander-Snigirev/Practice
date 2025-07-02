#include "tsp.h"
#include <cstdlib>

using namespace std;

int main() {
    vector<Town> twns = console_input();
    vector<double> results = Evolution(twns, 100, 100, 0.2, 0.8, "data.csv");
    //cout << "Длины путей лучших хромосом:\n";
    //print_dvector(results);
    return 0;
}
