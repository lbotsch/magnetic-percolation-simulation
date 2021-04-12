#include <cassert>
#include <chrono>
#include <cstring>
#include <functional>
#include <future>
#include <fstream>
#include <iostream>
#include <list>
#include <memory>
#include <random>
#include <set>
#include <string>
#include <vector>

using namespace std;

static const int Nthreads = 8;

static const bool DEBUG = false;
static const bool INFO = false;

#define I                 \
  if (!INFO && !DEBUG) {} \
  else cerr

#define D        \
  if (!DEBUG) {} \
  else cerr

#include "grid.h"

struct SimulationParams
{
  size_t L;
  size_t T;
  double P;
  int Niter;
  int Ngrids;
  int seed;
  Grid::GridType grid_type;
};

struct SimulationResults
{
  double P = 0.0;
  double avg_num_domains = 0.0;
  double std_num_domains = 0.0;
  double avg_max_domain_size = 0.0;
  double std_max_domain_size = 0.0;
  double avg_mean_domain_size = 0.0;
  double std_mean_domain_size = 0.0;
  double avg_moment = 0.0;
  double std_moment = 0.0;
  double avg_magnetization = 0.0;
  double std_magnetization = 0.0;
};

SimulationResults simulate(SimulationParams params)
{
  Grid grid(params.P, {params.L, params.L, params.T}, params.grid_type);
  SimulationResults res;
  res.P = params.P;

  vector<double> nds(params.Ngrids);
  vector<double> mds(params.Ngrids);
  vector<double> ads(params.Ngrids);
  vector<double> ms(params.Ngrids);
  vector<double> Ms(params.Ngrids);

  auto t_simulation_start = chrono::high_resolution_clock::now();
  for (int i = 0; i < params.Ngrids; ++i) {
    auto t_grid_start = chrono::high_resolution_clock::now();
    auto t0 = chrono::high_resolution_clock::now();
    grid.build();
    I << "  - build took "
      << chrono::duration_cast<chrono::duration<double>>(chrono::high_resolution_clock::now()-t0).count()
      << " s." << endl;
    t0 = chrono::high_resolution_clock::now();
    I << "  - calc_magnetization took "
      << chrono::duration_cast<chrono::duration<double>>(chrono::high_resolution_clock::now()-t0).count()
      << " s." << endl;

    nds[i] = (double)grid.num_domains();
    res.avg_num_domains += (double)grid.num_domains();
    mds[i] = (double)grid.max_domain_len();
    res.avg_max_domain_size += (double)grid.max_domain_len();
    ads[i] = grid.avg_domain_len();
    res.avg_mean_domain_size += grid.avg_domain_len();
    I << "Grid " << i+1 << "/" << params.Ngrids << "("
      << chrono::duration_cast<chrono::duration<double>>(chrono::high_resolution_clock::now()-t_grid_start).count()
      << " s - "
      << chrono::duration_cast<chrono::duration<double>>(chrono::high_resolution_clock::now()-t_simulation_start).count()
      << " s total)" << endl;
  }
  res.avg_num_domains /= params.Ngrids;
  res.avg_max_domain_size /= params.Ngrids;
  res.avg_mean_domain_size /= params.Ngrids;
  /*
  res.avg_moment /= params.Ngrids;
  res.avg_magnetization /= params.Ngrids;
  */

  for (int i = 0; i < params.Ngrids; ++i) {
    res.std_num_domains += abs(nds[i]-res.avg_num_domains);
    res.std_max_domain_size += abs(mds[i]-res.avg_max_domain_size);
    res.std_mean_domain_size += abs(ads[i]-res.avg_mean_domain_size);
    /*
    res.std_moment += abs(ms[i]-res.avg_moment);
    res.std_magnetization += abs(Ms[i]-res.avg_magnetization);
    */
  }
  res.std_num_domains /= params.Ngrids;
  res.std_max_domain_size /= params.Ngrids;
  res.std_mean_domain_size /= params.Ngrids;
  /*
  res.std_moment /= params.Ngrids;
  res.std_magnetization /= params.Ngrids;
  */

  return res;
}

void print_usage(const char *progname)
{
  cerr << "Usage: " << progname << " L T P N GRID" << endl;
  cerr << "  L: Lateral grid dimension" << endl;
  cerr << "  T: Grid thickness (in z-direction)" << endl;
  cerr << "  P: Number of defect density steps between (0,1)" << endl;
  cerr << "  N: Number of grids to simulate" << endl;
  cerr << "  GRID: One of \"sc\", \"hex\"" << endl;
}

int main(int argc, char **argv)
{
  SimulationParams params;
  params.Ngrids = 10;
  params.Niter = 100;
  params.grid_type = Grid::GRID_SC;
  size_t Psteps;

  if (argc <= 3) {
    print_usage(argv[0]);
    return 1;
  }

  params.L = atoi(argv[1]);
  params.T = atoi(argv[2]);
  Psteps = atoi(argv[3]);

  if (argc > 4)
    params.Ngrids = atoi(argv[4]);
  if (argc > 5) {
    string s(argv[5]);
    if (s == "hex") params.grid_type = Grid::GRID_HEX;
    else if (s != "sc") {
      print_usage(argv[0]);
      return 1;
    }
  }

  // Output csv header
  cout << "Defect Probability,Domain Count (AVG),Domain Count (STD),Max Domain Size (AVG),Max Domain Size (STD),Mean Domain Size (AVG),Mean Domain Size (STD)" << endl;


  future<SimulationResults> results[Nthreads];
  size_t step = 1;
  for (size_t t = 0; t < Nthreads && step+t < Psteps; t++) {
    params.P = (double)(step+t)/(double)Psteps;
    results[t] = async(launch::async, simulate, params);
  }
  while(step < Psteps) {
    for (size_t t = 0; t < Nthreads && step+t < Psteps; t++) {
      auto res = results[t].get();
      if (step+Nthreads+t < Psteps) {
        params.P = (double)(step+Nthreads+t)/(double)Psteps;
        results[t] = async(launch::async, simulate, params);
      }
      cout << res.P
           << "," << res.avg_num_domains
           << "," << res.std_num_domains
           << "," << res.avg_max_domain_size
           << "," << res.std_max_domain_size
           << "," << res.avg_mean_domain_size
           << "," << res.std_mean_domain_size
           << endl;
    }
    step += Nthreads;
  }

  return 0;
}
