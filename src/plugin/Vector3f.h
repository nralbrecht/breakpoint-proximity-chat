#pragma once

#include <cmath>
#include "teamspeak/public_definitions.h"


class Vector3f
{
public:
    float x;
    float y;
    float z;

    Vector3f(float x, float y, float z);
    Vector3f(float *data);

    Vector3f operator+(const Vector3f& other);
    Vector3f operator-(const Vector3f& other);
    Vector3f operator*(const int& value);
    Vector3f operator/(const int& value);

    TS3_VECTOR toTS3();

    float length();
    Vector3f swapYandZ();
    Vector3f normalize(float size = 1.0f);
    // Vector3f lerp(const Vector3f& target, float percentage);
    Vector3f crossProduct(const Vector3f& other);
};
