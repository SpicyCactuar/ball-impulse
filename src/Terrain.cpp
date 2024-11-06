#include "Terrain.h"

#include <fstream>

Terrain::Terrain(): xyScale(1) {
}

bool Terrain::readTerrainFile(const char* fileName, float xyScale) {
    std::ifstream inFile(fileName);
    if (inFile.bad()) {
        return false;
    }

    // save the xy scale
    this->xyScale = xyScale;

    long height = 0, width = 0;
    inFile >> height >> width;

    // Per row height values
    heightValues.resize(height);
    for (int row = 0; row < height; row++) {
        heightValues[row].resize(width);

        for (int col = 0; col < width; col++) {
            inFile >> heightValues[row][col];
        }
    }

    // We want the triangles to be centred at the origin,
    // with the zero elevation set at 0 z, so we have to juggle things somewhat
    // compute a temporary midpoint for the data so that it will end up centered at the origin
    Cartesian3 midPoint{
        midPoint.x = xyScale * (width / 2),
        midPoint.y = xyScale * (height / 2),
        midPoint.z = 0.0
    };

    // each square of data is two triangles, but the end values don't have squares,
    // so we don't need quite as many vertices
    int nValues = height * width;
    int nTriangles = (height - 1) * (width - 1) * 2;
    vertices.resize(nValues);
    faceVertices.resize(3 * nTriangles);

    // Load vertices
    int vertex = 0;
    for (int row = 0; row < height; row++) {
        for (int col = 0; col < width; col++) {
            vertices[vertex++] = Cartesian3(xyScale * col - midPoint.x,
                                            midPoint.y - xyScale * row,
                                            heightValues[row][col]);
        }
    }

    // Create 2 faces from square
    int faceVertex = 0;
    for (int row = 0; row < height - 1; row++) {
        for (int col = 0; col < width - 1; col++) {
            int baseIndex = row * width + col;
            // first (UR) triangle
            faceVertices[faceVertex++] = baseIndex;
            faceVertices[faceVertex++] = baseIndex + width + 1;
            faceVertices[faceVertex++] = baseIndex + 1;

            // second (LL) triangle
            faceVertices[faceVertex++] = baseIndex;
            faceVertices[faceVertex++] = baseIndex + width;
            faceVertices[faceVertex++] = baseIndex + width + 1;
        }
    }

    computeUnitNormalVectors();

    return true;
}

float Terrain::getHeight(float x, float y) const {
    float height = 0.0f;

    const long nRows = heightValues.size();
    const long nColumns = heightValues[0].size();

    const long totalHeight = (nRows - 1) * xyScale;

    // (0,0) is at the dead centre given the layout of the data
    // It is located at x = nRows / 2 (integer), y = nRows / 2 (integer)
    const long i = nRows / 2;
    const long j = nColumns / 2;

    // correct x and y for this offset (note rows are y, columns are x)
    x = x + j * xyScale;
    y = y + i * xyScale;

    // we need to flip coordinates vertically because the rows start at the top
    y = totalHeight - y;

    // now divide by the x-y scale to get the index
    const long xInteger = x / xyScale;
    const long yInteger = y / xyScale;

    // work out the fractional parts
    const float xRemainder = (x - xyScale * xInteger) / xyScale;
    const float yRemainder = (y - xyScale * yInteger) / xyScale;

    // find the row and column
    const long row = yInteger;
    const long column = xInteger;

    // OK. There are two possibilities - above or below the TL-BR diagonal
    // Since this is the line x = y, it's easy to check
    if (xRemainder < yRemainder) {
        // LL triangle
        // in theory, we want barycentric interpolation, but fortunately, it collapses for us because we have
        // right triangles.
        // y_remainder is alpha, the barycentric coordinate for the UL corner
        // (1.0 - y_remainder) * x_remainder is beta, the barycentric coordinate for the LR corner
        // (1.0 - y_remainder) * (1.0 - x_remainder) is gamma, the barycentric coordinate for the LL corner
        const float alpha = yRemainder;
        const float beta = (1.0 - yRemainder) * xRemainder;
        const float gamma = 1.0 - alpha - beta;

        // compute and return
        height = alpha * heightValues[row][column] +
                 beta * heightValues[row + 1][column + 1] +
                 gamma * heightValues[row + 1][column];
    } else {
        // UR triangle
        // (1.0 - x_remainder) is alpha, the barycentric coordinate for the UL corner
        // x_remainder * y_remainder is beta, the barycentric coordinate for the LR corner
        // x_remainder * (1.0 - y_remainder) is gamma, the barycentric coordinate for the UR corner
        const float alpha = 1.0 - yRemainder;
        const float beta = xRemainder * yRemainder;
        const float gamma = 1.0 - alpha - beta;

        height = alpha * heightValues[row][column] +
                 beta * heightValues[row + 1][column + 1] +
                 gamma * heightValues[row][column + 1];
    }

    return height;
}

Cartesian3 Terrain::getNormal(float x, float y) const {
    const long nRows = heightValues.size();
    const long nColumns = heightValues[0].size();

    // Use this to compute the logical size of the map
    const long totalHeight = (nRows - 1) * xyScale;

    // this returns the height at a known point.
    // we start by noting that (0,0) is in the dead centre, which is located at
    // x = nRows / 2 (integer), y = nRows / 2 (integer)
    const long i = nRows / 2;
    const long j = nColumns / 2;

    // now correct x and y for this offset (note rows are y, columns are x)
    x = x + j * xyScale;
    y = y + i * xyScale;

    // we need to flip coordinates vertically because the rows start at the top
    y = totalHeight - y;

    // now divide by the x-y scale to get the index
    const long xInteger = x / xyScale;
    const long yInteger = y / xyScale;

    // now work out the fractional parts
    const float xRemainder = (x - xyScale * xInteger) / xyScale;
    const float yRemainder = (y - xyScale * yInteger) / xyScale;

    // now we can find the row and column easily
    const long row = yInteger;
    const long column = xInteger;

    // once we have that, we can work out the ID of the square the point is in
    const long squareID = row * (nColumns - 1) + column;

    // There are two possibilities - above or below the TL-BR diagonal
    // Since this is the line x = y, it's easy to check
    if (xRemainder < yRemainder) {
        // LL triangle
        // second triangle in the square
        long faceID = 2 * squareID + 1;

        // return its normal
        return normals[faceID];
    } else {
        // UR triangle
        // first triangle in the square
        long faceID = 2 * squareID;

        // return its normal
        return normals[faceID];
    }
}
