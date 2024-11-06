#ifndef INDEXED_FACE_SURFACE_H
#define INDEXED_FACE_SURFACE_H

#include <vector>

#include "Cartesian3.h"
#include "Matrix3.h"

class IndexedFaceSurface {
public:
    // vector to hold the vertex indices for each face in CCW
    std::vector<int> faceVertices;
    std::vector<Cartesian3> vertices;
    std::vector<Cartesian3> normals;

    IndexedFaceSurface();

    bool readIndexedFaceFile(const char* fileName);

    void computeUnitNormalVectors();

    void render() const;

    // return the inertial tensor, assuming all vertices are equal weight
    Matrix3 inertialTensor() const;
};

#endif
