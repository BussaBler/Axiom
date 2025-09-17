#include "axpch.h"
#include "AxMath.h"

namespace Math {
	float axSin(float radians) {
		return std::sinf(radians);
	}
	float axCos(float radians) {
		return cosf(radians);
	}
	float axTan(float radians) {
		return tanf(radians);
	}
	float axAsin(float value) {
		return asinf(value);
	}
	float axAcos(float value) {
		return acosf(value);
	}
	float axAtan(float value) {
		return atanf(value);
	}
	float axSqrt(float value) {
		return sqrtf(value);
	}
	float axPow(float base, float exponent) {
		return powf(base, exponent);
	}
	float axAbs(float value) {
		return std::abs(value);
	}
	float toRadians(float degrees) {
		return degrees * AX_DEG_TO_RAD;
	}
	float axClamp(float value, float min, float max) {
		return std::fmax(min, std::fmin(max, value));
	}

	bool axIsPowerOfTwo(uint64_t value) {
		return (value != 0 && (value & (value - 1)) == 0);
	}
	static std::mt19937& axRandomEngine() {
		thread_local std::mt19937 engine([] {
			std::random_device rd;
			std::seed_seq seq{ rd(), rd(), rd(), rd(), rd(), rd(), rd(), rd() };
			return std::mt19937(seq);
		}());
		return engine;
	}
	static std::mt19937 axRandomEngine(float seed) {
		uint32_t x = std::bit_cast<uint32_t>(seed);
		x += 0x9E3779B9u;
		x ^= x >> 16; x *= 0x7feb352du;
		x ^= x >> 15; x *= 0x846ca68bu;
		x ^= x >> 16;
		return std::mt19937(x);
	}
	float axRandom() {
		std::uniform_real_distribution<float> distribution(0.0f, 1.0f);
		return distribution(axRandomEngine());
	}
	float axRandom(float seed) {
		std::uniform_real_distribution<float> distribution(0.0f, 1.0f);
		auto eng = axRandomEngine(seed);
		return distribution(eng);
	}
	float axRandom(float min, float max) {
		std::uniform_real_distribution<float> distribution(min, max);
		return distribution(axRandomEngine());
	}
	float axRandom(float min, float max, float seed) {
		std::uniform_real_distribution<float> distribution(min, max);
		auto eng = axRandomEngine(seed);
		return distribution(eng);
	}
	int axRandomInt(int min, int max) {
		std::uniform_int_distribution<int> distribution(min, max);
		return distribution(axRandomEngine());
	}
	int axRandomInt(int min, int max, float seed) {
		std::uniform_int_distribution<int> distribution(min, max);
		auto eng = axRandomEngine(seed);
		return distribution(eng);
	}
}