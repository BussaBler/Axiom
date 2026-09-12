#pragma once

struct UIEdgeInsets {
    float left = 0.0f;
    float top = 0.0f;
    float right = 0.0f;
    float bottom = 0.0f;

    UIEdgeInsets() = default;
    UIEdgeInsets(float value) : left(value), top(value), right(value), bottom(value) {}
    UIEdgeInsets(float l, float t, float r, float b) : left(l), top(t), right(r), bottom(b) {}
};
