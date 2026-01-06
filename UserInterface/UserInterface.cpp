import Models.ColorType;
import Models.Player;
import Core.GameState;
#include "UserInterface.h"
#include "PlayerPanelWidget.h"
#include "Preparation.h"
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

	if (player1 && !player1->m_player) {
		player1->m_player = std::make_unique<Models::Player>(1, std::string("Player1"));
	}
	if (player2 && !player2->m_player) {
		player2->m_player = std::make_unique<Models::Player>(2, std::string("Player2"));
	}

	PreparationDialog dlg(this);
	if (dlg.exec() == QDialog::Accepted)
	{
		QString p1 = dlg.player1Name();
		QString p2 = dlg.player2Name();

		if (p1.isEmpty()) p1 = QString("Player1");
		if (p2.isEmpty()) p2 = QString("Player2");

		if (player1 && player1->m_player)
			player1->m_player->setPlayerUsername(p1.toStdString());
		if (player2 && player2->m_player)
			player2->m_player->setPlayerUsername(p2.toStdString());
	}


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

	splitter->setStretchFactor(0, 2);
	splitter->setStretchFactor(1, 4);
	splitter->setStretchFactor(2, 2);

	setCentralWidget(splitter);
}

UserInterface::~UserInterface()
{
}
