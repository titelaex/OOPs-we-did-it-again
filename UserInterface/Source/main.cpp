#include "UserInterface.h"
#include <QtWidgets/QApplication>

import Core.Game;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    Core::Game::preparation();

    UserInterface window;
    window.show();
    return app.exec();
}
