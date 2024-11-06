#include <QtWidgets/QApplication>
#include <iostream>
#include <string>

#include "Scene.h"
#include "BallImpulseWidget.h"

int main(int argc, char** argv) {
    QApplication application(argc, argv);

    try {
        Scene scene;

        BallImpulseWidget animationWindow(nullptr, &scene);
        animationWindow.resize(600, 600);
        animationWindow.show();

        return application.exec();
    } catch (std::string errorString) {
        std::cout << "Unable to run application." << errorString << std::endl;
        return EXIT_FAILURE;
    }
}
