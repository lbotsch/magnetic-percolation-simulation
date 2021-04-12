#include <algorithm>
#include "grid.h"

using namespace std;

#define TO_1D(x,y,z) ((z)+(y)*dim.Z+(x)*dim.Y*dim.Z)
#define X_FROM_1D(i) ((i) / (dim.Y*dim.Z))
#define Y_FROM_1D(i) (((i) / dim.Z) % dim.Y)
#define Z_FROM_1D(i) ((i) % dim.Z)

#define FOR3(x,y,z)                                 \
  for (size_t x = 0; x < dim.X; ++x)                   \
    for (size_t y = 0; y < dim.Y; ++y)                 \
      for (size_t z = 0; z < dim.Z; ++z)

static const double UC_X = 3.7845e-8;
static const double UC_Y = 3.7845e-8;
static const double UC_Z = 9.5143e-8;
static const double MU_B = 9.274e-21; // emu

static int MU_B_PER_CELL = 1;


Grid::Grid(double P, Grid::Dimensions dim, GridType grid_type, int seed)
  : grid_type(grid_type), P(P), dim(dim), seed(seed), generator(seed),
    cells(dim.volume(), 0), labels(dim.volume(), 0), domains()
{
  switch(grid_type) {
  case GRID_SC:
    neighbor_generator = generate_neighbors_SC;
    break;
  case GRID_HEX:
    neighbor_generator = generate_neighbors_Hex;
    break;
  }
}

Grid::~Grid()
{
}

void Grid::search_domains()
{
  vector<bool> visited(cells.size(), false);
  domains.clear();
  //fill(labels.begin(), labels.end(), 0);

  FOR3(x,y,z) {
    int i = TO_1D(x,y,z);
    forward_list<int> queue;
    if (!visited[i] && cells[i]) {
      list<int> domain;
      size_t cur_label;
      size_t biggest_domain_size;

      queue.push_front(i);
      visited[i] = true;
      if (labels[i] != 0) {
        // The cell i was already labeled in the original grid
        cur_label = labels[i];
        biggest_domain_size = label_sizes[labels[i]];
        label_sizes.erase(cur_label);
      } else {
        cur_label = next_new_label;
        biggest_domain_size = 1;
        labels[i] = cur_label;
      }
      domain.push_back(i);

      while(!queue.empty()) {
        i = queue.front();
        queue.pop_front();

        auto neighbors = neighbor_generator(i, dim);
        for (auto ni : neighbors) {
          if (!visited[ni] && cells[ni]) {
            queue.push_front(ni);
            visited[ni] = true;
            if (labels[i] != 0 && labels[i] != cur_label) {
              // We reached another domain. We keep the label of the bigger
              // domain and merge them together.
              if (label_sizes[labels[i]] > biggest_domain_size) {
                cur_label = labels[i];
                biggest_domain_size = label_sizes[labels[i]];
              }
              // Either the label will not be used anymore, or we update the
              // size at the end. So we don't need the entry anymore.
              label_sizes.erase(labels[i]);
            }
            labels[ni] = cur_label;
            domain.push_back(ni);
          }
        }
      }

      // If a completely new label was used, increment the next label used
      if (cur_label == next_new_label) next_new_label++;

      // Relabel all cells of the domain and update the label size
      for (auto i : domain) labels[i] = cur_label;
      label_sizes[cur_label] = domain.size();

      domains.push_back(domain);
    }
  }
}

void Grid::build()
{
  fill(cells.begin(), cells.end(), false);

  // randomly distribute defects
  vector<int> candidates;
  candidates.reserve(dim.volume());
  list<int> defects;
  for (size_t i = 0; i < dim.volume(); ++i) candidates.push_back(i);
  sample(candidates.begin(), candidates.end(), back_inserter(defects),
         static_cast<size_t>(dim.volume()*P), generator);
  for (auto i : defects) cells[i] = true;

  search_domains();
}

void Grid::update(double newP)
{
  assert(newP >= P);
  if (newP == P) return;

  // randomly distribute defects
  vector<int> candidates;
  candidates.reserve(dim.volume());
  list<int> defects;
  for (size_t i = 0; i < dim.volume(); ++i) if (!cells[i]) candidates.push_back(i);
  size_t sample_size = static_cast<size_t>(dim.volume()*newP - dim.volume()*P);
  //assert(sample_size <= candidates.size());
  sample(candidates.begin(), candidates.end(), back_inserter(defects),
         sample_size, generator);
  for (auto i : defects) cells[i] = true;
  P = newP;
  search_domains();
}

forward_list<int> generate_neighbors_SC(int i, const Grid::Dimensions &dim) {
  forward_list<int> neighbors;
  int x = X_FROM_1D(i), y = Y_FROM_1D(i), z = Z_FROM_1D(i);
  neighbors.push_front(TO_1D((dim.X+x-1)%dim.X, y, z));
  neighbors.push_front(TO_1D((dim.X+x+1)%dim.X, y, z));
  neighbors.push_front(TO_1D(x, (dim.Y+y-1)%dim.Y, z));
  neighbors.push_front(TO_1D(x, (dim.Y+y+1)%dim.Y, z));
  neighbors.push_front(TO_1D(x, y, (dim.Z+z-1)%dim.Z));
  neighbors.push_front(TO_1D(x, y, (dim.Z+z+1)%dim.Z));
  return neighbors;
}

forward_list<int> generate_neighbors_Hex(int i, const Grid::Dimensions &dim)
{
  forward_list<int> neighbors;
  int x = X_FROM_1D(i), y = Y_FROM_1D(i), z = Z_FROM_1D(i);
  neighbors.push_front(TO_1D((dim.X+x-1)%dim.X, y, z));
  neighbors.push_front(TO_1D((dim.X+x+1)%dim.X, y, z));
  neighbors.push_front(TO_1D((dim.X+x-(y%2))%dim.X, (dim.Y+y-1)%dim.Y, z));
  neighbors.push_front(TO_1D((dim.X+x-(y%2)+1)%dim.X, (dim.Y+y-1)%dim.Y, z));
  neighbors.push_front(TO_1D((dim.X+x-(y%2))%dim.X, (dim.Y+y+1)%dim.Y, z));
  neighbors.push_front(TO_1D((dim.X+x-(y%2)+1)%dim.X, (dim.Y+y+1)%dim.Y, z));
  neighbors.push_front(TO_1D(x, y, (dim.Z+z-1)%dim.Z));
  neighbors.push_front(TO_1D(x, y, (dim.Z+z+1)%dim.Z));
  return neighbors;
}

void Grid::project_grid(vector<double> &out) const
{
  if (out.size() != dim.area()) out.resize(dim.area(), 0.0);
  for (size_t x = 0; x < dim.X; x++) {
    for (size_t y = 0; y < dim.Y; y++) {
      size_t i = x*dim.Y+y;
      out[i] = 0.0;
      for (size_t z = 0; z < dim.Z; z++) {
        out[i] += (double)cells[TO_1D(x,y,z)];
      }
      out[i] /= (double)dim.Z;
    }
  }
}

void Grid::project_domains(vector<size_t> &out) const
{
  if (out.size() != dim.area()) out.resize(dim.area(), 0.0);
  vector<int> zindex(dim.area(), 0);
  int nd = 1;
  for (auto& domain : domains) {
    for (auto i : domain) {
      int idx = i / dim.Z;
      int z = Z_FROM_1D(i);
      if (zindex[idx] <= z) {
        zindex[idx] = z;
        out[idx] = labels[i];
      }
    }
    nd++;
  }
}

void Grid::project_spins(vector<double> &out) const
{
  mt19937 rng(seed);
  if (out.size() != dim.area()) out.resize(dim.area(), 0.0);
  uniform_int_distribution<int> spin_dist(0,1);

  for (auto& domain : domains) {
    int s = spin_dist(rng) ? 1 : -1;
    for (auto i : domain) {
      out[i / dim.Z] += (double)s;
    }
  }
  for (size_t i = 0; i < out.size(); i++)
    out[i] /= (double)dim.Z;
}
