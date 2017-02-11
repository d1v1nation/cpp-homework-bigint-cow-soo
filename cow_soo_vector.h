//
// Created by cat on 20.01.17.
//


#ifndef BIGINT_COW_SOO_VECTOR_H
#define BIGINT_COW_SOO_VECTOR_H

#include <cstddef>
#include <cstdint>
#include <algorithm>
#include <cassert>

template <typename T>
class cow_soo_vector {
    static constexpr int SMALL_SIZE = 4;
    typedef typename std::aligned_storage<std::max(sizeof(size_t), sizeof(T)), std::max(alignof(size_t), alignof(T))>::type aligned_tp;

    struct small {
        T data[SMALL_SIZE];
    };


    class big {
        size_t *refcount;
        T *data;

    public:

        big() : refcount(nullptr), data(nullptr) {assert(false);} // this should never happen, but as a part of an anonymous union it should exist
        big(big const &other) : refcount(other.refcount), data(other.data) {
            if (refcount != nullptr)
                *refcount += 1;
        }

        big(big&& other) : refcount(other.refcount), data(other.data) {
            other.data = nullptr;
            other.refcount = nullptr;
        }

        big &operator=(big const &other) {
            if (other.data == data)
                return *this;

            unlink();

            data = other.data;
            refcount = other.refcount;

            *refcount += 1;

            return *this;
        }
        big &operator=(big&& other) {
            if (other.data == data)
                return *this;

            unlink();

            data = other.data;
            refcount = other.refcount;

            other.data = nullptr;
            other.refcount = nullptr;

            return *this;
        }

        big(size_t sz) {
            aligned_tp* na = new aligned_tp[1 + sz]; // strong guarantee by usage: new bigs are only made in constructors or temporaries

            refcount = (size_t*) na;
            *refcount = 1;

            data = (T*) (void*) (na + 1); // dirty. but contiguous!
        }

        ~big() {
            unlink();
        }

        T* get_data() const {
            return data;
        }

        size_t get_refcount() const {
            return *refcount;
        }

        void inline unlink() __attribute__((always_inline)) {
            if (refcount != nullptr && data != nullptr) {
                if (*refcount == 1) {
                    delete[]((aligned_tp *) refcount);
                } else {
                    (*refcount) -= 1;
                }

                refcount = nullptr;
                data = nullptr;
            }
        }

    };


    enum data_state {
        DATA_SMALL,
        DATA_BIG
    };

    size_t capacity;
    size_t sz;


    data_state state;
    union {
        small d_small;
        big d_big;
    };


    void inline ensure_fits(size_t esz) __attribute__((always_inline)) {
        if (esz > capacity) {
            reserve(std::max(2 * capacity, esz));
        }
    }

    void inline ensure_uniq() __attribute__((always_inline)) {
        if (state == DATA_BIG && (d_big.get_refcount()) > 1) {
            big t(capacity);
            std::copy(d_big.get_data(), d_big.get_data() + sz, t.get_data());

            d_big = std::move(t);
        }
    }

    T &unsafe_at(size_t index) __attribute__((always_inline)) {
        if (state == DATA_SMALL) {
            return d_small.data[index];
        } else {
            return d_big.get_data()[index];
        }
    }

    
public:
    cow_soo_vector() : capacity(SMALL_SIZE), sz(0), state(DATA_SMALL) {
//        d_small.data[0] = 0;
    }

    cow_soo_vector(size_t asz) {
        if (asz <= SMALL_SIZE) { // operating in smalls
            state = DATA_SMALL;
            capacity = SMALL_SIZE;
            sz = asz;
            std::fill_n(d_small.data, sz, T());
        } else {
            state = DATA_BIG;
            capacity = sz;
            sz = asz;
            new(&d_big) big(sz);
            std::fill_n(d_big.get_data(), sz, T());
        }
    }

    cow_soo_vector(cow_soo_vector<T> const &other) : capacity(other.capacity), sz(other.sz), state(other.state) {
        if (state == DATA_SMALL) {
            d_small = other.d_small;
        } else {
            new(&d_big) big(other.d_big);
        }
    };

    cow_soo_vector(cow_soo_vector<T> &&other) : capacity(other.capacity), sz(other.sz), state(other.state) {
        if (state == DATA_SMALL) {
            d_small = other.d_small;
        } else {
            new(&d_big) big(std::move(other.d_big));
        }
    }

    cow_soo_vector(size_t sz, T const &value) : sz(sz) {
        if (sz <= SMALL_SIZE) { // operating in smalls
            state = DATA_SMALL;
            capacity = SMALL_SIZE;
            std::fill_n(d_small.data, sz, value);
        } else {
            state = DATA_BIG;
            capacity = sz;
            new(&d_big) big(sz);
            std::fill_n(d_big.get_data(), sz, value);
        }
    }


    cow_soo_vector<T> &operator=(cow_soo_vector<T> const &other) {

        if (state == other.state) {
            if (state == DATA_BIG) {
                d_big = other.d_big;
            } else {
                d_small = other.d_small;
            }
        } else {
            if (state == DATA_SMALL) { // other is big
                new(&d_big) big(other.d_big);
                state = DATA_BIG;
            } else { // other is small, we are big
                d_big.~big();
                d_small = other.d_small;
                state = DATA_SMALL;
            }
        }

        sz = other.sz;
        capacity = other.capacity;

        return (*this);
    };

    cow_soo_vector<T> &operator=(cow_soo_vector<T> &&other) {
        if (state == other.state) {
            if (state == DATA_BIG) {
                d_big = std::move(other.d_big);
            } else {
                d_small = other.d_small;
            }
        } else {
            if (state == DATA_SMALL) { // other is big
                new(&d_big) big(std::move(other.d_big));
                state = DATA_BIG;
            } else { // other is small, we are big
                d_big.~big();
                d_small = other.d_small;
                state = DATA_SMALL;
            }
        }

        sz = other.sz;
        capacity = other.capacity;

        return (*this);
    }

    ~cow_soo_vector() {
        if (state == DATA_BIG) {
            d_big.~big();
        }
    }

    inline T const &at(size_t index) const __attribute__((always_inline)) {
        if (state == DATA_SMALL) {
            return d_small.data[index];
        } else {
            return d_big.get_data()[index];
        }
    }

    inline T &operator[](size_t index) __attribute__((always_inline)) {
        ensure_uniq();

        assert(index < sz);
        if (state == DATA_SMALL) {
            return d_small.data[index];
        } else {
            return d_big.get_data()[index];
        }
    }

    size_t size() const {
        return sz;
    }

    // don't bother with shrinking just yet
    void reserve(size_t ncpt) {
        if (ncpt <= capacity) {
            return;
        }

        if (state == DATA_SMALL) {
            if (ncpt > SMALL_SIZE) {
                big nb(ncpt);

                std::copy(d_small.data, d_small.data + sz, nb.get_data());

                new(&d_big) big(std::move(nb));

                state = DATA_BIG;
            } else {
                // ok : still in small data.
            }
        } else { // we in big data now.
            big nb(ncpt);

            std::copy(d_big.get_data(), d_big.get_data() + sz, nb.get_data());
            d_big = std::move(nb);
        }

        capacity = ncpt;
    }

    void resize(size_t nsz) {
        resize(nsz, T());
    }

    void resize(size_t nsz, T const &nelems) {
        ensure_fits(nsz);
        ensure_uniq();

        size_t osz = sz;
        sz = nsz;

        if (nsz > osz) {
            if (state == DATA_SMALL) {
                std::fill_n(d_small.data + osz, nsz - osz, nelems);
            } else {
                std::fill_n(d_big.get_data() + osz, nsz - osz, nelems);
            }
        }
    }


    void push_back(T const &val) {
        ensure_fits(sz + 1);
        ensure_uniq();

        sz++;
        unsafe_at(sz - 1) = val;
    }

    T const &back() const __attribute__((always_inline)) {
        return at(sz - 1);
    }

    void pop_back() {
        assert(sz != 0);
        sz--;
    }

    void assign(size_t nsz, T const &copy) {
        ensure_fits(nsz);
        ensure_uniq();
        sz = nsz;

        for (size_t i = 0; i < nsz; i++) {
            unsafe_at(i) = copy;
        }

        if (state == DATA_SMALL) {
            std::fill_n(d_small.data, nsz, copy);
        } else {
            std::fill_n(d_big.get_data(), nsz, copy);
        }
    }
};


#endif //BIGINT_COW_SOO_VECTOR_H
