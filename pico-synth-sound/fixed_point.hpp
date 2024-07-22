/**
 * \file fixed_point.hpp
 * \author JunSotohigashi (c6h4ohcooc6h5@gmail.com)
 * \brief Fast operations of Fixed Point numbers
 * \version 0.1
 * \date 2024-07-20
 *
 *
 */

#ifndef FIXED_POINT_HPP
#define FIXED_POINT_HPP

#include "pico/stdlib.h"

class Fixed_16_16
{
public:
    Fixed_16_16()
        : raw_value(0) {}

    int32_t raw_value;

    static Fixed_16_16 from_raw_value(int32_t raw_value)
    {
        Fixed_16_16 result;
        result.raw_value = raw_value;
        return result;
    }

    static Fixed_16_16 from_float(float value)
    {
        Fixed_16_16 result;
        result.raw_value = static_cast<int32_t>(value * 65536.0f);
        return result;
    }

    static Fixed_16_16 from_int32(int32_t value)
    {
        Fixed_16_16 result;
        result.raw_value = value << 16;
        return result;
    }

    inline Fixed_16_16 &operator=(Fixed_16_16 value)
    {
        raw_value = value.raw_value;
        return *this;
    }

    // Arithmetic operators
    inline Fixed_16_16 operator+(Fixed_16_16 y) const
    {
        return Fixed_16_16::from_raw_value(raw_value + y.raw_value);
    }

    inline Fixed_16_16 operator-(Fixed_16_16 y) const
    {
        return Fixed_16_16::from_raw_value(raw_value - y.raw_value);
    }

    inline Fixed_16_16 operator*(Fixed_16_16 y) const
    {
        // Perform the multiplication and adjust back to 16.16 fixed-point
        int64_t result = static_cast<int64_t>(raw_value) * y.raw_value;
        return Fixed_16_16::from_raw_value(static_cast<int32_t>(result >> 16));
    }

    inline Fixed_16_16 operator/(Fixed_16_16 y) const
    {
        // Perform the division and adjust to 16.16 fixed-point
        int64_t result = (static_cast<int64_t>(raw_value) << 16) / y.raw_value;
        return Fixed_16_16::from_raw_value(static_cast<int32_t>(result));
    }

    inline Fixed_16_16 &operator+=(Fixed_16_16 y)
    {
        raw_value += y.raw_value;
        return *this;
    }

    inline Fixed_16_16 &operator-=(Fixed_16_16 y)
    {
        raw_value -= y.raw_value;
        return *this;
    }

    inline Fixed_16_16 &operator*=(Fixed_16_16 y)
    {
        *this = *this * y;
        return *this;
    }

    inline Fixed_16_16 &operator/=(Fixed_16_16 y)
    {
        *this = *this / y;
        return *this;
    }

    // Comparison operators
    inline bool operator<(Fixed_16_16 y) const
    {
        return raw_value < y.raw_value;
    }

    inline bool operator<=(Fixed_16_16 y) const
    {
        return raw_value <= y.raw_value;
    }

    inline bool operator>(Fixed_16_16 y) const
    {
        return raw_value > y.raw_value;
    }

    inline bool operator>=(Fixed_16_16 y) const
    {
        return raw_value >= y.raw_value;
    }

    inline bool operator==(Fixed_16_16 y) const
    {
        return raw_value == y.raw_value;
    }

private:
};

#endif