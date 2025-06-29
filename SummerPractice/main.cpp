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
        std::cerr << "Invalid chromosome: wrong size (" << individ.size() << " != " << n << ")\n";
        return false;
    }
    std::vector<bool> seen(n, false);
    for (int idx : individ) {
        if (idx < 0 || idx >= static_cast<int>(n) || seen[idx]) {
            std::cerr << "Invalid chromosome: duplicate or out-of-range index " << idx << "\n";
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

void ox1_crossover(const std::vector<int>& parent_fir, const std::vector<int>& parent_sec,
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

    for (const auto& [priority, group] : priority_groups) {
        if (group.size() <= 1) continue;

        
        std::vector<int> group_positions_fir(group.size()), group_positions_sec(group.size());
        for (size_t i = 0; i < group.size(); i++) {
            for (size_t j = 0; j < parent_fir.size(); j++) {
                if (parent_fir[j] == group[i]) group_positions_fir[i] = j;
                if (parent_sec[j] == group[i]) group_positions_sec[i] = j;
            }
        }
        std::sort(group_positions_fir.begin(), group_positions_fir.end());
        std::sort(group_positions_sec.begin(), group_positions_sec.end());

        
        auto [start_pos, end_pos] = group_ranges[priority];
        for (size_t i = 0; i < group.size(); i++) {
            if (group_positions_fir[i] < start_pos || group_positions_fir[i] >= end_pos ||
                group_positions_sec[i] < start_pos || group_positions_sec[i] >= end_pos) {
                std::cerr << "Invalid group positions for priority " << priority << "\n";
                return;
            }
        }

        
        std::vector<int> group_fir(group.size()), group_sec(group.size());
        for (size_t i = 0; i < group.size(); i++) {
            group_fir[i] = parent_fir[group_positions_fir[i]];
            group_sec[i] = parent_sec[group_positions_sec[i]];
        }

        std::vector<int> temp1(group.size()), temp2(group.size());
        if (group.size() == 2) {
            
            if (prob(gen) < 0.5) {
                temp1[0] = group_sec[0]; temp1[1] = group_sec[1];
                temp2[0] = group_fir[0]; temp2[1] = group_fir[1];
            } else {
                temp1 = group_fir;
                temp2 = group_sec;
            }
        } else {
            
            std::uniform_int_distribution<> dis(0, group.size() - 1);
            int start_idx = dis(gen);
            int end_idx = dis(gen);
            if (start_idx > end_idx) std::swap(start_idx, end_idx);

            
            std::vector<bool> used1(parent_fir.size(), false), used2(parent_fir.size(), false);
            for (int i = start_idx; i <= end_idx; i++) {
                temp1[i] = group_sec[i];
                temp2[i] = group_fir[i];
                used1[temp1[i]] = true;
                used2[temp2[i]] = true;
            }

            
            int pos_idx = 0, j = 0;
            while (pos_idx < group.size()) {
                if (pos_idx < start_idx || pos_idx > end_idx) {
                    while (j < group.size() && used1[group_fir[j]]) j++;
                    if (j < group.size()) {
                        temp1[pos_idx] = group_fir[j];
                        used1[temp1[pos_idx]] = true;
                        j++;
                    }
                    pos_idx++;
                } else {
                    pos_idx = end_idx + 1;
                }
            }

            pos_idx = 0, j = 0;
            while (pos_idx < group.size()) {
                if (pos_idx < start_idx || pos_idx > end_idx) {
                    while (j < group.size() && used2[group_sec[j]]) j++;
                    if (j < group.size()) {
                        temp2[pos_idx] = group_sec[j];
                        used2[temp2[pos_idx]] = true;
                        j++;
                    }
                    pos_idx++;
                } else {
                    pos_idx = end_idx + 1;
                }
            }
        }

        
        for (size_t i = 0; i < group.size(); i++) {
            child_fir[group_positions_fir[i]] = temp1[i];
            child_sec[group_positions_sec[i]] = temp2[i];
        }
    }

    
    if (!is_valid_chromosome(child_fir, priority_groups) ||
        !is_valid_chromosome(child_sec, priority_groups)) {
        std::cerr << "Invalid chromosome after crossover:\n";
        std::cerr << "Parent_fir: "; for (int x : parent_fir) std::cerr << x << " "; std::cerr << "\n";
        std::cerr << "Parent_sec: "; for (int x : parent_sec) std::cerr << x << " "; std::cerr << "\n";
        std::cerr << "Child_fir:  "; for (int x : child_fir) std::cerr << x << " "; std::cerr << "\n";
        std::cerr << "Child_sec:  "; for (int x : child_sec) std::cerr << x << " "; std::cerr << "\n";
    }
}


/*bool is_valid_chromosome(const std::vector<int>& individ, const std::map<int, std::vector<int>>& priority_groups) {
    size_t n = individ.size();
    std::vector<bool> seen(n, false);
    for (int idx : individ) {
        if (idx < 0 || idx >= static_cast<int>(n) || seen[idx]) {
            std::cerr << "Invalid chromosome: duplicate or out-of-range index " << idx << "\n";
            return false;
        }
        seen[idx] = true;
    }

    
    std::map<int, std::pair<size_t, size_t>> group_ranges;
    size_t pos = 0;
    for (const auto& [priority, group] : priority_groups) {
        group_ranges[priority] = {pos, pos + group.size()};
        pos += group.size();
    }

    
    for (const auto& [priority, group] : priority_groups) {
        auto [start_pos, end_pos] = group_ranges[priority];
        std::set<int> group_set(group.begin(), group.end());
        for (size_t i = start_pos; i < end_pos; i++) {
            if (group_set.find(individ[i]) == group_set.end()) {
                std::cerr << "Invalid chromosome: index " << individ[i] << " at position " << i
                          << " does not belong to group with priority " << priority << "\n";
                return false;
            }
        }
    }
    return true;
}


void group_crossover(const std::vector<int>& group1, const std::vector<int>& group2,
                     std::vector<int>& temp1, std::vector<int>& temp2,
                     std::mt19937& gen) {
    temp1.resize(group1.size());
    temp2.resize(group2.size());

    if (group1.size() <= 1) {
        temp1 = group1;
        temp2 = group2;
        return;
    }

    std::uniform_real_distribution<> prob(0, 1);
    if (prob(gen) < 0.5 && group1.size() == 2) {
        
        temp1[0] = group2[0]; temp1[1] = group2[1];
        temp2[0] = group1[0]; temp2[1] = group1[1];
    } else {
        temp1 = group1;
        temp2 = group2;
    }

    if (group1.size() > 2) {
        
        std::uniform_int_distribution<> dis(0, group1.size() - 1);
        int start_idx = dis(gen);
        int end_idx = dis(gen);
        if (start_idx > end_idx) std::swap(start_idx, end_idx);

        
        std::vector<bool> used1(group1.size(), false), used2(group1.size(), false);
        for (int i = start_idx; i <= end_idx; i++) {
            temp1[i] = group2[i];
            temp2[i] = group1[i];
            used1[temp1[i]] = true;
            used2[temp2[i]] = true;
        }

        
        int pos_idx = 0, j = 0;
        while (pos_idx < group1.size()) {
            if (pos_idx < start_idx || pos_idx > end_idx) {
                while (j < group1.size() && used1[group1[j]]) j++;
                if (j < group1.size()) {
                    temp1[pos_idx] = group1[j];
                    used1[temp1[pos_idx]] = true;
                    j++;
                }
                pos_idx++;
            } else {
                pos_idx = end_idx + 1;
            }
        }

        pos_idx = 0, j = 0;
        while (pos_idx < group2.size()) {
            if (pos_idx < start_idx || pos_idx > end_idx) {
                while (j < group2.size() && used2[group2[j]]) j++;
                if (j < group2.size()) {
                    temp2[pos_idx] = group2[j];
                    used2[temp2[pos_idx]] = true;
                    j++;
                }
                pos_idx++;
            } else {
                pos_idx = end_idx + 1;
            }
        }
    }
}

void ox1_crossover(const std::vector<int>& parent_fir, const std::vector<int>& parent_sec,
                   std::vector<int>& child_fir, std::vector<int>& child_sec,
                   const std::map<int, std::vector<int>>& priority_groups,
                   double cross_prob = 0.8) {
    
    if (!is_valid_chromosome(parent_fir, priority_groups) || !is_valid_chromosome(parent_sec, priority_groups)) {
        std::cerr << "Invalid input chromosomes for crossover:\n";
        std::cerr << "Parent_fir: "; for (int x : parent_fir) std::cerr << x << " "; std::cerr << "\n";
        std::cerr << "Parent_sec: "; for (int x : parent_sec) std::cerr << x << " "; std::cerr << "\n";
        child_fir = parent_fir;
        child_sec = parent_sec;
        return;
    }

    
    child_fir.assign(parent_fir.size(), -1);
    child_sec.assign(parent_sec.size(), -1);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> prob(0, 1);

    
    if (prob(gen) > cross_prob) {
        child_fir = parent_fir;
        child_sec = parent_sec;
        return;
    }

    
    std::map<int, std::pair<size_t, size_t>> group_ranges;
    size_t pos = 0;
    for (const auto& [priority, group] : priority_groups) {
        group_ranges[priority] = {pos, pos + group.size()};
        pos += group.size();
    }

    
    for (auto it = priority_groups.rbegin(); it != priority_groups.rend(); ++it) {
        int priority = it->first;
        const std::vector<int>& group = it->second;
        if (group.size() <= 1) {
            
            auto [start_pos, end_pos] = group_ranges[priority];
            for (size_t i = start_pos; i < end_pos; i++) {
                child_fir[i] = parent_fir[i];
                child_sec[i] = parent_sec[i];
            }
            continue;
        }

        
        std::vector<int> group_fir(group.size()), group_sec(group.size());
        auto [start_pos, end_pos] = group_ranges[priority];
        for (size_t i = 0; i < group.size(); i++) {
            group_fir[i] = parent_fir[start_pos + i];
            group_sec[i] = parent_sec[start_pos + i];
        }

        
        std::set<int> group_set(group.begin(), group.end());
        for (size_t i = 0; i < group.size(); i++) {
            if (group_set.find(group_fir[i]) == group_set.end() ||
                group_set.find(group_sec[i]) == group_set.end()) {
                std::cerr << "Invalid indices in group for priority " << priority << "\n";
                std::cerr << "Group_fir: "; for (int x : group_fir) std::cerr << x << " "; std::cerr << "\n";
                std::cerr << "Group_sec: "; for (int x : group_sec) std::cerr << x << " "; std::cerr << "\n";
                child_fir = parent_fir;
                child_sec = parent_sec;
                return;
            }
        }

        
        std::vector<int> temp1, temp2;
        group_crossover(group_fir, group_sec, temp1, temp2, gen);

        
        std::set<int> temp1_set(temp1.begin(), temp1.end());
        std::set<int> temp2_set(temp2.begin(), temp2.end());
        if (temp1_set.size() != group.size() || temp2_set.size() != group.size()) {
            std::cerr << "Invalid crossover result for priority " << priority << "\n";
            std::cerr << "Temp1: "; for (int x : temp1) std::cerr << x << " "; std::cerr << "\n";
            std::cerr << "Temp2: "; for (int x : temp2) std::cerr << x << " "; std::cerr << "\n";
            child_fir = parent_fir;
            child_sec = parent_sec;
            return;
        }

        
        for (size_t i = 0; i < group.size(); i++) {
            child_fir[start_pos + i] = temp1[i];
            child_sec[start_pos + i] = temp2[i];
        }
    }

    
    for (int x : child_fir) {
        if (x == -1) {
            std::cerr << "Child_fir contains unfilled positions (-1)\n";
            std::cerr << "Child_fir: "; for (int x : child_fir) std::cerr << x << " "; std::cerr << "\n";
            child_fir = parent_fir;
            child_sec = parent_sec;
            return;
        }
    }
    for (int x : child_sec) {
        if (x == -1) {
            std::cerr << "Child_sec contains unfilled positions (-1)\n";
            std::cerr << "Child_sec: "; for (int x : child_sec) std::cerr << x << " "; std::cerr << "\n";
            child_fir = parent_fir;
            child_sec = parent_sec;
            return;
        }
    }

    
    if (!is_valid_chromosome(child_fir, priority_groups) || !is_valid_chromosome(child_sec, priority_groups)) {
        std::cerr << "Invalid chromosomes after crossover:\n";
        std::cerr << "Child_fir: "; for (int x : child_fir) std::cerr << x << " "; std::cerr << "\n";
        std::cerr << "Child_sec: "; for (int x : child_sec) std::cerr << x << " "; std::cerr << "\n";
        child_fir = parent_fir;
        child_sec = parent_sec;
        return;
    }
}
*/



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
            std::vector<int> parent_first = tournament_selection(population, fitnesses, 5);
            std::vector<int> parent_second = tournament_selection(population, fitnesses, 5);
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
    vector<Town> twns = console_input();  
    std::vector<double> results = Evolution(twns, 150, 150, 0.8, 0.02);
    print_vector(results);

    return 0;
}
