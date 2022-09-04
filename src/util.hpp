#ifndef ILMENDUR_UTIL_HPP
#define ILMENDUR_UTIL_HPP
#include <SDL2/SDL.h>
#include <cmath>
#include <stdexcept>

/// Reads the entire file `file' (an std::ifstream) and returns it as a string.
#define READ_FILE(file) std::istreambuf_iterator<char>(file.rdbuf()), std::istreambuf_iterator<char>()

/***
 * Returns true if the two floats `a' and `b' are close enough
 * (given by delta, which defaults to 0.1) to count as being
 * equal. Otherwise returns false.
 */
inline bool float_equal(float a, float b, float delta = 0.1)
{
    return fabs(a - b) < delta;
}

/**
 * Simple 2-dimensional vector.
 */
template<typename T>
class Vector2
{
public:
    T x;
    T y;

    Vector2<T>() { x = 0; y = 0; } ///< Default constructor creates a zero-length vector.
    Vector2<T>(const Vector2<T>& other) { x = other.x; y = other.y; }
    Vector2<T>(Vector2<T>&& other) { x = other.x; y = other.y; other.clear(); }
    Vector2<T>(T other_x, T other_y) { x = other_x; y = other_y; }

    /// Resets this vector to a zero length vector.
    void clear() {
        x = 0;
        y = 0;
    }

    /**
     * Performant check for whether this vector is of zero length
     * (only checks the x and y members).
     */
    bool isZeroLength() const {
        return x == 0.0f && y == 0.0f;
    }

    /**
     * Calculates the vector’s length. This takes a square root,
     * which is an expensive operation, so use only if needed.
     * Checking for zero length is more performant with isZeroLength().
     */
    float length() const {
        return sqrtf(x*x + y*y);
    }

    /**
     * Normalises the vector, that is, scales it so that its length()
     * is exactly 1, keeping its direction. Note that the result of
     * this operation always is a vector on the float base type, because
     * such a scale operation is unlikely to yield exact integral values
     * for the x and y components.
     *
     * If this vector isInfinite(), special rules apply. It is possible
     * to normalise an infinite vector if and only if one of the following
     * conditions is true:
     *
     * - Both components are infinite.
     * - One component is infinite, the other component is zero.
     *
     * In all other cases, trying to normalise the infinite vector causes
     * an exception of type std::domain_error.
     */
    Vector2<float> normalise() const {
        if (std::isinf(x)) {
            if (std::isinf(y)) { // Infinity handling
                return Vector2<float>(x > 0 ? 1.0f : -1.0f, y > 0 ? 1.0f : -1.0f);
            } else if (y == 0.0f) {
                return Vector2<float>(x > 0 ? 1.0f : -1.0f, 0.0f);
            } else {
                throw(std::domain_error("Cannot normalise vector with only one infinite component if the other component is not zero"));
            }
        } else if (std::isinf(y)) { // Infinity handling
            if (std::isinf(x)) {
                return Vector2<float>(x > 0 ? 1.0f : -1.0f, y > 0 ? 1.0f : -1.0f);
            } else if (x == 0.0f) {
                return Vector2<float>(0.0f, y > 0 ? 1.0f : -1.0f);
            } else {
                throw(std::domain_error("Cannot normalise vector with only one infinite component if the other component is not zero"));
            }
        } else { // The normal case.
            return (*this) * (1.0f / length());
        }
    }

    /**
     * Determines if this vector has an infinite length, i.e. whether
     * one of its components is infinite.
     */
    bool isInfinite() const {
        return std::isinf(x) || std::isinf(y);
    }

    /// Copy assignment operator.
    Vector2<T>& operator=(const Vector2<T>& other) {
        x = other.x;
        y = other.y;
        return *this;
    }

    /// Move assignment operator.
    Vector2<T>& operator=(Vector2<T>&& other) {
        x = other.x;
        y = other.y;
        other.clear();
        return *this;
    }

    Vector2<T> operator+(const Vector2<T>& other) const {
        return Vector2<T>(x + other.x, y + other.y);
    }

    Vector2<T> operator-(const Vector2<T>& other) const {
        return Vector2<T>(x - other.x, y - other.y);
    }

    Vector2<T> operator*(T other) const {
        return Vector2<T>(x * other, y * other);
    }

    // Dot product
    T operator*(Vector2<T> other) const {
        return x * other.x + y * other.y;
    }

    bool operator==(const Vector2<T>& other) const {
        return x == other.x && y == other.y;
    }

    // Cross product does not make much sense for 2D vectors,
    // thus leave it out.

    /**
     * Calculates the enclosed angle φ between this vector and another vector.
     * Note this will always be the smaller angle. The result is in radians.
     */
    float angleWith(Vector2<T> other) const {
        return acosf(((*this) * other) / (length() * other.length()));
    }
};

// Type shortcuts.
typedef Vector2<float> Vector2f;
typedef Vector2<int>   Vector2i;

/// Tests whether the given `point` is located within the given `rect`.
inline bool isPointInRect(const Vector2f& point, const SDL_Rect& rect)
{
    return point.x >= rect.x &&
        point.y >= rect.y &&
        point.x < rect.x + rect.w &&
        point.y < rect.y + rect.h;
}

#endif /* ILMENDUR_UTIL_HPP */
