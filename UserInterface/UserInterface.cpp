#include "BoardWidget.h"
#include "UserInterface.h"
#include "PlayerPanelWidget.h"
#include "WonderSelectionWidget.h"
#include "WonderSelectionController.h"
#include "Preparation.h"
#include "AgeTreeWidget.h"
#include <QtWidgets/QSplitter>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QLabel>
#include <QtCore/QTimer>

import Core.Board;
import Core.Game;
import Core.Player;
import Core.GameState;

UserInterface::UserInterface(QWidget* parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	
	initializePlayers();
	setupLayout();
	startWonderSelection();

	if (m_boardWidget) m_boardWidget->refresh();
}

UserInterface::~UserInterface()
{
	delete m_wonderController;
}

void UserInterface::initializePlayers()
{
	auto& gameState = Core::GameState::getInstance();
	auto p1 = gameState.GetPlayer1();
	auto p2 = gameState.GetPlayer2();

	if (p1 && !p1->m_player) {
		p1->m_player = std::make_unique<Models::Player>(1, std::string("Player1"));
	}
	if (p2 && !p2->m_player) {
		p2->m_player = std::make_unique<Models::Player>(2, std::string("Player2"));
	}

	PreparationDialog dlg(this);
	if (dlg.exec() == QDialog::Accepted) {
		QString n1 = dlg.player1Name();
		QString n2 = dlg.player2Name();

		if (p1 && p1->m_player && !n1.isEmpty())
			p1->m_player->setPlayerUsername(n1.toStdString());
		if (p2 && p2->m_player && !n2.isEmpty())
			p2->m_player->setPlayerUsername(n2.toStdString());
	}

	Core::Game::preparation();
}

void UserInterface::setupLayout()
{
	auto splitter = new QSplitter(this);
	auto& gameState = Core::GameState::getInstance();

	// Left panel
	m_leftPanel = new PlayerPanelWidget(gameState.GetPlayer1(), splitter, true);
	auto leftScroll = new QScrollArea(splitter);
	leftScroll->setWidgetResizable(true);
	leftScroll->setWidget(m_leftPanel);
	leftScroll->setMinimumWidth(260);
	leftScroll->setMaximumWidth(320);
	leftScroll->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
	splitter->addWidget(leftScroll);

	// Center container
	setupCenterPanel(splitter);

	// Right panel
	m_rightPanel = new PlayerPanelWidget(gameState.GetPlayer2(), splitter, false);
	auto rightScroll = new QScrollArea(splitter);
	rightScroll->setWidgetResizable(true);
	rightScroll->setWidget(m_rightPanel);
	rightScroll->setMinimumWidth(260);
	rightScroll->setMaximumWidth(320);
	rightScroll->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
	splitter->addWidget(rightScroll);

	// Splitter configuration
	splitter->setStretchFactor(0, 0);
	splitter->setStretchFactor(1, 1);
	splitter->setStretchFactor(2, 0);
	splitter->setCollapsible(0, true);
	splitter->setCollapsible(2, true);

	setCentralWidget(splitter);
}

void UserInterface::setupCenterPanel(QSplitter* splitter)
{
	m_centerContainer = new QWidget(splitter);
	auto* centerLayout = new QVBoxLayout(m_centerContainer);
	centerLayout->setContentsMargins(0, 0, 0, 0);
	centerLayout->setSpacing(0);

	// Top: Phase banner
	m_centerTop = new QWidget(m_centerContainer);
	auto* topLayout = new QHBoxLayout(m_centerTop);
	topLayout->setContentsMargins(0, 0, 0, 0);
	m_phaseBanner = new QLabel("", m_centerTop);
	m_phaseBanner->setAlignment(Qt::AlignCenter);
	m_phaseBanner->setStyleSheet(
		"QLabel {"
		"  background-color: rgba(245,158,11,180);"
		"  color: #111827;"
		"  padding:6px10px;"
		"  border-radius:8px;"
		"  font-weight: bold;"
		"  font-size:16px;"
		"}");
	m_phaseBanner->hide();
	topLayout->addWidget(m_phaseBanner);

	// Middle: Wonder selection / Age tree
	m_centerMiddle = new QWidget(m_centerContainer);
	auto* middleLayout = new QVBoxLayout(m_centerMiddle);
	middleLayout->setContentsMargins(8, 8, 8, 8);
	middleLayout->setSpacing(8);
	m_centerWidget = new WonderSelectionWidget(m_centerMiddle);
	middleLayout->addWidget(m_centerWidget);

	// Bottom: Board
	m_centerBottom = new QWidget(m_centerContainer);
	m_centerBottom->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_centerBottom->setMinimumHeight(250);
	auto* bottomLayout = new QVBoxLayout(m_centerBottom);
	bottomLayout->setContentsMargins(8, 8, 8, 8);
	m_boardWidget = new BoardWidget(m_centerBottom);
	bottomLayout->addWidget(m_boardWidget);

	centerLayout->addWidget(m_centerTop, 0);
	centerLayout->addWidget(m_centerMiddle, 1);
	centerLayout->addWidget(m_centerBottom, 1);

	splitter->addWidget(m_centerContainer);
}

void UserInterface::startWonderSelection()
{
	m_wonderController = new WonderSelectionController(
		m_centerWidget,
		m_leftPanel,
		m_rightPanel,
		this);

	m_wonderController->onSelectionComplete = [this]() {
		showPhaseTransitionMessage();
	};

	m_wonderController->start();
}

void UserInterface::showPhaseTransitionMessage()
{
	// Clear middle panel
	if (m_centerMiddle && m_centerMiddle->layout()) {
		QLayoutItem* item;
		while ((item = m_centerMiddle->layout()->takeAt(0)) != nullptr) {
			if (item->widget()) item->widget()->deleteLater();
			delete item;
		}
		delete m_centerMiddle->layout();
	}

	// Show "Phase 1 incepe" message
	auto* msgLayout = new QVBoxLayout(m_centerMiddle);
	msgLayout->setContentsMargins(8, 8, 8, 8);

	QLabel* phaseMsg = new QLabel("Phase 1 incepe", m_centerMiddle);
	phaseMsg->setAlignment(Qt::AlignCenter);
	phaseMsg->setStyleSheet("color: white;");
	QFont f = phaseMsg->font();
	f.setPointSize(24);
	f.setBold(true);
	phaseMsg->setFont(f);
	msgLayout->addWidget(phaseMsg);

	m_centerMiddle->setLayout(msgLayout);
	m_centerMiddle->update();
	QApplication::processEvents();

	QTimer::singleShot(1800, this, [this]() {
		showAgeTree(1);
	});
}

void UserInterface::showAgeTree(int age)
{
	// Clear middle panel
	if (m_centerMiddle && m_centerMiddle->layout()) {
		QLayoutItem* item;
		while ((item = m_centerMiddle->layout()->takeAt(0)) != nullptr) {
			if (item->widget()) item->widget()->deleteLater();
			delete item;
		}
		delete m_centerMiddle->layout();
	}

	// Show panels
	if (m_centerTop) m_centerTop->setVisible(true);
	if (m_centerBottom) m_centerBottom->setVisible(true);

	// Create age tree widget
	auto* layout = new QVBoxLayout(m_centerMiddle);
	layout->setContentsMargins(8, 8, 8, 8);
	
	m_ageTreeWidget = new AgeTreeWidget(m_centerMiddle);
	m_ageTreeWidget->setPlayerPanels(m_leftPanel, m_rightPanel);
	
	// Sync current player
	Core::Player* coreCur = Core::getCurrentPlayer();
	auto& gs = Core::GameState::getInstance();
	auto p1 = gs.GetPlayer1().get();
	auto p2 = gs.GetPlayer2().get();
	
	if (coreCur) {
		m_currentPlayerIndex = (coreCur == p1) ? 0 : 1;
	} else {
		m_currentPlayerIndex = 0;
		if (p1) Core::setCurrentPlayer(p1);
	}
	
	m_ageTreeWidget->setCurrentPlayerIndex(m_currentPlayerIndex);
	
	// Handle turn changes
	m_ageTreeWidget->onPlayerTurnChanged = [this](int newPlayerIndex, const QString& playerName) {
		m_currentPlayerIndex = newPlayerIndex;
		if (m_phaseBanner) {
			m_phaseBanner->setText("Turn: " + playerName);
			m_phaseBanner->show();
		}
		if (m_boardWidget) m_boardWidget->refresh();
	};
	
	// Update banner
	if (m_phaseBanner) {
		QString curName = "<unknown>";
		if (m_currentPlayerIndex == 0 && p1 && p1->m_player)
			curName = QString::fromStdString(p1->m_player->getPlayerUsername());
		else if (m_currentPlayerIndex == 1 && p2 && p2->m_player)
			curName = QString::fromStdString(p2->m_player->getPlayerUsername());
		m_phaseBanner->setText("Turn: " + curName);
		m_phaseBanner->show();
	}

	layout->addWidget(m_ageTreeWidget);
	m_ageTreeWidget->showAgeTree(age);
}