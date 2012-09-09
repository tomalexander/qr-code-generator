#include "bit_grid.h"
#include <cmath>

bit_grid::bit_grid(fu32 _width, fu32 _height) :
    width(_width),
    height(_height)
{
    fu64 num_of_bits = width * height;
    fu64 num_of_bytes = ceil((double)num_of_bits / (double)8);
    data = new u8[num_of_bytes];
    for (fu64 i = 0; i < num_of_bytes; ++i)
        data[i] = 0;
}

bit_grid::~bit_grid()
{
    delete [] data;
}

bit_grid::bit_grid(const bit_grid & other):
    width(other.width),
    height(other.height)
{
    fu64 num_of_bits = width * height;
    fu64 num_of_bytes = ceil((double)num_of_bits / (double)8);
    data = new u8[num_of_bytes];
    for (fu64 i = 0; i < num_of_bytes; ++i)
        data[i] = other.data[i];
}

bool bit_grid::get(fu32 x, fu32 y) const
{
    fu64 bit_index = y + width*x;
    fu64 byte_index = floor(bit_index / 8);
    fu64 offset = bit_index % 8;
    u8 current_byte = data[byte_index];
    u8 mask = (0x1 << offset);
    return ((current_byte & mask) != 0);
}

void bit_grid::set(fu32 x, fu32 y, bool value)
{
    fu64 bit_index = y + width*x;
    fu64 byte_index = floor(bit_index / 8);
    fu64 offset = bit_index % 8;
    if (value)
    {
        data[byte_index] = data[byte_index] | (0x1 << offset);
    } else {
        u8 mask = 0xFF;
        mask ^= (0x1 << offset);
        data[byte_index] = data[byte_index] & mask;
    }
}

void bit_grid::print(ostream & out) const
{
    for (size_t y = 0; y < height; ++y)
    {
        for (size_t x = 0; x < width; ++x)
        {
            if (get(x,y))
                out << "1";
            else
                out << "0";
        }
        out << "\n";
    }
}

bool bit_grid::within_bounds(fu32 x, fu32 y) const
{
    return x <= get_width() && y <= get_height();
}

bit_grid& bit_grid::operator=(const bit_grid & other)
{
    width = other.width;
    height = other.height;

    fu64 num_of_bits = width * height;
    fu64 num_of_bytes = ceil((double)num_of_bits / (double)8);
    u8* tmp_data = new u8[num_of_bytes];
    for (fu64 i = 0; i < num_of_bytes; ++i)
        tmp_data[i] = other.data[i];

    delete [] data;
    
    data = tmp_data;
    return *this;
}
