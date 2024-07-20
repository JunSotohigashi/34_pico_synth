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

/**
 * \brief Calcurate the product of x and y
 *
 * \param x 16bit signed integer
 * \param y 4+12bit unsigned fixed-point
 * \return 16bit signed integer
 */
int16_t mul_i16_q12(int16_t x, uint16_t y);

/**
 * \brief Calcurate the product of x and y
 *
 * \param x 4+28bit unsigned fixed-point
 * \param y 4+28bit unsigned fixed-point
 * \return uint32_t 4+28bit unsigned fixed-point
 */
uint32_t mul_q28_q28(uint32_t x, uint32_t y);

/**
 * \brief Calculate the quotient of x divided by y
 *
 * \param x 4+28bit unsigned fixed-point
 * \param y 4+28bit unsigned fixed-point
 * \return uint32_t 4+28bit unsigned fixed-point
 */
uint32_t div_q28_q28(uint32_t x, uint32_t y);

#endif