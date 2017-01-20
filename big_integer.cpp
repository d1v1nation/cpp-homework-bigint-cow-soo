//
// Created by cat on 30.04.16.
//
#include <vector>
#include <limits.h>
#include <ostream>
#include <iostream>
#include <limits>
#include "big_integer.h"

static const int BASE_POW = 30;
static const unsigned CONTAINER_BASE = (1 << BASE_POW);


big_integer::big_integer() : storage(std::vector<unsigned>(1, 0)), sign(1) {
}

big_integer::big_integer(big_integer const& other) : storage(other.storage), sign(other.sign) {
}

big_integer::big_integer(int a) {

    if (a == std::numeric_limits<int>::min()) {
        // bit magic;

        this->storage.resize(2, 0);
        this->sign = -1;
        this->storage[1] = 2;

        return;
    }


    if (a < 0) {
        a *= -1;
        sign = -1;
    } else {
        sign = 1;
    }
    storage = std::vector<unsigned>();
    if (a >= (int)CONTAINER_BASE) {
        storage.push_back((unsigned)a % CONTAINER_BASE);
        if ((a >>= BASE_POW) != 0)
            storage.push_back(a);
    } else {
        storage.push_back(a);
    }
}

big_integer::big_integer(std::string const& str) : storage(1, 0) {
    size_t pos = 0;
    size_t len = str.length();
    sign = 1;
    signed char ds = 1;

    while(pos < len && (str[pos] == '-' || str[pos] == '+')) {
        if (str[pos] == '-')
            ds *= -1;
        pos++;
    }

    std::vector<char> numbers;
    numbers.reserve(len + 1);
    while (pos < len) {
        numbers.push_back(str[pos++] - '0');
    }

    for (char i : numbers) {
        this->mul_l_s(10);
        *this += i;
    }

    this->sign = ds;
}

big_integer::~big_integer() {

}

bool big_integer::is_z() const {
    return (storage.size() == 1 && storage[0] == 0);
}

big_integer big_integer::add(big_integer const& other) const {
    big_integer r(*this);
    r.sign = this->sign;

    long ms = std::min(this->storage.size(), other.storage.size());
    r.storage.resize(std::max(this->storage.size(), other.storage.size()) + 1);

    long long acc = 0;

    for (long i = 0; i < ms || acc; i++) {
        unsigned lh = (i < (int)this->storage.size() ? this->storage[i] : 0);
        unsigned rh = (i < (int)other.storage.size() ? other.storage[i] : 0);

        acc += lh;
        acc += rh;

        r.storage[i] = acc & (CONTAINER_BASE - 1);
        acc >>= BASE_POW;
    }

    r.cutoff();
    return r;
}

big_integer big_integer::sub(big_integer const& other) const {
    if (other.is_z()) {
        return *this;
    }

    big_integer r(*this);
    r.sign = this->sign;

    long ms = std::min(this->storage.size(), other.storage.size());
    r.storage.resize(std::max(this->storage.size(), other.storage.size()));

    long ts = this->storage.size();
    long os = other.storage.size();


    long long acc = 0;
    int borrow = 0;

    for (long i = 0; i < ms || borrow; i++) {
        unsigned lh = ((i < ts) ? this->storage[i] : 0);
        unsigned rh = ((i < os) ? other.storage[i] : 0);

        acc += lh;
        acc -= rh;
        acc -= borrow;
        borrow = 0;

        while (acc < 0) {
            acc += CONTAINER_BASE;
            borrow++;
        }

        r.storage[i] = (unsigned)(acc & (CONTAINER_BASE - 1));
        acc >>= BASE_POW;
    }


    r.cutoff();
    return r;
}

big_integer big_integer::abs()const {
    big_integer c = *this;
    c.sign = 1;
    return c;
}

big_integer& big_integer::operator=(big_integer const& other) {
    sign = other.sign;
    storage = other.storage;
    return *this;
}

big_integer& big_integer::operator+=(big_integer const& other) {
    if (this->sign == other.sign) {
        return *this = this->add(other);
    } else {
        if (this->sign == -1) {
            big_integer a = this->abs();
            if (a > other) {
                big_integer r = a.sub(other);
                r.sign = -1;
                return *this = r;
            } else {
                big_integer r = other.sub(a);
                r.sign = 1;
                return *this = r;
            }
        } else {
            big_integer a = other.abs();
            if (a > *this) {
                big_integer r = a.sub(*this);
                r.sign = -1;
                return *this = r;
            } else {
                big_integer r = this->sub(a);
                r.sign = 1;
                return *this = r;
            }
        }
    }

}

bool operator<(const big_integer &a, const big_integer &b) {
    if (a.is_z() && b.is_z()) {
        return false;
    }

    if (a.sign != b.sign) {
        return a.sign < 0;
    } else {
        if (a.storage.size() < b.storage.size()) {
            return a.sign > 0;
        }
        if (a.storage.size() > b.storage.size()) {
            return a.sign < 0;
        }

        int i = a.storage.size() - 1;
        while (i >= 0 && a.storage[i] == b.storage[i])
            i--;

        return (i >= 0) && (a.storage[i] < b.storage[i]);
    }
}

bool operator>(const big_integer &a, const big_integer &b) {
    return b < a;
}

bool operator==(const big_integer &a, const big_integer &b) {
    if (a.is_z())
        return b.is_z();

    if (a.sign != b.sign)
        return false;

    if (a.storage.size() != b.storage.size())
        return false;

    int i = a.storage.size() - 1;

    while (i >= 0 && a.storage[i] == b.storage[i])
        i--;

    return i < 0;
}

bool operator!=(const big_integer &a, const big_integer &b) {
    return !(a == b);
}

bool operator<=(const big_integer &a, const big_integer &b) {
    return !(a > b);
}

bool operator>=(big_integer const &a, big_integer const &b) {
    return !(a < b);
}

big_integer big_integer::operator~() const {
    return -(*this) - 1;
}

big_integer big_integer::operator+() const {
    return (*this);
}

big_integer big_integer::operator-() const {
    big_integer copy = *this;
    copy.sign *= -1;
    return copy;
}

big_integer& big_integer::operator++() {
    return operator+=(1);
}

big_integer& big_integer::operator--() {
    return operator-=(1);
}

big_integer big_integer::operator++(int i) {
    return *this + i;
}

big_integer big_integer::operator--(int i) {
    return *this - i;
}

big_integer &big_integer::operator-=(big_integer const &rhs) {
    if (this->sign != rhs.sign) {
        return *this = this->add(rhs);
    } else {
        if (this->sign == -1) {
            big_integer a = this->abs();
            if (a > rhs.abs()) {
                big_integer r = a.sub(rhs);
                r.sign = -1;
                return *this = r;
            } else {
                big_integer r = rhs.sub(a);
                r.sign = 1;
                return *this = r;
            }
        } else {
            big_integer a = rhs.abs();
            if (a > this->abs()) {
                big_integer r = a.sub(*this);
                r.sign = -1;
                return *this = r;
            } else {
                big_integer r = this->sub(a);
                r.sign = 1;
                return *this = r;
            }
        }
    }
}

big_integer &big_integer::operator&=(big_integer const &rhs) {
    big_integer r = rhs;
    int sz = std::max(this->storage.size(), r.storage.size());
    this->comp(sz);
    r.comp(sz);

    this->storage.reserve(sz);
    for (int i = 0; i < sz; i++) {
        unsigned ld = (i < (int)this->storage.size()) ? this->storage[i] : CONTAINER_BASE - 1;
        unsigned rd = (i < (int)r.storage.size()) ? r.storage[i] : CONTAINER_BASE - 1;
        if (i == (int)this->storage.size())
            this->storage.push_back(ld & rd);
        else
            this->storage[i] = ld & rd;
    }

    this->comp(sz);
    while (this->storage.size() > 1 && this->storage.back() == 0) {
        this->storage.pop_back();
    }

    return *this;
}

big_integer &big_integer::operator|=(big_integer const &rhs) {
    big_integer r = rhs;
    int sz = std::max(this->storage.size(), r.storage.size());
    this->comp(sz);
    r.comp(sz);

    this->storage.reserve(sz);
    for (int i = 0; i < sz; i++) {
        unsigned ld = (i < (int)this->storage.size()) ? this->storage[i] : 0;
        unsigned rd = (i < (int)r.storage.size()) ? r.storage[i] : 0;
        if (i == (int)this->storage.size())
            this->storage.push_back(ld & rd);
        else
            this->storage[i] = ld | rd;
    }

    this->sign = (signed char) ((r.sign == -1) ? -1 : 1);
    this->comp(sz);
    while (this->storage.size() > 1 && this->storage.back() == 0) {
        this->storage.pop_back();
    }

    return *this;
}

big_integer &big_integer::operator^=(big_integer const &rhs) {
    big_integer r = rhs;
    int sz = std::max(this->storage.size(), r.storage.size());
    this->comp(sz);
    r.comp(sz);

    this->storage.reserve(sz);
    for (int i = 0; i < sz; i++) {
        unsigned ld = (i < (int)this->storage.size()) ? this->storage[i] : 0;
        unsigned rd = (i < (int)r.storage.size()) ? r.storage[i] : 0;
        if (i == (int)this->storage.size())
            this->storage.push_back(ld & rd);
        else
            this->storage[i] = ld ^ rd;
    }

    this->sign *= r.sign;
    this->comp(sz);
    while (this->storage.size() > 1 && this->storage.back() == 0) {
        this->storage.pop_back();
    }

    return *this;
}

big_integer &big_integer::operator<<=(int rhs) { // FIXME too lazy
    size_t csz = this->storage.size();
    int full = rhs / BASE_POW, part = rhs % BASE_POW;

    size_t nsz = csz + full + 2;

    this->storage.resize(nsz);

    if (full != 0)
        for (int i = csz - 1; i >= 0; i--) {
            storage[i + full] = storage[i];
            storage[i] = 0;
        }

    if (part != 0)
        for (int i = csz + full; i >= 0; i--) {
            storage[i + 1] += storage[i] >> (BASE_POW - part);
            storage[i] &= (1 << (BASE_POW - part)) - 1;
            storage[i] <<= part;
        }

    this->cutoff();

    return *this;
}

big_integer operator+(big_integer a, big_integer const &b) {
    return (big_integer(a) += b);
}

big_integer operator-(big_integer a, big_integer const &b) {
    return (big_integer(a) -= b);
}

std::string to_string(big_integer const &a) {// FIXME implement lsd / lsm

    if (a.is_z())
        return "0";

    std::string r = "";
    big_integer c = a;
    while (c != 0) {
        big_integer div = c / 1e8;
        big_integer mod = c - div * 1e8;

        const std::string &string = std::to_string(mod.storage[0]);

        if (string == "") {
            std::cout << "dicks";
        }
        r = ((string == "0") ? "00000000" : string) + r;
        c = div;
    }

    if (a.sign == -1)
        r = "-" + r;

    return r;
}

std::ostream &operator<<(std::ostream &s, big_integer const &a) {
    s << to_string(a);
    s << std::endl;
    return s;
}

big_integer operator&(big_integer a, big_integer const &b) {
    return (big_integer(a) &= b);
}

big_integer operator|(big_integer a, big_integer const &b) {
    return (big_integer(a) |= b);
}

big_integer operator^(big_integer a, big_integer const &b) {
    return big_integer(a) ^= b;
}

big_integer &big_integer::operator>>=(int rhs) {
    size_t csz = this->storage.size();
    int full = rhs / BASE_POW, part = rhs % BASE_POW;

    this->comp(csz);

    size_t nsz = csz - full;
    if (nsz <= 0)
        return (*this = 0);

    for (size_t i = 0; i < nsz; i++) {
        this->storage[i] = this->storage[i + full];
    }
    this->storage.resize(nsz);

    this->storage.push_back(this->sign == 1 ? 0 : -1);

    for (size_t i = 0; i < nsz; i++) {
        unsigned head = this->storage[i] >> BASE_POW;
        this->storage[i] &= CONTAINER_BASE - 1;
        this->storage[i] >>= part;
        this->storage[i] += (this->storage[i + 1] << (BASE_POW - part));
        this->storage[i] &= CONTAINER_BASE - 1;
        this->storage[i] |= head << BASE_POW;
    }

    this->storage.pop_back();

    for (int i = this->storage.size() - 1; (i >= 0) && (this->storage[i] == (unsigned)((this->sign == 1) ? 0 : -1)); i--) {
        this->storage.pop_back();
    }

    this->storage.resize(nsz);
    this->comp(nsz);

    this->cutoff();

    return *this;
}

big_integer operator<<(big_integer a, int b) {
    return big_integer(a) <<= b;
}

big_integer operator>>(big_integer a, int b) {
    return big_integer(a) >>= b;
}

big_integer &big_integer::operator*=(big_integer const &rhs) {
    if (rhs.storage.size() == 1) {
        this->sign *= rhs.sign;
        return this->mul_l_s(rhs.storage[0]);
    }

    if (this->is_z()) {
        return *this;
    }

    big_integer c = *this;
    this->storage.assign(c.storage.size() + rhs.storage.size(), 0);
    for (int i = 0; i < (int)c.storage.size(); i++) {
        long long carry = 0, cur;
        for (int j = 0; j < (int)rhs.storage.size() || carry; j++) {
            cur = this->storage[i + j] + c.storage[i] * 1ll * (j < (int)rhs.storage.size() ? rhs.storage[j] : 0) + carry;
            this->storage[i + j] = (unsigned)(cur & (CONTAINER_BASE - 1));
            carry = (cur >>= BASE_POW);
        }
    }
    this->sign = c.sign * rhs.sign;
    this->cutoff();

    return *this;
}

big_integer &big_integer::operator/=(big_integer const &rhop) {
    if (*this == 0)
        return *this;

    if (rhop.storage.size() == 1) {
        this->sign *= rhop.sign;
        return this->div_l_s(rhop.storage[0]);
    }

    big_integer rhs = rhop;
    rhs.sign = 1;
    big_integer res, cur, t;
    res.storage.resize(this->storage.size());
    cur.storage.reserve(this->storage.size());

    for (int i = this->storage.size() - 1; i >= 0; i--) {
        cur <<= BASE_POW;
        cur.storage[0] = this->storage[i];
        bool cache = false;

        int l = 0, r = CONTAINER_BASE;
        if (rhs <= cur) {
            while (l + 1 < r) {
                int m = (l + r) >> 1;
                t = rhs;
                t.mul_l_s(m);
                if (t <= cur) {
                    l = m;
                    cache = true;
                }
                else {
                    r = m;
                    cache = false;
                }
            }
        }

        res.storage[i] = l;
        if (!cache) {
            t = rhs;
            t.mul_l_s(l);
        }
        cur -= t;
    }

    res.cutoff();
    res.sign = this->sign * rhop.sign;
    std::swap(*this, res);
    return *this;
}

big_integer &big_integer::operator%=(big_integer const &rhs) {
    big_integer d = *this / rhs;
    return (*this -= d * rhs);
}

big_integer operator*(big_integer a, big_integer const &b) {
    if (a < b)
        return big_integer(b) *= a;
    else
        return a *= b;
}

big_integer operator/(big_integer a, big_integer const &b) {
    return a /= b;
}

big_integer operator%(big_integer a, big_integer const &b) {
    return a %= b;
}


void big_integer::cutoff() {
    while (this->storage.size() > 1 && this->storage[this->storage.size() - 1] == 0) {
        this->storage.pop_back();
    }

    return;
}

big_integer &big_integer::comp(int sz) {
    this->storage.resize(sz, 0);

    if (this->sign == 1)
        return *this;

    for (size_t i = 0; i < this->storage.size(); i++) {
        this->storage[i] = -this->storage[i];
    }

    return *this;
}

big_integer &big_integer::mul_l_s(unsigned s) {
    long long acc = 0;
    int sz = this->storage.size();

    if (s == 0) {
        *this = 0;
        return *this;
    }

    for (int i = 0; i < sz; i++) {
        acc += this->storage[i] * 1ll * s;
        this->storage[i] = acc & (CONTAINER_BASE - 1);
        acc >>= BASE_POW;
    }

    if (acc != 0) {
        this->storage.push_back(acc);
    }

    this->cutoff();

    return *this;
}

big_integer &big_integer::div_l_s(unsigned s) {
    int carry = 0;
    for (int i = this->storage.size() - 1; i >= 0; i--) {
        long long cur = this->storage[i] + carry * 1ll * CONTAINER_BASE;
        this->storage[i] = (unsigned)(cur / s);
        carry = (int)(cur % s);
    }

    this->cutoff();

    return *this;
}
