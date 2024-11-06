#ifndef TERRAIN
#define TERRAIN

#include <vector>

#include "IndexedFaceSurface.h"

class Terrain : public IndexedFaceSurface {
public:
    // height value per (x, y) coordinate
    std::vector<std::vector<float>> heightValues;
    float xyScale;

    Terrain();

    // reads .dem elevation/terrain model
    // xyScale gives the scale factor to use in the x-y directions
    bool readTerrainFile(const char* fileName, float xyScale);

    // query height at a given (x, y) coordinate
    float getHeight(float x, float y) const;

    // find normal vector at a given (x,y) coordinate
    Cartesian3 getNormal(float x, float y) const;
};

#endif
