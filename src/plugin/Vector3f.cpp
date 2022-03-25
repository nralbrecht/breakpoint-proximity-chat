#include "Vector3f.h"


Vector3f::Vector3f(float x, float y, float z) {
    this->x = x;
    this->y = y;
    this->z = z;
}

Vector3f::Vector3f(float *data) {
    x = data[0];
    y = data[1];
    z = data[2];
}

Vector3f Vector3f::operator+(const Vector3f& other) {
    return Vector3f(
        x + other.x,
        y + other.y,
        z + other.z
    );
}

Vector3f Vector3f::operator-(const Vector3f& other) {
    return Vector3f(
        x - other.x,
        y - other.y,
        z - other.z
    );
}

Vector3f Vector3f::operator*(const int& value) {
    return Vector3f(
        x * value,
        y * value,
        z * value
    );
}

Vector3f Vector3f::operator/(const int& value) {
    return Vector3f(
        x / value,
        y / value,
        z / value
    );
}


TS3_VECTOR Vector3f::toTS3() {
    return TS3_VECTOR { x, y, z };
}


float Vector3f::length() {
    return std::sqrt(
        (x * x) +
        (y * y) +
        (z * z)
    );
}

Vector3f Vector3f::normalize(float size) {
    float ownLength = length();

    return Vector3f(
        (size / ownLength) * x,
        (size / ownLength) * y,
        (size / ownLength) * z
    );
}

Vector3f Vector3f::swapYandZ() {
    return Vector3f(
        x,
        z,
        y
    );
}

Vector3f Vector3f::crossProduct(const Vector3f& other) {
    return Vector3f(
        y * other.z - z * other.y,
        z * other.x - x * other.z,
        x * other.y - y * other.x
    );
}
