#include "filter.hpp"

Filter::Filter()
    : filter_type_(FilterType::LPF),
      coeff_{Fixed_16_16::one, Fixed_16_16::zero, Fixed_16_16::zero, Fixed_16_16::zero, Fixed_16_16::zero},
      x1_(Fixed_16_16::zero),
      x2_(Fixed_16_16::zero),
      y1_(Fixed_16_16::zero),
      y2_(Fixed_16_16::zero)
{
}

void Filter::set_filter_type(bool is_hpf)
{
    filter_type_ = is_hpf ? FilterType::HPF : FilterType::LPF;
}

void Filter::set_coefficients(Fixed_16_16 b0, Fixed_16_16 b1, Fixed_16_16 b2, Fixed_16_16 a1, Fixed_16_16 a2)
{
    coeff_[0] = b0;
    coeff_[1] = b1;
    coeff_[2] = b2;
    coeff_[3] = a1;
    coeff_[4] = a2;
}

void Filter::set_coefficient_by_index(uint8_t index, Fixed_16_16 value)
{
    if (index < coeff_.size())
    {
        coeff_[index] = value;
    }
}

Fixed_16_16 Filter::process(Fixed_16_16 x)
{
    Fixed_16_16 y = coeff_[0] * x + coeff_[1] * x1_ + coeff_[2] * x2_ - coeff_[3] * y1_ - coeff_[4] * y2_;

    x2_ = x1_;
    x1_ = x;
    y2_ = y1_;
    y1_ = y;

    return y;
}
