#ifndef ILMENDUR_UTIL_HPP
#define ILMENDUR_UTIL_HPP
#include <cmath>

/// Reads the entire file `file' (an std::ifstream) and returns it as a string.
#define READ_FILE(file) std::istreambuf_iterator<char>(file.rdbuf()), std::istreambuf_iterator<char>()

/**
 * Simple 2-dimensional vector.
 */
template<typename T>
class Vector2
{
public:
    T x;
    T y;

    Vector2<T>() { x = 0; y = 0; }
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
     */
    Vector2<float> normalise() {
        return (*this) * (1.0f / length());
    }

    /// Copy assignment operator.
    Vector2<T> operator=(const Vector2<T>& other) {
        x = other.x;
        y = other.y;
        return *this;
    }

    /// Move assignment operator.
    Vector2<T> operator=(Vector2<T>&& other) {
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

#endif /* ILMENDUR_UTIL_HPP */
