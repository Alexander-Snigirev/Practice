#include "tsp.h"
#include <cstdlib>

using namespace std;

int main(int argc, char** argv) {
    if (argc != 3) {
        cerr << "Использование: " << argv[0] << " <размер_популяции> <число_поколений>\n";
        return 1;
    }
    vector<Town> twns = console_input();
    vector<double> results = Evolution(twns, atoi(argv[1]), atoi(argv[2]), 0.2, 0.8);
    cout << "Длины путей лучших хромосом:\n";
    print_dvector(results);
    return 0;
}
