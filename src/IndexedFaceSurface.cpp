#include "IndexedFaceSurface.h"

#include <iomanip>
#include <fstream>
#include <cmath>
#include <cstring>

#ifdef _WIN32
#include <windows.h>
#endif
#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#endif

constexpr int LINE_SIZE_LIMIT = 256;

IndexedFaceSurface::IndexedFaceSurface() {
    vertices.resize(0);
    normals.resize(0);
}

bool IndexedFaceSurface::readIndexedFaceFile(const char* fileName) {
    std::ifstream inFile(fileName);
    if (inFile.bad()) {
        return false;
    }

    char lineBuffer[LINE_SIZE_LIMIT];
    char token[LINE_SIZE_LIMIT];

    long nTriangles = 0, nVertices = 0;

    // read the only header line we care about
    inFile.getline(lineBuffer, LINE_SIZE_LIMIT);
    sscanf(lineBuffer, "# Surface vertices=%ld faces=%ld", &nVertices, &nTriangles);

    // skip next line
    inFile.getline(lineBuffer, LINE_SIZE_LIMIT);

    // allocate space for them all
    vertices.resize(nVertices);
    faceVertices.resize(nTriangles * 3);

    // loop to read the vertices in
    for (size_t vertex = 0; vertex < nVertices; vertex++) {
        long vertexID;
        inFile >> token >> vertexID;

        if (strcmp(token, "Vertex") != 0 || vertexID != vertex) {
            printf("Invalid vertex %ld\n", vertex);
            exit(0);
        }

        inFile >> vertices[vertex].x >> vertices[vertex].y >> vertices[vertex].z;
    }

    // loop to read the faceVertices in
    for (size_t face = 0; face < faceVertices.size() / 3; face++) {
        long faceID;
        inFile >> token >> faceID;

        if (strcmp("Face", token) != 0 || faceID != face) {
            printf("Invalid face %ld\n", face);
            exit(0);
        }

        inFile >> faceVertices[3 * face] >> faceVertices[3 * face + 1] >> faceVertices[3 * face + 2];
    }

    computeUnitNormalVectors();

    return true;
}

void IndexedFaceSurface::computeUnitNormalVectors() {
    // Each 3-indexed faces has 1 normal
    normals.resize(faceVertices.size() / 3);

    // loop through the triangles, computing normal vectors
    for (size_t triangle = 0; triangle < normals.size(); triangle++) {
        Cartesian3 p = vertices[faceVertices[3 * triangle]];
        Cartesian3 q = vertices[faceVertices[3 * triangle + 1]];
        Cartesian3 r = vertices[faceVertices[3 * triangle + 2]];

        Cartesian3 u = q - p;
        Cartesian3 v = r - p;

        normals[triangle] = u.cross(v).unit();
    }
}

void IndexedFaceSurface::render() const {
    // Render all triangles
    glBegin(GL_TRIANGLES);

    for (size_t triangle = 0; triangle < normals.size(); triangle++) {
        glNormal3fv(&normals[triangle].x);
        glVertex3fv(&vertices[faceVertices[3 * triangle]].x);
        glVertex3fv(&vertices[faceVertices[3 * triangle + 1]].x);
        glVertex3fv(&vertices[faceVertices[3 * triangle + 2]].x);
    }

    glEnd();
}

Matrix3 IndexedFaceSurface::inertialTensor() const {
    Matrix3 result;

    // loop through all vertices, adding their contributions to each entry
    for (size_t vertex = 0; vertex < (int) vertices.size(); vertex++) {
        const Cartesian3 contribution = vertices[vertex];

        result[0][0] += contribution.y * contribution.y + contribution.z * contribution.z;
        result[0][1] -= contribution.x * contribution.y;
        result[0][2] -= contribution.x * contribution.z;

        result[1][0] -= contribution.x * contribution.y;
        result[1][1] += contribution.x * contribution.x + contribution.y * contribution.y;
        result[1][2] -= contribution.y * contribution.z;

        result[2][0] -= contribution.x * contribution.z;
        result[2][1] -= contribution.y * contribution.z;
        result[2][2] += contribution.x * contribution.x + contribution.y * contribution.y;
    }

    // assume that each vertex has unit mass
    return result;
}
