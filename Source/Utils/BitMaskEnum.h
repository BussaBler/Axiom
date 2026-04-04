#pragma once
#include <type_traits>

namespace Axiom {
	template<typename T>
	struct EnableBitMaskOperators : std::false_type {};

	template<typename T>
	concept BitMaskEnum = std::is_enum_v<T> && EnableBitMaskOperators<T>::value;

	template<BitMaskEnum E>
	constexpr E operator|(E lhs, E rhs) {
		return static_cast<E>(static_cast<std::underlying_type_t<E>>(lhs) | static_cast<std::underlying_type_t<E>>(rhs));
	}

	template<BitMaskEnum E>
	constexpr E operator&(E lhs, E rhs) {
		return static_cast<E>(static_cast<std::underlying_type_t<E>>(lhs) & static_cast<std::underlying_type_t<E>>(rhs));
	}

	template<BitMaskEnum E>
	constexpr E operator^(E lhs, E rhs) {
		return static_cast<E>(static_cast<std::underlying_type_t<E>>(lhs) ^ static_cast<std::underlying_type_t<E>>(rhs));
	}
}
