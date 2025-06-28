#include <iostream>
#include <map>
#include <algorithm>
#include <cmath>
#include <string>
#include <climits>
#include <random>


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
void print_vector(std::vector<double>& vec){
    std::cout<<"[";
    for(double item: vec){
        std::cout<<item<<", ";
    }
    std::cout<<"]"<<std::endl;
}
void print_vector(std::vector<int>& vec){
    std::cout<<"[";
    for(int item: vec){
        std::cout<<item<<", ";
    }
    std::cout<<"]"<<std::endl;
}

//std::vector<Town> file_input


std::vector<Town> console_input(){
    std::vector<Town> towns;
    int towns_count;
    std::cout<<"Towns count:\n";
    cin>>towns_count;
    int curr_priority;
    double x,y;
    std::cout<<"Enter your towns in shape <x> <y> <priority>"<<std::endl;
    for(int i=0;i<towns_count;i++){
        //std::cout<<i<<": ";
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
                           std::map<int, std::vector<int>> priority_groups, int p_size){
    std::vector<std::vector<int>> start_population(p_size, std::vector<int>(towns.size(), -1));    // Вектор особей, каждая особь - последовательность городов.
    for(int i=0;i<p_size;i++){
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


double fitness_f(std::vector<int>& individ, std::vector<std::vector<double>>& matrix){
    int mod = 1e6;
    double way_length = 0.0;
    for(int i = 1;i<individ.size();i++){
        way_length+=matrix[individ[i-1]][individ[i]];
    }
    way_length += matrix[individ.back()][individ[0]];
    return mod - way_length;
}


std::vector<int> tournament_selection(const vector<vector<int>>& population, const vector<double>& fitnesses, int k){
    std::vector<int> candidates(k);
    // Случайный выбор 3 кандидатов
    for(int i=0;i<k;i++){
        candidates[i] = rand()%population.size();
    }
    int best = candidates[0];
    double best_fitness = fitnesses[best];
    for(int i=1;i<k;i++){
        double current_fitness = fitnesses[candidates[i]];
        if(current_fitness > best_fitness){
            best = candidates[i];
            best_fitness = current_fitness;
        }
    }
    return population[best];
}


std::vector<double> calculate_fitnesses(vector<vector<int>>& population, vector<vector<double>>& matrix, int p_size){
    std::vector<double> fitnesses(p_size);
    for(int i=0;i<population.size();i++){
        fitnesses[i] = fitness_f(population[i], matrix);
    }
    return fitnesses;
}

bool is_valid_chromosome(const std::vector<int>& individ, int n) {
    if (individ.size() != n) {
        //std::cerr << "Invalid chromosome: wrong size (" << individ.size() << " != " << n << ")\n";
        return false;
    }
    std::vector<bool> seen(n, false);
    for (int idx : individ) {
        if (idx < 0 || idx >= n || seen[idx]) {
            //std::cerr << "Invalid chromosome: duplicate or out-of-range index " << idx << "\n";
            return false;
        }
        seen[idx] = true;
    }
    return true;
}

/*void ox1_crossover(std::vector<int>& parent_fir, std::vector<int>& parent_sec,
               std::vector<int>& child_fir, std::vector<int>& child_sec,
               const std::map<int, std::vector<int>>& priority_groups, double cross_prob){
    child_fir = parent_sec;
    child_sec = parent_fir;
    std::random_device rd;
    std::mt19937 gen(rd());

    for(const auto& [priority, group]: priority_groups){
        if(group.size()<=1) continue;
        if ((double)rand() / RAND_MAX < cross_prob) {
            if (group.size() == 2) { // Специальный случай для групп из 2 городов
                if ((double)rand() / RAND_MAX < 0.5) {
                    child_fir[group[0]] = parent_sec[group[0]];
                    child_fir[group[1]] = parent_sec[group[1]];
                    child_sec[group[0]] = parent_fir[group[0]];
                    child_sec[group[1]] = parent_fir[group[1]];
                }
            } else {
                std::uniform_int_distribution<> dis(0, group.size()-1);
                int start = group[dis(gen)];
                int end = group[dis(gen)];
                if(start > end) std::swap(start, end);

                if (end - start >= group.size()) end = start;
                std::vector<int> temp1(group.size()), temp2(group.size());
                for (size_t i = 0; i < group.size(); i++) {
                    temp1[i] = child_fir[group[i]];
                    temp2[i] = child_sec[group[i]];
                }
                for (int i = start; i <= end; i++) {
                    temp1[i] = parent_sec[group[i]];
                    temp2[i] = parent_fir[group[i]];
                }
                std::vector<int> used1, used2;
                for (int i = start; i <= end; i++) {
                    used1.push_back(temp1[i]);
                    used2.push_back(temp2[i]);
                }

                int pos1 = 0;
                for (size_t i = 0; i < group.size(); i++) {
                    if (pos1 < start || pos1 > end) {
                        while (std::find(used1.begin(), used1.end(), parent_fir[group[i]]) != used1.end()) {
                            i++;
                            if (i >= group.size()) break;
                        }
                        if (i < group.size()) {
                            temp1[pos1] = parent_fir[group[i]];
                            used1.push_back(temp1[pos1]);
                        }
                        pos1++;
                    } else if (pos1 == start) {
                        pos1 = end + 1;
                    }
                }
                int pos2 = 0;
                for (size_t i = 0; i < group.size(); i++) {
                    if (pos2 < start || pos2 > end) {
                        while (std::find(used2.begin(), used2.end(), parent_sec[group[i]]) != used2.end()) {
                            i++;
                            if (i >= group.size()) break;
                        }
                        if (i < group.size()) {
                            temp2[pos2] = parent_sec[group[i]];
                            used2.push_back(temp2[pos2]);
                        }
                        pos2++;
                    } else if (pos2 == start) {
                        pos2 = end + 1;
                    }
                }
                for (size_t i = 0; i < group.size(); i++) {
                    child_fir[group[i]] = temp1[i];
                    child_sec[group[i]] = temp2[i];
                }
            }
        }
    }
    if(!is_valid_chromosome(child_fir, parent_fir.size()) && is_valid_chromosome(parent_fir, parent_fir.size()) && is_valid_chromosome(parent_sec, parent_fir.size())){
        std::cout<<"Invalid chromosome\n";
    }
    if(!is_valid_chromosome(child_sec, parent_fir.size()) && is_valid_chromosome(parent_fir, parent_fir.size()) && is_valid_chromosome(parent_sec, parent_fir.size())){
        std::cout<<"Invalid chromosome\n";
    }
}*/

void ox1_crossover(const std::vector<int>& parent_fir, const std::vector<int>& parent_sec,
                   std::vector<int>& child_fir, std::vector<int>& child_sec,
                   const std::map<int, std::vector<int>>& priority_groups,
                   double cross_prob = 0.8) {
    // Инициализация детей копиями родителей
    child_fir = parent_fir;
    child_sec = parent_sec;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> prob(0, 1);

    for (const auto& [priority, group] : priority_groups) {
        if (group.size() <= 1) continue;
        if (prob(gen) < cross_prob) {
            if (group.size() == 2) {
                // Для группы из 2 городов: с вероятностью 0.5 меняем местами
                if (prob(gen) < 0.5) {
                    std::swap(child_fir[group[0]], child_fir[group[1]]);
                    std::swap(child_sec[group[0]], child_sec[group[1]]);
                }
            } else {
                // Выбираем отрезок внутри группы (индексы в group)
                std::uniform_int_distribution<> dis(0, group.size() - 1);
                int start_idx = dis(gen);
                int end_idx = dis(gen);
                if (start_idx > end_idx) std::swap(start_idx, end_idx);

                // Временные массивы для работы с группой
                std::vector<int> temp1(group.size()), temp2(group.size());
                for (size_t i = 0; i < group.size(); i++) {
                    temp1[i] = child_fir[group[i]];
                    temp2[i] = child_sec[group[i]];
                }

                // Копируем отрезок [start_idx, end_idx] из другого родителя
                std::vector<bool> used1(group.size(), false), used2(group.size(), false);
                for (int i = start_idx; i <= end_idx; i++) {
                    temp1[i] = parent_sec[group[i]];
                    temp2[i] = parent_fir[group[i]];
                    used1[temp1[i]] = true;
                    used2[temp2[i]] = true;
                }

                // Заполняем оставшиеся позиции в порядке другого родителя
                int pos1 = 0, j = 0;
                while (pos1 < group.size()) {
                    if (pos1 < start_idx || pos1 > end_idx) {
                        while (j < group.size() && used1[parent_fir[group[j]]]) j++;
                        if (j < group.size()) {
                            temp1[pos1] = parent_fir[group[j]];
                            used1[temp1[pos1]] = true;
                            j++;
                        }
                        pos1++;
                    } else {
                        pos1 = end_idx + 1;
                    }
                }

                int pos2 = 0;
                j=0;
                while (pos2 < group.size()) {
                    if (pos2 < start_idx || pos2 > end_idx) {
                        while (j < group.size() && used2[parent_sec[group[j]]]) j++;
                        if (j < group.size()) {
                            temp2[pos2] = parent_sec[group[j]];
                            used2[temp2[pos2]] = true;
                            j++;
                        }
                        pos2++;
                    } else {
                        pos2 = end_idx + 1;
                    }
                }

                // Копируем обратно в child_fir и child_sec
                for (size_t i = 0; i < group.size(); i++) {
                    child_fir[group[i]] = temp1[i];
                    child_sec[group[i]] = temp2[i];
                }
            }
        }
    }

    // Отладка
    if (!is_valid_chromosome(child_fir, parent_fir.size()) || !is_valid_chromosome(child_sec, parent_fir.size())) {
        std::cerr << "Invalid chromosome after crossover:\n";
        std::cerr << "Parent_fir: "; for (int x : parent_fir) std::cerr << x << " "; std::cerr << "\n";
        std::cerr << "Parent_sec: "; for (int x : parent_sec) std::cerr << x << " "; std::cerr << "\n";
        std::cerr << "Child_fir:  "; for (int x : child_fir) std::cerr << x << " "; std::cerr << "\n";
        std::cerr << "Child_sec:  "; for (int x : child_sec) std::cerr << x << " "; std::cerr << "\n";
    }
}

void mutate(std::vector<int>& individ, const std::map<int, std::vector<int>>& priority_groups, double mutation_prob = 0.01) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> prob(0, 1);

    std::vector<int> start = individ;

    if (prob(gen) > mutation_prob) return;

    if (priority_groups.size() == 1) {
        std::uniform_int_distribution<> idx_dis(0, individ.size() - 1);
        int pos1 = idx_dis(gen);
        int pos2 = idx_dis(gen);
        while (pos2 == pos1) pos2 = idx_dis(gen);

        std::swap(individ[pos1], individ[pos2]);
    } else {
        std::uniform_int_distribution<> group_dis(0, priority_groups.size() - 1);
        auto it = priority_groups.begin();
        std::advance(it, group_dis(gen));
        const std::vector<int>& group = it->second;

        if (group.size() < 2) return;

        std::uniform_int_distribution<> idx_dis(0, group.size() - 1);
        int pos1 = idx_dis(gen);
        int pos2 = idx_dis(gen);
        while (pos2 == pos1) pos2 = idx_dis(gen);

        std::swap(individ[group[pos1]], individ[group[pos2]]);
    }
    if(!is_valid_chromosome(individ, individ.size()) && is_valid_chromosome(start, start.size())){
        std::cout<<"Invalid mutation\n";
    }

}


int find_best_individ(std::vector<double>& fitnesses){
    double best = 0.0;
    int index=0;
    for(int i = 0;i<fitnesses.size();i++){
        if(fitnesses[i] > best){
            best = fitnesses[i];
            index = i;
        }
    }
    return index;
}




std::vector<double> Evolution(std::vector<Town>& towns, int population_size, int generations_number, double mut_prob, double cross_prob){
    std::vector<std::vector<double>> matrix = calculate_distances(towns);                                   // матрица расстояний
    std::map<int, std::vector<int>> priority_groups = make_priority_groups(towns);                          // словарь, сопоставляющий каждому значению приоритета индексы городов
    std::vector<std::vector<int>> population = make_start_population(towns, matrix, priority_groups, population_size);  // стартовая популяция
    std::vector<double> fitnesses = calculate_fitnesses(population, matrix, population_size);
    std::vector<std::vector<int>> best_individs(generations_number, std::vector<int>(towns.size(), -1));
    int best_index = find_best_individ(fitnesses);
    std::vector<double> best_fitnesses(generations_number);
    best_individs[0] = population[best_index];
    best_fitnesses[0] = fitnesses[best_index];

    for(int i=1;i<generations_number;i++){
        std::vector<std::vector<int>> new_population(population_size, std::vector<int>(towns.size(), 0));
        int index=0;
        new_population[index] = population[best_index];
        new_population[index+1] = population[best_index];
        index+=2;
        while(index < population_size){
            std::vector<int> parent_first = tournament_selection(population, fitnesses, 4);
            std::vector<int> parent_second = tournament_selection(population, fitnesses, 4);
            std::vector<int> child_first, child_second;
            ox1_crossover(parent_first, parent_second, child_first, child_second, priority_groups, cross_prob);
            mutate(child_first, priority_groups, mut_prob);
            mutate(child_second, priority_groups, mut_prob);
            new_population[index] = child_first;
            new_population[index+1] = child_second;
            index+=2;
        }
        population = new_population;
        fitnesses = calculate_fitnesses(population, matrix, population_size);
        best_index = find_best_individ(fitnesses);
        best_individs[i] = population[best_index];
        best_fitnesses[i] = fitnesses[best_index];
    }
    print_matrix(best_individs);
    return best_fitnesses;

}




int main()
{
    vector<Town> twns = console_input();  // вектор городов
    std::vector<double> results = Evolution(twns, 1000, 2000, 0.8, 0.04);
    print_vector(results);

    return 0;
}
