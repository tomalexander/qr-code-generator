#include "polynomial.h"
#include <unordered_map>
#include <map>
#include <cmath>
#include <unordered_set>
#include <list>

polynomial polynomial::operator*(const polynomial & other)
{
    polynomial ret;
    for (const term & cur_a : *this)
    {
        for (const term & cur_b : other)
        {
            term new_term;
            new_term.coefficient = cur_a.coefficient * cur_b.coefficient;
            for (const term_element & te : cur_a.element)
            {
                new_term.element.push_back(te);
            }
            for (const term_element & te : cur_b.element)
            {
                new_term.element.push_back(te);
            }
            ret.push_back(::reduce(new_term));
        }
    }
    return ret;
}

polynomial polynomial::xor_coefficients(const polynomial & rhs)
{
    polynomial ret;
    map<vector<term_element>, fu16> filtered_terms;
    for (const term & cur : terms)
    {
        if (filtered_terms.find(cur.element) == filtered_terms.end())
        {//Never saw this term before
            filtered_terms[cur.element] = cur.coefficient;
        } else {
            filtered_terms[cur.element] ^= cur.coefficient;
        }
    }
    for (const term & cur : rhs.terms)
    {
        if (filtered_terms.find(cur.element) == filtered_terms.end())
        {//Never saw this term before
            filtered_terms[cur.element] = cur.coefficient;
        } else {
            filtered_terms[cur.element] ^= cur.coefficient;
        }
    }

    for (const pair<vector<term_element>, fu16> cur : filtered_terms)
    {
        term new_term;
        new_term.element = cur.first;
        new_term.coefficient = cur.second;
        ret.push_back(new_term);
    }
    return ret;
}

bool polynomial::has_constant_term()
{
    for (const term & cur : terms)
    {
        if (cur.element.size() == 0)
            return true;
        fu16 sum_exponent = 0;
        for (const term_element & elem : cur.element)
            sum_exponent += elem.exponent;
        if (sum_exponent == 0)
            return true;
    }
    return false;
}

vector<u8> polynomial::grab_coefficients(const string & sort_variable)
{
    vector<u8> ret;
    unordered_set<fu16> used_exponents;
    bool first = true;
    while (used_exponents.size() < terms.size())
    {
        fs16 best_coefficient = 0;
        fs16 best_exponent = 0;
        for (const term & cur : terms)
        {
            for (const term_element & elem : cur.element)
            {
                if (elem.variable == sort_variable)
                {
                    if (used_exponents.find(elem.exponent) == used_exponents.end() && elem.exponent >= best_exponent)
                    {
                        best_coefficient = cur.coefficient;
                        best_exponent = elem.exponent;
                    }
                }
            }
        }
        used_exponents.insert(best_exponent);
        //If the first coefficient is 0, remove it. oddly enough we want to keep 0 coefficients on later nodes
        if (!(best_coefficient == 0 && first))
            ret.push_back(best_coefficient);
        first = false;
    }

    return ret;
}

polynomial& polynomial::reduce()
{
    map<vector<term_element>, fu16> filtered_terms;
    for (const term & cur : terms)
    {
        if (filtered_terms.find(cur.element) == filtered_terms.end())
        {//Never saw this term before
            filtered_terms[cur.element] = cur.coefficient;
        } else {
            filtered_terms[cur.element] += cur.coefficient;
        }
    }
    terms.clear();
    for (const pair<vector<term_element>, fu16> cur : filtered_terms)
    {
        if (cur.second == 0)
            continue;
        term new_term;
        new_term.element = cur.first;
        new_term.coefficient = cur.second;
        push_back(new_term);
    }
    return *this;
}

extern u8 log_table[], anti_log_table[];

polynomial& polynomial::galois_reduce()
{
    //Find exponents larger than 255
    for (term & cur : terms)
        for (term_element & elem : cur.element)
            if (elem.exponent > 255)
                elem.exponent = (elem.exponent%256) + floor(elem.exponent/256);

    //Convert a's to coefficient
    galois_to_integer();

    //Similar to reduce, but instead of adding we need to xor coefficients
    {
        map<vector<term_element>, fu16> filtered_terms;
        for (const term & cur : terms)
        {
            if (filtered_terms.find(cur.element) == filtered_terms.end())
            {//Never saw this term before
                filtered_terms[cur.element] = cur.coefficient;
            } else {
                filtered_terms[cur.element] ^= cur.coefficient;
            }
        }
        terms.clear();
        for (const pair<vector<term_element>, fu16> cur : filtered_terms)
        {
            if (cur.second == 0)
                continue;
            term new_term;
            new_term.element = cur.first;
            new_term.coefficient = cur.second;
            push_back(new_term);
        }
    }

    //Convert coefficients back to a's
    galois_to_alpha();
    
    return *this;
}

polynomial& polynomial::galois_to_alpha()
{
    for (term & cur : terms)
    {
        cur.add_element("a", log_table[cur.coefficient]);
        cur.coefficient = 1;
    }
    return *this;
}

polynomial& polynomial::galois_to_integer()
{
    for (term & cur : terms)
    {
        for (vector<term_element>::iterator it = cur.element.begin(); it != cur.element.end();)
        {
            if (it->variable == "a")
            {
                cur.coefficient = anti_log_table[it->exponent];
                it = cur.element.erase(it);
            } else {
                ++it;
            }
        }
    }
return *this;
}

void polynomial::print(ostream & out) const
{
    bool first = true;
    for (const term & cur : *this)
    {
        if (first)
            first = false;
        else
            out << " + ";
        out << cur.coefficient;
        for (const term_element & elem : cur.element)
            out << elem.variable << "^(" << elem.exponent << ")";
    }
    out << "\n";
}

polynomial foil(vector<polynomial> data)
{
    while (data.size() > 1)
    {
        polynomial a = data.back();
        data.pop_back();
        polynomial b = data.back();
        data.pop_back();
        data.push_back(a*b);
    }
    return data[0];
}

polynomial galois_foil(vector<polynomial> data)
{
    while (data.size() > 1)
    {
        polynomial a = data.back();
        data.pop_back();
        polynomial b = data.back();
        data.pop_back();
        polynomial ret = a*b;
        ret.galois_reduce();
        data.push_back(ret);
    }
    return data[0];
}

void print_polynomial(const polynomial & data, ostream & out)
{
    bool first = true;
    for (const term & cur : data)
    {
        if (first)
            first = false;
        else
            out << " + ";
        out << cur.coefficient;
        for (const term_element & elem : cur.element)
            out << elem.variable << "^(" << elem.exponent << ")";
    }
    out << "\n";
}

polynomial multiply(const polynomial & a, const polynomial & b)
{
    polynomial ret;
    for (const term & cur_a : a)
    {
        for (const term & cur_b : b)
        {
            term new_term;
            new_term.coefficient = cur_a.coefficient * cur_b.coefficient;
            for (const term_element & te : cur_a.element)
            {
                new_term.element.push_back(te);
            }
            for (const term_element & te : cur_b.element)
            {
                new_term.element.push_back(te);
            }
            ret.push_back(reduce(new_term));
        }
    }
    return ret;
}

term reduce(const term & inp)
{
    term ret(inp.coefficient);

    unordered_map<string, fu16> elements;
    for (const term_element & cur : inp.element)
    {
        if (elements.find(cur.variable) == elements.end())
        {//Never before seen
            elements[cur.variable] = cur.exponent;
        } else {
            elements[cur.variable] += cur.exponent;
        }
    }
    for (const pair<string, fu16> & cur : elements)
    {
        term_element tmp(cur.first, cur.second);
        ret.element.push_back(tmp);
    }

    return ret;
}
