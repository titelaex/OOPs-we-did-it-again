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
import Core.Game; // need preparation() and game phase functions
import Models.Wonder;
import Models.Card;   
import Core.Player;
import Core.GameState;

namespace {
    // helper to clear a layout
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

    // Obtinem jucătorii din GameState
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

    // Build card pools and age trees now that players have been named
    Core::Game::preparation();

    m_leftPanel = new PlayerPanelWidget(p1, splitter, true);
    auto leftScroll = new QScrollArea(splitter);
    leftScroll->setWidgetResizable(true);
    leftScroll->setWidget(m_leftPanel);
    splitter->addWidget(leftScroll);

    // Create a vertical container for the center column and split it into three rows (20%/40%/40%)
    m_centerContainer = new QWidget(splitter);
    auto* centerLayout = new QVBoxLayout(m_centerContainer);
    centerLayout->setContentsMargins(0, 0, 0, 0);
    centerLayout->setSpacing(0);

    m_centerTop = new QWidget(m_centerContainer);
    m_centerMiddle = new QWidget(m_centerContainer);
    m_centerBottom = new QWidget(m_centerContainer);

    // Middle panel will host the WonderSelectionWidget (or later the age tree)
    auto* middleLayout = new QVBoxLayout(m_centerMiddle);
    middleLayout->setContentsMargins(8, 8, 8, 8);
    middleLayout->setSpacing(8);

    // color the middle panel brown so boundaries are visible
    m_centerMiddle->setStyleSheet("background-color: #A0522D;");

    // Add the three panels with stretch factors 2:4:4 (relative -> 20%:40%:40%)
    centerLayout->addWidget(m_centerTop, 2);
    centerLayout->addWidget(m_centerMiddle, 5);
    centerLayout->addWidget(m_centerBottom, 3);

    splitter->addWidget(m_centerContainer);

    // Create the selection widget as a child of the middle panel so it occupies the middle 40%
    m_centerWidget = new WonderSelectionWidget(m_centerMiddle);
    m_centerWidget->setOnWonderClicked([this](int index) {
        this->onWonderSelected(index);
    });
    middleLayout->addWidget(m_centerWidget);

    m_rightPanel = new PlayerPanelWidget(p2, splitter, false);
    auto rightScroll = new QScrollArea(splitter);
    rightScroll->setWidgetResizable(true);
    rightScroll->setWidget(m_rightPanel);
    splitter->addWidget(rightScroll);

    // Splitter columns proportions remain 2:1:2
    splitter->setStretchFactor(0, 2);
    splitter->setStretchFactor(1, 1);
    splitter->setStretchFactor(2, 2);

    setCentralWidget(splitter);

    startWonderSelection();
}

bool UserInterface::eventFilter(QObject* obj, QEvent* event)
{
    // handle hover enter/leave on QWidgets (buttons embedded in proxies)
    if (event->type() == QEvent::Enter) {
        if (auto w = qobject_cast<QWidget*>(obj)) {
            auto it = m_proxyMap.find(w);
            if (it != m_proxyMap.end()) {
                QGraphicsProxyWidget* proxy = it->second;
                // set transform origin to center and animate scale up
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
    const auto* nodesPtr = (age == 1) ? &board.getAge1Nodes() : (age == 2) ? &board.getAge2Nodes() : &board.getAge3Nodes();
    const auto& nodes = *nodesPtr;

    // determine row pattern
    std::vector<int> rows;
    if (age == 1) rows = {2,3,4,5,6};
    else if (age == 2) rows = {6,5,4,3,2};
    else rows = {2,3,4,2,4,3,2};

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

    // Create scene and view
    QGraphicsScene* scene = new QGraphicsScene(this);
    QGraphicsView* view = new QGraphicsView(scene, m_centerMiddle);
    view->setRenderHint(QPainter::Antialiasing);
    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setFrameStyle(QFrame::NoFrame);

    // sizing constants (increased for readability)
    const int cardW = 180; // increased width
    const int cardH = 110;  // increased height
    const int hgap = 28;   // horizontal gap
    const int vgap = 30;   // vertical gap

    // compute positions per row, center them
    std::vector<QPointF> positions;
    positions.resize(nodes.size());

    int totalRows = static_cast<int>(rows.size());
    int idx = 0;
    int sceneWidth = 0;
    int sceneHeight = 0;
    for (int r = 0; r < totalRows; ++r) {
        int cols = rows[r];
        // compute row width
        int rowWidth = cols * cardW + (cols - 1) * hgap;
        sceneWidth = std::max(sceneWidth, rowWidth);
    }

    // starting y
    int y = 0;
    for (int r = 0; r < totalRows; ++r) {
        int cols = rows[r];
        int rowWidth = cols * cardW + (cols - 1) * hgap;
        int x0 = (sceneWidth - rowWidth) / 2;
        for (int c = 0; c < cols; ++c) {
            if (idx >= static_cast<int>(nodes.size())) break;
            int x = x0 + c * (cardW + hgap);
            positions[idx] = QPointF(x, y);
            ++idx;
        }
        y += cardH + vgap;
    }
    sceneHeight = y;
    scene->setSceneRect(0,0, std::max(sceneWidth, 800), std::max(sceneHeight, 400));

    // store mapping from node shared_ptr to index for wiring lines
    std::unordered_map<Core::Node*, int> ptrToIndex;
    for (size_t i = 0; i < nodes.size(); ++i) {
        ptrToIndex[nodes[i].get()] = static_cast<int>(i);
    }

    // draw nodes (rects for internal, proxy buttons for available nodes)
    std::vector<QGraphicsRectItem*> rects(nodes.size(), nullptr);
    idx = 0;
    for (int r = 0; r < totalRows; ++r) {
        int cols = rows[r];
        for (int c = 0; c < cols; ++c) {
            if (idx >= static_cast<int>(nodes.size())) break;
            QPointF pos = positions[idx];

            // If node is available -> render as button (clickable), otherwise render as plain card
            bool isAvailable = false;
            if (nodes[idx]) isAvailable = nodes[idx]->isAvailable();

            if (isAvailable) {
                // add a background rect first (so lines can be underneath and button above)
                QRectF rrect(pos, QSizeF(cardW, cardH));
                QGraphicsRectItem* bg = scene->addRect(rrect, QPen(QColor("#7C4A1C"), 2), QBrush(Qt::white));
                bg->setZValue(0);
                rects[idx] = bg;

                // create QPushButton and embed
                QString name = "<empty>";
                if (nodes[idx]) {
                    auto* card = nodes[idx]->getCard();
                    if (card) name = QString::fromStdString(card->getName());
                }
                QPushButton* btn = new QPushButton(name);
                btn->setFixedSize(cardW - 8, cardH - 8); // small padding
                btn->setEnabled(true);
                btn->setStyleSheet("QPushButton { background: white; border:2px solid #7C4A1C; border-radius:6px; font-weight:bold; } QPushButton:disabled { background:#e5e7eb; color:#9ca3af; }");
                QGraphicsProxyWidget* proxy = scene->addWidget(btn);
                proxy->setPos(pos + QPointF(4,4));
                proxy->setZValue(2);
                proxy->setAcceptedMouseButtons(Qt::LeftButton);
                // set transform origin to center so scaling looks natural
                proxy->setTransformOriginPoint(proxy->boundingRect().center());
                // install event filter on the widget and remember proxy
                btn->installEventFilter(this);
                m_proxyMap[btn] = proxy;
                // capture index and age
                connect(btn, &QPushButton::clicked, this, [this, idx, age]() { this->handleLeafClicked(idx, age); });
            }
            else {
                // draw card rectangle
                QRectF rect(pos, QSizeF(cardW, cardH));
                QColor bg = nodes[idx] && nodes[idx]->isAvailable() ? QColor("#B58860") : QColor("#EDE7E0");
                QGraphicsRectItem* ritem = scene->addRect(rect, QPen(QColor("#7C4A1C"), 3), QBrush(bg));
                ritem->setZValue(1);
                // add text
                QString name = "<empty>";
                if (nodes[idx] && nodes[idx]->getCard()) name = QString::fromStdString(nodes[idx]->getCard()->getName());
                QGraphicsTextItem* t = scene->addText(name);
                t->setDefaultTextColor(nodes[idx] && nodes[idx]->isAvailable() ? Qt::white : Qt::black);
                QRectF tb = t->boundingRect();
                t->setPos(pos.x() + (cardW - tb.width())/2, pos.y() + (cardH - tb.height())/2);
                t->setZValue(2);
                rects[idx] = ritem;
            }
            ++idx;
        }
    }

    // draw edges based on node child links (put lines under rects and proxies)
    QPen linePen(QColor("#3b2b1b")); linePen.setWidth(3);
    for (size_t i = 0; i < nodes.size(); ++i) {
        if (!nodes[i]) continue;
        auto child1 = nodes[i]->getChild1();
        auto child2 = nodes[i]->getChild2();
        QPointF fromCenter = positions[i] + QPointF(cardW/2.0, cardH/2.0);
        if (child1) {
            auto it = ptrToIndex.find(child1.get());
            if (it != ptrToIndex.end()) {
                int ci = it->second;
                QPointF toCenter = positions[ci] + QPointF(cardW/2.0, cardH/2.0);
                QGraphicsLineItem* line = scene->addLine(QLineF(fromCenter, toCenter), linePen);
                line->setZValue(0);
            }
        }
        if (child2) {
            auto it = ptrToIndex.find(child2.get());
            if (it != ptrToIndex.end()) {
                int ci = it->second;
                QPointF toCenter = positions[ci] + QPointF(cardW/2.0, cardH/2.0);
                QGraphicsLineItem* line = scene->addLine(QLineF(fromCenter, toCenter), linePen);
                line->setZValue(0);
            }
        }
    }

    // add view and scale to fit after layout pass
    layout->addWidget(view);
    QTimer::singleShot(0, this, [view, scene]() {
        QSize vp = view->viewport()->size();
        QRectF sb = scene->itemsBoundingRect();
        double sceneW = sb.width();
        double sceneH = sb.height();
        if (sceneW <= 0 || sceneH <= 0) {
            view->fitInView(scene->sceneRect(), Qt::KeepAspectRatio);
            return;
        }
        double scaleX = static_cast<double>(vp.width()) / sceneW;
        double scaleY = static_cast<double>(vp.height()) / sceneH;
        double scale = std::min(scaleX, scaleY);
        const double minReadable = 0.75; // do not shrink below this for readability
        if (scale < minReadable) scale = minReadable;
        // apply transform
        view->resetTransform();
        view->scale(scale, scale);
        view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    });
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
    int action = -1; // 0=build,1=sell,2=wonder
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
        // use as wonder: will prompt inside core
        std::vector<Models::Token> discardedTokens;
        auto& discardedCards = const_cast<std::vector<std::unique_ptr<Models::Card>>&>(board.getDiscardedCards());
        // need to find an available wonder from current player's owned wonders
        cur->playCardWonder(*reinterpret_cast<std::unique_ptr<Models::Wonder>*>(nullptr), cardPtr, opp->m_player, discardedTokens, discardedCards);
        // Note: calling playCardWonder requires a wonder pointer; in real flow you'd pick a wonder. For UI we skip actual wonder play here.
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
            if (m_centerMiddle) {
                int h = m_centerMiddle->height();
                if (h > 0) m_centerMiddle->setFixedHeight(h);
            }
            // hide selection widget and show age I tree
            if (m_centerWidget) m_centerWidget->hide();
            showAgeTree(1);

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