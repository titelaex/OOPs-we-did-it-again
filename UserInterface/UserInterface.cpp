#include "UserInterface.h"
#include "PlayerPanelWidget.h"
#include "WonderSelectionWidget.h" 
#include "Preparation.h"
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

	centerLayout->addWidget(m_centerTop, 2);
	centerLayout->addWidget(m_centerMiddle, 5);
	centerLayout->addWidget(m_centerBottom, 3);

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
	auto& board = Core::Board::getInstance();
	const auto* nodesPtr = (age ==1) ? &board.getAge1Nodes() : (age ==2) ? &board.getAge2Nodes() : &board.getAge3Nodes();
	const auto& nodes = *nodesPtr;

	// Expand the middle area to maximum height for the tree
	if (auto* rootLayout = qobject_cast<QVBoxLayout*>(m_centerContainer->layout())) {
		rootLayout->setStretch(0,0); // top compact
		rootLayout->setStretch(1,1); // middle fills
		rootLayout->setStretch(2,0); // bottom compact
	}
	if (m_centerTop) m_centerTop->setVisible(false);
	if (m_centerBottom) m_centerBottom->setVisible(false);

	// determine row pattern
	std::vector<int> rows;
	if (age ==1) rows = {2,3,4,5,6 };
	else if (age ==2) rows = {6,5,4,3,2 };
	else rows = {2,3,4,2,4,3,2 };

	// clear existing layout/widgets in middle panel
	QLayout* existing = m_centerMiddle->layout();
	if (existing) {
		clearLayout(existing);
		delete existing;
	}

	auto* layout = new QVBoxLayout(m_centerMiddle);
	layout->setContentsMargins(8,8,8,8);
	layout->setSpacing(0);
	m_centerMiddle->setLayout(layout);

	// Dispose previous scene/view if any
	if (m_ageView) { m_ageView->deleteLater(); m_ageView = nullptr; }
	if (m_ageScene) { m_ageScene->deleteLater(); m_ageScene = nullptr; }

	// Create scene and view (members)
	m_ageScene = new QGraphicsScene(this);
	m_ageView = new QGraphicsView(m_ageScene, m_centerMiddle);
	m_ageView->setRenderHint(QPainter::Antialiasing);
	m_ageView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	m_ageView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	m_ageView->setFrameStyle(QFrame::NoFrame);
	m_ageView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_ageView->setAlignment(Qt::AlignCenter);

	// sizing constants (bigger for readability)
	const int cardW =220; // wider
	const int cardH =145; // taller
	const int hgap =40; // more gap
	const int vgap =44; // more gap

	// compute positions per row, center them
	std::vector<QPointF> positions;
	positions.resize(nodes.size());

	int totalRows = static_cast<int>(rows.size());
	int idx =0;
	int sceneWidth =0;
	int sceneHeight =0;
	for (int r =0; r < totalRows; ++r) {
		int cols = rows[r];
		// compute row width
		int rowWidth = cols * cardW + (cols -1) * hgap;
		sceneWidth = std::max(sceneWidth, rowWidth);
	}

	// starting y
	int y =0;
	for (int r =0; r < totalRows; ++r) {
		int cols = rows[r];
		int rowWidth = cols * cardW + (cols -1) * hgap;
		int x0 = (sceneWidth - rowWidth) /2;
		for (int c =0; c < cols; ++c) {
			if (idx >= static_cast<int>(nodes.size())) break;
			int x = x0 + c * (cardW + hgap);
			positions[idx] = QPointF(x, y);
			++idx;
		}
		y += cardH + vgap;
	}
	sceneHeight = y;
	m_ageScene->setSceneRect(0,0, std::max(sceneWidth,800), std::max(sceneHeight,400));

	// store mapping from node shared_ptr to index for wiring lines
	std::unordered_map<Core::Node*, int> ptrToIndex;
	for (size_t i =0; i < nodes.size(); ++i) {
		ptrToIndex[nodes[i].get()] = static_cast<int>(i);
	}

	// draw nodes (rects for internal, proxy buttons for available nodes)
	std::vector<QGraphicsItem*> rects(nodes.size(), nullptr);
	idx =0;
	for (int r =0; r < totalRows; ++r) {
		int cols = rows[r];
		for (int c =0; c < cols; ++c) {
			if (idx >= static_cast<int>(nodes.size())) break;
			QPointF pos = positions[idx];

			// If node is available -> render as button (clickable), otherwise render as plain card
			bool isAvailable = false;
			if (nodes[idx]) isAvailable = nodes[idx]->isAvailable();

			if (isAvailable) {
				// add a background rounded rect first (so lines can be underneath and button above)
				QRectF rrect(pos, QSizeF(cardW, cardH));
				QPen abgPen(QColor("#f59e0b"));
				abgPen.setWidth(3);
				abgPen.setJoinStyle(Qt::RoundJoin);
				QPainterPath bgPath; bgPath.addRoundedRect(rrect,12,12);
				QGraphicsPathItem* bg = m_ageScene->addPath(bgPath, abgPen, QBrush(QColor("#1f2937")));
				bg->setZValue(0);
				rects[idx] = bg;

				// create QPushButton and embed
				QString name = "<empty>";
				if (nodes[idx]) {
					auto* card = nodes[idx]->getCard();
					if (card) name = QString::fromStdString(card->getName());
				}
				QPushButton* btn = new QPushButton(name);
				btn->setFixedSize(cardW -8, cardH -8); // small padding
				btn->setEnabled(true);
				btn->setStyleSheet(
					"QPushButton {"
					" background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #334155, stop:1 #0f172a);"
					" color: #e5e7eb;"
					" border:2px solid #f59e0b;"
					" border-radius:10px;"
					" font-weight:600;"
					" font-size:14px;"
					" }"
					"QPushButton:disabled { background:#111827; color:#6b7280; border-color:#374151; }"
					"QPushButton:hover { border-color:#fbbf24; }"
				);
				QGraphicsProxyWidget* proxy = m_ageScene->addWidget(btn);
				proxy->setPos(pos + QPointF(4,4));
				proxy->setZValue(2);
				proxy->setAcceptedMouseButtons(Qt::LeftButton);
				// set transform origin to center so scaling looks natural
				proxy->setTransformOriginPoint(proxy->boundingRect().center());
				// subtle shadow for accessible cards
				auto* shadow = new QGraphicsDropShadowEffect();
				shadow->setBlurRadius(18);
				shadow->setOffset(0,3);
				shadow->setColor(QColor(0,0,0,160));
				proxy->setGraphicsEffect(shadow);
				// install event filter on the widget and remember proxy
				btn->installEventFilter(this);
				m_proxyMap[btn] = proxy;
				// capture index and age
				connect(btn, &QPushButton::clicked, this, [this, idx, age]() { this->handleLeafClicked(idx, age); });
			}
			else {
				// draw card rounded rectangle (unavailable)
				QRectF rect(pos, QSizeF(cardW, cardH));
				QColor bg = nodes[idx] && nodes[idx]->isAvailable() ? QColor("#334155") : QColor("#0f172a");
				QLinearGradient grad(rect.topLeft(), rect.bottomLeft());
				grad.setColorAt(0.0, bg.lighter(130));
				grad.setColorAt(1.0, bg);
				QPen pen(QColor("#94a3b8")); pen.setWidth(3); pen.setJoinStyle(Qt::RoundJoin);
				QPainterPath path; path.addRoundedRect(rect,12,12);
				QGraphicsPathItem* ritem = m_ageScene->addPath(path, pen, QBrush(grad));
				ritem->setZValue(1);
				ritem->setOpacity(0.95);
				// subtle shadow to lift the card
				auto* itemShadow = new QGraphicsDropShadowEffect();
				itemShadow->setBlurRadius(14);
				itemShadow->setOffset(0,2);
				itemShadow->setColor(QColor(0,0,0,110));
				ritem->setGraphicsEffect(itemShadow);
				// add text with modern font and drop shadow
				QString name = "<empty>";
				if (nodes[idx] && nodes[idx]->getCard()) name = QString::fromStdString(nodes[idx]->getCard()->getName());
				QGraphicsTextItem* t = m_ageScene->addText(name, QFont("Segoe UI",12, QFont::Bold));
				t->setDefaultTextColor(Qt::white);
				QRectF tb = t->boundingRect();
				t->setPos(pos.x() + (cardW - tb.width()) /2, pos.y() + (cardH - tb.height()) /2);
				t->setZValue(2);
				// subtle shadow behind text
				QGraphicsTextItem* shadowText = m_ageScene->addText(name, QFont("Segoe UI",12, QFont::Bold));
				shadowText->setDefaultTextColor(QColor(0,0,0,120));
				shadowText->setPos(t->pos() + QPointF(1.5,1.5));
				shadowText->setZValue(1.5);
				rects[idx] = ritem;
			}
			++idx;
		}
	}

	// draw edges based on node child links (put lines under rects and proxies)
	QPen linePen(QColor("#3b2b1b")); linePen.setWidth(3);
	for (size_t i =0; i < nodes.size(); ++i) {
		if (!nodes[i]) continue;
		auto child1 = nodes[i]->getChild1();
		auto child2 = nodes[i]->getChild2();
		QPointF fromCenter = positions[i] + QPointF(cardW /2.0, cardH /2.0);
		if (child1) {
			auto it = ptrToIndex.find(child1.get());
			if (it != ptrToIndex.end()) {
				int ci = it->second;
				QPointF toCenter = positions[ci] + QPointF(cardW /2.0, cardH /2.0);
				QGraphicsLineItem* line = m_ageScene->addLine(QLineF(fromCenter, toCenter), linePen);
				line->setZValue(0);
			}
		}
		if (child2) {
			auto it = ptrToIndex.find(child2.get());
			if (it != ptrToIndex.end()) {
				int ci = it->second;
				QPointF toCenter = positions[ci] + QPointF(cardW /2.0, cardH /2.0);
				QGraphicsLineItem* line = m_ageScene->addLine(QLineF(fromCenter, toCenter), linePen);
				line->setZValue(0);
			}
		}
	}

	// add view and scale to fit after layout pass
	layout->addWidget(m_ageView);

	// Initial fit after layout
	QTimer::singleShot(0, this, [this]() { this->fitAgeTree(); });

	// Refitting on viewport resize to keep tree fully visible
	m_ageView->viewport()->installEventFilter(this);
}

void UserInterface::fitAgeTree()
{
	if (!m_ageView || !m_ageScene) return;
	QRectF sb = m_ageScene->itemsBoundingRect();
	if (sb.isEmpty()) {
		m_ageView->fitInView(m_ageScene->sceneRect(), Qt::KeepAspectRatio);
		return;
	}
	m_ageView->resetTransform();
	m_ageView->fitInView(sb, Qt::KeepAspectRatio);
	m_ageView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	m_ageView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
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
	if (!cardPtr) return;

	if (action == 0) {
		// build -> call playCardBuilding
		cur->playCardBuilding(cardPtr, opp->m_player);
	}
	else if (action == 1) {
		// sell
		auto& discarded = const_cast<std::vector<std::unique_ptr<Models::Card>>&>(board.getDiscardedCards());
		cur->sellCard(cardPtr, discarded);
	}
	else if (action == 2) {
		// use as wonder: would require selecting a specific owned wonder.
		// Skipping actual wonder play here to avoid invalid placeholder cast.
		QMessageBox::information(this, "Use as Wonder", "Selectia de minune trebuie implementata. Actiunea este ignorata momentan.");
	}

	// refresh UI: update left/right panels and redraw tree
	m_leftPanel->refreshWonders();
	m_rightPanel->refreshWonders();
	showAgeTree(age);
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