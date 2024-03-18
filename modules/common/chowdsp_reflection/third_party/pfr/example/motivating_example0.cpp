// Copyright (c) 2016-2023 Antony Polukhin
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//[pfr_motivating_example
#include <iostream>
#include <string>

#include "pfr.hpp"

struct some_person
{
    std::string name;
    unsigned birth_year;
};

int main()
{
    some_person val { "Edgar Allan Poe", 1809 };

    std::cout << pfr::get<0> (val) // No macro!
              << " was born in " << pfr::get<1> (val); // Works with any aggregate initializables!

    std::cout << pfr::io (val); // Outputs: {"Edgar Allan Poe", 1809}
}
//]
