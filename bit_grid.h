#pragma once
#include "types.h"
#include <iostream>

using namespace std;

class bit_grid
{
  public:
    bit_grid(fu32 _width = 1, fu32 _height = 1);
    bit_grid(const bit_grid & other);
    ~bit_grid();

    bool get(fu32 x, fu32 y) const;
    void set(fu32 x, fu32 y, bool value);
    void print(ostream & out = cout) const;
    const fu32 & get_height() const { return height; }
    const fu32 & get_width() const { return width; }
    bool within_bounds(fu32 x, fu32 y) const;

    bit_grid& operator=(const bit_grid & other);
  private:
    fu32 width;
    fu32 height;
    u8* data;
};
