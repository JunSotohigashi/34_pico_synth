#include "filter.hpp"

Filter::Filter()
    : filter_type_(FilterType::LPF),
      coeff_{1.0f, 0.0f, 0.0f, 0.0f, 0.0f},
      x1_(0.0f),
      x2_(0.0f),
      y1_(0.0f),
      y2_(0.0f)
{
}

void Filter::set_filter_type(bool is_hpf)
{
    filter_type_ = is_hpf ? FilterType::HPF : FilterType::LPF;
}

void Filter::set_coefficients(float b0, float b1, float b2, float a1, float a2)
{
    coeff_[0] = b0;
    coeff_[1] = b1;
    coeff_[2] = b2;
    coeff_[3] = a1;
    coeff_[4] = a2;
}

void Filter::set_coefficient_by_index(uint8_t index, float value)
{
    if (index < coeff_.size())
    {
        coeff_[index] = value;
    }
}

float Filter::process(float x)
{
    const float y = coeff_[0] * x + coeff_[1] * x1_ + coeff_[2] * x2_ - coeff_[3] * y1_ - coeff_[4] * y2_;

    x2_ = x1_;
    x1_ = x;
    y2_ = y1_;
    y1_ = y;

    return y;
}
