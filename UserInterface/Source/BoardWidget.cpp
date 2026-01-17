#include "Header/BoardWidget.h"
#include <QtWidgets/QGraphicsPixmapItem>
#include <QtWidgets/QGraphicsEllipseItem>
#include <QtWidgets/QGraphicsTextItem>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QGraphicsDropShadowEffect>
#include <QtGui/QPainter>
#include <QtGui/QPen>
#include <QtGui/QBrush>
#include <QtGui/QPixmap>
#include <QtWidgets/QGraphicsSceneMouseEvent>

import Core.Board;
import Models.Token;

// Clickable token item class
class ClickableTokenItem : public QGraphicsEllipseItem {
public:
	ClickableTokenItem(const QRectF& rect, int tokenIndex, std::function<void(int)> onClick)
		: QGraphicsEllipseItem(rect), m_tokenIndex(tokenIndex), m_onClick(onClick)
	{
		setAcceptHoverEvents(true);
		setCursor(Qt::PointingHandCursor);
		m_normalPen = pen();
		m_hoverPen = QPen(QColor("#fbbf24"), 6); // Gold hover
	}

protected:
	void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override {
		setPen(m_hoverPen);
		setZValue(zValue() + 10);
		QGraphicsEllipseItem::hoverEnterEvent(event);
	}

	void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override {
		setPen(m_normalPen);
		setZValue(zValue() - 10);
		QGraphicsEllipseItem::hoverLeaveEvent(event);
	}

	void mousePressEvent(QGraphicsSceneMouseEvent* event) override {
		if (event->button() == Qt::LeftButton && m_onClick) {
			m_onClick(m_tokenIndex);
		}
		QGraphicsEllipseItem::mousePressEvent(event);
	}

private:
	int m_tokenIndex;
	std::function<void(int)> m_onClick;
	QPen m_normalPen;
	QPen m_hoverPen;
};

BoardWidget::BoardWidget(QWidget* parent) : QWidget(parent)
{
	setAttribute(Qt::WA_TranslucentBackground);

	auto* layout = new QVBoxLayout(this);
	layout->setContentsMargins(0,0,0,0);
	layout->setSpacing(0);

	m_scene = new QGraphicsScene(this);
	m_scene->setBackgroundBrush(Qt::NoBrush);

	m_view = new QGraphicsView(m_scene, this);
	m_view->setRenderHint(QPainter::Antialiasing);
	m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	m_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	m_view->setFrameStyle(QFrame::NoFrame);
	m_view->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_view->setStyleSheet("background: transparent;");
	if (m_view->viewport()) {
		m_view->viewport()->setAttribute(Qt::WA_TranslucentBackground);
	}

	layout->addWidget(m_view);
	drawBoard();
}

void BoardWidget::enableTokenSelection(std::function<void(int)> onTokenClicked)
{
	m_tokenSelectionEnabled = true;
	m_onTokenClicked = onTokenClicked;
	refresh(); // Redraw with clickable tokens
}

void BoardWidget::disableTokenSelection()
{
	m_tokenSelectionEnabled = false;
	m_onTokenClicked = nullptr;
	refresh(); // Redraw without clickable tokens
}

void BoardWidget::refresh()
{
	drawBoard(); // Just redraw, don't clear here since drawBoard clears
}

void BoardWidget::drawBoard()
{
	auto& board = Core::Board::getInstance();	
	m_scene->clear();
	m_tokenItems.clear();
	m_scene->setBackgroundBrush(Qt::NoBrush);
	m_scene->setSceneRect(0,0,2400,600);

	qreal boardWidth =2200;
	qreal boardHeight =280;
	qreal centerX =1200;
	qreal boardY =250;
	qreal endAngleWidth =200;

	const int kTrackSlots =19;
	std::vector<QPointF> track;
	track.reserve(static_cast<size_t>(kTrackSlots));
	double step = (boardWidth *0.8) / (kTrackSlots -1);
	double startX = centerX - (boardWidth *0.4);
	double y = boardY + boardHeight /2;
	for (int i =0; i < kTrackSlots; ++i) track.emplace_back(startX + i * step, y);

	// Main board - now light blue!
	QPainterPath boardPath;
	boardPath.moveTo(centerX - boardWidth /2 + endAngleWidth, boardY);
	boardPath.lineTo(centerX + boardWidth /2 - endAngleWidth, boardY);
	boardPath.lineTo(centerX + boardWidth /2, boardY + boardHeight /2);
	boardPath.lineTo(centerX + boardWidth /2 - endAngleWidth, boardY + boardHeight);
	boardPath.lineTo(centerX - boardWidth /2 + endAngleWidth, boardY + boardHeight);
	boardPath.lineTo(centerX - boardWidth /2, boardY + boardHeight /2);
	boardPath.closeSubpath();

	// Light blue gradient for the board
	QLinearGradient baseGrad(0, boardY,0, boardY + boardHeight);
	baseGrad.setColorAt(0, QColor("#7dd3fc")); // Light blue
	baseGrad.setColorAt(1, QColor("#38bdf8")); // Slightly darker light blue
	m_scene->addPath(boardPath, QPen(QColor("#0ea5e9"),8), QBrush(baseGrad));

	// Token bar - matching light blue theme
	qreal tokenBarWidth =1000;
	qreal tokenBarHeight =140;
	QPainterPath tokenPath;
	tokenPath.addRoundedRect(centerX - tokenBarWidth /2, boardY - tokenBarHeight +20, tokenBarWidth, tokenBarHeight,25,25);
	QLinearGradient topGrad(0, boardY - tokenBarHeight,0, boardY);
	topGrad.setColorAt(0, QColor("#bae6fd"));
	topGrad.setColorAt(1, QColor("#7dd3fc"));
	m_scene->addPath(tokenPath, QPen(QColor("#0ea5e9"),6), QBrush(topGrad));

	int pawnPos = (m_pawnPosition >= 0) ? m_pawnPosition : static_cast<int>(board.getPawnPos());

	// Pawn track circles
	for (int i =0; i < kTrackSlots; ++i) {
		auto& p = track[i];
		bool hasPawn = (i == pawnPos);
		QColor fillColor = hasPawn ? QColor("#fbbf24") : QColor(255,255,255,40);
		QColor borderColor = hasPawn ? QColor("#f59e0b") : QColor("#e6e6e6");
		int borderWidth = hasPawn ? 5 : 4;
		QGraphicsEllipseItem* e = m_scene->addEllipse(
			QRectF(p.x() -30, p.y() -30,60,60), 
			QPen(borderColor, borderWidth), 
			QBrush(fillColor)
		);
		e->setZValue(1);
	}

	// Dotted delimiters
	QPen dottedPen(QColor("#e9c46a"),3, Qt::DotLine);
	for (int i : {0,3,6,8,9,11,14,17}) {
		qreal x_pos = track[i].x() + step /2.0;
		m_scene->addLine(x_pos, y -40, x_pos, y +40, dottedPen);
	}

	drawPawn(pawnPos,0, kTrackSlots -1);

	// Progress tokens
	const auto& tokens = board.getProgressTokens();
	double circleSize =100.0;
	double spacing =25.0;
	const int maxSlots =5;
	double totalWidth = maxSlots * circleSize + (maxSlots -1) * spacing;
	double startTX = centerX - totalWidth /2.0;
	double ty = boardY - tokenBarHeight +30;
	
	for (int i =0; i < maxSlots; ++i) {
		QRectF cRect(startTX + i * (circleSize + spacing), ty, circleSize, circleSize);
		
		if (i < static_cast<int>(tokens.size()) && tokens[i]) {
			// Token exists - draw filled circle with image
			const Models::Token* tok = tokens[i].get();
			QString tokenName = QString::fromStdString(tok->getName());
			
			// Create circular clipping path for the image
			QPainterPath circle;
			circle.addEllipse(cRect);
			
			// Add background circle with border - make it clickable if selection is enabled
			QGraphicsItem* ring = nullptr;
			if (m_tokenSelectionEnabled && m_onTokenClicked) {
				// Create a clickable item
				auto* clickableRing = new ClickableTokenItem(cRect, i, m_onTokenClicked);
				clickableRing->setPen(QPen(QColor("#0ea5e9"),5));
				clickableRing->setBrush(QBrush(QColor("#0284c7")));
				clickableRing->setZValue(3);
				m_scene->addItem(clickableRing);
				ring = clickableRing;
				
				auto* shadow = new QGraphicsDropShadowEffect();
				shadow->setBlurRadius(18);
				shadow->setOffset(0,4);
				shadow->setColor(QColor(0,0,0,150));
				clickableRing->setGraphicsEffect(shadow);
			} else {
				// Non-clickable ring
				auto* pathRing = m_scene->addPath(circle, QPen(QColor("#0ea5e9"),5), QBrush(QColor("#0284c7")));
				pathRing->setZValue(3);
				ring = pathRing;
				
				auto* shadow = new QGraphicsDropShadowEffect();
				shadow->setBlurRadius(18);
				shadow->setOffset(0,4);
				shadow->setColor(QColor(0,0,0,150));
				pathRing->setGraphicsEffect(shadow);
			}
			
			// Load and display token image
			// Try loading from file system first (easier for development)
			QString imagePath = QString("Resources/tokens/%1.png").arg(tokenName);
			QPixmap tokenPixmap(imagePath);
			
			// If file system load fails, try from Qt resources
			if (tokenPixmap.isNull()) {
				imagePath = QString(":/tokens/%1.png").arg(tokenName);
				tokenPixmap = QPixmap(imagePath);
			}
			
			// Images are ready!
			bool useImages = true; // Set to false if you want to use letter fallback
			
			if (useImages && !tokenPixmap.isNull()) {
				// Scale image to fill the entire circle (100x100)
				QPixmap scaledPixmap = tokenPixmap.scaled(
					circleSize, circleSize,
					Qt::KeepAspectRatio,
					Qt::SmoothTransformation
				);
				
				// Center the image in the circle
				auto* pixmapItem = m_scene->addPixmap(scaledPixmap);
				pixmapItem->setPos(
					cRect.center().x() - scaledPixmap.width() / 2.0,
					cRect.center().y() - scaledPixmap.height() / 2.0
				);
				pixmapItem->setZValue(4);
				
				// Make image clickable too if selection is enabled
				if (m_tokenSelectionEnabled && m_onTokenClicked) {
					pixmapItem->setAcceptedMouseButtons(Qt::LeftButton);
					pixmapItem->setCursor(Qt::PointingHandCursor);
					// Note: QGraphicsPixmapItem doesn't have click events by default,
					// so clicks will go through to the ring below
				}
			} else {
				// Fallback to letter if image not found
				QString icon = tokenName.left(1).toUpper();
				QGraphicsTextItem* iconText = m_scene->addText(icon, QFont("Segoe UI",36, QFont::Bold));
				iconText->setDefaultTextColor(QColor("#e9d5ff"));
				QRectF ib = iconText->boundingRect();
				iconText->setPos(cRect.center().x() - ib.width() /2.0, cRect.center().y() - ib.height() /2.0);
				iconText->setZValue(4);
			}
			
			// Add token name label below
			QGraphicsTextItem* t = m_scene->addText(tokenName, QFont("Segoe UI",11, QFont::Bold));
			t->setDefaultTextColor(QColor("#1e1b4b"));
			QRectF tb = t->boundingRect();
			t->setPos(cRect.center().x() - tb.width() /2.0, cRect.bottom() +5);
			t->setZValue(4);
		} else {
			// Empty slot - draw placeholder like player panel
			QPainterPath emptyCircle;
			emptyCircle.addEllipse(cRect);
			auto* placeholder = m_scene->addPath(emptyCircle, 
				QPen(QColor("#0ea5e9"),2), 
				QBrush(QColor(255,255,255,10)));
			placeholder->setZValue(2);
		}
	}

	m_view->fitInView(m_scene->sceneRect(), Qt::KeepAspectRatio);
}

void BoardWidget::resizeEvent(QResizeEvent* event)
{
	QWidget::resizeEvent(event);
	if (m_view && m_scene) {
		m_view->fitInView(m_scene->sceneRect(), Qt::KeepAspectRatio);
	}
}

void BoardWidget::drawPawn(int position, int minPos, int maxPos)
{
	qreal boardWidth =2200;
	qreal boardHeight =280;
	qreal centerX =1200;
	qreal boardY =250;
	double step = (boardWidth *0.8) / (19 -1);
	double startX = centerX - (boardWidth *0.4);
	double x = startX + position * step;
	double y = boardY + boardHeight /2;

	QPainterPath pawnPath;
	pawnPath.addEllipse(QPointF(x, y -30),22,22);
	pawnPath.addRect(x -12, y -30,24,50);
	pawnPath.addEllipse(QPointF(x, y +28),14,14);
	auto* pawn = m_scene->addPath(pawnPath, QPen(QColor("#f87171"),6), QBrush(QColor("#ef4444")));
	pawn->setZValue(5);
	auto* shadow = new QGraphicsDropShadowEffect();
	shadow->setBlurRadius(25);
	shadow->setOffset(5,5);
	shadow->setColor(QColor(0,0,0,120));
	pawn->setGraphicsEffect(shadow);
}

void BoardWidget::setPawnPosition(int position)
{
	m_pawnPosition = position;
	refresh();
}