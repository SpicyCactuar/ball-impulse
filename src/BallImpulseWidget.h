#ifndef BALL_COLLISION_WIDGET
#define BALL_COLLISION_WIDGET

#include <QtGlobal>
#include <QTimer>
#include <QMouseEvent>

// this is necessary to allow compilation in both Qt 5 and Qt 6
#if (QT_VERSION < 0x060000)
#include <QGLWidget>
#define _GEOMETRIC_WIDGET_PARENT_CLASS QGLWidget
#define _GL_WIDGET_UPDATE_CALL updateGL
#else
#include <QtOpenGLWidgets/QOpenGLWidget>
#define _GEOMETRIC_WIDGET_PARENT_CLASS QOpenGLWidget
#define _GL_WIDGET_UPDATE_CALL update
#endif

#include "Scene.h"

class BallImpulseWidget : public _GEOMETRIC_WIDGET_PARENT_CLASS {
    Q_OBJECT

public:
    Scene* scene;

    QTimer* animationTimer;

    BallImpulseWidget(QWidget* parent, Scene* TheScene);

protected:
    void initializeGL() override;

    void resizeGL(int width, int height) override;

    void paintGL() override;

    void keyPressEvent(QKeyEvent* event) override;

public slots:
    void nextFrame();
};

#endif
