#include <iostream>
#include <map>
#include <algorithm>
#include <cmath>
#include <string>

#define POPULATION_SIZE 11
#define GENERATIONS_NUMBER 100
#define MUTATION_PROB 0.05
#define CROSS_PROB 0.8

using namespace std;

struct Town{
    int priority;
    string name;
    double x,y;
    Town(double x, double y, int priority, string name){
        this->name = name;
        this->priority = priority;
        this->x = x;
        this->y = y;
    }
};

std::vector<Town> identify_towns(){
    std::vector<Town> towns;
    int towns_count;
    std::cout<<"Towns count:\n";
    cin>>towns_count;
    int curr_priority;
    double x,y;
    std::cout<<"Enter your towns in shape <x> <y> <priority>"<<std::endl;
    for(int i=0;i<towns_count;i++){
        std::cout<<i<<": ";
        std::cin>>x>>y>>curr_priority;
        Town new_town = Town(x,y,curr_priority,to_string(i));
        towns.push_back(new_town);
    }
    return towns;
}

std::vector<vector<double>> calculate_distances(std::vector<Town> towns){
    std::vector<vector<double>> dists(towns.size(), vector<double>(towns.size(), 0.0));
    for(int i = 0;i<towns.size();i++){
        for(int j=0;j<towns.size();j++){
            if(i!=j && dists[i][j]==0.0)
            dists[i][j] = sqrt(pow(towns[i].x-towns[j].x,2)+pow(towns[i].y-towns[j].y,2));
            dists[j][i] = sqrt(pow(towns[i].x-towns[j].x,2)+pow(towns[i].y-towns[j].y,2));
        }
    }
    return dists;
}

std::map<int, std::vector<int>> make_priority_groups(std::vector<Town>& towns){
    std::map<int, std::vector<int>> priority_groups;
    for(int i=0;i<towns.size();i++){
        priority_groups[towns[i].priority].push_back(i);
    }
    return priority_groups;
}

std::vector<std::vector<int>> make_start_population(std::vector<Town>& towns,
                           std::vector<std::vector<double>>& distances,
                           std::map<int, std::vector<int>> priority_groups){
    std::vector<std::vector<int>> start_population(POPULATION_SIZE, std::vector<int>(towns.size(), -1));    // Вектор особей, каждая особь - последовательность городов.
    for(int i=0;i<POPULATION_SIZE;i++){
        std::vector<int>& chromosome = start_population[i];
        int pos = 0;
        for (auto it = priority_groups.rbegin(); it != priority_groups.rend(); ++it) {
            std::vector<int> group = it->second;                // Копируем индексы группы
            std::random_shuffle(group.begin(), group.end());    // Случайная перестановка
            // Добавляем индексы группы в хромосому
            for (int idx : group) {
                chromosome[pos++] = idx;
            }
        }
    }
    return start_population;
}


void print_lst(std::vector<Town> towns){
    for(auto town: towns){
        std::cout<<town.name<<": "<<"("<<town.x<<", "<<town.y<<") - "<<town.priority<<std::endl;
    }
}

void print_matrix(std::vector<std::vector<double>> matrix){
    for(int i=0;i<matrix.size();i++){
        for(int j=0;j<matrix.size();j++){
            std::cout<<matrix[i][j]<<" ";
        }
        std::cout<<"\n";
    }
}
void print_matrix(std::vector<std::vector<int>> matrix){
    for(int i=0;i<matrix.size();i++){
        for(int j=0;j<matrix[0].size();j++){
            std::cout<<matrix[i][j]<<" ";
        }
        std::cout<<"\n\n";
    }
}


int main()
{
    vector<Town> twns = identify_towns();  // вектор городов
    print_lst(twns);
    std::vector<std::vector<double>> matrix = calculate_distances(twns);    // матрица расстояний
    std::map<int, std::vector<int>> priority_groups = make_priority_groups(twns);   // словарь, сопоставляющий каждому значению приоритета индексы городов
    std::vector<std::vector<int>> start_population = make_start_population(twns, matrix, priority_groups);

    //print_matrix(matrix);
    print_matrix(start_population);
    return 0;
}
