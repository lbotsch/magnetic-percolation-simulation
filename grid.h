#ifndef GRID_H
#define GRID_H

#include <cassert>
#include <cstring>
#include <functional>
#include <forward_list>
#include <fstream>
#include <iostream>
#include <list>
#include <random>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>



class Grid
{
public:
  enum ProjectionType {PROJECT_GRID, PROJECT_DOMAINS, PROJECT_SPINS};
  enum GridType {GRID_SC, GRID_HEX};
  struct Dimensions
  {
    size_t X;
    size_t Y;
    size_t Z;
    size_t volume() const { return X*Y*Z; }
    size_t area() const { return X*Y; }
  };
  typedef std::function<std::forward_list<int>(int, const Grid::Dimensions&)> NeighborGenerator;
  
protected:
  GridType grid_type;
  double P;
  Dimensions dim;
  NeighborGenerator neighbor_generator;

  int seed = 0;
  std::mt19937 generator{seed};
  
  std::vector<bool> cells;
  std::vector<size_t> labels;
  std::unordered_map<size_t, size_t> label_sizes;
  size_t next_new_label = 1;
  std::list<std::list<int>> domains;

public:
  explicit Grid(double P, Dimensions dim, GridType grid_type=GRID_SC, int seed=0);
  ~Grid();
  void set_seed(int val) { seed = val; generator.seed(val); }
  void build();
  void update(double newP);

  void project_grid(std::vector<double> &out) const;
  void project_domains(std::vector<size_t> &out) const;
  void project_spins(std::vector<double> &out) const;
  std::vector<double> project_grid() const {
    std::vector<double> out;
    project_grid(out);
    return out;
  }
  std::vector<size_t> project_domains() const {
    std::vector<size_t> out;
    project_domains(out);
    return out;
  }
  std::vector<double> project_spins() const {
    std::vector<double> out;
    project_spins(out);
    return out;
  }

  GridType type() const { return grid_type; }
  double density() const { return P; }
  const Dimensions& dimensions() const { return dim; }
  size_t num_domains() const { return domains.size(); }
  size_t max_domain_len() const {
    size_t l = 0;
    for (auto& it : domains)
      if (it.size() > l) l = it.size();
    return l;
  }
  double avg_domain_len() const {
    size_t l = 0;
    for (auto& it : domains)
      l += it.size();
    return (double)l / (double)domains.size();
  }
protected:
  void search_domains();
};

std::forward_list<int> generate_neighbors_SC(int i, const Grid::Dimensions &dim);
std::forward_list<int> generate_neighbors_Hex(int i, const Grid::Dimensions &dim);


#endif
