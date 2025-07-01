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
        for(int j=0;j<matrix[i].size();j++){
            std::cout<<matrix[i][j]<<" ";
        }
        std::cout<<"\n";
    }
}
void print_matrix(std::vector<std::vector<int>> matrix){
    for(int i=0;i<matrix.size();i++){
        for(int j=0;j<matrix[i].size();j++){
            std::cout<<matrix[i][j]<<" ";
        }
        std::cout<<"\n\n";
    }
}
void print_dvector(std::vector<double>& vec){
    std::cout<<"[";
    for(double item: vec){
        std::cout<<1e6-item<<", ";
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
void print_priority_groups(const std::map<int, std::vector<int>>& priority_groups) {
    std::cout << "Priority Groups:\n";
    size_t pos = 0;
    for (const auto& [priority, group] : priority_groups) {
        std::cout << "Priority " << priority << "\n";
        std::cout << "  Indices: ";
        for (int idx : group) {
            std::cout << idx << " ";
        }
        std::cout << "\n  Size: " << group.size() << "\n";
        pos += group.size();
    }
    std::cout << "Total positions covered: " << pos << "\n";
}

void print_priority_groups_with_ranges(const std::map<int, std::vector<int>>& priority_groups) {
    std::cout << "Priority Groups with Position Ranges:\n";

    // Вычисляем диапазоны позиций в порядке возрастания приоритетов
    std::map<int, std::pair<size_t, size_t>> group_ranges;
    size_t pos = 0;
    for (const auto& [priority, group] : priority_groups) {
        group_ranges[priority] = {pos, pos + group.size()};
        pos += group.size();
    }

    // Выводим в порядке убывания приоритетов
    for (auto it = priority_groups.rbegin(); it != priority_groups.rend(); ++it) {
        int priority = it->first;
        const auto& group = it->second;
        const auto& [start_pos, end_pos] = group_ranges[priority];

        std::cout << "Priority " << priority << " (positions [" << start_pos << ", " << end_pos << ")):\n";
        std::cout << "  Indices: ";
        for (int idx : group) {
            std::cout << idx << " ";
        }
        std::cout << "\n  Size: " << group.size() << "\n";
    }

    std::cout << "Total positions covered: " << pos << "\n";
}

std::vector<Town> console_input(){
    std::vector<Town> towns;
    int towns_count;
    std::cout<<"Towns count:\n";
    cin>>towns_count;
    int curr_priority;
    double x,y;
    std::cout<<"Enter your towns in shape <x> <y> <priority>"<<std::endl;
    for(int i=0;i<towns_count;i++){

        std::cin>>x>>y>>curr_priority;
        Town new_town(x,y,curr_priority,to_string(i));
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
    print_priority_groups(priority_groups);
    return priority_groups;
}

std::vector<std::vector<int>> make_start_population(std::vector<Town>& towns,
                                                    std::vector<std::vector<double>>& distances,
                                                    std::map<int, std::vector<int>> priority_groups, int p_size){
    std::vector<std::vector<int>> start_population(p_size, std::vector<int>(towns.size(), -1));
    for(int i=0;i<p_size;i++){
        std::vector<int>& chromosome = start_population[i];
        int pos = 0;
        for (auto it = priority_groups.rbegin(); it != priority_groups.rend(); ++it) {
            std::vector<int> group = it->second;
            std::random_shuffle(group.begin(), group.end());

            for (int idx : group) {
                chromosome[pos++] = idx;
            }
        }
    }
    return start_population;
}


double fitness_f(std::vector<int>& individ, std::vector<std::vector<double>>& matrix){
    double mod = 1e6;
    double way_length = 0.0;
    for(int i = 1;i<individ.size();i++){
        way_length+=matrix[individ[i-1]][individ[i]];
    }
    way_length += matrix[individ.back()][individ[0]];
    return mod - way_length;
}


std::vector<int> tournament_selection(const vector<vector<int>>& population, const vector<double>& fitnesses, int k){
    std::vector<int> candidates(k);

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


bool is_valid_chromosome(const std::vector<int>& individ, const std::map<int, std::vector<int>>& priority_groups) {
    size_t n = individ.size();
    if (individ.size() != n) {
        //std::cerr << "Invalid chromosome: wrong size (" << individ.size() << " != " << n << ")\n";
        return false;
    }
    std::vector<bool> seen(n, false);
    for (int idx : individ) {
        if (idx < 0 || idx >= static_cast<int>(n) || seen[idx]) {
            //std::cerr << "Invalid chromosome: duplicate or out-of-range index " << idx << "\n";
            return false;
        }
        seen[idx] = true;
    }
    std::map<int, std::vector<int>> group_positions;
    for (const auto& [priority, group] : priority_groups) {
        group_positions[priority] = {};
        for (int idx : group) {
            for (size_t i = 0; i < individ.size(); i++) {
                if (individ[i] == idx) {
                    group_positions[priority].push_back(i);
                    break;
                }
            }
        }
    }
    size_t pos = 0;
    for (auto it = priority_groups.rbegin(); it != priority_groups.rend(); ++it) {
        const auto& group = it->second;
        for (size_t i = 0; i < group.size(); i++, pos++) {
            bool found = false;
            for (int idx : group) {
                if (individ[pos] == idx) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                std::cerr << "Invalid chromosome: index " << individ[pos] << " at position " << pos
                          << " does not belong to group with priority " << it->first << "\n";
                return false;
            }
        }
    }
    return true;
}


void group_crossover(const std::vector<int>& group1, const std::vector<int>& group2,
                     std::vector<int>& child1, std::vector<int>& child2,
                     std::mt19937& gen) {
    child1.resize(group1.size());
    child2.resize(group2.size());

    if (group1.size() <= 1) {
        child1 = group1;
        child2 = group2;
        return;
    }

    std::uniform_real_distribution<> prob(0, 1);
    if (group1.size() == 2 && prob(gen) < 0.5) {
        child1[0] = group2[0]; child1[1] = group2[1];
        child2[0] = group1[0]; child2[1] = group1[1];
        return;
    }

    if (group1.size() > 2) {
        std::uniform_int_distribution<> dis(0, group1.size() - 1);
        int start_idx = dis(gen);
        int end_idx = dis(gen);
        if (start_idx > end_idx) std::swap(start_idx, end_idx);

        std::vector<bool> used1(group1.size(), false);
        std::vector<bool> used2(group2.size(), false);

        for (int i = start_idx; i <= end_idx; ++i) {
            child1[i] = group2[i];
            child2[i] = group1[i];
            used1[child1[i]] = true;
            used2[child2[i]] = true;
        }

        int pos_idx = 0, j = 0;
        while (pos_idx < group1.size()) {
            if (pos_idx < start_idx || pos_idx > end_idx) {
                while (j < group1.size() && used1[group1[j]]) ++j;
                if (j < group1.size()) {
                    child1[pos_idx] = group1[j];
                    used1[group1[j]] = true;
                    ++j;
                }
                pos_idx++;
            } else {
                pos_idx = end_idx + 1;
            }
        }

        pos_idx = 0, j = 0;
        while (pos_idx < group2.size()) {
            if (pos_idx < start_idx || pos_idx > end_idx) {
                while (j < group2.size() && used2[group2[j]]) ++j;
                if (j < group2.size()) {
                    child2[pos_idx] = group2[j];
                    used2[group2[j]] = true;
                    ++j;
                }
                pos_idx++;
            } else {
                pos_idx = end_idx + 1;
            }
        }
    } else {
        child1 = group1;
        child2 = group2;
    }
}

void merge_child_groups(const std::vector<std::vector<int>>& child_fir_groups,
                        const std::vector<std::vector<int>>& child_sec_groups,
                        std::vector<int>& child_fir,
                        std::vector<int>& child_sec) {
    size_t total_size = 0;
    for (const auto& group : child_fir_groups) {
        total_size += group.size();
    }
    child_fir.resize(total_size);
    child_sec.resize(total_size);

    size_t pos = 0;
    for (size_t group_idx = 0; group_idx < child_fir_groups.size(); ++group_idx) {
        for (size_t i = 0; i < child_fir_groups[group_idx].size(); ++i) {
            child_fir[pos] = child_fir_groups[group_idx][i];
            child_sec[pos] = child_sec_groups[group_idx][i];
            ++pos;
        }
    }

    // Выводим результаты для отладки
    //std::cout << "Child_fir_groups:\n";
    //print_matrix(child_fir_groups);
    //std::cout << "Child_fir: ";
    //print_vector(child_fir);
   // std::cout << "Child_sec_groups:\n";
    //print_matrix(child_sec_groups);
    //std::cout << "Child_sec: ";
    //print_vector(child_sec);
}

void ox1_crossover(std::vector<int>& parent_fir, std::vector<int>& parent_sec,
                   std::vector<int>& child_fir, std::vector<int>& child_sec,
                   const std::map<int, std::vector<int>>& priority_groups,
                   double cross_prob = 0.8) {
    child_fir = parent_fir;
    child_sec = parent_sec;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> prob(0, 1);

    if (prob(gen) > cross_prob) return;


    std::map<int, std::pair<size_t, size_t>> group_ranges;
    size_t pos = 0;
    for (auto it = priority_groups.rbegin(); it != priority_groups.rend(); ++it) {
        group_ranges[it->first] = {pos, pos + it->second.size()};
        pos += it->second.size();
    }
    std::vector<std::vector<int>> parent_fir_groups(priority_groups.size());
    std::vector<std::vector<int>> parent_sec_groups(priority_groups.size());
    std::vector<std::vector<int>> child_fir_groups(priority_groups.size());
    std::vector<std::vector<int>> child_sec_groups(priority_groups.size());
    size_t group_idx = 0;
    for (auto it = priority_groups.rbegin(); it != priority_groups.rend(); ++it) {
        int priority = it->first;
        const auto& [start_pos, end_pos] = group_ranges[priority];
        size_t group_size = end_pos - start_pos;
        parent_fir_groups[group_idx].resize(group_size);
        parent_sec_groups[group_idx].resize(group_size);
        for (size_t i = 0; i < group_size; ++i) {
            parent_fir_groups[group_idx][i] = parent_fir[start_pos + i];
            parent_sec_groups[group_idx][i] = parent_sec[start_pos + i];
        }
        ++group_idx;
    }
    group_idx = 0;
    for (auto it = priority_groups.rbegin(); it != priority_groups.rend(); ++it) {
        child_fir_groups[group_idx].resize(it->second.size());
        child_sec_groups[group_idx].resize(it->second.size());
        group_crossover(parent_fir_groups[group_idx], parent_sec_groups[group_idx],
                        child_fir_groups[group_idx], child_sec_groups[group_idx], gen);
        ++group_idx;
    }
    merge_child_groups(child_fir_groups, child_sec_groups, child_fir, child_sec);


    //print_vector(parent_fir);
    //std::cout << "Parent_fir_groups:\n";
    //group_idx = 0;
    //print_matrix(parent_fir_groups);
    //print_vector(parent_sec);
    //std::cout << "Parent_sec_groups:\n";
    //print_matrix(parent_sec_groups);
    //std::cout << "Child_fir:\n";
    //print_vector(child_fir);
    //std::cout << "Child_sec:\n";
    //print_vector(child_sec);
}





void mutate(std::vector<int>& individ, const std::map<int, std::vector<int>>& priority_groups, double mutation_prob = 0.05) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> prob(0, 1);

    if (prob(gen) > mutation_prob) return;
    std::map<int, std::pair<size_t, size_t>> group_ranges;
    size_t pos = 0;
    for (auto it = priority_groups.rbegin(); it != priority_groups.rend(); ++it) {
        group_ranges[it->first] = {pos, pos + it->second.size()};
        pos += it->second.size();
    }

    std::uniform_int_distribution<> group_dis(0, priority_groups.size() - 1);
    auto it = priority_groups.begin();
    std::advance(it, group_dis(gen));
    int priority = it->first;
    const auto& [start_pos, end_pos] = group_ranges[priority];

    if (end_pos - start_pos < 2) return;
    std::uniform_int_distribution<> idx_dis(start_pos, end_pos - 1);
    int pos1 = idx_dis(gen);
    int pos2 = idx_dis(gen);
    while (pos2 == pos1) pos2 = idx_dis(gen);
    std::swap(individ[pos1], individ[pos2]);

    // Для отладки (можно убрать)
    //std::cout << "Mutated chromosome at positions " << pos1 << " and " << pos2
    //          << " in group priority " << priority << ": ";
    //for (int x : individ) std::cout << x << " ";
    //std::cout << "\n";
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
    std::vector<std::vector<double>> matrix = calculate_distances(towns);
    std::map<int, std::vector<int>> priority_groups = make_priority_groups(towns);
    std::vector<std::vector<int>> population = make_start_population(towns, matrix, priority_groups, population_size);
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

int main(int argc, char** argv)
{
    vector<Town> twns = console_input();
    std::vector<double> results = Evolution(twns, atoi(argv[1]), atoi(argv[2]), 0.9, 0.02);
    print_dvector(results);

    return 0;
}
