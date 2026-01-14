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
#include <QtCore/QEvent>
#include <QtCore/QPropertyAnimation>
#include <QtGui/QTransform>
#include <algorithm> 
#include <vector> 
#include <memory> 
#include <QtCore/QPointer>
#include <QtCore/QMetaObject>
#include <QtCore/QDebug>



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
	// If an AgeTreeWidget already exists, reuse it to avoid deleting widgets
	if (m_ageTreeWidget) {
		m_ageTreeWidget->showAgeTree(age);
		// ensure top/bottom visibility
		if (m_centerTop) m_centerTop->setVisible(false);
		if (m_centerBottom) m_centerBottom->setVisible(true);
		return;
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

	if (action == 0) {
		qDebug() << "Attempting to build card";
		// capture name and color so we can show it in player's panel if build succeeds
		QString playedName = QString::fromStdString(cardPtr->getName());
		Models::ColorType playedColor = cardPtr->getColor();
		cur->playCardBuilding(cardPtr, opp->m_player);
		qDebug() << "Before playCardBuilding: cardPtr=" << static_cast<const void*>(cardPtr.get()) << " cur=" << static_cast<const void*>(cur);
		cur->playCardBuilding(cardPtr, opp->m_player);
		qDebug() << "After playCardBuilding: cardPtr=" << static_cast<const void*>(cardPtr.get());
		// if cardPtr was moved into the player (build succeeded), cardPtr will be null
		if (!cardPtr) {
			if (m_currentPlayerIndex == 0) {
				if (m_leftPanel) m_leftPanel->showPlayedCard(playedName, playedColor);
			} else {
				if (m_rightPanel) m_rightPanel->showPlayedCard(playedName, playedColor);
			}
		}
		qDebug() << "playCardBuilding returned";
	}
	else if (action == 1) {
		qDebug() << "Selling card";
		auto& discarded = const_cast<std::vector<std::unique_ptr<Models::Card>>&>(board.getDiscardedCards());
		cur->sellCard(cardPtr, discarded);
		qDebug() << "sellCard returned";
	}
	else if (action == 2) {
		qDebug() << "Use as wonder selected - not implemented";
		QMessageBox::information(this, "Use as Wonder", "Selectia de minune trebuie implementata. Actiunea este ignorata momentan.");
	}

	// After action, check if any parent became available
	bool parentBecameAvailable = false;
	for (size_t pi : affectedParents) {
		if (pi < nodesVec.size()) {
			auto pnode = nodesVec[pi];
			if (pnode && pnode->isAvailable()) { parentBecameAvailable = true; break; }
		}
	}

	// refresh UI: update left/right panels and redraw tree
    m_leftPanel->refreshWonders();
    m_rightPanel->refreshWonders();
    qDebug() << "Refreshed panels (queued finishAction)";
    QMetaObject::invokeMethod(this, "finishAction", Qt::QueuedConnection,
                              Q_ARG(int, age), Q_ARG(bool, parentBecameAvailable));
}

void UserInterface::onWonderSelected(int index)
{
	if (index < 0 || static_cast<size_t>(index) >= m_currentBatch.size()) return;

	auto& gameState = Core::GameState::getInstance();
	auto p1 = gameState.GetPlayer1();
	auto p2 = gameState.GetPlayer2();
	auto& board = Core::Board::getInstance();

	std::shared_ptr<Core::Player> currentPlayer;

	if (m_selectionPhase == 0) {
		if (m_cardsPickedInPhase == 0) currentPlayer = p1;
		else if (m_cardsPickedInPhase == 1) currentPlayer = p2;
		else if (m_cardsPickedInPhase == 2) currentPlayer = p2;
		else currentPlayer = p1;
	}
	else {
		if (m_cardsPickedInPhase == 0) currentPlayer = p2;
		else if (m_cardsPickedInPhase == 1) currentPlayer = p1;
		else if (m_cardsPickedInPhase == 2) currentPlayer = p1;
		else currentPlayer = p2;
	}

	Models::Wonder* selectedRawPtr = m_currentBatch[index];

	auto& unusedPool = const_cast<std::vector<std::unique_ptr<Models::Card>>&>(board.getUnusedWonders());

	for (auto it = unusedPool.begin(); it != unusedPool.end(); ++it) {
		if (it->get() == selectedRawPtr) {
			std::unique_ptr<Models::Card> cardPtr = std::move(*it);

			unusedPool.erase(it);

			Models::Wonder* rawW = static_cast<Models::Wonder*>(cardPtr.release());
			std::unique_ptr<Models::Wonder> wonderPtr(rawW);

			currentPlayer->m_player->addWonder(std::move(wonderPtr));
			break;
		}
	}

	m_cardsPickedInPhase++;

	m_currentBatch.erase(m_currentBatch.begin() + index);
	m_centerWidget->loadWonders(m_currentBatch);

	m_leftPanel->refreshWonders();
	m_rightPanel->refreshWonders();

	if (m_currentBatch.size() == 1) {
		if (m_selectionPhase == 0) currentPlayer = p1;
		else currentPlayer = p2;

		Models::Wonder* lastRawPtr = m_currentBatch[0];

		for (auto it = unusedPool.begin(); it != unusedPool.end(); ++it) {
			if (it->get() == lastRawPtr) {
				std::unique_ptr<Models::Card> cardPtr = std::move(*it);
				unusedPool.erase(it);
				Models::Wonder* rawW = static_cast<Models::Wonder*>(cardPtr.release());
				std::unique_ptr<Models::Wonder> wonderPtr(rawW);
				currentPlayer->m_player->addWonder(std::move(wonderPtr));

				m_leftPanel->refreshWonders();
				m_rightPanel->refreshWonders();

				break;
			}
		}

		m_currentBatch.clear();
		m_centerWidget->loadWonders(m_currentBatch);

		m_selectionPhase++;
		m_cardsPickedInPhase = 0;

		if (m_selectionPhase < 2) {
			loadNextBatch();
		}
		else {
			m_centerWidget->setTurnMessage("");
			// lock middle panel height so it remains fixed after selection finished
			// hide selection widget
			if (m_centerWidget) m_centerWidget->hide();

			// Show a simple large text message in the same panel before the age tree
			if (m_centerMiddle) {
				// Clear current middle layout contents
				if (auto* existing = m_centerMiddle->layout()) {
					// remove all items
					QLayoutItem* item;
					while ((item = existing->takeAt(0)) != nullptr) {
						if (item->widget()) delete item->widget();
						delete item;
					}
				}
				else {
					m_centerMiddle->setLayout(new QVBoxLayout(m_centerMiddle));
				}
				auto* msgLayout = qobject_cast<QVBoxLayout*>(m_centerMiddle->layout());
				msgLayout->setContentsMargins(8, 8, 8, 8);
				msgLayout->setSpacing(0);

				QLabel* phaseMsg = new QLabel("Phase 1 incepe", m_centerMiddle);
				phaseMsg->setAlignment(Qt::AlignCenter);
				phaseMsg->setStyleSheet(""); // no formatting, just default
				QFont f = phaseMsg->font();
				f.setPointSize(24); // large text
				f.setBold(true);
				phaseMsg->setFont(f);
				msgLayout->addWidget(phaseMsg);

				// After a short delay, replace message with the age tree
				QTimer::singleShot(1800, this, [this]() {
					showAgeTree(1);
					});
			}

			// Optionally start phase I automatically (console version) in background - not starting here to keep UI responsive
		}
	}
	if (m_currentBatch.size() > 1)
	{
		updateTurnLabel();
	}
}

UserInterface::~UserInterface()
{
}

void UserInterface::updateTurnLabel()
{
	auto& gameState = Core::GameState::getInstance();
	auto p1 = gameState.GetPlayer1();
	auto p2 = gameState.GetPlayer2();

	QString p1Name = QString::fromStdString(p1->m_player->getPlayerUsername());
	QString p2Name = QString::fromStdString(p2->m_player->getPlayerUsername());

	QString currentPlayerName;

	if (m_selectionPhase == 0) {
		if (m_cardsPickedInPhase == 0) currentPlayerName = p1Name;
		else if (m_cardsPickedInPhase == 1) currentPlayerName = p2Name;
		else if (m_cardsPickedInPhase == 2) currentPlayerName = p2Name;
		else currentPlayerName = p1Name;
	}
	else {
		if (m_cardsPickedInPhase == 0) currentPlayerName = p2Name;
		else if (m_cardsPickedInPhase == 1) currentPlayerName = p1Name;
		else if (m_cardsPickedInPhase == 2) currentPlayerName = p1Name;
		else currentPlayerName = p2Name;
	}

	m_centerWidget->setTurnMessage("Este randul lui: " + currentPlayerName);
}

void UserInterface::finishAction(int age, bool parentBecameAvailable)
{
	m_leftPanel->refreshWonders();
	m_rightPanel->refreshWonders();
	qDebug() << "finishAction: refreshing panels and age tree";
	if (m_ageTreeWidget) {
		m_ageTreeWidget->showAgeTree(age);
	}
	// if age1 finished, schedule phase2 transition
	if (age == 1) {
		auto& board = Core::Board::getInstance();
		const auto& nodes = board.getAge1Nodes();
		bool anyAvailable = false;
		for (const auto& n : nodes) {
			if (!n) continue;
			if (n->isAvailable() && n->getCard()) { anyAvailable = true; break; }
		}
		if (!anyAvailable) {
			// show transition
			if (m_centerMiddle) {
				if (auto* existing = m_centerMiddle->layout()) clearLayout(existing);
				else m_centerMiddle->setLayout(new QVBoxLayout(m_centerMiddle));
				auto* msgLayout = qobject_cast<QVBoxLayout*>(m_centerMiddle->layout());
				msgLayout->setContentsMargins(8,8,8,8);
				msgLayout->setSpacing(0);
				QLabel* phaseMsg = new QLabel("Phase 2 incepe", m_centerMiddle);
				phaseMsg->setAlignment(Qt::AlignCenter);
				QFont f = phaseMsg->font(); f.setPointSize(24); f.setBold(true);
				phaseMsg->setFont(f);
				msgLayout->addWidget(phaseMsg);
				QTimer::singleShot(1800, this, [this]() { this->showAgeTree(2); });
			}
		}
	}
}