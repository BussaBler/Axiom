#pragma once
// TODO: implement SIMD versions of these types and improve the overall implementation
#include "Vec.h"
#include "Mat.h"

namespace Math {
	// ------- Axiom Math Constants -------
	constexpr float AX_PI = 3.14159265358979323846f;
	constexpr float AX_TAU = 6.28318530717958647692f; // 2 * PI
	constexpr float AX_HALF_PI = 1.57079632679489661923f; // PI / 2
	constexpr float AX_QUARTER_PI = 0.78539816339744830962f; // PI / 4
	constexpr float AX_DEG_TO_RAD = 0.01745329251994329577f; // PI / 180
	constexpr float AX_RAD_TO_DEG = 57.2957795130823208768f; // 180 / PI
	constexpr float AX_E = 2.71828182845904523536f;
	constexpr float AX_SQRT2 = 1.41421356237309504880f; // sqrt(2)
	constexpr float AX_SQRT3 = 1.73205080756887729352f; // sqrt(3)

	constexpr float AX_INFINITY = std::numeric_limits<float>::infinity();
	constexpr float AX_EPSILON = 1e-7f;

	// ------- Axiom Math functions -------
	float axSin(float radians);
	float axCos(float radians);
	float axTan(float radians);
	float axAsin(float value);
	float axAcos(float value);
	float axAtan(float value);
	float axSqrt(float value);
	float axPow(float base, float exponent);
	float axAbs(float value);
	float toRadians(float degrees);

	bool axIsPowerOfTwo(uint64_t value);
	// Returns a random float in the range [0.0, 1.0)
	float axRandom();
	float axRandom(float seed);
	// Returns a random float in the range [min, max]
	float axRandom(float min, float max);
	float axRandom(float min, float max, float seed);
	// Returns a random integer in the range [min, max]
	int axRandomInt(int min, int max);
	int axRandomInt(int min, int max, float seed);

}

// ------- Axiom Math Type Formatters -------
template <typename T, size_t N, typename CharT>
struct std::formatter<Math::Vec<T, N>, CharT> {
	std::formatter<T, CharT> elem;
	constexpr auto parse(std::basic_format_parse_context<CharT>& ctx) {
		auto it = ctx.begin();
		auto end = ctx.end();
		if (it == end || *it == CharT('}')) return it;
		return elem.parse(ctx);
	}

	template <typename FormatContext>
	auto format(const Math::Vec<T, N>& v, FormatContext& ctx) const {
		auto out = ctx.out();
		*out++ = CharT('V');
		*out++ = CharT('e');
		*out++ = CharT('c');
		if constexpr (std::is_same_v<CharT, char>) {
			out = std::format_to(out, "{}", N);
		}
		else {
			out = std::format_to(out, L"{}", N);
		}
		*out++ = CharT('(');
		for (std::size_t i = 0; i < N; ++i) {
			out = elem.format(v[i], ctx);     
			if (i + 1 < N) {
				*out++ = CharT(',');
				*out++ = CharT(' ');
			}
		}
		*out++ = CharT(')');
		return out;
	}
};

template <typename T, size_t N, typename CharT>
struct std::formatter<Math::Mat<T, N>, CharT> {
	std::formatter<T, CharT> elem_;

	std::formatter<std::size_t, CharT> idx_;

	constexpr auto parse(std::basic_format_parse_context<CharT>& ctx) {
		auto it = ctx.begin();
		auto end = ctx.end();
		if (it == end || *it == CharT('}')) return it;   
		return elem_.parse(ctx);                        
	}

	template <typename FormatContext>
	auto format(const Math::Mat<T, N>& m, FormatContext& ctx) const {
		auto out = ctx.out();

		*out++ = CharT('M');
		*out++ = CharT('a');
		*out++ = CharT('t');
		out = idx_.format(N, ctx);
		*out++ = CharT('[');
		for (std::size_t i = 0; i < N; ++i) {
			*out++ = CharT('(');
			for (std::size_t j = 0; j < N; ++j) {
				out = elem_.format(m[j][i], ctx);
				if (j + 1 < N) {
					*out++ = CharT(',');
					*out++ = CharT(' ');
				}
			}
			*out++ = CharT(')');
			if (i + 1 < N) {
				*out++ = CharT(',');
				*out++ = CharT(' ');
			}
		}
		*out++ = CharT(']');
		return out;
	}
};
