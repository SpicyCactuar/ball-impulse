#include "Scene.h"

#include <array>
#include <limits>
#include <cmath>

#include "Quaternion.h"

#ifdef _WIN32
#include <windows.h>
#endif

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

// three local variables with the hardcoded file names
const std::string flatLandModelName = "assets/flatland.dem";
const std::string stripeLandModelName = "assets/stripeland.dem";
const std::string rollingLandModelName = "assets/rollingland.dem";
const std::string sphereModelName = "assets/spheroid.face";
const std::string dodecahedronModelName = "assets/dodecahedron.face";

// the speed of camera movement
constexpr float cameraSpeed = 5.0f;

// this is 60 fps nominal speed
constexpr float frameTime = 0.0166667f;

// permanent downwards vector for gravity
const Cartesian3 gravity(0.0, 0.0, -9.8);

// radius of the sphere
constexpr float sphereRadius = 1.0f;

// bounce properties
constexpr float elasticity = 0.6f;

// initial ball position
const Cartesian3 initialBallPosition(0.0f, 0.0f, 10.0f);
const Cartesian3 initialBallVelocity(5.0f, 0.0f, 0.0f);
const Quaternion initialBallOrientation({0.0f, 0.0f, 1.0f}, 0.0f);
const Cartesian3 initialBallAngularVelocity(0.0f, 0.0f, 0.0f);

const Homogeneous4 sunDirection(0.5, -0.5, 0.3, 0.0);
constexpr std::array<float, 4> groundColour{0.2, 0.5, 0.2, 1.0};
constexpr std::array<float, 4> ballColour{0.6, 0.6, 0.6, 1.0};
constexpr std::array<float, 4> sunAmbient{0.1, 0.1, 0.1, 1.0};
constexpr std::array<float, 4> sunDiffuse{0.7, 0.7, 0.7, 1.0};
constexpr std::array<float, 4> blackColour{0.0, 0.0, 0.0, 1.0};

// constructor
Scene::Scene() {
    sphere.readIndexedFaceFile(sphereModelName.data());
    dodecahedron.readIndexedFaceFile(dodecahedronModelName.data());
    flatLand.readTerrainFile(flatLandModelName.data(), 3);
    stripeLand.readTerrainFile(stripeLandModelName.data(), 3);
    rollingLand.readTerrainFile(rollingLandModelName.data(), 3);

    // initial active terrain is flat
    activeTerrain = &flatLand;
    viewMatrix = Matrix4::translation(Cartesian3(0.0, 15.0, -10.0));
    frameNumber = 0;
    // show sphere as default
    useSphere = true;
    launchAngle = 0.0f;

    resetPhysics();
}

void Scene::update() {
    frameNumber++;

    // Gravity is a permanent force
    ballVelocity = ballVelocity + gravity * frameTime;

    // The rest depends on whether we have the sphere or the dodecahedron.
    // For simplicity, we will code it redundantly
    if (useSphere) {
        // if colliding against the terrain, apply bounce impulse instantaneously
        const float terrainHeight = activeTerrain->getHeight(ballPosition.x, ballPosition.y);
        const float dz = ballPosition.z - terrainHeight;
        const bool isBallColliding = dz < sphereRadius || std::abs(dz) < std::numeric_limits<float>::epsilon();
        if (isBallColliding) {
            const Cartesian3 terrainNormal = activeTerrain->getNormal(ballPosition.x, ballPosition.y);
            const Cartesian3 bounceImpulse = -(1.0f + elasticity) * ballVelocity.dot(terrainNormal) * terrainNormal;
            ballVelocity = ballVelocity + bounceImpulse;
            // Snap the sphere on top of the terrain to avoid penetration
            ballPosition.z = terrainHeight + sphereRadius;
        }
    } else {
        // Find the vertex that is colliding deepest inside the terrain
        const float terrainHeight = activeTerrain->getHeight(ballPosition.x, ballPosition.y);
        const Cartesian3 terrainPoint(ballPosition.x, ballPosition.y, terrainHeight);
        const Cartesian3 terrainNormal = activeTerrain->getNormal(ballPosition.x, ballPosition.y);
        float minProjection = std::numeric_limits<float>::infinity();
        Cartesian3 deepestVertex;
        for (const auto& vertice : dodecahedron.vertices) {
            const Cartesian3 vertexWcs = Matrix4::translation(ballPosition) * ballOrientation.asMatrix() * vertice;
            const Cartesian3 terrainToVertex = vertexWcs - terrainPoint;
            if (const float distance = terrainToVertex.dot(terrainNormal); distance < minProjection) {
                minProjection = distance;
                deepestVertex = vertice;
            }
        }

        // If half-space test is < 0, means that the point is in the opposite side of the terrain
        // Therefore, it is colliding with the terrain
        const bool isDodecahedronColliding = minProjection < 0.0f;
        if (isDodecahedronColliding) {
            const Cartesian3 bounceImpulse = -(1.0f + elasticity) * ballVelocity.dot(terrainNormal) * terrainNormal;
            ballVelocity = ballVelocity + bounceImpulse;
            const Matrix3 inertia = ballOrientation.asMatrix().asMatrix3() * dodecahedron.inertialTensor() *
                                    ballOrientation.asMatrix().asMatrix3().transpose();
            ballAngularVelocity = ballAngularVelocity + inertia.inverse() * deepestVertex.cross(bounceImpulse);
            // Snap the dodecahedron on top of the terrain to avoid penetration
            ballPosition = ballPosition + std::abs(minProjection) * terrainNormal;
        }

        // Update rotation, avoiding ||w|| = 0 edge case
        if (ballAngularVelocity.length() > 0.0f) {
            ballOrientation = ballOrientation * Quaternion(ballAngularVelocity.unit(),
                                                           ballAngularVelocity.length() * frameTime);
        }
    }

    // After calculating velocity, update position with it
    ballPosition = ballPosition + ballVelocity * frameTime;
}

// routine to tell the scene to render itself
void Scene::render() {
    // enable Z-buffering
    glEnable(GL_DEPTH_TEST);

    // set lighting parameters
    glShadeModel(GL_FLAT);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHTING);
    glLightfv(GL_LIGHT0, GL_AMBIENT, sunAmbient.data());
    glLightfv(GL_LIGHT0, GL_DIFFUSE, sunDiffuse.data());
    glLightfv(GL_LIGHT0, GL_SPECULAR, blackColour.data());
    glLightfv(GL_LIGHT0, GL_EMISSION, blackColour.data());

    // background is sky-blue
    glClearColor(0.7, 0.7, 1.0, 1.0);

    // clear the buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // set the modelview matrix
    glMatrixMode(GL_MODELVIEW);

    // start with the identity
    glLoadIdentity();

    // add the final rotation from z-up to z-backwords
    glRotatef(-90.0, 1.0, 0.0, 0.0);

    // now compute the view matrix by combining camera translation & rotation
    glMultMatrixf(reinterpret_cast<const GLfloat*>(viewMatrix.columnMajor().coordinates));

    // set the light position
    glLightfv(GL_LIGHT0, GL_POSITION, &sunDirection.x);

    // and set a material colour for the ground
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, groundColour.data());
    glMaterialfv(GL_FRONT, GL_SPECULAR, blackColour.data());
    glMaterialfv(GL_FRONT, GL_EMISSION, blackColour.data());

    // render the terrain
    activeTerrain->render();

    // set the colour for the ball
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, ballColour.data());

    // and update the modelview matrix
    glTranslatef(ballPosition.x, ballPosition.y, ballPosition.z);
    glMultMatrixf(reinterpret_cast<GLfloat*>(ballOrientation.asMatrix().columnMajor().coordinates));

    // now render the ball
    if (useSphere) {
        sphere.render();
    } else {
        dodecahedron.render();
    }
}

void Scene::eventCameraForward() {
    viewMatrix = Matrix4::translation(Cartesian3(0.0, -1.0, 0.0) * cameraSpeed) * viewMatrix;
}

void Scene::eventCameraBackward() {
    viewMatrix = Matrix4::translation(Cartesian3(0.0, 1.0, 0.0) * cameraSpeed) * viewMatrix;
}

void Scene::eventCameraLeft() {
    viewMatrix = Matrix4::translation(Cartesian3(1.0, 0.0, 0.0) * cameraSpeed) * viewMatrix;
}

void Scene::eventCameraRight() {
    viewMatrix = Matrix4::translation(Cartesian3(-1.0, 0.0, 0.0) * cameraSpeed) * viewMatrix;
}

void Scene::eventCameraUp() {
    viewMatrix = Matrix4::translation(Cartesian3(0.0, 0.0, -1.0) * cameraSpeed) * viewMatrix;
}

void Scene::eventCameraDown() {
    viewMatrix = Matrix4::translation(Cartesian3(0.0, 0.0, 1.0) * cameraSpeed) * viewMatrix;
}

void Scene::eventCameraTurnLeft() {
    // separate the translation & rotation
    Matrix4 rotation = viewMatrix.rotationMatrix();
    const Cartesian3 translation = viewMatrix.translation();

    // find the delta of the rotation
    const Matrix4 rotationDelta = Matrix4::rotationZ(2.0f);

    // update the translation vector from the rotation delta
    const Cartesian3 newTranslation = rotationDelta * translation;

    // update the rotation matrix
    rotation = rotationDelta * rotation;

    // now update the view matrix
    viewMatrix = Matrix4::translation(newTranslation) * rotation;
}

void Scene::eventCameraTurnRight() {
    // separate the translation & rotation
    Matrix4 rotation = viewMatrix.rotationMatrix();
    const Cartesian3 translation = viewMatrix.translation();

    // find the delta of the rotation
    const Matrix4 rotationDelta = Matrix4::rotationZ(-2.0f);

    // update the translation vector from the rotation delta
    const Cartesian3 newTranslation = rotationDelta * translation;

    // update the rotation matrix
    rotation = rotationDelta * rotation;

    // now update the view matrix
    viewMatrix = Matrix4::translation(newTranslation) * rotation;
}


void Scene::resetPhysics() {
    ballPosition = initialBallPosition;
    ballVelocity = Matrix4::rotationZ(launchAngle) * initialBallVelocity;
    ballOrientation = initialBallOrientation;
    ballAngularVelocity = initialBallAngularVelocity;
}

void Scene::switchTerrain() {
    if (activeTerrain == &flatLand) {
        activeTerrain = &stripeLand;
    } else if (activeTerrain == &stripeLand) {
        activeTerrain = &rollingLand;
    } else if (activeTerrain == &rollingLand) {
        activeTerrain = &flatLand;
    }
}

void Scene::switchModel() {
    useSphere = !useSphere;
    resetPhysics();
}

void Scene::rotateLaunchLeft() {
    launchAngle -= 5.0;
}

void Scene::rotateLaunchRight() {
    launchAngle += 5.0;
}
