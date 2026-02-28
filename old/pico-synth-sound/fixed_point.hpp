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
#include "hardware/divider.h"

class Fixed_16_16
{
public:
    Fixed_16_16()
        : raw_value(0) {}

    int32_t raw_value;

    static const Fixed_16_16 zero;
    static const Fixed_16_16 one;
    static const Fixed_16_16 epsilon;

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
        return Fixed_16_16::from_raw_value(static_cast<int32_t>((static_cast<int64_t>(raw_value) << 16) / y.raw_value));

        // with hw-divider
        // uint32_t abs_x = raw_value < 0 ? -raw_value : raw_value;
        // uint32_t abs_y = y.raw_value < 0 ? -y.raw_value : y.raw_value;
        // divmod_result_t qr = hw_divider_divmod_u32(abs_x << 16, abs_y);
        // uint32_t q_abs = to_quotient_u32(qr);
        // uint32_t r_abs = to_remainder_u32(qr);
        // int32_t q = (raw_value < 0) ^ (y.raw_value < 0) ? -static_cast<int32_t>(q_abs) : static_cast<int32_t>(q_abs);
        // return Fixed_16_16::from_raw_value(q);
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

    // Unary negation
    inline Fixed_16_16 operator-() const
    {
        return Fixed_16_16::from_raw_value(-raw_value);
    }

private:
};

inline const Fixed_16_16 Fixed_16_16::zero = Fixed_16_16::from_int32(0);
inline const Fixed_16_16 Fixed_16_16::one = Fixed_16_16::from_int32(1);
inline const Fixed_16_16 Fixed_16_16::epsilon = Fixed_16_16::from_raw_value(1);

#endif