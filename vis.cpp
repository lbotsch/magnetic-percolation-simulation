#include <cassert>
#include <cstring>
#include <functional>
#include <fstream>
#include <iostream>
#include <list>
#include <random>
#include <set>
#include <string>
#include <vector>

using namespace std;

static const bool DEBUG = false;
static const bool INFO = false;

#define VIDEO_FPS 8


#include "grid.h"
#include "graphics.h"

struct SimulationParams
{
  size_t L;
  size_t T;
  size_t Psteps;
  Grid::ProjectionType projection_type;
  Grid::GridType grid_type;
};

void vis(SimulationParams params, const string &base_path, size_t img_width, size_t img_height)
{
  double step = 1.0/(double)params.Psteps;
  double P = 0.0;
  Grid grid(P, {params.L, params.L, params.T}, params.grid_type);
  grid.build();

  int counter = 0;
  while(P <= 1.0) {
    Image frame;
    string filename(base_path + "_" + to_string(counter++) + ".png");
    switch(params.projection_type) {
    case Grid::PROJECT_GRID:
    default: {
      frame = draw_grid(filename, grid, img_width, img_height);
      break;
    }
    case Grid::PROJECT_DOMAINS: {
      frame = draw_domains(filename, grid, img_width, img_height);
      break;
    }
    case Grid::PROJECT_SPINS: {
      frame = draw_spins(filename, grid, img_width, img_height);
      break;
    }
    }

    P += step;
    grid.update(P);
  }
}

void print_usage(const char *progname)
{
  cerr << "Usage: " << progname << " L T P PROJ GRID PATH" << endl;
  cerr << "  L: Lateral grid dimension" << endl;
  cerr << "  T: Grid thickness (in z-direction)" << endl;
  cerr << "  P: Number of defect density steps between (0,1)" << endl;
  cerr << "  PROJ: One of \"grid\", \"domains\", \"spins\"" << endl;
  cerr << "  GRID: One of \"sc\", \"hex\"" << endl;
  cerr << "  PATH: Base path for output files" << endl;
}

int main(int argc, char **argv)
{
  size_t img_width = 1920, img_height = 1080;
  SimulationParams params;
  params.projection_type = Grid::PROJECT_GRID;
  params.grid_type = Grid::GRID_SC;

  if (argc <= 3) {
    print_usage(argv[0]);
    return 1;
  }

  params.L = atoi(argv[1]);
  params.T = atoi(argv[2]);
  params.Psteps = atoi(argv[3]);

  string base_path("data/domains_");
  base_path += argv[1];
  base_path += "x";
  base_path += argv[1];
  base_path += "x";
  base_path += argv[2];
  base_path += "_";

  if (argc > 4) {
    string s(argv[4]);
    if (s == "domains") params.projection_type = Grid::PROJECT_DOMAINS;
    else if (s == "spins") params.projection_type = Grid::PROJECT_SPINS;
    else if (s != "grid") {
      cerr << "Error: Projection type " << s << " is unknown!" << endl;
      print_usage(argv[0]);
      return 1;
    }
  }

  if (argc > 5) {
    string s(argv[5]);
    if (s == "hex") params.grid_type = Grid::GRID_HEX;
    else if (s != "sc") {
      cerr << "Error: Grid type " << s << " is unknown!" << endl;
      print_usage(argv[0]);
      return 1;
    }
  }

  if (argc > 6)
    base_path = string(argv[6]);

  init_video_lib();

  vis(params, base_path, img_width, img_height);

  return 0;
}
