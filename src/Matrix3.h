#ifndef MATRIX3_H
#define MATRIX3_H

#include <iostream>

#include "Cartesian3.h"

#define DEG2RAD(x) (M_PIf*static_cast<float>(x)/180.0f)

class Matrix3 {
public:
    float coordinates[3][3];

    // default to the zero matrix
    Matrix3();

    float* operator [](int rowIndex);

    const float* operator [](int rowIndex) const;

    Matrix3 operator *(float factor) const;

    Cartesian3 operator *(const Cartesian3& vector) const;

    Matrix3 operator *(const Matrix3& other) const;

    Matrix3 transpose() const;

    Matrix3 inverse() const;
};

Matrix3 operator *(float factor, const Matrix3& matrix);

std::ostream& operator <<(std::ostream& outStream, const Matrix3& value);

#endif
