#include "UUID.h"

namespace Axiom {
    std::random_device UUID::randomDevice;
    std::mt19937_64 UUID::generator(randomDevice());
    std::uniform_int_distribution<uint64_t> UUID::distribution;
} // namespace Axiom