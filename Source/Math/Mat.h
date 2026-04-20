#pragma once
#include "Vec.h"
#include "axpch.h"

namespace Math {
    /// <summary>
    /// Matrices are represented as column vectors (collumn-major order).
    /// To apply a matrix to a vector, you multiply the matrix by the vector: v' = M * v.
    /// Mtotal = Mn * Mn-1 * ... * M1 means that the last matrix in the product is applied first.
    /// To acess the data of the matrix, you can use M[col][row].
    /// </summary>
    template <typename T, size_t N> struct Mat {
        Mat() = default;
        template <typename... Args> Mat(Args... args) : data{static_cast<Vec<T, N>>(args)...} {
            AX_CORE_ASSERT(sizeof...(args) == N, "Number of arguments must match the size of the matrix");
        }

        static Mat<T, N> identity() {
            Mat<T, N> result;
            for (size_t i = 0; i < N; ++i) {
                for (size_t j = 0; j < N; ++j) {
                    result.data[i][j] = (i == j) ? static_cast<T>(1) : static_cast<T>(0);
                }
            }
            return result;
        }

        // Returns a the i-th column vector
        constexpr Vec<T, N>& operator[](size_t i) { return data[i]; }
        // Returns a the i-th column vector
        constexpr const Vec<T, N>& operator[](size_t i) const { return data[i]; }

        constexpr Mat<T, N> operator+(const Mat<T, N>& other) const {
            Mat<T, N> result;
            for (size_t i = 0; i < N; ++i)
                for (size_t j = 0; j < N; ++j)
                    result.data[i][j] = data[i][j] + other.data[i][j];
            return result;
        }
        constexpr Mat<T, N> operator-(const Mat<T, N>& other) const {
            Mat<T, N> result;
            for (size_t i = 0; i < N; ++i)
                for (size_t j = 0; j < N; ++j)
                    result.data[i][j] = data[i][j] - other.data[i][j];
            return result;
        }
        constexpr Mat<T, N>& operator+=(const Mat<T, N>& other) {
            for (size_t i = 0; i < N; ++i)
                for (size_t j = 0; j < N; ++j)
                    data[i][j] += other.data[i][j];
            return *this;
        }
        constexpr Mat<T, N>& operator-=(const Mat<T, N>& other) {
            for (size_t i = 0; i < N; ++i)
                for (size_t j = 0; j < N; ++j)
                    data[i][j] -= other.data[i][j];
            return *this;
        }
        constexpr Mat<T, N> operator*(const T& scalar) const {
            Mat<T, N> result;
            for (size_t i = 0; i < N; ++i)
                for (size_t j = 0; j < N; ++j)
                    result.data[i][j] = data[i][j] * scalar;
            return result;
        }
        constexpr Mat<T, N>& operator*=(const T& scalar) {
            for (size_t i = 0; i < N; ++i)
                for (size_t j = 0; j < N; ++j)
                    data[i][j] *= scalar;
            return *this;
        }
        constexpr Mat<T, N> operator/(const T& scalar) const {
            AX_CORE_ASSERT(scalar != static_cast<T>(0), "Division by zero in Mat::operator/.");
            Mat<T, N> result;
            for (size_t i = 0; i < N; ++i)
                for (size_t j = 0; j < N; ++j)
                    result.data[i][j] = data[i][j] / scalar;
            return result;
        }
        constexpr Mat<T, N>& operator/=(const T& scalar) {
            AX_CORE_ASSERT(scalar != static_cast<T>(0), "Division by zero in Mat::operator/=.");
            for (size_t i = 0; i < N; ++i)
                for (size_t j = 0; j < N; ++j)
                    data[i][j] /= scalar;
            return *this;
        }
        friend constexpr Mat<T, N> operator*(const T& scalar, const Mat<T, N>& m) { return m * scalar; }
        friend constexpr Vec<T, N> operator*(const Mat<T, N>& m, const Vec<T, N>& v) {
            Vec<T, N> result;
            for (size_t i = 0; i < N; ++i) {
                result[i] = static_cast<T>(0);
                for (size_t j = 0; j < N; ++j) {
                    result[i] += m.data[j][i] * v[j];
                }
            }
            return result;
        }
        friend constexpr Mat<T, N> operator*(const Mat<T, N>& a, const Mat<T, N>& b) {
            Mat<T, N> result;
            for (size_t i = 0; i < N; ++i) {
                result[i] = a * b[i];
            }
            return result;
        }

        static Mat<float, 4> orthographic(float left, float right, float bottom, float top, float zNear, float zFar) {
            Mat<float, 4> result;
            result[0] = Vec4(2.0f / (right - left), 0.0f, 0.0f, 0.0f);
            result[1] = Vec4(0.0f, -2.0f / (top - bottom), 0.0f, 0.0f);
            result[2] = Vec4(0.0f, 0.0f, 1.0f / (zNear - zFar), 0.0f);
            result[3] = Vec4(-(right + left) / (right - left), (top + bottom) / (top - bottom), zNear / (zNear - zFar), 1.0f);
            return result;
        }
        static Mat<float, 4> perspective(float fov, float aspectRatio, float zNear, float zFar) {
            Mat<float, 4> result;
            float tanHalfFov = std::tan(fov * 0.5f);
            float f = 1.0f / tanHalfFov;
            result[0] = Vec4(f / aspectRatio, 0.0f, 0.0f, 0.0f);
            result[1] = Vec4(0.0f, f, 0.0f, 0.0f);
            result[2] = Vec4(0.0f, 0.0f, zFar / (zNear - zFar), -1.0f);
            result[3] = Vec4(0.0f, 0.0f, (zFar * zNear) / (zNear - zFar), 0.0f);
            return result;
        }
        static Mat<float, 4> lookAt(const Vec3& position, const Vec3& target, const Vec3& up) {
            Vec3 zAxis = normalize(position - target); // Forward (technically "Backward" in right-handed standard)
            Vec3 xAxis = normalize(cross(up, zAxis));  // Right
            Vec3 yAxis = cross(zAxis, xAxis);          // Up

            Mat<float, 4> result;

            result[0] = Vec4(xAxis.x(), yAxis.x(), zAxis.x(), 0.0f); // column 0
            result[1] = Vec4(xAxis.y(), yAxis.y(), zAxis.y(), 0.0f); // column 1
            result[2] = Vec4(xAxis.z(), yAxis.z(), zAxis.z(), 0.0f); // column 2

            result[3] = Vec4(-dot(xAxis, position), -dot(yAxis, position), -dot(zAxis, position), 1.0f); // column 3

            return result;
        }

        Mat<float, 4> inverse() const
            requires(N >= 4)
        {
            const float a00 = (*this)[0][0], a01 = (*this)[1][0], a02 = (*this)[2][0], a03 = (*this)[3][0];
            const float a10 = (*this)[0][1], a11 = (*this)[1][1], a12 = (*this)[2][1], a13 = (*this)[3][1];
            const float a20 = (*this)[0][2], a21 = (*this)[1][2], a22 = (*this)[2][2], a23 = (*this)[3][2];
            const float a30 = (*this)[0][3], a31 = (*this)[1][3], a32 = (*this)[2][3], a33 = (*this)[3][3];

            const float s0 = a00 * a11 - a10 * a01;
            const float s1 = a00 * a12 - a10 * a02;
            const float s2 = a00 * a13 - a10 * a03;
            const float s3 = a01 * a12 - a11 * a02;
            const float s4 = a01 * a13 - a11 * a03;
            const float s5 = a02 * a13 - a12 * a03;

            const float c5 = a22 * a33 - a32 * a23;
            const float c4 = a21 * a33 - a31 * a23;
            const float c3 = a21 * a32 - a31 * a22;
            const float c2 = a20 * a33 - a30 * a23;
            const float c1 = a20 * a32 - a30 * a22;
            const float c0 = a20 * a31 - a30 * a21;

            const float det = s0 * c5 - s1 * c4 + s2 * c3 + s3 * c2 - s4 * c1 + s5 * c0;

            if (std::fabs(det) < std::numeric_limits<float>::epsilon()) {
                return Mat<float, 4>::identity();
            }

            const float invDet = 1.0f / det;

            const float i00 = (+(a11 * c5 - a12 * c4 + a13 * c3)) * invDet;
            const float i01 = (-(a01 * c5 - a02 * c4 + a03 * c3)) * invDet;
            const float i02 = (+(a31 * s5 - a32 * s4 + a33 * s3)) * invDet;
            const float i03 = (-(a21 * s5 - a22 * s4 + a23 * s3)) * invDet;

            const float i10 = (-(a10 * c5 - a12 * c2 + a13 * c1)) * invDet;
            const float i11 = (+(a00 * c5 - a02 * c2 + a03 * c1)) * invDet;
            const float i12 = (-(a30 * s5 - a32 * s2 + a33 * s1)) * invDet;
            const float i13 = (+(a20 * s5 - a22 * s2 + a23 * s1)) * invDet;

            const float i20 = (+(a10 * c4 - a11 * c2 + a13 * c0)) * invDet;
            const float i21 = (-(a00 * c4 - a01 * c2 + a03 * c0)) * invDet;
            const float i22 = (+(a30 * s4 - a31 * s2 + a33 * s0)) * invDet;
            const float i23 = (-(a20 * s4 - a21 * s2 + a23 * s0)) * invDet;

            const float i30 = (-(a10 * c3 - a11 * c1 + a12 * c0)) * invDet;
            const float i31 = (+(a00 * c3 - a01 * c1 + a02 * c0)) * invDet;
            const float i32 = (-(a30 * s3 - a31 * s1 + a32 * s0)) * invDet;
            const float i33 = (+(a20 * s3 - a21 * s1 + a22 * s0)) * invDet;

            Mat<float, 4> result;
            result[0] = Vec4(i00, i10, i20, i30);
            result[1] = Vec4(i01, i11, i21, i31);
            result[2] = Vec4(i02, i12, i22, i32);
            result[3] = Vec4(i03, i13, i23, i33);
            return result;
        }
        Mat<float, 4> transpose() const
            requires(N >= 4)
        {
            Mat<float, 4> result;
            for (size_t i = 0; i < 4; ++i) {
                for (size_t j = 0; j < 4; ++j) {
                    result[i][j] = data[j][i];
                }
            }
            return result;
        }
        static Mat<float, 4> translate(const Vec3& translation) {
            Mat<float, 4> result = Mat<float, 4>::identity();
            result[3][0] = translation.x();
            result[3][1] = translation.y();
            result[3][2] = translation.z();
            return result;
        }
        static Mat<float, 4> scale(const Vec3& scale) {
            Mat<float, 4> result = Mat<float, 4>::identity();
            result[0][0] = scale.x();
            result[1][1] = scale.y();
            result[2][2] = scale.z();
            return result;
        }
        static Mat<float, 4> rotate(const Vec3& axis, float radians) {
            if (dot(axis, axis) < std::numeric_limits<float>::epsilon()) {
                return Mat<float, 4>::identity();
            }
            Vec3 a = normalize(axis);
            const float x = a[0], y = a[1], z = a[2];
            const float c = std::cos(radians);
            const float s = std::sin(radians);
            const float t = 1.0f - c;

            Mat<float, 4> result{};
            result[0] = Vec4(t * x * x + c, t * x * y + s * z, t * x * z - s * y, 0.0f);
            result[1] = Vec4(t * x * y - s * z, t * y * y + c, t * y * z + s * x, 0.0f);
            result[2] = Vec4(t * x * z + s * y, t * y * z - s * x, t * z * z + c, 0.0f);
            result[3] = Vec4(0.0f, 0.0f, 0.0f, 1.0f);
            return result;
        }
        static Mat<float, 4> rotateX(float radians) { return rotate(Vec3(1.0f, 0.0f, 0.0f), radians); }
        static Mat<float, 4> rotateY(float radians) { return rotate(Vec3(0.0f, 1.0f, 0.0f), radians); }
        static Mat<float, 4> rotateZ(float radians) { return rotate(Vec3(0.0f, 0.0f, 1.0f), radians); }
        static Mat<float, 4> model(const Vec3& translation, const Vec3& rotation, const Vec3& scaleVec) {
            Mat<float, 4> t = translate(translation);
            Mat<float, 4> r = rotateY(rotation.y()) * rotateX(rotation.x()) * rotateZ(rotation.z());
            Mat<float, 4> s = scale(scaleVec);
            return t * r * s;
        }
        Vec3 getForward() const
            requires(N >= 4)
        {
            return normalize(Vec3(-data[2][0], -data[2][1], -data[2][2]));
        }
        Vec3 getRight() const
            requires(N >= 4)
        {
            return normalize(Vec3(data[0][0], data[0][1], data[0][2]));
        }
        Vec3 getUp() const
            requires(N >= 4)
        {
            return normalize(Vec3(data[1][0], data[1][1], data[1][2]));
        }
        Vec3 getBackward() const
            requires(N >= 4)
        {
            return -getForward();
        }
        Vec3 getLeft() const
            requires(N >= 4)
        {
            return -getRight();
        }

      protected:
        std::array<Vec<T, N>, N> data{};
    };

    using Mat2 = Mat<float, 2>;
    using Mat3 = Mat<float, 3>;
    using Mat4 = Mat<float, 4>;
} // namespace Math
