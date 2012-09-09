#pragma once
#include "types.h"
#include "bit_grid.h"
#include <vector>

using namespace std;

class bitmap
{
  public:
    bitmap(const bit_grid & _grid, fu8 scale=1);
    ~bitmap();

    void write_to_file(const string & name);
  private:
    fs32 pixel_width;
    fs32 pixel_height;
    bit_grid grid;
};
