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

inline int64_t div_s64_s64(int64_t a, int64_t b){
    // 分子と分母の符号を取得
    int sign_a = (a < 0) ? -1 : 1;
    int sign_b = (b < 0) ? -1 : 1;
    int sign_result = sign_a * sign_b;

    // 分子と分母を正の値に変換
    uint64_t abs_a = (a < 0) ? -a : a;
    uint64_t abs_b = (b < 0) ? -b : b;

    // 64ビット整数を32ビット整数に分割
    uint32_t a_high = abs_a >> 32;
    uint32_t a_low = abs_a & 0xFFFFFFFF;
    uint32_t b_high = abs_b >> 32;
    uint32_t b_low = abs_b & 0xFFFFFFFF;

    // 結果を格納する変数
    int64_t result = 0;

    // 64ビット整数の除算
    if (b_high == 0) {
        // 分母の上位32ビットが0の場合、分子の64ビットを分母の下位32ビットで除算
        uint64_t temp = ((uint64_t)a_high << 32) | a_low;
        divmod_result_t result_low = hw_divider_divmod_s32(temp, b_low);
        result = to_quotient_s32(result_low);
    } else {
        // 分母の上位32ビットが0でない場合、64ビット整数を32ビット整数に分割して除算
        divmod_result_t result_high = hw_divider_divmod_s32(a_high, b_high);
        uint64_t temp = ((uint64_t)to_remainder_s32(result_high) << 32) | a_low;
        divmod_result_t result_low = hw_divider_divmod_s32(temp, b_low);
        result = ((int64_t)to_quotient_s32(result_high) << 32) | (uint32_t)to_quotient_s32(result_low);
    }

    // 結果に符号を付ける
    result *= sign_result;

    return result;
}

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
        // int64_t result = (static_cast<int64_t>(raw_value) << 16) / y.raw_value;
        int64_t result = div_s64_s64((static_cast<int64_t>(raw_value) << 16), y.raw_value);
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