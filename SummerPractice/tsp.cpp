#include "tsp.h"
#include <algorithm>
#include <cmath>
#include <climits>


using namespace std;

Town::Town(double x, double y, int priority, string name) {
    this->name = name;
    this->priority = priority;
    this->x = x;
    this->y = y;
}

void print_lst(vector<Town> towns) {
    for(auto town: towns) {
        cout << town.name << ": (" << town.x << ", " << town.y << ") - " << town.priority << endl;
    }
}

void print_matrix(vector<vector<double>> matrix) {
    for(int i = 0; i < matrix.size(); i++) {
        for(int j = 0; j < matrix[i].size(); j++) {
            cout << matrix[i][j] << " ";
        }
        cout << "\n";
    }
}

void print_matrix(vector<vector<int>> matrix) {
    for(int i = 0; i < matrix.size(); i++) {
        for(int j = 0; j < matrix[i].size(); j++) {
            cout << matrix[i][j] << " ";
        }
        cout << "\n";
    }
}

void print_dvector(vector<double>& vec) {
    if (vec.empty()) {
        cout << "[]\n";
        return;
    }
    cout << "[";
    for(size_t i = 0; i < vec.size(); i++) {
        cout << (1e6 - vec[i]);
        if (i < vec.size() - 1) cout << ", ";
    }
    cout << "]\n";
}

void format_dvector(vector<double>& vec) {
    if (vec.empty()) {
        return;
    }
    for(size_t i = 0; i < vec.size(); i++) {
        vec[i] = (1e6 - vec[i]);
    }
}

void print_vector(vector<int>& vec) {
    cout << "[";
    for(size_t i = 0; i < vec.size(); i++) {
        cout << vec[i];
        if (i < vec.size() - 1) cout << ", ";
    }
    cout << "]\n";
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

    std::map<int, std::pair<size_t, size_t>> group_ranges;
    size_t pos = 0;
    for (const auto& [priority, group] : priority_groups) {
        group_ranges[priority] = {pos, pos + group.size()};
        pos += group.size();
    }

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

map<int, vector<int>> make_priority_groups(vector<Town>& towns) {
    map<int, vector<int>> priority_groups;
    for(int i = 0; i < towns.size(); i++) {
        priority_groups[towns[i].priority].push_back(i);
    }
    print_priority_groups(priority_groups);
    return priority_groups;
}

vector<vector<int>> make_start_population(vector<Town>& towns,
                                          vector<vector<double>>& distances,
                                          map<int, vector<int>> priority_groups, int p_size) {
    vector<vector<int>> start_population(p_size, vector<int>(towns.size(), -1));
    random_device rd;
    mt19937 gen(rd());
    for(int i = 0; i < p_size; i++) {
        vector<int>& chromosome = start_population[i];
        int pos = 0;
        for (auto it = priority_groups.rbegin(); it != priority_groups.rend(); ++it) {
            vector<int> group = it->second;
            shuffle(group.begin(), group.end(), gen);
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

vector<int> tournament_selection(const vector<vector<int>>& population, const vector<double>& fitnesses, int k) {
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dis(0, population.size() - 1);
    vector<int> candidates(k);
    for(int i = 0; i < k; i++) {
        candidates[i] = dis(gen);
    }
    int best = candidates[0];
    double best_fitness = fitnesses[best];
    for(int i = 1; i < k; i++) {
        double current_fitness = fitnesses[candidates[i]];
        if(current_fitness > best_fitness) {
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

double calculate_var_len(std::vector<double>& fitnesses){
    if (fitnesses.empty()) {
        return 0.0;
    }
    double sum = std::accumulate(fitnesses.begin(), fitnesses.end(), 0.0);
    return sum / fitnesses.size();
}

bool is_valid_chromosome(const vector<int>& individ, const map<int, vector<int>>& priority_groups) {
    if (individ.empty()) {
        cerr << "Invalid chromosome: empty\n";
        return false;
    }
    size_t n = individ.size();
    vector<bool> seen(n, false);
    for (int idx : individ) {
        if (idx < 0 || idx >= static_cast<int>(n) || seen[idx]) {
            cerr << "Invalid chromosome: duplicate or out-of-range index " << idx << "\n";
            return false;
        }
        seen[idx] = true;
    }
    map<int, vector<int>> group_positions;
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
                cerr << "Invalid chromosome: index " << individ[pos] << " at position " << pos
                     << " does not belong to group with priority " << it->first << "\n";
                return false;
            }
        }
    }
    return true;
}

void group_crossover(const vector<int>& group1, const vector<int>& group2,
                     vector<int>& child1, vector<int>& child2,
                     mt19937& gen) {
    child1.resize(group1.size());
    child2.resize(group2.size());
    if (group1.size() <= 1) {
        child1 = group1;
        child2 = group2;
        return;
    }
    uniform_real_distribution<> prob(0, 1);
    if (group1.size() == 2 && prob(gen) < 0.5) {
        child1[0] = group2[0]; child1[1] = group2[1];
        child2[0] = group1[0]; child2[1] = group1[1];
        return;
    }
    if (group1.size() > 2) {
        uniform_int_distribution<> dis(0, group1.size() - 1);
        int start_idx = dis(gen);
        int end_idx = dis(gen);
        if (start_idx > end_idx) swap(start_idx, end_idx);
        vector<bool> used1(group1.size(), false);
        vector<bool> used2(group2.size(), false);
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

void merge_child_groups(const vector<vector<int>>& child_fir_groups,
                        const vector<vector<int>>& child_sec_groups,
                        vector<int>& child_fir,
                        vector<int>& child_sec) {
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
}

void ox1_crossover(vector<int>& parent_fir, vector<int>& parent_sec,
                   vector<int>& child_fir, vector<int>& child_sec,
                   const map<int, vector<int>>& priority_groups,
                   double cross_prob) {
    child_fir = parent_fir;
    child_sec = parent_sec;
    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<> prob(0, 1);
    if (prob(gen) < cross_prob) {
        map<int, pair<size_t, size_t>> group_ranges;
        size_t pos = 0;
        for (auto it = priority_groups.rbegin(); it != priority_groups.rend(); ++it) {
            group_ranges[it->first] = {pos, pos + it->second.size()};
            pos += it->second.size();
        }
        vector<vector<int>> parent_fir_groups(priority_groups.size());
        vector<vector<int>> parent_sec_groups(priority_groups.size());
        vector<vector<int>> child_fir_groups(priority_groups.size());
        vector<vector<int>> child_sec_groups(priority_groups.size());
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
    }
    if (!is_valid_chromosome(child_fir, priority_groups)) {
        child_fir = parent_fir;
        cerr << "Недопустимая хромосома child_fir после кроссовера, возврат к родителю\n";
    }
    if (!is_valid_chromosome(child_sec, priority_groups)) {
        child_sec = parent_sec;
        cerr << "Недопустимая хромосома child_sec после кроссовера, возврат к родителю\n";
    }
}

void mutate(vector<int>& individ, const map<int, vector<int>>& priority_groups, double mutation_prob) {
    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<> prob(0, 1);
    if (prob(gen) < mutation_prob) {
        map<int, pair<size_t, size_t>> group_ranges;
        size_t pos = 0;
        for (auto it = priority_groups.rbegin(); it != priority_groups.rend(); ++it) {
            group_ranges[it->first] = {pos, pos + it->second.size()};
            pos += it->second.size();
        }
        uniform_int_distribution<> group_dis(0, priority_groups.size() - 1);
        auto it = priority_groups.begin();
        advance(it, group_dis(gen));
        int priority = it->first;
        const auto& [start_pos, end_pos] = group_ranges[priority];
        if (end_pos - start_pos < 2) return;
        uniform_int_distribution<> idx_dis(start_pos, end_pos - 1);
        int pos1 = idx_dis(gen);
        int pos2 = idx_dis(gen);
        while (pos2 == pos1) pos2 = idx_dis(gen);
        swap(individ[pos1], individ[pos2]);
        if (!is_valid_chromosome(individ, priority_groups)) {
            swap(individ[pos1], individ[pos2]);
            cerr << "Недопустимая хромосома после мутации, отмена мутации\n";
        }
    }
}

int find_best_individ(vector<double>& fitnesses) {
    if (fitnesses.empty()) return 0;
    double best = fitnesses[0];
    int index = 0;
    for(int i = 0; i < fitnesses.size(); i++) {
        if(fitnesses[i] > best) {
            best = fitnesses[i];
            index = i;
        }
    }
    return index;
}

std::vector<double> Evolution(std::vector<Town>& towns, int population_size, int generations_number, double mut_prob, double cross_prob, const std::string& filename){
    std::vector<std::vector<double>> matrix = calculate_distances(towns);
    std::map<int, std::vector<int>> priority_groups = make_priority_groups(towns);
    std::vector<std::vector<int>> population = make_start_population(towns, matrix, priority_groups, population_size);
    std::vector<double> fitnesses = calculate_fitnesses(population, matrix, population_size);
    std::vector<double> var_lens(generations_number);
    std::vector<std::vector<int>> best_individs(generations_number, std::vector<int>(towns.size(), -1));
    int best_index = find_best_individ(fitnesses);
    std::vector<double> best_fitnesses(generations_number);
    best_individs[0] = population[best_index];
    best_fitnesses[0] = fitnesses[best_index];
    var_lens[0] = calculate_var_len(fitnesses);
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
        var_lens[i] = calculate_var_len(fitnesses);
    }
    //print_matrix(best_individs);
    format_dvector(best_fitnesses);
    format_dvector(var_lens);
    save_to_csv(best_fitnesses, var_lens,best_individs, filename);
    return best_fitnesses;
}

void save_to_csv(const std::vector<double>& best_lens, const std::vector<double>& var_lens,
                 const std::vector<std::vector<int>>& best_individs, const std::string& filename) {
    if (best_lens.size() != var_lens.size() || best_lens.size() != best_individs.size()) {
        throw std::invalid_argument("Vectors best_lens, var_lens, and best_individs must have the same size");
    }
    if (best_lens.empty()) {
        throw std::invalid_argument("Input vectors are empty");
    }
    size_t n = best_individs[0].size();
    for (const auto& individ : best_individs) {
        if (individ.size() != n) {
            throw std::invalid_argument("All chromosomes in best_individs must have the same size");
        }
    }

    rapidcsv::Document doc("", rapidcsv::LabelParams(0, -1), rapidcsv::SeparatorParams(','));

    std::vector<std::string> headers = {"num"};
    for (size_t i = 1; i <= n; ++i) {
        headers.push_back("town" + std::to_string(i));
    }
    headers.push_back("best_len");
    headers.push_back("var_len");
    doc.SetRow(0, headers);

    for (size_t i = 0; i < best_lens.size(); ++i) {
        std::vector<std::string> row;
        row.push_back(std::to_string(i));
        for (int idx : best_individs[i]) {
            row.push_back(std::to_string(idx));
        }
        row.push_back(std::to_string(best_lens[i]));
        row.push_back(std::to_string(var_lens[i]));
        doc.SetRow(i + 1, row);
    }
    doc.Save(filename);
}
