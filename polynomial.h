#pragma once
#include <iostream>
#include "types.h"
#include <vector>
#include <tuple>

using namespace std;

class term_element
{
  public:
    term_element(const string & _var, const fu16 & _exp = 1) : exponent(_exp), variable(_var) {}
    bool operator==(const term_element & other) const {return variable == other.variable && exponent == other.exponent;}
    bool operator<(const term_element & other) const {return make_tuple(exponent, variable) < make_tuple(other.exponent, other.variable);}
    string variable;
    fu16 exponent;
};

class term
{
  public:
    term(fs16 _coefficient = 1) : coefficient(_coefficient) {}
    term(fs16 _coefficient, const string & variable, const fu16 & exponent) : coefficient(_coefficient) {add_element(variable, exponent);}
    fs16 coefficient;
    vector<term_element> element;
    void add_element(const string & variable, const fu16 & exponent = 1) {element.push_back(term_element(variable, exponent));}
};

class polynomial
{
  public:
    void push_back(const term & data) { terms.push_back(data); }
    vector<term>::iterator begin() {return terms.begin();}
    vector<term>::iterator end() {return terms.end();}
    vector<term>::const_iterator begin() const {return terms.begin();}
    vector<term>::const_iterator end() const {return terms.end();}

    polynomial& reduce();
    polynomial& galois_reduce();
    void print(ostream & out = cout) const;

    polynomial& galois_to_alpha();
    polynomial& galois_to_integer();
    polynomial xor_coefficients(const polynomial & rhs);
    bool has_constant_term();
    vector<u8> grab_coefficients(const string & sort_variable = "x");
    
    polynomial operator*(const polynomial & other);
    vector<term> terms;
  private:
    
};

void print_polynomial(const polynomial & data, ostream & out = cout);
polynomial multiply(const polynomial & a, const polynomial & b);
term reduce(const term & inp);
polynomial foil(vector<polynomial> data);
polynomial galois_foil(vector<polynomial> data);
