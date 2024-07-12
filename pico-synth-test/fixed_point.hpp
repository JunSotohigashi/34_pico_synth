#ifndef FIXED_POINT_HPP
#define FIXED_POINT_HPP

#include "pico/stdlib.h"

// 符号付16bit整数 × 符号なし4+12bit固定小数
int16_t mul_i16_q12(int16_t x, uint16_t y);
// 符号なし4+28bit固定小数 × 符号なし4+28bit固定小数
uint32_t mul_q28_q28(uint32_t x, uint32_t y);

#endif