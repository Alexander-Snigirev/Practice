#ifndef TSP_H
#define TSP_H

#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <random>
#include "town.h"
#include "rapidcsv.h"


void print_lst(std::vector<Town> towns);
void print_matrix(std::vector<std::vector<double>> matrix);
void print_matrix(std::vector<std::vector<int>> matrix);
void print_dvector(std::vector<double>& vec);
void print_vector(std::vector<int>& vec);
void print_priority_groups(const std::map<int, std::vector<int>>& priority_groups);
void print_priority_groups_with_ranges(const std::map<int, std::vector<int>>& priority_groups);
std::vector<Town> console_input();
std::vector<std::vector<double>> calculate_distances(std::vector<Town> towns);
std::map<int, std::vector<int>> make_priority_groups(std::vector<Town>& towns);
std::vector<std::vector<int>> make_start_population(std::vector<Town>& towns,
                                                    std::vector<std::vector<double>>& distances,
                                                    std::map<int, std::vector<int>> priority_groups, int p_size);
double fitness_f(std::vector<int>& individ, std::vector<std::vector<double>>& matrix);
std::vector<int> tournament_selection(const std::vector<std::vector<int>>& population,
                                      const std::vector<double>& fitnesses, int k);
std::vector<double> calculate_fitnesses(std::vector<std::vector<int>>& population,
                                        std::vector<std::vector<double>>& matrix, int p_size);
double calculate_var_len(std::vector<double>& fitnesses);
bool is_valid_chromosome(const std::vector<int>& individ,
                         const std::map<int, std::vector<int>>& priority_groups);
void group_crossover(const std::vector<int>& group1, const std::vector<int>& group2,
                     std::vector<int>& child1, std::vector<int>& child2,
                     std::mt19937& gen);
void merge_child_groups(const std::vector<std::vector<int>>& child_fir_groups,
                        const std::vector<std::vector<int>>& child_sec_groups,
                        std::vector<int>& child_fir,
                        std::vector<int>& child_sec);
void ox1_crossover(std::vector<int>& parent_fir, std::vector<int>& parent_sec,
                   std::vector<int>& child_fir, std::vector<int>& child_sec,
                   const std::map<int, std::vector<int>>& priority_groups,
                   double cross_prob);
void mutate(std::vector<int>& individ,
            const std::map<int, std::vector<int>>& priority_groups,
            double mutation_prob);
int find_best_individ(std::vector<double>& fitnesses);
std::vector<double> Evolution(std::vector<Town>& towns, int population_size,
                              int generations_number, double mut_prob, double cross_prob, const std::string& filename);
void save_to_csv(const std::vector<double>& best_lens, const std::vector<double>& var_lens,
                 const std::vector<std::vector<int>>& best_individs, const std::string& filename);
double calculateFitness(const std::vector<int>& path, const std::vector<Town>& towns);

#endif