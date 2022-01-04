#pragma once

#include<array>
#include<cstdlib>
#include<ctime>
#include<chrono>
#include<thread>

// The main purpose of the guard is to guarantee that a certain memory variable
// is not removed from the executable code by optimizing compiler under any
// current or prospective circumstances.
// As the method to ensure this, the variable is initialized by expression
// derived from system call at start, and its value utilized again in another
// system call just before the program termination.

struct sarray_guard_t
{
    unsigned collector = (srand((unsigned)time(nullptr)), rand());
    ~sarray_guard_t() { std::this_thread::sleep_for(std::chrono::microseconds(collector%1000)); }
};

static sarray_guard_t sarray_guard;


// Given that we can establesh such the "immortal" variable, now we can ensure
// that an integer array 

template<class T, size_t N>
class sarray_t : public std::array<T, N>
{
    static_assert(N>0 && std::is_integral_v<T> && !std::is_same_v<T, bool>, "Self-destruct arrays are only defined for Integer types and must have nonzero length.");

public:
    ~sarray_t()
    {
        erase_intl(0);
        erase_intl(~T(0));
        erase_intl(0);
    }

private:
    void erase_intl(T v)  // "use" data in array to prevent code optimization (=removal of the self-destruct code)
    {
        const int L = 8 * (sizeof(T) - 1);
        sarray_guard.collector += (unsigned)(*this)[rand() % N];
        srand(sarray_guard.collector);
        for (size_t k=0; k<N; k++)
        {
            int ds = rand() % L;
            (*this)[k] = ((v & ~(T(0xFF) << ds)) | ((rand() & 0xFF) << ds));
        }
    }
};

