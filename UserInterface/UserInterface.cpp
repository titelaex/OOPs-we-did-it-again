#include "BoardWidget.h"
#include "UserInterface.h"
#include "PlayerPanelWidget.h"
#include "WonderSelectionWidget.h" 
#include "Preparation.h"
#include "AgeTreeWidget.h"
#include <QtWidgets/QSplitter>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QWidget> 
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QGraphicsView>
#include <QtWidgets/QGraphicsScene>
#include <QtWidgets/QGraphicsRectItem>
#include <QtWidgets/QGraphicsTextItem>
#include <QtWidgets/QGraphicsProxyWidget>
#include <QtWidgets/QGraphicsDropShadowEffect>
#include <QtGui/QPen>
#include <QtGui/QBrush>
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
import Models.Wonder;
import Models.Card; 
import Core.Player;
import Core.GameState;

namespace {

	void clearLayout(QLayout* layout) {
		if (!layout) return;
		QLayoutItem* item;
		while ((item = layout->takeAt(0)) != nullptr) {
			if (item->widget()) {
				delete item->widget();
			}
			else if (item->layout()) {
				clearLayout(item->layout());
				delete item->layout();
			}
			delete item;
		}
	}
}

UserInterface::UserInterface(QWidget* parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	auto splitter = new QSplitter(this);

	auto& gameState = Core::GameState::getInstance();
	std::shared_ptr<Core::Player> p1 = gameState.GetPlayer1();
	std::shared_ptr<Core::Player> p2 = gameState.GetPlayer2();


	if (p1 && !p1->m_player) {
		p1->m_player = std::make_unique<Models::Player>(1, std::string("Player1"));
	}
	if (p2 && !p2->m_player) {
		p2->m_player = std::make_unique<Models::Player>(2, std::string("Player2"));
	}


	PreparationDialog dlg(this);
	if (dlg.exec() == QDialog::Accepted)
	{
		QString n1 = dlg.player1Name();
		QString n2 = dlg.player2Name();

		if (p1 && p1->m_player && !n1.isEmpty())
			p1->m_player->setPlayerUsername(n1.toStdString());

		if (p2 && p2->m_player && !n2.isEmpty())
			p2->m_player->setPlayerUsername(n2.toStdString());
	}

	Core::Game::preparation();

	m_leftPanel = new PlayerPanelWidget(p1, splitter, true);
	auto leftScroll = new QScrollArea(splitter);
	leftScroll->setWidgetResizable(true);
	leftScroll->setWidget(m_leftPanel);
	// Constrain left panel width so center tree has room
	leftScroll->setMinimumWidth(260);
	leftScroll->setMaximumWidth(320);
	leftScroll->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
	splitter->addWidget(leftScroll);

	m_centerContainer = new QWidget(splitter);
	auto* centerLayout = new QVBoxLayout(m_centerContainer);
	centerLayout->setContentsMargins(0, 0, 0, 0);
	centerLayout->setSpacing(0);

	m_centerTop = new QWidget(m_centerContainer);
	m_centerMiddle = new QWidget(m_centerContainer);
	m_centerBottom = new QWidget(m_centerContainer);
	m_centerBottom->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_centerBottom->setMinimumHeight(250); // Ensure it has a minimum size

	// Add BoardWidget in bottom panel
	auto* bottomLayout = new QVBoxLayout(m_centerBottom);
	bottomLayout->setContentsMargins(8, 8, 8, 8);
	bottomLayout->setSpacing(0);
	m_boardWidget = new BoardWidget(m_centerBottom);
	bottomLayout->addWidget(m_boardWidget);

	// Add a banner label to top area for phase messages
	auto* topLayout = new QHBoxLayout(m_centerTop);
	topLayout->setContentsMargins(0, 0, 0, 0);
	topLayout->setSpacing(0);
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
		" }"
	);
	m_phaseBanner->hide();
	topLayout->addWidget(m_phaseBanner);

	auto* middleLayout = new QVBoxLayout(m_centerMiddle);
	middleLayout->setContentsMargins(8, 8, 8, 8);
	middleLayout->setSpacing(8);
	m_centerMiddle->setStyleSheet("background-color: transparent;");

	centerLayout->addWidget(m_centerTop, 0);
	centerLayout->addWidget(m_centerMiddle, 1);
	centerLayout->addWidget(m_centerBottom, 1);

	splitter->addWidget(m_centerContainer);

	m_centerWidget = new WonderSelectionWidget(m_centerMiddle);
	m_centerWidget->setOnWonderClicked([this](int index) {
		this->onWonderSelected(index);
		});
	middleLayout->addWidget(m_centerWidget);

	m_rightPanel = new PlayerPanelWidget(p2, splitter, false);
	auto rightScroll = new QScrollArea(splitter);
	rightScroll->setWidgetResizable(true);
	rightScroll->setWidget(m_rightPanel);
	// Constrain right panel width so center tree has room
	rightScroll->setMinimumWidth(260);
	rightScroll->setMaximumWidth(320);
	rightScroll->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
	splitter->addWidget(rightScroll);

	// Give most space to center; keep side panels narrow
	splitter->setStretchFactor(0, 0);
	splitter->setStretchFactor(1, 1);
	splitter->setStretchFactor(2, 0);
	splitter->setCollapsible(0, true);
	splitter->setCollapsible(2, true);

	setCentralWidget(splitter);

	startWonderSelection();

	// Refresh board after setup
	if (m_boardWidget) m_boardWidget->refresh();
}

bool UserInterface::eventFilter(QObject* obj, QEvent* event)
{
	if (event->type() == QEvent::Enter) {
		if (auto w = qobject_cast<QWidget*>(obj)) {
			auto it = m_proxyMap.find(w);
			if (it != m_proxyMap.end()) {
				QGraphicsProxyWidget* proxy = it->second;
				proxy->setTransformOriginPoint(proxy->boundingRect().center());
				QPropertyAnimation* anim = new QPropertyAnimation(proxy, "scale", proxy);
				anim->setDuration(140);
				anim->setStartValue(proxy->scale());
				anim->setEndValue(1.08);
				anim->setEasingCurve(QEasingCurve::OutCubic);
				anim->start(QAbstractAnimation::DeleteWhenStopped);
			}
		}
	}
	else if (event->type() == QEvent::Leave) {
		if (auto w = qobject_cast<QWidget*>(obj)) {
			auto it = m_proxyMap.find(w);
			if (it != m_proxyMap.end()) {
				QGraphicsProxyWidget* proxy = it->second;
				// animate scale back to normal
				QPropertyAnimation* anim = new QPropertyAnimation(proxy, "scale", proxy);
				anim->setDuration(120);
				anim->setStartValue(proxy->scale());
				anim->setEndValue(1.0);
				anim->setEasingCurve(QEasingCurve::OutCubic);
				anim->start(QAbstractAnimation::DeleteWhenStopped);
			}
		}
	}
	return QMainWindow::eventFilter(obj, event);
}

void UserInterface::startWonderSelection()
{
	m_selectionPhase = 0;
	m_cardsPickedInPhase = 0;
	loadNextBatch();
}

void UserInterface::loadNextBatch()
{
	auto& boardInstance = Core::Board::getInstance();

	auto& unusedWonders = const_cast<std::vector<std::unique_ptr<Models::Card>>&>(boardInstance.getUnusedWonders());

	m_currentBatch.clear();

	int count = 0;
	for (const auto& cardUniquePtr : unusedWonders) {

		if (!cardUniquePtr) continue;

		if (auto w = dynamic_cast<Models::Wonder*>(cardUniquePtr.get())) {
			m_currentBatch.push_back(w);
			count++;
		}

		if (count == 4) break;
	}

	if (m_currentBatch.empty()) {
		m_centerWidget->hide();
		return;
	}

	m_centerWidget->show();
	m_centerWidget->loadWonders(m_currentBatch);

	updateTurnLabel();
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

	// hide top and show bottom
	if (m_centerTop) m_centerTop->setVisible(false);
	if (m_centerBottom) m_centerBottom->setVisible(true);

	// create and add AgeTreeWidget (it will build the scene)
	m_ageTreeWidget = new AgeTreeWidget(m_centerMiddle);
	m_ageTreeWidget->onLeafClicked = [this](int nodeIndex, int age) { this->handleLeafClicked(nodeIndex, age); };
	if (!m_centerMiddle->layout()) m_centerMiddle->setLayout(new QVBoxLayout(m_centerMiddle));
	m_centerMiddle->layout()->addWidget(m_ageTreeWidget);
	m_ageTreeWidget->showAgeTree(age);
}

void UserInterface::handleLeafClicked(int nodeIndex, int age)
{
	auto& board = Core::Board::getInstance();
	const auto& nodes = (age == 1) ? board.getAge1Nodes() : (age == 2) ? board.getAge2Nodes() : board.getAge3Nodes();
	if (nodeIndex < 0 || static_cast<size_t>(nodeIndex) >= nodes.size()) return;
	auto node = nodes[static_cast<size_t>(nodeIndex)];
	if (!node) return;
	auto* card = node->getCard();
	if (!card) return;

	// show choices: Build, Sell, Use as Wonder
	QMessageBox msg(this);
	msg.setWindowTitle("Choose action");
	msg.setText(QString::fromStdString(card->getName()));
	QPushButton* buildBtn = msg.addButton("Build", QMessageBox::ActionRole);
	QPushButton* sellBtn = msg.addButton("Sell", QMessageBox::ActionRole);
	QPushButton* wonderBtn = msg.addButton("Use as Wonder", QMessageBox::ActionRole);
	msg.addButton(QMessageBox::Cancel);

	msg.exec();

	QPushButton* clicked = qobject_cast<QPushButton*>(msg.clickedButton());
	if (!clicked) return;

	// Determine selected action
	int action = -1; //0=build,1=sell,2=wonder
	if (clicked == buildBtn) action = 0;
	else if (clicked == sellBtn) action = 1;
	else if (clicked == wonderBtn) action = 2;
	else return;

	// Perform action using Core::Player wrappers. Determine current player from m_currentPlayerIndex
	auto& gs = Core::GameState::getInstance();
	Core::Player* cur = (m_currentPlayerIndex == 0) ? gs.GetPlayer1().get() : gs.GetPlayer2().get();
	Core::Player* opp = (m_currentPlayerIndex == 0) ? gs.GetPlayer2().get() : gs.GetPlayer1().get();
	if (!cur || !opp) return;

	// set current player for CSV actions
	Core::setCurrentPlayer(cur);

	// release card from node into unique_ptr
	std::unique_ptr<Models::Card> cardPtr = node->releaseCard();
	if (!cardPtr) {
		qDebug() << "releaseCard returned null";
		return;
	}
	qDebug() << "Card released:" << QString::fromStdString(cardPtr->getName());

	// Determine affected parent nodes so we can check if they became available
	auto& nodesVec = (age == 1) ? board.getAge1Nodes() : (age == 2) ? board.getAge2Nodes() : board.getAge3Nodes();
	std::vector<size_t> affectedParents;
	auto parent1 = node->getParent1();
	if (parent1) {
		for (size_t i = 0; i < nodesVec.size(); ++i) {
			if (nodesVec[i].get() == parent1.get()) { affectedParents.push_back(i); break; }
		}
	}
	auto parent2 = node->getParent2();
	if (parent2) {
		for (size_t i = 0; i < nodesVec.size(); ++i) {
			if (nodesVec[i].get() == parent2.get()) { affectedParents.push_back(i); break; }
		}
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