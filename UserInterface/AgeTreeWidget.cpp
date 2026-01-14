#include "AgeTreeWidget.h"
<<<<<<< HEAD
#include <QtCore/QDebug>
=======
#include "PlayerPanelWidget.h"
>>>>>>> 615931ae86c80e982d01255dc960b97da9e52d78
#include <QtWidgets/QGraphicsView>
#include <QtWidgets/QGraphicsScene>
#include <QtWidgets/QGraphicsRectItem>
#include <QtWidgets/QGraphicsTextItem>
#include <QtWidgets/QGraphicsProxyWidget>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QLayout>
#include <QtWidgets/QMessageBox>
#include <QtGui/QPen>
#include <QtGui/QBrush>
#include <QtCore/QTimer>
#include <QtCore/QEvent>
#include <QtCore/QPropertyAnimation>
#include <QtWidgets/QGraphicsDropShadowEffect>
#include <QtCore/QVariantAnimation>
#include <QtCore/QDebug>

#include <algorithm>
#include <vector>
#include <unordered_map>

import Core.Board;
import Core.Node;
import Core.Player;
import Core.GameState;
import Models.Card;

// Styled ClickableRect: draws a rounded rect with gradient, border, shadow and text handled separately
class ClickableRect : public QGraphicsRectItem {
public:
    ClickableRect(const QRectF& r)
        : QGraphicsRectItem(r)
    {
        // no hover handling per user's request
        setAcceptHoverEvents(false);
        setAcceptedMouseButtons(Qt::LeftButton);
        m_radius = 8.0;
        m_pen = QPen(QColor("#7C4A1C"), 2); // default border (brown)
        m_topColor = QColor("#4B5563");
        m_bottomColor = QColor("#1F2937");

        // subtle shadow as child behind
        m_shadow = new QGraphicsRectItem(r, this);
        m_shadow->setBrush(QBrush(QColor(0,0,0,40)));
        m_shadow->setPen(Qt::NoPen);
        m_shadow->setZValue(-1);
        m_shadow->setPos(4,4);
    }

    void setGradientColors(const QColor& top, const QColor& bottom) { m_topColor = top; m_bottomColor = bottom; update(); }
    void setBorderColor(const QColor& c) { m_pen.setColor(c); update(); }

    std::function<void()> onClicked;

protected:
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override {
        Q_UNUSED(option);
        Q_UNUSED(widget);
        QRectF r = rect();
        QLinearGradient grad(r.topLeft(), r.bottomLeft());
        grad.setColorAt(0.0, m_topColor);
        grad.setColorAt(1.0, m_bottomColor);
        painter->setRenderHint(QPainter::Antialiasing, true);
        painter->setBrush(QBrush(grad));
        painter->setPen(m_pen);
        painter->drawRoundedRect(r, m_radius, m_radius);
    }

    void mousePressEvent(QGraphicsSceneMouseEvent* event) override {
        QTimer::singleShot(0, [cb = onClicked]() { if (cb) cb(); });
    }

private:
    QGraphicsRectItem* m_shadow{ nullptr };
    qreal m_radius{8.0};
    QPen m_pen;
    QColor m_topColor;
    QColor m_bottomColor;
};

AgeTreeWidget::AgeTreeWidget(QWidget* parent)
    : QWidget(parent), m_view(nullptr), m_scene(nullptr)
{
    qDebug() << "AgeTreeWidget::ctor this=" << static_cast<const void*>(this);
}

AgeTreeWidget::~AgeTreeWidget()
{
    qDebug() << "AgeTreeWidget::dtor this=" << static_cast<const void*>(this);
}

void AgeTreeWidget::setPlayerPanels(PlayerPanelWidget* left, PlayerPanelWidget* right)
{
    m_leftPanel = left;
    m_rightPanel = right;
}

void AgeTreeWidget::setCurrentPlayerIndex(int index)
{
    m_currentPlayerIndex = index;
}

void AgeTreeWidget::refreshPanels()
{
if (m_leftPanel) {
        m_leftPanel->refreshStats();
        m_leftPanel->refreshCards();
        m_leftPanel->refreshWonders();
    }
    if (m_rightPanel) {
   m_rightPanel->refreshStats();
        m_rightPanel->refreshCards();
        m_rightPanel->refreshWonders();
    }
}

void AgeTreeWidget::handleLeafClicked(int nodeIndex, int age)
{
    qDebug() << "AgeTreeWidget::handleLeafClicked called. currentPlayerIndex=" << m_currentPlayerIndex;

    auto& board = Core::Board::getInstance();
    const auto& nodes = (age ==1) ? board.getAge1Nodes() : (age ==2) ? board.getAge2Nodes() : board.getAge3Nodes();
    if (nodeIndex <0 || static_cast<size_t>(nodeIndex) >= nodes.size()) return;
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

 int action = -1;
    if (clicked == buildBtn) action =0;
    else if (clicked == sellBtn) action =1;
    else if (clicked == wonderBtn) action =2;
    else return;

    auto& gs = Core::GameState::getInstance();
    Core::Player* cur = (m_currentPlayerIndex ==0) ? gs.GetPlayer1().get() : gs.GetPlayer2().get();
    Core::Player* opp = (m_currentPlayerIndex ==0) ? gs.GetPlayer2().get() : gs.GetPlayer1().get();
    if (!cur || !opp) return;

    Core::setCurrentPlayer(cur);

    std::unique_ptr<Models::Card> cardPtr = node->releaseCard();
    if (!cardPtr) {
     qDebug() << "releaseCard returned null";
        return;
    }
    qDebug() << "Card released:" << QString::fromStdString(cardPtr->getName());

    // Execute chosen action
    if (action ==0) {
        qDebug() << "Attempting to build card";
        cur->playCardBuilding(cardPtr, opp->m_player);
     qDebug() << "playCardBuilding returned";
  }
    else if (action ==1) {
        qDebug() << "Selling card";
        auto& discarded = const_cast<std::vector<std::unique_ptr<Models::Card>>&>(board.getDiscardedCards());
        cur->sellCard(cardPtr, discarded);
        qDebug() << "sellCard returned";
    }
    else if (action ==2) {
        qDebug() << "Use as wonder selected - not implemented";
    QMessageBox::information(this, "Use as Wonder", "Selectia de minune trebuie implementata. Actiunea este ignorata momentan.");
    }

    // If the action failed (cardPtr still holds), return card to node and do not advance turn
    if (cardPtr) {
  node->setCard(std::move(cardPtr));
        refreshPanels();
        qDebug() << "Action failed; card returned to tree; panels refreshed";
      QTimer::singleShot(0, this, [this, age]() {
          this->showAgeTree(age);
        });
        return;
    }

    // Action succeeded: refresh UI and advance current player
    refreshPanels();

    m_currentPlayerIndex = (m_currentPlayerIndex ==0) ?1 :0;
    auto newCur = (m_currentPlayerIndex ==0) ? gs.GetPlayer1().get() : gs.GetPlayer2().get();
    Core::setCurrentPlayer(newCur);

  // Notify parent UI about player turn change
    if (onPlayerTurnChanged) {
 auto p1 = gs.GetPlayer1();
        auto p2 = gs.GetPlayer2();
        QString curName = (m_currentPlayerIndex ==0 && p1 && p1->m_player)
     ? QString::fromStdString(p1->m_player->getPlayerUsername())
          : (p2 && p2->m_player) ? QString::fromStdString(p2->m_player->getPlayerUsername()) : QString("<unknown>");
        onPlayerTurnChanged(m_currentPlayerIndex, curName);
    }

    qDebug() << "Action succeeded; refreshed panels and advanced turn. newIndex=" << m_currentPlayerIndex;
    QTimer::singleShot(0, this, [this, age]() {
   this->showAgeTree(age);
    });
}

void AgeTreeWidget::showAgeTree(int age)
{
    qDebug() << "AgeTreeWidget::showAgeTree age=" << age << "this=" << static_cast<const void*>(this)
             << " m_scene=" << static_cast<const void*>(m_scene) << " m_view=" << static_cast<const void*>(m_view);
    m_currentAge = age;
    auto& board = Core::Board::getInstance();
    const auto* nodesPtr = (age == 1) ? &board.getAge1Nodes() : (age == 2) ? &board.getAge2Nodes() : &board.getAge3Nodes();
 const auto& nodes = *nodesPtr;

    std::vector<int> rows;
    if (age == 1) rows = {2,3,4,5,6};
    else if (age == 2) rows = {6,5,4,3,2};
    else rows = {2,3,4,2,4,3,2};

    // choose palette depending on age
    QColor visibleTopColor;
    QColor visibleBottomColor;
    QColor sectionBorderColor;
    QColor lineColor;
    QColor visibleTextColor = Qt::white;
    QColor invisibleBorderColor = Qt::white;
    if (age == 2) {
        // blue palette for Age II
        visibleTopColor = QColor("#60A5FA"); // light blue
        visibleBottomColor = QColor("#1E3A8A"); // indigo/dark blue
        sectionBorderColor = QColor("#0369A1");
        lineColor = QColor("#0461A8");
    } else {
        // default brown palette for Age I and others
        visibleTopColor = QColor("#B58860");
        visibleBottomColor = QColor("#7C4A1C");
        sectionBorderColor = QColor("#7C4A1C");
        lineColor = QColor("#3b2b1b");
    }

    // clear previous layout
    if (auto oldLayout = this->layout()) {
  while (auto item = oldLayout->takeAt(0)) {
   if (auto w = item->widget()) {
                w->removeEventFilter(this);
    }
 delete item;
        }
      delete oldLayout;
    }

    if (m_view) { m_view->deleteLater(); m_view = nullptr; }
 if (m_scene) { m_scene->clear(); m_scene->deleteLater(); m_scene = nullptr; }

    m_scene = new QGraphicsScene(this);
    m_view = new QGraphicsView(m_scene, this);
    qDebug() << "AgeTreeWidget: created scene=" << static_cast<const void*>(m_scene) << " view=" << static_cast<const void*>(m_view);
    m_view->setRenderHint(QPainter::Antialiasing);
    m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_view->setFrameStyle(QFrame::NoFrame);
    m_view->setAlignment(Qt::AlignCenter);

    const int cardW = 180;
    const int cardH = 110;
    const int hgap = 28;
    const int vgap = 30;

    std::vector<QPointF> positions(nodes.size());

    int totalRows = static_cast<int>(rows.size());
    int idx = 0;
    int sceneWidth = 0;
 for (int r = 0; r < totalRows; ++r) {
        int cols = rows[r];
    int rowWidth = cols * cardW + (cols - 1) * hgap;
        sceneWidth = std::max(sceneWidth, rowWidth);
    }

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

    m_scene->setSceneRect(0,0, std::max(sceneWidth, 800), std::max(y, 400));

    std::unordered_map<Core::Node*, int> ptrToIndex;
    for (size_t i = 0; i < nodes.size(); ++i) ptrToIndex[nodes[i].get()] = static_cast<int>(i);

    std::vector<QGraphicsRectItem*> rects(nodes.size(), nullptr);
    idx = 0;
    for (int r = 0; r < totalRows; ++r) {
        int cols = rows[r];
        for (int c = 0; c < cols; ++c) {
  if (idx >= static_cast<int>(nodes.size())) break;
QPointF pos = positions[idx];
         bool isAvailable = (nodes[idx] && nodes[idx]->isAvailable());
   if (isAvailable) {
  QRectF rrect(pos, QSizeF(cardW, cardH));
 ClickableRect* item = new ClickableRect(rrect);
           item->setZValue(1);
m_scene->addItem(item);
           rects[idx] = item;

         bool isVisible = false;
        if (nodes[idx] && nodes[idx]->getCard()) isVisible = nodes[idx]->getCard()->isVisible();
if (isVisible) {
         item->setGradientColors(QColor("#B58860"), QColor("#7C4A1C"));
    item->setBorderColor(QColor("#7C4A1C"));
    } else {
              item->setGradientColors(QColor("#FFFFFF"), QColor("#FFFFFF"));
                item->setBorderColor(QColor("#7C4A1C"));
  }

           QString name = "<empty>";
   if (nodes[idx]) {
           auto* card = nodes[idx]->getCard();
           if (card) name = QString::fromStdString(card->getName());
    }
   QGraphicsTextItem* t = m_scene->addText(name);
              QFont f = t->font(); f.setBold(true); f.setPointSize(15);
        t->setFont(f);
        t->setDefaultTextColor(isVisible ? Qt::white : Qt::black);
        QRectF tb = t->boundingRect();
        t->setPos(pos.x() + (cardW - tb.width())/2, pos.y() + (cardH - tb.height())/2);
                t->setZValue(2);

     int nodeIndex = idx;
     item->onClicked = [this, nodeIndex]() {
    this->handleLeafClicked(nodeIndex, m_currentAge);
    };
            } else {
   QRectF rect(pos, QSizeF(cardW, cardH));
     QColor bg = QColor("#EDE7E0");
        QGraphicsRectItem* ritem = m_scene->addRect(rect, QPen(QColor("#7C4A1C"), 3), QBrush(bg));
      ritem->setZValue(1);
     QString name = "<empty>";
          if (nodes[idx] && nodes[idx]->getCard()) name = QString::fromStdString(nodes[idx]->getCard()->getName());
             QGraphicsTextItem* t = m_scene->addText(name);
        t->setDefaultTextColor(Qt::black);
      QRectF tb = t->boundingRect();
       t->setPos(pos.x() + (cardW - tb.width())/2, pos.y() + (cardH - tb.height())/2);
          t->setZValue(2);
     rects[idx] = ritem;
          }
            ++idx;
        }
    }

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
 QGraphicsLineItem* line = m_scene->addLine(QLineF(fromCenter, toCenter), linePen);
            line->setZValue(0);
            }
    }
        if (child2) {
            auto it = ptrToIndex.find(child2.get());
       if (it != ptrToIndex.end()) {
        int ci = it->second;
        QPointF toCenter = positions[ci] + QPointF(cardW/2.0, cardH/2.0);
         QGraphicsLineItem* line = m_scene->addLine(QLineF(fromCenter, toCenter), linePen);
    line->setZValue(0);
          }
        }
    }

    auto* vlayout = new QVBoxLayout(this);
 vlayout->setContentsMargins(8,8,8,8);
    vlayout->setSpacing(0);
    vlayout->addWidget(m_view);
    setLayout(vlayout);

    // Queue fitAgeTree but guard with QPointer to avoid calling on deleted widget
    QPointer<AgeTreeWidget> guard(this);
    QTimer::singleShot(0, [guard]() mutable {
        if (guard) guard->fitAgeTree();
    });

    if (m_view && m_view->viewport()) m_view->viewport()->installEventFilter(this);
}

void AgeTreeWidget::fitAgeTree()
{
    if (!m_view || !m_scene) return;
    QRectF sb = m_scene->itemsBoundingRect();
    if (sb.isEmpty()) {
        m_view->fitInView(m_scene->sceneRect(), Qt::KeepAspectRatio);
        return;
    }
    m_view->resetTransform();
    m_view->fitInView(sb, Qt::KeepAspectRatio);
    m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

bool AgeTreeWidget::eventFilter(QObject* obj, QEvent* event)
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
                QPropertyAnimation* anim = new QPropertyAnimation(proxy, "scale", proxy);
                anim->setDuration(120);
                anim->setStartValue(proxy->scale());
                anim->setEndValue(1.0);
                anim->setEasingCurve(QEasingCurve::OutCubic);
                anim->start(QAbstractAnimation::DeleteWhenStopped);
            }
        }
    }
    else if (m_view && obj == m_view->viewport() && event->type() == QEvent::Resize) {
        // Refit when viewport changes size
        fitAgeTree();
    }

    return QWidget::eventFilter(obj, event);
}
