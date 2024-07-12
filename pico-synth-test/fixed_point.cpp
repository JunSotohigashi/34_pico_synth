#include "fixed_point.hpp"

int16_t mul_i16_q12(int16_t x, uint16_t y)
{
    bool negative = x & (1 << 15);
    uint32_t tmp;
    if (negative)
    {
        tmp = ~x + 1;
        tmp = (tmp * y) >> 12;
        tmp = ~tmp + 1;
    }
    else
    {
        tmp = x;
        tmp = (tmp * y) >> 12;
    }
    return (int16_t)tmp;
}

uint32_t mul_q28_q28(uint32_t x, uint32_t y)
{
    uint64_t tmp = x;
    tmp *= y;
    tmp = tmp >> 28;
    return (uint32_t)tmp;
}