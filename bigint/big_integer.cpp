//
// Created by cat on 30.04.16.
//
#include <vector>
#include "big_integer.h"

int main() {
    big_integer a = 2;
    big_integer b = 2;

    big_integer c = a + b;

    return 0;
}

static const int DIGITS_PER_INT = 9;
static const int STR_DIV = (const int) 1e9;
static const int INPUT_BASE = 10;
static const int BASE_POW = 30;
static const int CONTAINER_BASE = (1 << BASE_POW); // optimal AF.



static unsigned msbmask(unsigned arg) {
    unsigned p = 1, h = arg;
    for (int i = 0; i < 4; i++) {
        h |= h >> p;
        p <<= 1;
    }
    h -= (h >> 1);

    h <<= 1;
    h -= 1;
    return h;
}

big_integer::big_integer() : storage(std::vector<unsigned>(1, 0)), sign(1) {
}

big_integer::big_integer(big_integer const& other) : storage(other.storage), sign(other.sign) {
}

big_integer::big_integer(int a) {
    if (a < 0) {
        a *= -1;
        sign = -1;
    } else {
        sign = 1;
    }
    storage = std::vector<unsigned>();
    if (a > CONTAINER_BASE) {
        storage.push_back((unsigned)a % CONTAINER_BASE);
        storage.push_back((unsigned)a / CONTAINER_BASE);
    } else {
        storage.push_back(a);
    }
}

big_integer::big_integer(std::string const& str) {
    size_t pos = 0;
    size_t len = str.length();
    sign = 1;

    while(pos < len && (str[pos] == '-' || str[pos] == '+')) {
        if (str[pos] == '-')
            sign *= -1;
        pos++;
    }
    size_t a = pos;

    std::vector<char> numbers;
    numbers.reserve(len);
    while (pos < len) {
        numbers.push_back(str[pos++] - '0');
    }
    long long acc = 0;
    for (char i : numbers) {
        if (acc > CONTAINER_BASE) {
            storage.push_back(acc % CONTAINER_BASE);
            acc /= CONTAINER_BASE;
        }

        acc += i;
        acc *= INPUT_BASE;
    }
    if (acc > 0) {
        if (acc > CONTAINER_BASE) {
            storage.push_back(acc % CONTAINER_BASE);
            acc /= CONTAINER_BASE;
        }
        storage.push_back(acc);
    }
}

big_integer::~big_integer() {

}

big_integer big_integer::add(big_integer const& other) const {
    big_integer r;

    long ms = std::min(this->storage.size(), other.storage.size());
    r.storage.resize(std::max(this->storage.size(), other.storage.size()));

    long long acc = 0;

    for (size_t i = 0; i < ms; i++) {
        unsigned lh = this->storage[i];
        unsigned rh = other.storage[i];

        acc += lh;
        acc += rh;

        r.storage[i] = acc % CONTAINER_BASE;
        acc /= CONTAINER_BASE;
    }

    if (acc != 0)
        r.storage.push_back(acc);

    return r;
}

big_integer big_integer::sub(big_integer const& other) const {
    big_integer r;

    long ms = std::min(this->storage.size(), other.storage.size());
    r.storage.resize(std::max(this->storage.size(), other.storage.size()));

    long long acc = 0;
    int borrow = 0;

    for (size_t i = 0; i < ms; i++) {
        unsigned lh = this->storage[i];
        unsigned rh = other.storage[i];

        acc += lh;
        acc -= rh;
        acc -= borrow;
        borrow = 0;

        if (acc < 0) {
            acc += CONTAINER_BASE;
            borrow++;
        }

        r.storage[i] = acc % CONTAINER_BASE;
        acc /= CONTAINER_BASE;
    }

    if (acc != 0)
        r.storage.push_back(acc);

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
    if (a.sign != b.sign) {
        return a.sign < 0;
    } else {
        if (a.storage.size() < b.storage.size()) {
            return a.sign > 0;
        }
        if (a.storage.size() > b.storage.size()) {
            return a.sign < 0;
        }

        size_t i = a.storage.size() - 1;
        while (i >= 0 && a.storage[i] == b.storage[i])
            i--;

        return (i >= 0) && (a.storage[i] < b.storage[i]);
    }
}

bool operator>(const big_integer &a, const big_integer &b) {
    return b > a;
}

bool operator==(const big_integer &a, const big_integer &b) {
    if (a.sign != b.sign)
        return false;

    if (a.storage.size() != b.storage.size())
        return false;

    size_t i = a.storage.size() - 1;

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
    big_integer copy = *this;
    copy.sign *= -1;
    size_t l = copy.storage.size() - 1;
    unsigned h = msbmask(copy.storage[l]);

    for (size_t i = 0; i < l; i++) {
        copy.storage[i] ^= CONTAINER_BASE - 1;
    }


    copy.storage[l] ^= h;

    return copy;
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
            if (a > rhs) {
                big_integer r = a.sub(rhs);
                r.sign = -1;
                *this = r;
                return *this;
            } else {
                big_integer r = rhs.sub(a);
                r.sign = 1;
                return *this = r;
            }
        } else {
            big_integer a = rhs.abs();
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

big_integer &big_integer::operator&=(big_integer const &rhs) {
    size_t sz = std::min(this->storage.size(), rhs.storage.size());
    this->sign = (signed char) ((rhs.sign == -1 && this->sign == -1) ? -1 : 1);
    this->storage.resize(sz);

    for (int i = 0; i < sz; i++) {
        this->storage[i] = this->storage[i] & rhs.storage[i];
    }

    return *this;
}

big_integer &big_integer::operator|=(big_integer const &rhs) {
    unsigned long szt = this->storage.size();
    unsigned long szo = rhs.storage.size();
    size_t sz = std::max(szt, szo);
    this->sign = (signed char) ((rhs.sign == -1 || this->sign == -1) ? -1 : 1);
    this->storage.resize(sz);

    for (int i = 0; i < sz; i++) {
        unsigned a = (i < szt) ? this->storage[i] : 0;
        unsigned b = (i < szo) ? rhs.storage[i] : 0;

        this->storage[i] = a | b;
    }

    return *this;
}

big_integer &big_integer::operator^=(big_integer const &rhs) {
    unsigned long szt = this->storage.size();
    unsigned long szo = rhs.storage.size();
    size_t sz = std::max(szt, szo);
    this->sign = (signed char) ((rhs.sign == -1 ^ this->sign == -1) ? -1 : 1);
    this->storage.resize(sz);

    for (int i = 0; i < sz; i++) {
        unsigned a = (i < szt) ? this->storage[i] : 0;
        unsigned b = (i < szo) ? rhs.storage[i] : 0;

        this->storage[i] = a ^ b;
    }

    return *this;
}

big_integer &big_integer::operator<<=(int rhs) { // FIXME too lazy
    size_t csz = this->storage.size();
    int full = rhs / BASE_POW, part = rhs % BASE_POW;
    unsigned head = this->storage[csz - 1];
    int g = 31;
    for (int i = 1 << 31; i > 0; i >>= 1) {
        if (i & head) {
            g = i;
            break;
        }
    }
    size_t nsz = csz + full + 1;

    bool oflow = false;

    if ( g + part > BASE_POW) {
        nsz++;
        oflow = true;
    }

    this->storage.resize(nsz);

    if (full != 0)
        for (size_t i = csz - 1; i >= 0; i--) {
            storage[i + full] = storage[i];
            storage[i] = 0;
        }

    if (part != 0)
        for (size_t i = nsz - 1; i >= 0; i--) {
            storage[i + 1] += storage[i] >> (BASE_POW - part);
            storage[i] &= 1 << (BASE_POW - part) - 1;
            storage[i] <<= part;
        }

    if (!oflow) {
        storage.pop_back();
    }

    return *this;
}

big_integer operator+(big_integer a, big_integer const &b) {
    return (big_integer(a) += b);
}

big_integer operator-(big_integer a, big_integer const &b) {
    return (big_integer(a) -= b);
}

std::string to_string(big_integer const &a) { // FIXME implement lsd / lsm
}

std::ostream &operator<<(std::ostream &s, big_integer const &a) {
    return s << to_string(a);
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
    unsigned head = this->storage[csz - 1];
    int g = 31;
    for (int i = 1 << 31; i > 0; i >>= 1) {
        if (i & head) {
            g = i;
            break;
        }
    }
    size_t nsz = csz - full;

    if (nsz <= 0)
        return (*this = 0);

    for (size_t i = 0; i < nsz; i++) {
        this->storage[i] = this->storage[i + full];
    }


    for (size_t i = 0; i < nsz; i++) {
        this->storage[i] >>= part;
        if (i + 1 < csz) {
            this->storage[i] += (this->storage[i + 1] << (BASE_POW - part));
        }
        this->storage[i] &= 1 << BASE_POW - 1;
    }

    if (this->storage[nsz - 1] == 0 && nsz > 1)
        nsz--;
    this->storage.resize(nsz);
}

big_integer operator<<(big_integer a, int b) {
    return big_integer(a) <<= b;
}

big_integer operator>>(big_integer a, int b) {
    return big_integer(a) >>= b;
}

big_integer &big_integer::operator*=(big_integer const &rhs) {
    big_integer copy = *this;


    this->sign *= rhs.sign;

    this->storage.resize(this->storage.size() + rhs.storage.size() + 1, 0);

    for (size_t i = 0; i < rhs.storage.size(); i++) {
        long long acc = 0;
        for (size_t j = 0; j < copy.storage.size(); j++) {
            acc += rhs.storage[i] * (long long) copy.storage[j];
            this->storage[i + j] += acc & (CONTAINER_BASE - 1);
            acc >>= BASE_POW;
        }

        this->storage[i + copy.storage.size()] += acc;
    }

    while (this->storage.size() > 1 && this->storage[this->storage.size() - 1] == 0)
        this->storage.pop_back();

    return *this;
}

big_integer &big_integer::operator/=(big_integer const &rhs) {
    size_t i = this->storage.size() - 1;
    std::vector<unsigned> inverse_ans;
    big_integer acc = 0;
    while (i >= 0) {
        if (acc < rhs) {
            acc <<= BASE_POW;
            acc += this->storage[i];
            i--;
            continue;
        } else {
            long long r = CONTAINER_BASE, l = 1; // [l, r)
            while (l + 1 != r) {
                long long mid = (l + r) / 2;
                big_integer t = rhs * mid;
                if (t < acc) l = mid;
                else r = mid;
            } // rhs * l <= acc; l is the closest
            acc -= (rhs * l);
            inverse_ans.push_back((unsigned)l);
        }
    }

    this->sign *= rhs.sign;
    size_t isz = inverse_ans.size();
    this->storage.resize(isz);
    for (int i = 0; i < isz; i++) {
        this->storage[i] = inverse_ans[isz - i - 1];
    }

    return *this;
}

big_integer &big_integer::operator%=(big_integer const &rhs) {
    big_integer d = *this / rhs;
    return (*this -= d * rhs);
}

big_integer operator*(big_integer a, big_integer const &b) {
    return big_integer(a) *= b;
}

big_integer operator/(big_integer a, big_integer const &b) {
    return big_integer(a) /= b;
}

big_integer operator%(big_integer a, big_integer const &b) {
    return big_integer(a) %= b;
}

