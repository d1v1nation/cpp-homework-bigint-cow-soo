#include "big_integer.h"
#include <iostream>
#include <cassert>

static const int MAX = 1000000;
template <typename T>
using vector = cow_soo_vector<T>;

int main() {
    cow_soo_vector<unsigned> fuck, shit;

    for (int j = 0; j < 10; j++) {
        fuck.resize(j);
//        fuck = shit;

        for (int i = 0; i < 100; i++)
            fuck.push_back(i);

        for (int i = 0; i < 100; i++) {
            std::cout << fuck.at(i);
        }

        std::cout << '\n';
    }


    return 0;
}