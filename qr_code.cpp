#include "qr_code.h"
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <functional>
#include "polynomial.h"

using namespace std;
#define BLACK true
#define WHITE false
#define DOWN false
#define UP true

namespace
{
    void fill_log_arrays();
    fu32 calculate_penalty_score(const bit_grid & grid);
}

qr_code::qr_code(const string & input, fu8 _mode, fu8 _version, correction_level _error):
    encoding_string(input),
    mode(_mode),
    version(_version),
    error(_error)
{
    fill_log_arrays();
    vector<u8> data = generate_binary_string();
    fu8 best_mask_pattern = 0;
    fu32 best_penalty_score = maxfu32;
    bit_grid best_grid;
    for (fu8 i = 0; i < 1; ++i)
    {
        bit_grid grid = generate_bit_grid(data, i);
        fu32 penalty = calculate_penalty_score(grid);
        cout << "Mask " << (int)i << " Penalty: " << penalty << "\n";
        if (penalty <= best_penalty_score)
        {
            best_mask_pattern = i;
            best_penalty_score = penalty;
            best_grid = grid;
        }
    }
    mask_pattern = best_mask_pattern;
    generated_grid = best_grid;
    generated_grid.print();
    cout << "Chose mask pattern: " << (int)mask_pattern << "\n";
}

qr_code::~qr_code()
{
    
}

fu8 qr_code::get_image_dimension()
{
    return 17 + 4*version;
}

#define GF 256 // define the Size & Prime Polynomial of this Galois field 
#define PP 285
//anti_log_table[exponent_of_a] = integer
//log_table[integer] = exponent_of_a
u8 log_table[GF], anti_log_table[GF];

namespace
{
    void fill_log_arrays() {
        log_table[0] = (u8)(1-GF);
        anti_log_table[0] = 1; 
        for (fu16 i=1; i<GF; i++)
        {
            fu16 tmp = anti_log_table[i-1] * 2;
            if (tmp >= GF)
                tmp ^= PP; 
            anti_log_table[i] = (u8)tmp; 
            log_table[anti_log_table[i]] = i; 
        }
        log_table[1] = 0;
    } 

    
    void add_bits(const fu64 & value, const fu64 & length, vector<bool> & destination)
    {
        fu64 current_value = value;
        for (fu64 i = 0; i < length; ++i)
        {
            fu64 real_index = length - i -1;
            fu64 power = pow(2, real_index);
            if (current_value >= power)
            {
                destination.push_back(true);
                current_value -= power;
            } else {
                destination.push_back(false);
            }
        }
    }

    void print_bits(const vector<bool> & source, ostream & out = cout)
    {
        for (const bool & cur : source)
            out << (cur ? "1" : "0");
        out << "\n";
    }

    fu8 get_alphanumeric_value(const char input)
    {
        const string dictionary = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ $%*+-./:";
        for (fu8 i = 0; i < dictionary.size(); ++i)
            if (dictionary[i] == input)
                return i;
        cerr << "Character " << input << " is not a valid alphanumeric value!\n";
        exit(EXIT_FAILURE);
        return -1;
    }

    vector<u8> bits_to_bytes(const vector<bool> & bits)
    {
        vector<u8> bytes;
        if (bits.size() % 8 != 0)
        {
            cerr << "bits_to_bytes called without a multiple of 8\n";
            exit(EXIT_FAILURE);
        }
        for (fu64 i = 0; i < bits.size(); i += 8)
        {
            u8 cur = 0;
            for (u8 x = 0; x < 8; ++x)
                if (bits[i+x])
                    cur += 1<<(7-x);
            bytes.push_back(cur);
        }
        return bytes;
    }

    vector<bool> bytes_to_bits(const vector<u8> & bytes)
    {
        vector<bool> bits;
        for (const u8 & cur : bytes)
            add_bits(cur, 8, bits);
        return bits;
    }

    void advance_to_next_position(fs16 & x, fs16 & y, bool & direction, bool & move_left_next, const bit_grid & no_fly_zone)
    {
        if (move_left_next)
        {
            --x;
            move_left_next = false;
        } else {
            ++x;
            y += (direction == UP ? -1 : 1);
            move_left_next = true;
        }
        if (y == -1)
        {
            x -= 2;
            y = 0;
            direction = DOWN;
        }
        else if (y == no_fly_zone.get_height())
        {
            x -= 2;
            y = no_fly_zone.get_height()-1;
            direction = UP;
        }
        
        while ((no_fly_zone.get(x,y)))
        {
            y += (direction == UP ? -1 : 1);
            if (y == -1)
            {
                x -= 2;
                y = 0;
                direction = DOWN;
            }
            else if (y == no_fly_zone.get_height())
            {
                x -= 2;
                y = no_fly_zone.get_height()-1;
                direction = UP;
            }
            if (x == 6)
                x = 5;
        }
    }

    fu32 calculate_penalty_score(const bit_grid & grid)
    {
        fu32 ret = 0;
        //Penalty rule 1
        //if 5 of same color are in row or column = 3
        //+1 for each additional in that row or column
        //Test horizontal
        for (fu16 y = 0; y < grid.get_height(); ++y)
        {
            u8 in_a_row = 0;
            bool current_color = BLACK;
            for (fu16 x = 0; x < grid.get_width(); ++x)
            {
                if (grid.get(x,y) == current_color)
                {
                    in_a_row++;
                    if (in_a_row == 5)
                        ret += 3;
                    else if (in_a_row > 5)
                        ret += 1;
                } else {
                    in_a_row = 1;
                    current_color = grid.get(x,y);
                }
            }
        }
        //Test Vertical
        for (fu16 x = 0; x < grid.get_width(); ++x)
        {
            u8 in_a_row = 0;
            bool current_color = BLACK;
            for (fu16 y = 0; y < grid.get_height(); ++y)
            {
                if (grid.get(x,y) == current_color)
                {
                    in_a_row++;
                    if (in_a_row == 5)
                        ret += 3;
                    else if (in_a_row > 5)
                        ret += 1;
                } else {
                    in_a_row = 1;
                    current_color = grid.get(x,y);
                }
            }
        }

        //Penalty rule 2
        //3 points for every 2x2 block of the same color
        for (fu16 y = 0; y < grid.get_height(); ++y)
            for (fu16 x = 0; x < grid.get_width(); ++x)
                if (grid.within_bounds(x+1,y+1) && grid.get(x,y) == grid.get(x+1,y) && grid.get(x,y) == grid.get(x,y+1) && grid.get(x,y) == grid.get(x+1,y+1))
                    ret += 3;
        
        //Penality rule 3
        //horizontal
        for (fu16 y = 0; y < grid.get_height(); ++y)
        {
            for (fu16 x = 0; x < grid.get_width(); ++x)
            {
                vector<bool> search_for_after = {WHITE, WHITE, WHITE, WHITE, BLACK, WHITE, BLACK, BLACK, BLACK, WHITE, BLACK};
                vector<bool> search_for_prior = {BLACK, WHITE, BLACK, BLACK, BLACK, WHITE, BLACK, WHITE, WHITE, WHITE, WHITE};
                bool found_after = true;
                bool found_prior = true;
                for (fu8 i = 0; i < 11; ++i)
                {
                    if (!grid.within_bounds(x+i,y) || grid.get(x+i,y) != search_for_after.back())
                    {
                        found_after = false;
                    }
                    if (!grid.within_bounds(x+i,y) || grid.get(x+i,y) != search_for_prior.back())
                    {
                        found_prior = false;
                    }
                    if (!found_after && !found_prior)
                        break;
                    search_for_after.pop_back();
                    search_for_prior.pop_back();
                }
                if (found_after || found_prior)
                    ret += 40;
            }
        }
        //vertical
        for (fu16 x = 0; x < grid.get_width(); ++x)
        {
            for (fu16 y = 0; y < grid.get_height(); ++y)
            {
                vector<bool> search_for_after = {WHITE, WHITE, WHITE, WHITE, BLACK, WHITE, BLACK, BLACK, BLACK, WHITE, BLACK};
                vector<bool> search_for_prior = {BLACK, WHITE, BLACK, BLACK, BLACK, WHITE, BLACK, WHITE, WHITE, WHITE, WHITE};
                bool found_after = true;
                bool found_prior = true;
                for (fu8 i = 0; i < 11; ++i)
                {
                    if (!grid.within_bounds(x,y+i) || grid.get(x,y+i) != search_for_after.back())
                    {
                        found_after = false;
                    }
                    if (!grid.within_bounds(x,y+i) || grid.get(x,y+i) != search_for_prior.back())
                    {
                        found_prior = false;
                    }
                    if (!found_after && !found_prior)
                        break;
                    search_for_after.pop_back();
                    search_for_prior.pop_back();
                }
                if (found_after || found_prior)
                    ret += 40;
            }

        }

        //Penalty rule 4
        //Black/White ratio
        fu32 num_of_dark = 0;
        fu32 num_total = grid.get_width() * grid.get_height();
        for (fu16 y = 0; y < grid.get_height(); ++y)
            for (fu16 x = 0; x < grid.get_width(); ++x)
                if (grid.get(x,y) == BLACK)
                    ++num_of_dark;
        double percent = ((double)(num_of_dark))/((double)(num_total));
        percent *= 100;
        percent -= 50;
        if (percent < 0)
            percent *= -1;
        percent = floor(percent);
        percent /= 5;
        percent *= 10;
        ret += percent;
        return ret;
    }
}

vector<u8> qr_code::generate_binary_string()
{
    vector<u8> data = generate_data_string();
    for (u8 cur : data)
    {
        cout << (int)cur << " ";
    }
    cout << "\n";
    vector<u8> ec = generate_ec_string(data);
    for (u8 cur : ec)
    {
        cout << (int)cur << " ";
    }
    cout << "\n";
    for (u8 cur : ec)
    {
        data.push_back(cur);
    }
    return data;
}

vector<u8> qr_code::generate_data_string()
{
    vector<bool> bit_stream;
    //function<void (const fu64 & value, const fu64 & length)> add_print = [&bit_stream](const fu64 & value, const fu64 & length) {add_bits(value, length, bit_stream); print_bits(bit_stream);};
    add_bits(mode, 4, bit_stream);
    add_bits(encoding_string.size(), get_length_of_size(), bit_stream);

    //Encode the ascii
    for (fu64 i = 0; i < encoding_string.size(); i += 2)
    {
        if (i + 1 == encoding_string.size()) //last character of odd length string
        {
            add_bits(get_alphanumeric_value(encoding_string[i]), 6, bit_stream);
        } else {
            fu64 value = get_alphanumeric_value(encoding_string[i]) * 45;
            value += get_alphanumeric_value(encoding_string[i+1]);
            add_bits(value, 11, bit_stream);
        }
    }

    for (fu64 i = 0; i < 4 && bit_stream.size() < data_bits[version][error]; ++i)
        bit_stream.push_back(false);
    for (fu8 remainder = bit_stream.size() % 8; remainder != 0 && remainder != 8; ++remainder)
        bit_stream.push_back(false);
    
    static const u8 padding[] = {0xEC, 0x11};
    for (u8 index = 0; bit_stream.size() < data_bits[version][error]; index = (index == 0 ? 1 : 0))
        add_bits(padding[index], 8, bit_stream);
    

    return bits_to_bytes(bit_stream);
}

polynomial get_first_alpha(const polynomial & inp)
{
    polynomial ret;
    fu16 best_x = 0;
    fu16 best_a = 0;
    for (const term & cur : inp.terms)
    {
        fu16 current_x = 0;
        fu16 current_a = 0;
        for (const term_element & elem : cur.element)
        {
            if (elem.variable == "a")
            {
                current_a = elem.exponent;
            }
            if (elem.variable == "x")
            {
                current_x = elem.exponent;
            }
        }
        if (current_x > best_x)
        {
            best_x = current_x;
            best_a = current_a;
        }
    }
    ret.push_back(term(1,"a",best_a));
    return ret;
}

vector<u8> qr_code::generate_ec_string(const vector<u8> & data)
{
    if (data.size() != ec_table[version][error].block_1_data_code_words)
    {
        cerr << "Incorrect number of data bytes\n";
        exit(EXIT_FAILURE);
    }
    u8 num_ec_blocks = ec_table[version][error].ec_code_words_per_block * ec_table[version][error].block_1_count;

    //Generate message polynomial
    polynomial message_polynomial;
    {
        fu16 exponent = ec_table[version][error].block_1_data_code_words + num_ec_blocks - 1;
        for (const u8 & cur : data)
        {
            term new_term;
            new_term.coefficient = cur;
            new_term.add_element("x", exponent);
            message_polynomial.push_back(new_term);
            exponent -= 1;
        }
    }
    //print_polynomial(message_polynomial);

    //Generate generator polynomial
    polynomial generator_polynomial;
    {
        vector<polynomial> generator_polynomials;
        for (u8 i = 0; i < num_ec_blocks; ++i)
            //for (u8 i = 0; i < 3; ++i)
        {
            polynomial tmp;
            tmp.push_back(term(1,"x", 1));
            tmp.push_back(term(1,"a", i));
            generator_polynomials.push_back(tmp);
        }
        polynomial multiplied = galois_foil(generator_polynomials);
        polynomial x_mult;
        x_mult.push_back(term(1,"x",data.size()-1));
        multiplied = multiplied * x_mult;
        generator_polynomial = multiplied;
    }

    polynomial div_x;
    div_x.push_back(term(1,"x",-1));

    while (true)
        //for (u8 i = 0; i < 13; ++i)
    {
        //Convert message polynomial to alpha notation
        message_polynomial.galois_to_alpha();
        
        polynomial a5 = get_first_alpha(message_polynomial);
        polynomial out = generator_polynomial * a5;
        out.galois_reduce();
        out.galois_to_integer();
        message_polynomial.galois_to_integer();
        message_polynomial = message_polynomial.xor_coefficients(out);
        if (message_polynomial.has_constant_term())
            break;
        message_polynomial.reduce();
        generator_polynomial = generator_polynomial * div_x;
        generator_polynomial.reduce();
    }
    return message_polynomial.grab_coefficients("x");
}

fu8 qr_code::get_length_of_size()
{
    if (version >= 1 && version <= 9)
    {
        switch(mode)
        {
          case NUMERIC_MODE:
            return 10;
            break;
          case ALPHANUMERIC_MODE:
            return 9;
            break;
          case BINARY_MODE:
            return 8;
            break;
          case JAPANESE_MODE:
            return 8;
            break;
        }
    } else if (version >= 10 && version <= 26) {
        switch(mode)
        {
          case NUMERIC_MODE:
            return 12;
            break;
          case ALPHANUMERIC_MODE:
            return 11;
            break;
          case BINARY_MODE:
            return 16;
            break;
          case JAPANESE_MODE:
            return 10;
            break;
        }
    } else if (version >= 27 && version <= 40) {
        switch(mode)
        {
          case NUMERIC_MODE:
            return 14;
            break;
          case ALPHANUMERIC_MODE:
            return 13;
            break;
          case BINARY_MODE:
            return 16;
            break;
          case JAPANESE_MODE:
            return 12;
            break;
        }
    }
    cerr << "Failed to find length of side\n";
    exit(EXIT_FAILURE);
    return -1;
}

bit_grid qr_code::generate_bit_grid(const vector<u8> & data, const u8 mask_pattern)
{
    print_bits(bytes_to_bits(data));
    bit_grid ret(get_image_dimension(), get_image_dimension());
    bit_grid no_fly_zone(get_image_dimension(), get_image_dimension());
    //---Default data
    //Top Left
    for (u8 x = 0; x < 8; ++x)
        for (u8 y = 0; y < 8; ++y)
            no_fly_zone.set(x,y,BLACK);
    for (u8 i = 0; i < 7; ++i)
    {
        ret.set(i,0,BLACK);
        ret.set(i,6,BLACK);
    }
    for (u8 i = 0; i < 7; ++i)
    {
        ret.set(0,i,BLACK);
        ret.set(6,i,BLACK);
    }
    for (u8 x = 2; x < 5; ++x)
        for (u8 y = 2; y < 5; ++y)
            ret.set(x,y,BLACK);
    //Bottom Left
    for (u8 x = 0; x < 8; ++x)
        for (u8 y = ret.get_height()-8; y < ret.get_height(); ++y)
            no_fly_zone.set(x,y,BLACK);
    for (u8 i = 0; i < 7; ++i)
    {
        ret.set(0,ret.get_height()-i-1,BLACK);
        ret.set(6,ret.get_height()-i-1,BLACK);
    }
    for (u8 i = 0; i < 7; ++i)
    {
        ret.set(i,ret.get_height()-1,BLACK);
        ret.set(i,ret.get_height()-7,BLACK);
    }
    for (u8 x = 2; x < 5; ++x)
        for (u8 y = ret.get_height()-5; y < ret.get_height()-2; ++y)
            ret.set(x,y,BLACK);
    //Top Right
    for (u8 x = ret.get_width()-8; x < ret.get_width(); ++x)
        for (u8 y = 0; y < 8; ++y)
            no_fly_zone.set(x,y,BLACK);
    for (u8 i = 0; i < 7; ++i)
    {
        ret.set(ret.get_width()-1,i,BLACK);
        ret.set(ret.get_width()-7,i,BLACK);
    }
    for (u8 i = 0; i < 7; ++i)
    {
        ret.set(ret.get_width()-i-1,0,BLACK);
        ret.set(ret.get_width()-i-1,6,BLACK);
    }
    for (u8 x = ret.get_width()-5; x < ret.get_width()-2; ++x)
        for (u8 y = 2; y < 5; ++y)
            ret.set(x,y,BLACK);
    //Black Pixel
    ret.set(8,ret.get_height()-8, BLACK);
    no_fly_zone.set(8,ret.get_height()-8, BLACK);
    //Vertical timing pattern
    {
        bool current_pixel = BLACK;
        for (u8 y = 8; y < ret.get_height()-8; ++y)
        {
            ret.set(6, y, current_pixel);
            no_fly_zone.set(6, y, BLACK);
            current_pixel = !current_pixel;
        }
    }
    //Horizontal timing pattern
    {
        bool current_pixel = BLACK;
        for (u8 x = 8; x < ret.get_width()-8; ++x)
        {
            ret.set(x, 6, current_pixel);
            no_fly_zone.set(x, 6, BLACK);
            current_pixel = !current_pixel;
        }
    }
    {
        string type_information = type_information_bits[mode][mask_pattern];
        //horizontal type information
        u8 i = 0;
        for (s8 cur : {0,1,2,3,4,5,7,-8,-7,-6,-5,-4,-3,-2,-1})
        {
            bool color = (type_information[i] == '1' ? BLACK : WHITE);
            ret.set((cur >= 0 ? cur : ret.get_width()+cur), 8, color);
            no_fly_zone.set((cur >= 0 ? cur : ret.get_width()+cur), 8, BLACK);
            ++i;
        }
        //vertical type information
        i = 0;
        for (s8 cur : {-1,-2,-3,-4,-5,-6,-7,8,7,5,4,3,2,1,0})
        {
            bool color = (type_information[i] == '1' ? BLACK : WHITE);
            ret.set(8, (cur >= 0 ? cur : ret.get_height()+cur), color);
            no_fly_zone.set(8, (cur >= 0 ? cur : ret.get_height()+cur), BLACK);
            ++i;
        }
        //x= 8
    }
    vector<bool> bits = bytes_to_bits(data);
    fs16 x = ret.get_width()-1;
    fs16 y = ret.get_height()-1;
    bool direction = UP;
    bool move_left_next = true;
    for (size_t i = 0; ; i++)
    {
        ret.set(x,y,apply_mask((bits[i] ? BLACK : WHITE),mask_pattern, x, y));
        if (i+1 == bits.size())
            break;
        //Advance to next position
        advance_to_next_position(x, y, direction, move_left_next, no_fly_zone);
    }
    return ret;
}

bool qr_code::apply_mask(bool original_color, u8 mask, fs16 x, fs16 y)
{
    switch (mask)
    {
      case 0:
        return ((y+x)%2 == 0 ? !original_color : original_color);
        break;
      case 1:
        return (y%2 == 0 ? !original_color : original_color);
        break;
      case 2:
        return (x%3 == 0 ? !original_color : original_color);
        break;
      case 3:
        return ((y+x)%3 == 0 ? !original_color : original_color);
        break;
      case 4:
        return (((y/2)+(x/3))%2 == 0 ? !original_color : original_color);
        break;
      case 5:
        return (((y*x)%2) + ((y*x)%3) == 0 ? !original_color : original_color);
        break;
      case 6:
        return ((((y*x)%3 + y*x)%2) == 0 ? !original_color : original_color);
        break;
      case 7:
        return ((((y*x)%3) + y + x)%2 ? !original_color : original_color);
        break;
    }
    return original_color;
}
