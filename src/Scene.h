#ifndef SCENE
#define SCENE

#include "IndexedFaceSurface.h"
#include "Terrain.h"
#include "Matrix4.h"
#include "Quaternion.h"

class Scene {
public:
    Scene();

    void update();

    void render();

    /* camera control events: WASD for motion */
    void eventCameraForward();

    void eventCameraLeft();

    void eventCameraRight();

    void eventCameraBackward();

    /* camera control events: RF for vertical motion */
    void eventCameraUp();

    void eventCameraDown();

    /* camera rotation events: QE for left and right */
    void eventCameraTurnLeft();

    void eventCameraTurnRight();

    /* simulation events */
    void resetPhysics();

    void switchTerrain();

    void switchModel();

    void rotateLaunchLeft();

    void rotateLaunchRight();

private:
    Terrain flatLand;
    Terrain stripeLand;
    Terrain rollingLand;

    Terrain* activeTerrain;

    IndexedFaceSurface sphere;
    IndexedFaceSurface dodecahedron;

    // true -> show sphere, false -> show dodecahedron
    bool useSphere;

    // angle of launch of ball (rotation around Z)
    float launchAngle;

    Matrix4 viewMatrix;

    // the frame number for use in animating
    unsigned long frameNumber;

    // ball properties
    Cartesian3 ballPosition;
    // it is assumed mass = 1 => velocity is effectively linear momentum
    Cartesian3 ballVelocity;
    Quaternion ballOrientation;
    Cartesian3 ballAngularVelocity;
};

#endif
