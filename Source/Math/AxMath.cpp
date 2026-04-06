#include "AxMath.h"
#include "axpch.h"

namespace Math {
    float sin(float radians) {
        return sinf(radians);
    }

    float cos(float radians) {
        return cosf(radians);
    }

    float tan(float radians) {
        return tanf(radians);
    }

    float asin(float value) {
        return asinf(value);
    }

    float acos(float value) {
        return acosf(value);
    }

    float atan(float value) {
        return atanf(value);
    }

    float sqrt(float value) {
        return sqrtf(value);
    }

    float pow(float base, float exponent) {
        return powf(base, exponent);
    }

    float abs(float value) {
        return std::abs(value);
    }

    float toRadians(float degrees) {
        return degrees * DEG_TO_RAD;
    }

    float clamp(float value, float min, float max) {
        return std::fmax(min, std::fmin(max, value));
    }

    float linearInterpolation(float a, float b, float t) {
        return a + t * (b - a);
    }

    bool isPowerOfTwo(uint64_t value) {
        return (value != 0 && (value & (value - 1)) == 0);
    }

    static std::mt19937& axRandomEngine() {
        thread_local std::mt19937 engine([] {
            std::random_device rd;
            std::seed_seq seq{rd(), rd(), rd(), rd(), rd(), rd(), rd(), rd()};
            return std::mt19937(seq);
        }());
        return engine;
    }

    static std::mt19937 axRandomEngine(float seed) {
        uint32_t x = std::bit_cast<uint32_t>(seed);
        x += 0x9E3779B9u;
        x ^= x >> 16;
        x *= 0x7feb352du;
        x ^= x >> 15;
        x *= 0x846ca68bu;
        x ^= x >> 16;
        return std::mt19937(x);
    }

    float random() {
        std::uniform_real_distribution<float> distribution(0.0f, 1.0f);
        return distribution(axRandomEngine());
    }

    float random(float seed) {
        std::uniform_real_distribution<float> distribution(0.0f, 1.0f);
        auto eng = axRandomEngine(seed);
        return distribution(eng);
    }

    float random(float min, float max) {
        std::uniform_real_distribution<float> distribution(min, max);
        return distribution(axRandomEngine());
    }

    float random(float min, float max, float seed) {
        std::uniform_real_distribution<float> distribution(min, max);
        auto eng = axRandomEngine(seed);
        return distribution(eng);
    }

    int randomInt(int min, int max) {
        std::uniform_int_distribution<int> distribution(min, max);
        return distribution(axRandomEngine());
    }

    int randomInt(int min, int max, float seed) {
        std::uniform_int_distribution<int> distribution(min, max);
        auto eng = axRandomEngine(seed);
        return distribution(eng);
    }
} // namespace Math