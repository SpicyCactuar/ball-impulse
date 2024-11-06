#include "BallImpulseWidget.h"

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

BallImpulseWidget::BallImpulseWidget(QWidget* parent, Scene* TheScene)
    : _GEOMETRIC_WIDGET_PARENT_CLASS(parent),
      scene(TheScene) {
    animationTimer = new QTimer(this);
    connect(animationTimer, SIGNAL(timeout()), this, SLOT(nextFrame()));
    animationTimer->start(16.6667f);
}

void BallImpulseWidget::initializeGL() {
}

void BallImpulseWidget::resizeGL(const int width, const int height) {
    // reset the viewport
    glViewport(0, 0, width, height);

    // set projection matrix based on zoom & window size
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    // compute the aspect ratio of the widget
    float aspectRatio = static_cast<float>(width) / static_cast<float>(height);

    // we want a 90 degree vertical field of view, as wide as the window allows
    // and we want to see from just in front of us to 100km away
    gluPerspective(90.0, aspectRatio, 0.1, 100000);

    // set model view matrix
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void BallImpulseWidget::paintGL() {
    scene->render();
}

void BallImpulseWidget::keyPressEvent(QKeyEvent* event) {
    switch (event->key()) {
        case Qt::Key_X:
            exit(0);
        // camera controls
        case Qt::Key_W:
            scene->eventCameraForward();
            break;
        case Qt::Key_A:
            scene->eventCameraLeft();
            break;
        case Qt::Key_S:
            scene->eventCameraBackward();
            break;
        case Qt::Key_D:
            scene->eventCameraRight();
            break;
        case Qt::Key_F:
            scene->eventCameraDown();
            break;
        case Qt::Key_R:
            scene->eventCameraUp();
            break;
        case Qt::Key_Q:
            scene->eventCameraTurnLeft();
            break;
        case Qt::Key_E:
            scene->eventCameraTurnRight();
            break;
        // Environment controls
        case Qt::Key_Space:
            scene->resetPhysics();
            break;
        case Qt::Key_L:
            scene->switchTerrain();
            break;
        case Qt::Key_M:
            scene->switchModel();
            break;
        case Qt::Key_Greater:
            scene->rotateLaunchLeft();
            break;
        case Qt::Key_Less:
            scene->rotateLaunchRight();
            break;
        default:
            break;
    }
}

void BallImpulseWidget::nextFrame() {
    scene->update();

    update();
}
