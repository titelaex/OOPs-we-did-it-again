import Models.ColorType;
import Models.Player;
#include "UserInterface.h"
#include "PlayerPanelWidget.h"
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QListWidget>
#include <QtCore/QMap>
#include <QString>
#include <bitset>

UserInterface::UserInterface(QWidget* parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

    auto splitter = new QSplitter(this);

    QWidget* central = this->centralWidget();
    if (!central) {
        central = new QWidget(this);
        setCentralWidget(central);
    }

    splitter->addWidget(central);

    auto& gameState = Core::GameState::getInstance();
    std::shared_ptr<Core::Player> player1 = gameState.GetPlayer1();
    std::shared_ptr<Core::Player> player2 = gameState.GetPlayer2();

    auto rightPanel = new PlayerPanelWidget(player1, splitter, false);
    auto scroll = new QScrollArea(splitter);
    scroll->setWidgetResizable(true);
    scroll->setWidget(rightPanel);

    splitter->addWidget(scroll);

   auto leftPanel = new PlayerPanelWidget(player2, splitter, true);
   auto leftScroll = new QScrollArea(splitter);
   leftScroll->setWidgetResizable(true);
   leftScroll->setWidget(leftPanel);

   splitter->insertWidget(0, leftScroll);

    splitter->setStretchFactor(0, 0.7);
    splitter->setStretchFactor(1, 4);
	splitter->setStretchFactor(2, 0.7);

    setCentralWidget(splitter);
}

UserInterface::~UserInterface()
{
}
