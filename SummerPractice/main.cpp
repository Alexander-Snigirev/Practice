#include <iostream>
#include <map>
#include <algorithm>
#include <cmath>
#include <string>
#include <climits>
#include <random>
#include <set>

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

void print_matrix(std::vector<std::vector<double> > matrix){
    for(int i=0;i<matrix.size();i++){
        for(int j=0;j<matrix.size();j++){
            std::cout<<matrix[i][j]<<" ";
        }
        std::cout<<"\n";
    }
}
void print_matrix(std::vector<std::vector<int> > matrix){
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

std::vector<std::vector<double> > calculate_distances(std::vector<Town>& towns) {
    int n = towns.size();
    std::vector<std::vector<double> > matrix(n, std::vector<double>(n, 0));
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            matrix[i][j] = sqrt(pow(towns[i].x - towns[j].x, 2) + pow(towns[i].y - towns[j].y, 2));
            std::cout << "Distance [" << i << "][" << j << "] = " << matrix[i][j] << "\n";
        }
    }
    return matrix;
}

std::map<int, std::vector<int> > make_priority_groups(std::vector<Town>& towns) {
    std::map<int, std::vector<int> > priority_groups;
    for (int i = 0; i < towns.size(); i++) {
        priority_groups[towns[i].priority].push_back(i);
    }
    // Отладочный вывод
    /* for (const auto& [priority, group] : priority_groups) {
         std::cout << "Priority " << priority << ": ";
         for (int idx : group) {
             std::cout << idx << " ";
         }
         std::cout << std::endl;
     }*/
    return priority_groups;
}
bool is_valid_chromosome(const std::vector<int>& individ, const std::map<int, std::vector<int> >& priority_groups) {
    size_t n = individ.size();
    std::vector<bool> seen(n, false);
    for (int idx : individ) {
        if (idx < 0 || idx >= static_cast<int>(n) || seen[idx]) {
            std::cerr << "Invalid chromosome: duplicate or out-of-range index " << idx << "\n";
            return false;
        }
        seen[idx] = true;
    }

    // Формируем диапазоны групп в порядке убывания приоритетов
    std::map<int, std::pair<size_t, size_t> > group_ranges;
    size_t pos = 0;
    for (auto it = priority_groups.rbegin(); it != priority_groups.rend(); ++it) {
        group_ranges[it->first] = {pos, pos + it->second.size()};
        pos += it->second.size();
    }

    // Проверяем в порядке убывания приоритетов
    for (auto it = priority_groups.rbegin(); it != priority_groups.rend(); ++it) {
        int priority = it->first;
        const std::vector<int>& group = it->second;
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

    // Отладочный вывод
    /*std::cout << "Validating chromosome: ";
    for (int x : individ) std::cout << x << " ";
    std::cout << (individ.size() == n ? " - Valid" : " - Invalid") << "\n";*/
    return true;
}

std::vector<std::vector<int> > make_start_population(std::vector<Town>& towns,
                                                    std::vector<std::vector<double> >& distances,
                                                    std::map<int, std::vector<int> > priority_groups, int p_size) {
    std::vector<std::vector<int> > start_population(p_size, std::vector<int>(towns.size(), -1));
    std::random_device rd;
    std::mt19937 gen(rd());

    for (int i = 0; i < p_size; i++) {
        std::vector<int>& chromosome = start_population[i];
        int pos = 0;
        for (auto it = priority_groups.rbegin(); it != priority_groups.rend(); ++it) {
            std::vector<int> group = it->second;
            std::shuffle(group.begin(), group.end(), gen);
            for (int idx : group) {
                chromosome[pos++] = idx;
            }
        }
        // Отладка
        /*std::cout << "Chromosome " << i << ": ";
        for (int idx : chromosome) {
            std::cout << idx << "(" << towns[idx].priority << ") ";
        }
        std::cout << std::endl;
        // Проверка валидности
        if (!is_valid_chromosome(chromosome, priority_groups)) {
            std::cerr << "Invalid chromosome in initial population at index " << i << ": ";
            for (int x : chromosome) std::cerr << x << " ";
            std::cerr << "\n";
            // Исправляем невалидную хромосому
            chromosome = std::vector<int>(towns.size(), -1);
            pos = 0;
            for (auto it = priority_groups.rbegin(); it != priority_groups.rend(); ++it) {
                std::vector<int> group = it->second;
                std::shuffle(group.begin(), group.end(), gen);
                for (int idx : group) {
                    chromosome[pos++] = idx;
                }
            }
            std::cerr << "Corrected chromosome: ";
            for (int x : chromosome) std::cerr << x << " ";
            std::cerr << "\n";
        }*/
    }
    return start_population;
}

double fitness_f(const std::vector<int>& individ, const std::vector<std::vector<double> >& matrix) {
    int mod = 1000000;
    double way_length = 0.0;
    for (int i = 0; i < individ.size(); i++) {
        if (individ[i] < 0 || individ[i] >= static_cast<int>(matrix.size())) {
            std::cerr << "Invalid index in chromosome: " << individ[i] << "\n";
            return 0.0; // Возвращаем минимальную стоимость для невалидной хромосомы
        }
    }
    std::set<int> seen(individ.begin(), individ.end());
    if (seen.size() != individ.size()) {
        std::cerr << "Invalid chromosome in fitness_f: ";
        for (int x : individ) std::cerr << x << " ";
        std::cerr << "\n";
        return 0.0; // Штраф за дубли
    }
    for (int i = 1; i < individ.size(); i++) {
        way_length += matrix[individ[i-1]][individ[i]];
    }
    way_length += matrix[individ.back()][individ[0]];
    return mod - way_length;
}


std::vector<int> tournament_selection(const std::vector<std::vector<int> >& population,
                                      const std::vector<std::vector<double> >& matrix, int tournament_size) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, population.size() - 1);

    int idx = dis(gen);
    std::vector<int> best = population[idx];
    double best_fitness = fitness_f(best, matrix);

    for (int i = 1; i < tournament_size; i++) {
        idx = dis(gen);
        std::vector<int> candidate = population[idx];
        double candidate_fitness = fitness_f(candidate, matrix);
        if (candidate_fitness > best_fitness) {
            best = candidate;
            best_fitness = candidate_fitness;
        }
    }
    std::cout << "Selected chromosome in tournament: ";
    for (int x : best) std::cout << x << " ";
    std::cout << "with fitness: " << best_fitness << "\n";
    return best;
}


std::vector<double> calculate_fitnesses(const std::vector<std::vector<int> >& population,
                                        const std::vector<std::vector<double> >& matrix, int population_size) {
    std::vector<double> fitnesses(population_size);
    for (int i = 0; i < population_size; i++) {
        const auto& chromosome = population[i];
        double distance = 0;
        bool valid = true;
        for (size_t j = 0; j < chromosome.size() - 1; j++) {
            if (chromosome[j] < 0 || chromosome[j] >= static_cast<int>(matrix.size()) ||
                chromosome[j + 1] < 0 || chromosome[j + 1] >= static_cast<int>(matrix.size())) {
                valid = false;
                break;
            }
            distance += matrix[chromosome[j]][chromosome[j + 1]];
        }
        if (valid && chromosome.size() > 0) {
            distance += matrix[chromosome.back()][chromosome[0]];
        }
        std::set<int> seen(chromosome.begin(), chromosome.end());
        if (seen.size() != chromosome.size()) {
            valid = false;
        }
        if (!valid) {
            fitnesses[i] = 0; // Штраф за невалидную хромосому
            std::cerr << "Invalid chromosome in calculate_fitnesses at index " << i << ": ";
            for (int x : chromosome) std::cerr << x << " ";
            std::cerr << "\n";
        } else {
            fitnesses[i] = 1000000 - distance; // Преобразуем расстояние в стоимость
        }
        std::cout << "Fitness for chromosome " << i << ": ";
        for (int x : chromosome) std::cout << x << " ";
        std::cout << " = " << fitnesses[i] << " (Distance: " << distance << ")\n";
    }
    return fitnesses;
}

void ox1_crossover(const std::vector<int>& parent_fir, const std::vector<int>& parent_sec,
                   std::vector<int>& child_fir, std::vector<int>& child_sec,
                   const std::map<int, std::vector<int> >& priority_groups,
                   double cross_prob) {
    child_fir = parent_fir;
    child_sec = parent_sec;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> prob(0, 1);

    if (prob(gen) > cross_prob) {
        std::cout << "Crossover skipped due to probability\n";
        return;
    }

    // Определяем диапазоны для каждой группы приоритетов
    std::map<int, std::pair<size_t, size_t> > group_ranges;
    size_t pos = 0;
    for (auto it = priority_groups.rbegin(); it != priority_groups.rend(); ++it) {
        group_ranges[it->first] = {pos, pos + it->second.size()};
        pos += it->second.size();
    }

    // Выполняем кроссовер для каждой группы приоритетов
    for (const auto& [priority, group] : priority_groups) {
        if (group.size() <= 1) continue;

        // Извлекаем подгруппы из родителей
        std::vector<int> group_fir, group_sec;
        auto [start_pos, end_pos] = group_ranges[priority];
        if (start_pos >= parent_fir.size() || end_pos > parent_fir.size()) {
            std::cerr << "Invalid group range for priority " << priority << ": [" << start_pos << ", " << end_pos << ")\n";
            return;
        }
        for (size_t i = start_pos; i < end_pos; i++) {
            group_fir.push_back(parent_fir[i]);
            group_sec.push_back(parent_sec[i]);
        }

        // Проверяем, что group_fir и group_sec содержат корректные индексы
        std::set<int> group_set(group.begin(), group.end());
        for (int idx : group_fir) {
            if (group_set.find(idx) == group_set.end()) {
                std::cerr << "Invalid index in group_fir for priority " << priority << ": " << idx << "\n";
                return;
            }
        }
        for (int idx : group_sec) {
            if (group_set.find(idx) == group_set.end()) {
                std::cerr << "Invalid index in group_sec for priority " << priority << ": " << idx << "\n";
                return;
            }
        }

        // Проверяем идентичность родителей
        bool identical = true;
        for (size_t i = 0; i < group.size(); i++) {
            if (group_fir[i] != group_sec[i]) {
                identical = false;
                break;
            }
        }

        // Создаём потомков
        std::vector<int> temp1(group.size()), temp2(group.size());
        if (identical) {
            // Если родители идентичны, создаём обратный порядок
            temp1 = group_fir;
            temp2 = group_fir;
            std::reverse(temp1.begin(), temp1.end());
            std::reverse(temp2.begin(), temp2.end());
            std::cout << "Parents identical for priority " << priority << ", reversing order\n";
        } else {
            // Выбираем точки кроссовера
            std::uniform_int_distribution<> dis(0, group.size() - 1);
            size_t start_idx = dis(gen);
            size_t end_idx = dis(gen);
            if (start_idx > end_idx) std::swap(start_idx, end_idx);

            std::vector<bool> used1(parent_fir.size(), false), used2(parent_fir.size(), false);

            // Копируем фрагмент от start_idx до end_idx
            for (size_t i = start_idx; i <= end_idx; i++) {
                temp1[i] = group_sec[i];
                temp2[i] = group_fir[i];
                used1[temp1[i]] = true;
                used2[temp2[i]] = true;
            }

            // Заполняем оставшиеся позиции из group_sec для temp1 и group_fir для temp2
            size_t pos_idx = 0;
            for (int idx : group_sec) {
                if (pos_idx >= group.size()) break;
                if (pos_idx >= start_idx && pos_idx <= end_idx) {
                    pos_idx = end_idx + 1;
                    continue;
                }
                if (!used1[idx]) {
                    temp1[pos_idx++] = idx;
                    used1[idx] = true;
                }
            }

            pos_idx = 0;
            for (int idx : group_fir) {
                if (pos_idx >= group.size()) break;
                if (pos_idx >= start_idx && pos_idx <= end_idx) {
                    pos_idx = end_idx + 1;
                    continue;
                }
                if (!used2[idx]) {
                    temp2[pos_idx++] = idx;
                    used2[idx] = true;
                }
            }
        }

        // Проверяем, что temp1 и temp2 заполнены корректно
        for (size_t i = 0; i < group.size(); i++) {
            if (temp1[i] < 0 || temp1[i] >= static_cast<int>(parent_fir.size()) || !group_set.count(temp1[i])) {
                std::cerr << "Error: temp1 contains invalid index " << temp1[i] << " for priority " << priority << "\n";
                return;
            }
            if (temp2[i] < 0 || temp2[i] >= static_cast<int>(parent_sec.size()) || !group_set.count(temp2[i])) {
                std::cerr << "Error: temp2 contains invalid index " << temp2[i] << " for priority " << priority << "\n";
                return;
            }
        }

        // Помещаем результаты обратно в потомков
        for (size_t i = 0; i < group.size(); i++) {
            child_fir[start_pos + i] = temp1[i];
            child_sec[start_pos + i] = temp2[i];
        }

        // Отладочный вывод для текущей группы
        /*std::cout << "After crossover for priority " << priority << ":\n";
        std::cout << "Group_fir: "; for (int x : group_fir) std::cout << x << " "; std::cout << "\n";
        std::cout << "Group_sec: "; for (int x : group_sec) std::cout << x << " "; std::cout << "\n";
        std::cout << "Temp1: "; for (int x : temp1) std::cout << x << " "; std::cout << "\n";
        std::cout << "Temp2: "; for (int x : temp2) std::cout << x << " "; std::cout << "\n";*/
    }

    // Отладочный вывод всей хромосомы
    /*std::cout << "After crossover:\n";
    std::cout << "Parent_fir: "; for (int x : parent_fir) std::cout << x << " "; std::cout << "\n";
    std::cout << "Parent_sec: "; for (int x : parent_sec) std::cout << x << " "; std::cout << "\n";
    std::cout << "Child_fir:  "; for (int x : child_fir) std::cout << x << " "; std::cout << "\n";
    std::cout << "Child_sec:  "; for (int x : child_sec) std::cout << x << " "; std::cout << "\n";*/
}

void mutate(std::vector<int>& individ, const std::map<int, std::vector<int> >& priority_groups, double mut_prob) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> prob(0, 1);

    // Определяем диапазоны для каждой группы приоритетов
    std::map<int, std::pair<size_t, size_t> > group_ranges;
    size_t pos = 0;
    for (auto it = priority_groups.rbegin(); it != priority_groups.rend(); ++it) {
        group_ranges[it->first] = {pos, pos + it->second.size()};
        pos += it->second.size();
    }

    // Выполняем мутацию для каждой группы приоритетов
    for (const auto& [priority, group] : priority_groups) {
        if (group.size() <= 1) continue;

        auto [start_pos, end_pos] = group_ranges[priority];
        if (prob(gen) < mut_prob) {
            // Выбираем две случайные позиции в пределах группы
            std::uniform_int_distribution<> dis(start_pos, end_pos - 1);
            size_t pos1 = dis(gen);
            size_t pos2 = dis(gen);
            while (pos1 == pos2) {
                pos2 = dis(gen);
            }
            // Меняем местами индексы в этих позициях
            std::swap(individ[pos1], individ[pos2]);
            std::cout << "Mutation in priority " << priority << ": swapped positions " << pos1 << " and " << pos2 << "\n";
            std::cout << "After mutation: ";
            for (int x : individ) std::cout << x << " ";
            std::cout << "\n";
        }
    }
}


int find_best_individ(const std::vector<double>& fitnesses) {
    int best_index = 0;
    double best_fitness = fitnesses[0];
    for (size_t i = 1; i < fitnesses.size(); i++) {
        if (fitnesses[i] > best_fitness) {
            best_fitness = fitnesses[i];
            best_index = i;
        }
    }
    std::cout << "Best fitness: " << best_fitness << " at index " << best_index << "\n";
    return best_index;
}



std::vector<double> Evolution(std::vector<Town>& towns, int population_size, int generations_number, double mut_prob, double cross_prob) {
    std::vector<std::vector<double> > matrix = calculate_distances(towns);
    std::map<int, std::vector<int> > priority_groups = make_priority_groups(towns);
    std::vector<std::vector<int> > population = make_start_population(towns, matrix, priority_groups, population_size);

    // Проверка начальной популяции
    for (size_t i = 0; i < population.size(); i++) {
        if (!is_valid_chromosome(population[i], priority_groups)) {
            std::cerr << "Invalid chromosome in initial population at index " << i << ": ";
            for (int x : population[i]) std::cerr << x << " ";
            std::cerr << "\n";
        }
    }

    std::vector<double> fitnesses = calculate_fitnesses(population, matrix, population_size);
    std::vector<std::vector<int> > best_individs(generations_number, std::vector<int>(towns.size(), -1));
    int best_index = find_best_individ(fitnesses);
    std::vector<double> best_fitnesses(generations_number);
    best_individs[0] = population[best_index];
    best_fitnesses[0] = fitnesses[best_index];

    std::cout << "Initial best fitness: " << best_fitnesses[0] << " for chromosome: ";
    for (int x : best_individs[0]) std::cout << x << " ";
    std::cout << "\n";

    for (int i = 1; i < generations_number; i++) {
        std::cout << "Generation " << i << "\n";
        std::vector<std::vector<int> > new_population(population_size, std::vector<int>(towns.size(), -1));
        int index = 0;

        if (index < population_size) {
            new_population[index] = population[best_index];
            std::cout << "Added best individual: ";
            for (int x : population[best_index]) std::cout << x << " ";
            std::cout << "\n";
            index++;
        }

        while (index < population_size) {
            std::vector<int> parent_first = tournament_selection(population, matrix, 5);
            std::vector<int> parent_second = tournament_selection(population, matrix, 5);
            std::vector<int> child_first(towns.size(), -1), child_second(towns.size(), -1);
            ox1_crossover(parent_first, parent_second, child_first, child_second, priority_groups, cross_prob);
            mutate(child_first, priority_groups, mut_prob);
            mutate(child_second, priority_groups, mut_prob);

            if (is_valid_chromosome(child_first, priority_groups) && index < population_size) {
                new_population[index] = child_first;
                std::cout << "Added child_first: ";
                for (int x : child_first) std::cout << x << " ";
                std::cout << "\n";
                index++;
            }
            if (is_valid_chromosome(child_second, priority_groups) && index < population_size) {
                new_population[index] = child_second;
                std::cout << "Added child_second: ";
                for (int x : child_second) std::cout << x << " ";
                std::cout << "\n";
                index++;
            }
            if (index < population_size) {
                new_population[index] = parent_first;
                std::cout << "Added parent_first: ";
                for (int x : parent_first) std::cout << x << " ";
                std::cout << "\n";
                index++;
            }
        }

        population = new_population;
        fitnesses = calculate_fitnesses(population, matrix, population_size);
        best_index = find_best_individ(fitnesses);
        best_individs[i] = population[best_index];
        best_fitnesses[i] = fitnesses[best_index];

        std::cout << "Best fitness in generation " << i << ": " << best_fitnesses[i] << " for chromosome: ";
        for (int x : best_individs[i]) std::cout << x << " ";
        std::cout << "\n";
    }

    print_matrix(best_individs);
    return best_fitnesses;
}

int main() {
    vector<Town> twns = console_input();
    std::vector<double> results = Evolution(twns, 50, 150, 0.1, 0.8);
    print_vector(results);
    return 0;
}
