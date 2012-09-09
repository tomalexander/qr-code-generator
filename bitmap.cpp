#include "bitmap.h"
#include <fstream>

using namespace std;
#define BLACK true
#define WHITE false

namespace
{
    vector<u8> get_4_bytes(const u32 & data)
    {
        vector<u8> ret;
        u8* tmp = (u8*)&data;
        ret.push_back(tmp[0]);
        ret.push_back(tmp[1]);
        ret.push_back(tmp[2]);
        ret.push_back(tmp[3]);
        return ret;
    }
    void insert_4_bytes(vector<u8> & dest, const u32 & data)
    {
        vector<u8> separated_data = get_4_bytes(data);
        for (u8 cur : separated_data)
            dest.push_back(cur);
        //copy(separated_data.begin(), separated_data.end(), back_inserter(dest));
    }
    vector<u8> get_2_bytes(const u16 & data)
    {
        vector<u8> ret;
        u8* tmp = (u8*)&data;
        ret.push_back(tmp[0]);
        ret.push_back(tmp[1]);
        return ret;
    }
    void insert_2_bytes(vector<u8> & dest, const u16 & data)
    {
        vector<u8> separated_data = get_2_bytes(data);
        for (u8 cur : separated_data)
            dest.push_back(cur);
        //copy(separated_data.begin(), separated_data.end(), back_inserter(dest));
    }

    void print_bytes(const vector<u8> & data, ostream & out = cout)
    {

        for (u8 cur : data)
        {
            char buffer[4];
            sprintf(buffer, "%x", cur);
            if (cur > 0x10)
                out << buffer;
            else
                out << "0" << buffer;
        }
        out << "\n";
    }
}

bitmap::bitmap(const bit_grid & _grid, fu8 scale) : grid(_grid)
{
    pixel_width = grid.get_width() * scale;
    pixel_height = grid.get_height() * scale;
}

bitmap::~bitmap()
{

}

void bitmap::write_to_file(const string & name)
{
    vector<u8> data;
    data.push_back(0x42);
    data.push_back(0x4D);
    size_t file_size_offset = data.size();
    insert_4_bytes(data, 0xFFFFFFFF); //File Size, fill later
    data.push_back(0x00);
    data.push_back(0x00);
    data.push_back(0x00);
    data.push_back(0x00);
    size_t pixel_info_offset_offset = data.size();
    insert_4_bytes(data, 0); //pixel info offset, fill later
    insert_4_bytes(data, 40);
    insert_4_bytes(data, pixel_width);
    insert_4_bytes(data, pixel_height);
    insert_2_bytes(data, 1);
    insert_2_bytes(data, 24);
    insert_4_bytes(data, 0);
    size_t raw_pixel_array_size_offset = data.size();
    insert_4_bytes(data, 0); //size of raw data in pixel array, fill later
    insert_4_bytes(data, 2835);
    insert_4_bytes(data, 2835);
    insert_4_bytes(data, 0);
    insert_4_bytes(data, 0);
    {
        u32 data_size = data.size();
        memcpy(&data[pixel_info_offset_offset], &data_size, 4);
    }
    u32 size_of_header = data.size();
    for (fu32 y = 0; y < grid.get_height(); ++y)
    {
        for (fu32 x = 0; x < grid.get_width(); ++x)
        {
            // data.push_back(0xFF); //B
            // data.push_back(0xFF); //G
            // data.push_back(0xFF); //R
            if (grid.get(x,grid.get_height()-1-y))
            {
                data.push_back(0);
                data.push_back(0);
                data.push_back(0);
            } else {
                data.push_back(0xFF);
                data.push_back(0xFF);
                data.push_back(0xFF);
            }
        }
        while ((data.size() - size_of_header)%4)
        {
            data.push_back(0);
        }
    }
    {
        u32 file_size = data.size();
        memcpy(&data[file_size_offset], &file_size, 4);
    }
    {
        u32 pixel_data_size = data.size() - size_of_header;
        memcpy(&data[raw_pixel_array_size_offset], &pixel_data_size, 4);
    }
    

    ofstream output;
    output.open(name);
    output.write((const char*)&data[0], data.size());
    output.close();
}

