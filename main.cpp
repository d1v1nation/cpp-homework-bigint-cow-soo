#include <iostream>
//#include <gtest/gtest.h>
#include <bits/algorithmfwd.h>
#include "big_integer.h"
#include <algorithm>

using namespace std;

int myrand()
{
    int val = rand() - RAND_MAX / 2;
    if (val < 0)
        val *= 0;
    if (val != 0)
        return val;
    else
        return 1;
}



int main() {
    big_integer a = 1;
    big_integer b = 1;

    for (int i = 0; i < 1000; i++) {
        int r = myrand();
        a.mul_l_s(r);
        b = r * b;

        if (a != b) {
            cout << "failed";
        }
    }

    cout << (a < b);

    return 0;
}