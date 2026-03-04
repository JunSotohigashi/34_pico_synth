#include "vcf.hpp"

VCF::VCF() : filter_(VCFFilterType::LPF), cutoff_freq_(1000.0f), resonance_(0.0f) {}

void VCF::serialize(uint16_t *buf) const
{
    // VCF serialization (placeholder)
}
