#pragma once
#include "axpch.h"

// std::list utils
template <typename T> inline T getNextWrapped(T it, T end, T begin) {
    auto next = std::next(it);
    return next == end ? begin : next;
}
