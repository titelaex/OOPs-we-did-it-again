#include "Header/BoardWidget.h"
#include <QtWidgets/QGraphicsPixmapItem>
#include <QtWidgets/QGraphicsEllipseItem>
#include <QtWidgets/QGraphicsTextItem>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QGraphicsDropShadowEffect>
#include <QtGui/QPainter>
#include <QtGui/QPen>
#include <QtGui/QBrush>

import Core.Board;
import Models.Token;

BoardWidget::BoardWidget(QWidget* parent) : QWidget(parent)
{
	// Transparent background for the widget/view/scene
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

void BoardWidget::refresh()
{
	m_scene->clear();
	drawBoard();
}

void BoardWidget::drawBoard()
{
	auto& board = Core::Board::getInstance();
	m_scene->clear();
	m_scene->setBackgroundBrush(Qt::NoBrush);
	m_scene->setSceneRect(0,0,2400,600);

	// Define board geometry
	qreal boardWidth =2200;
	qreal boardHeight =280;
	qreal centerX =1200;
	qreal boardY =250;
	qreal endAngleWidth =200;

	// Pawn track positions (19 slots)
	const int kTrackSlots =19;
	std::vector<QPointF> track;
	track.reserve(static_cast<size_t>(kTrackSlots));
	double step = (boardWidth *0.8) / (kTrackSlots -1);
	double startX = centerX - (boardWidth *0.4);
	double y = boardY + boardHeight /2;
	for (int i =0; i < kTrackSlots; ++i) track.emplace_back(startX + i * step, y);

	// Main board path (straight bottom edge, angled ends)
	QPainterPath boardPath;
	// Top edge
	boardPath.moveTo(centerX - boardWidth /2 + endAngleWidth, boardY);
	boardPath.lineTo(centerX + boardWidth /2 - endAngleWidth, boardY);
	// Right angled end
	boardPath.lineTo(centerX + boardWidth /2, boardY + boardHeight /2);
	// Bottom edge straight
	boardPath.lineTo(centerX + boardWidth /2 - endAngleWidth, boardY + boardHeight);
	boardPath.lineTo(centerX - boardWidth /2 + endAngleWidth, boardY + boardHeight);
	// Left angled end
	boardPath.lineTo(centerX - boardWidth /2, boardY + boardHeight /2);
	boardPath.closeSubpath();

	// Softer neutral wood palette
	QLinearGradient baseGrad(0, boardY,0, boardY + boardHeight);
	baseGrad.setColorAt(0, QColor("#C9A07A")); // light oak
	baseGrad.setColorAt(1, QColor("#8A5A44")); // walnut
	m_scene->addPath(boardPath, QPen(QColor("#E3B587"),8), QBrush(baseGrad));

	// Uniform VP rectangles and labels centered under selected circles
	const qreal rectWidth =160;
	const qreal rectHeight =50;
	const qreal rectYOffset =90; // distance below circle centers
	const qreal rectXOffset = step *0.12; // nudge right to compensate for visual centering

	auto addVPRect = [&](int trackIndex, const QString& text) {
		if (trackIndex <0 || trackIndex >= kTrackSlots) return;
		QPointF c = track[trackIndex];
		QRectF cutoutRect(c.x() - rectWidth /2.0 + rectXOffset, y + rectYOffset, rectWidth, rectHeight);
		QPainterPath cutoutPath;
		cutoutPath.addRoundedRect(cutoutRect,12,12);
		m_scene->addPath(cutoutPath, QPen(QColor("#6B3D2E"),3), QBrush(QColor("#7A4B3A")));

		QGraphicsTextItem* t = m_scene->addText(text, QFont("Segoe UI",18, QFont::Bold));
		t->setDefaultTextColor(QColor("#F3DFB2"));
		QRectF tb = t->boundingRect();
		t->setPos(cutoutRect.center().x() - tb.width() /2.0, cutoutRect.center().y() - tb.height() /2.0);
		t->setZValue(2);
	};

	// Place equally spaced under meaningful positions
	addVPRect(1, "10 VP");
	addVPRect(4, "5 VP");
	addVPRect(7, "2 VP");
	addVPRect(10, "2 VP");
	addVPRect(13, "5 VP");
	addVPRect(16, "10 VP");

	// Token bar path (muted amber palette)
	qreal tokenBarWidth =1000;
	qreal tokenBarHeight =140;
	QPainterPath tokenPath;
	tokenPath.addRoundedRect(centerX - tokenBarWidth /2, boardY - tokenBarHeight +20, tokenBarWidth, tokenBarHeight,25,25);
	QLinearGradient topGrad(0, boardY - tokenBarHeight,0, boardY);
	topGrad.setColorAt(0, QColor("#F1C27D")); // light amber
	topGrad.setColorAt(1, QColor("#D28E54")); // warm amber
	m_scene->addPath(tokenPath, QPen(QColor("#EFD3A5"),6), QBrush(topGrad));

	// Draw pawn track ellipses (soft off-white)
	for (auto& p : track) {
		QGraphicsEllipseItem* e = m_scene->addEllipse(QRectF(p.x() -30, p.y() -30,60,60), QPen(QColor("#e6e6e6"),4), QBrush(QColor(255,255,255,40)));
		e->setZValue(1);
	}

	// Dotted delimiters (golden)
	QPen dottedPen(QColor("#e9c46a"),3, Qt::DotLine);
	for (int i : {0,3,6,8,9,11,14,17}) {
		qreal x_pos = track[i].x() + step /2.0;
		m_scene->addLine(x_pos, y -40, x_pos, y +40, dottedPen);
	}

	// Pawn
	int pawnPos = (m_pawnPosition >=0) ? m_pawnPosition : static_cast<int>(board.getPawnPos());
	drawPawn(pawnPos,0, kTrackSlots -1);

	// Progress tokens (muted teal palette)
	const auto& tokens = board.getProgressTokens();
	const int count = std::min<int>(static_cast<int>(tokens.size()),5);
	double circleSize =100.0;
	double totalWidth = count * circleSize + (count -1) *25.0;
	double startTX = centerX - totalWidth /2.0;
	double ty = boardY - tokenBarHeight;
	for (int i =0; i < count; ++i) {
		const Models::Token* tok = tokens[i].get();
		if (!tok) continue;
		QRectF cRect(startTX + i * (circleSize +25.0), ty, circleSize, circleSize);
		QPainterPath circle; circle.addEllipse(cRect);
		auto* ring = m_scene->addPath(circle, QPen(QColor("#2a9d8f"),6), QBrush(QColor(245,245,245,240)));
		ring->setZValue(3);
		auto* shadow = new QGraphicsDropShadowEffect();
		shadow->setBlurRadius(18);
		shadow->setOffset(0,4);
		shadow->setColor(QColor(0,0,0,100));
		ring->setGraphicsEffect(shadow);
		QGraphicsTextItem* t = m_scene->addText(QString::fromStdString(tok->getName()), QFont("Segoe UI",14, QFont::Bold));
		t->setDefaultTextColor(QColor("#264653"));
		QRectF tb = t->boundingRect();
		t->setPos(cRect.center().x() - tb.width() /2.0, cRect.center().y() - tb.height() /2.0);
		t->setZValue(4);
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