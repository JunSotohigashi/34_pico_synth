#include "vca.hpp"

VCA::VCA() : gain_(1.0f), pan_(0.5f) {}

void VCA::serialize(uint16_t *buf) const
{
    // VCA serialization (placeholder)
}
