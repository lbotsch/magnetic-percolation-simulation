#include <string>
#include <iostream>

#include "grid.h"
#include "graphics.h"

using namespace std;

int main()
{
  cerr << "Building grid..." << endl;
  Grid grid(0.5, {200,200,1}, Grid::GRID_SC);
  grid.build();
  cerr << "Done. Drawing domains..." << endl;
  draw_domains("test_grid.png", grid, 1920, 1080);
  cerr << "Done. Saved output to \"test_grid.png\"" << endl;
  return 0;
}
