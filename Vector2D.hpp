#pragma once
#include <SFML/Graphics.hpp>
#include <cmath>
#include <stdexcept>

class Vector2D
{
private:
    float x;
    float y;

public:
    // constructors
    Vector2D() : x(0.0f), y(0.0f) {}
    Vector2D(float x, float y) : x(x), y(y) {}

    // getters
    float getX() const { return x; }
    float getY() const { return y; }

    // functions
    Vector2D operator+(const Vector2D &v2) const
    {
        return Vector2D(x + v2.x, y + v2.y);
    }

    Vector2D operator-(const Vector2D &v2) const
    {
        return Vector2D(x - v2.x, y - v2.y);
    }

    Vector2D operator*(float scalar) const
    {
        return Vector2D(x * scalar, y * scalar);
    }

    Vector2D operator/(float scalar) const
    {
        if (scalar == 0.0f)
        {
            throw std::runtime_error("Division by zero in Vector2D");
        }
        return Vector2D(x / scalar, y / scalar);
    }

    Vector2D &operator+=(const Vector2D &v2)
    {
        x += v2.x;
        y += v2.y;
        return *this;
    }

    Vector2D &operator-=(const Vector2D &v2)
    {
        x -= v2.x;
        y -= v2.y;
        return *this;
    }

    float dot(const Vector2D &v2) const
    {
        return x * v2.x + y * v2.y;
    }

    float length() const
    {
        return std::sqrt(x * x + y * y);
    }

    Vector2D normalize() const
    {
        float len = length();
        if (len == 0.0f)
            return Vector2D(0.0f, 0.0f);
        return *this / len;
    }

    Vector2D tangent() const
    {
        return Vector2D(-y, x);
    }
};