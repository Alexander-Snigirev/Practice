#ifndef TOWN_H
#define TOWN_H

#include <string>

struct Town {
    double x, y;
    int priority;
    std::string name;
    Town(double x = 0, double y = 0, int priority = 1, std::string name = "")
        : x(x), y(y), priority(priority), name(name) {}
};

#endif // TOWN_H