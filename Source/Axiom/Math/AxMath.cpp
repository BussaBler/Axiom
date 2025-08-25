#include "axpch.h"
#include "AxMath.h"

namespace Math {
	float axSin(float radians) {
		return std::sinf(radians);
	}
	float axCos(float radians) {
		return std::cosf(radians);
	}
	float axTan(float radians) {
		return std::tanf(radians);
	}
	float axAsin(float value) {
		return std::asinf(value);
	}
	float axAcos(float value) {
		return std::acosf(value);
	}
	float axAtan(float value) {
		return std::atanf(value);
	}
	float axSqrt(float value) {
		return std::sqrtf(value);
	}
	float axPow(float base, float exponent) {
		return std::powf(base, exponent);
	}
	float axAbs(float value) {
		return std::abs(value);
	}
	float toRadians(float degrees) {
		return degrees * AX_DEG_TO_RAD;
	}

	bool axIsPowerOfTwo(uint64_t value) {
		return (value != 0 && (value & (value - 1)) == 0);
	}
	float axRandom(float seed) {
		std::mt19937 generator(static_cast<unsigned int>(seed * 1000));
		std::uniform_real_distribution<float> distribution(0.0f, 1.0f);
		return distribution(generator);
	}
	float axRandom(float min, float max, float seed) {
		std::mt19937 generator(static_cast<unsigned int>(seed * 1000));
		std::uniform_real_distribution<float> distribution(min, max);
		return distribution(generator);
	}
	int axRandomInt(int min, int max, float seed) {
		std::mt19937 generator(static_cast<unsigned int>(seed * 1000));
		std::uniform_int_distribution<int> distribution(min, max);
		return distribution(generator);
	}
}