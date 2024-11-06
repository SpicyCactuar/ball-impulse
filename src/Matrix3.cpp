#include <iomanip>

#include "Matrix3.h"

Matrix3::Matrix3() {
    for (int row = 0; row < 3; row++) {
        for (int col = 0; col < 3; col++) {
            coordinates[row][col] = 0.0;
        }
    }
}

float* Matrix3::operator [](const int rowIndex) {
    return coordinates[rowIndex];
}

const float* Matrix3::operator [](const int rowIndex) const {
    return coordinates[rowIndex];
}

Matrix3 Matrix3::operator *(const float factor) const {
    Matrix3 result;
    // multiply by the factor
    for (int row = 0; row < 3; row++) {
        for (int col = 0; col < 3; col++) {
            result.coordinates[row][col] = coordinates[row][col] * factor;
        }
    }
    return result;
}

Cartesian3 Matrix3::operator *(const Cartesian3& vector) const {
    Cartesian3 result;

    // loop adding products
    for (int row = 0; row < 3; row++) {
        for (int col = 0; col < 3; col++) {
            result[row] += coordinates[row][col] * vector[col];
        }
    }

    return result;
}

Matrix3 Matrix3::operator *(const Matrix3& other) const {
    Matrix3 result;

    // loop adding products
    for (int row = 0; row < 3; row++) {
        for (int col = 0; col < 3; col++) {
            for (int entry = 0; entry < 3; entry++) {
                result.coordinates[row][col] += coordinates[row][entry] * other.coordinates[entry][col];
            }
        }
    }

    return result;
}

Matrix3 Matrix3::transpose() const {
    Matrix3 result;

    // loop flipping values
    for (int row = 0; row < 3; row++) {
        for (int col = 0; col < 3; col++) {
            result.coordinates[row][col] = coordinates[col][row];
        }
    }

    return result;
}

Matrix3 Matrix3::inverse() const {
    Matrix3 coMatrix;

    // fill in the individual entries with cofactors
    coMatrix[0][0] = coordinates[1][1] * coordinates[2][2] - coordinates[1][2] * coordinates[2][1];
    coMatrix[0][1] = coordinates[1][2] * coordinates[2][0] - coordinates[1][0] * coordinates[2][2];
    coMatrix[0][2] = coordinates[1][0] * coordinates[2][1] - coordinates[1][1] * coordinates[2][0];

    coMatrix[1][0] = coordinates[2][1] * coordinates[1][0] - coordinates[2][0] * coordinates[1][1];
    coMatrix[1][1] = coordinates[2][2] * coordinates[0][0] - coordinates[2][0] * coordinates[0][2];
    coMatrix[1][2] = coordinates[2][0] * coordinates[0][1] - coordinates[2][1] * coordinates[0][0];

    coMatrix[2][0] = coordinates[0][1] * coordinates[1][2] - coordinates[0][2] * coordinates[1][1];
    coMatrix[2][1] = coordinates[0][2] * coordinates[1][0] - coordinates[0][0] * coordinates[1][2];
    coMatrix[2][2] = coordinates[0][0] * coordinates[1][1] - coordinates[0][1] * coordinates[1][0];

    // we can also use these entries to compute the determinant, which is just a row or column-wise sum of the signed cofactors
    const float determinant = coordinates[0][0] * coMatrix[0][0]
                      + coordinates[0][1] * coMatrix[0][1]
                      + coordinates[0][2] * coMatrix[0][2];

    if (determinant == 0.0f) {
        // if the determinant is zero, return a zero matrix
        return {};
    } else {
        // otherwise transpose the comatrix and divide by the determinant
        return 1.0f / determinant * coMatrix.transpose();
    }
}

Matrix3 operator *(const float factor, const Matrix3& matrix) {
    // since this is commutative, call the other version
    return matrix * factor;
}

std::ostream& operator <<(std::ostream& outStream, const Matrix3& value) {
    for (int row = 0; row < 3; row++) {
        for (int col = 0; col < 3; col++) {
            outStream << std::setw(12) << std::setprecision(5)
                    << std::fixed << value.coordinates[row][col] << (col == 2 ? "\n" : " ");
        }
    }
    return outStream;
}
