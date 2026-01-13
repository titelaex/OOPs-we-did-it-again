#include "BoardWidget.h"
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
	auto* layout = new QVBoxLayout(this);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->setSpacing(0);
	m_scene = new QGraphicsScene(this);
	m_view = new QGraphicsView(m_scene, this);
	m_view->setRenderHint(QPainter::Antialiasing);
	m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	m_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	m_view->setFrameStyle(QFrame::NoFrame);
	m_view->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
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
	m_scene->setSceneRect(0, 0, 2400, 600);

	// Define board geometry
	qreal boardWidth = 2200;
	qreal boardHeight = 280;
	qreal centerX = 1200;
	qreal boardY = 250;
	qreal endAngleWidth = 200;

	// Pawn track positions (19 slots) - MOVED UP
	const int kTrackSlots = 19;
	std::vector<QPointF> track;
	track.reserve(static_cast<size_t>(kTrackSlots));
	double step = (boardWidth * 0.8) / (kTrackSlots - 1);
	double startX = centerX - (boardWidth * 0.4);
	double y = boardY + boardHeight / 2;
	for (int i = 0; i < kTrackSlots; ++i) track.emplace_back(startX + i * step, y);

	// Main board path
	QPainterPath boardPath;
	boardPath.moveTo(centerX - boardWidth / 2 + endAngleWidth, boardY);
	boardPath.lineTo(centerX + boardWidth / 2 - endAngleWidth, boardY);
	boardPath.lineTo(centerX + boardWidth / 2, boardY + boardHeight / 2);
	boardPath.lineTo(centerX + boardWidth / 2 - endAngleWidth, boardY + boardHeight);
	// Symmetrical bottom cutouts based on the new image
	boardPath.lineTo(centerX + 650, boardY + boardHeight);
	boardPath.lineTo(centerX + 630, boardY + boardHeight - 50);
	boardPath.lineTo(centerX + 450, boardY + boardHeight - 50);
	boardPath.lineTo(centerX + 430, boardY + boardHeight);
	boardPath.lineTo(centerX + 250, boardY + boardHeight);
	boardPath.lineTo(centerX + 230, boardY + boardHeight - 50);
	boardPath.lineTo(centerX + 50, boardY + boardHeight - 50);
	boardPath.lineTo(centerX + 30, boardY + boardHeight);
	boardPath.lineTo(centerX - 30, boardY + boardHeight);
	boardPath.lineTo(centerX - 50, boardY + boardHeight - 50);
	boardPath.lineTo(centerX - 230, boardY + boardHeight - 50);
	boardPath.lineTo(centerX - 250, boardY + boardHeight);
	boardPath.lineTo(centerX - 430, boardY + boardHeight);
	boardPath.lineTo(centerX - 450, boardY + boardHeight - 50);
	boardPath.lineTo(centerX - 630, boardY + boardHeight - 50);
	boardPath.lineTo(centerX - 650, boardY + boardHeight);
	boardPath.lineTo(centerX - boardWidth / 2 + endAngleWidth, boardY + boardHeight);
	boardPath.lineTo(centerX - boardWidth / 2, boardY + boardHeight / 2);
	boardPath.closeSubpath();

	QLinearGradient baseGrad(0, boardY, 0, boardY + boardHeight);
	baseGrad.setColorAt(0, QColor("#D2B48C")); // Tan
	baseGrad.setColorAt(1, QColor("#8B4513")); // SaddleBrown
	m_scene->addPath(boardPath, QPen(QColor("#DEB887"), 8), QBrush(baseGrad)); // BurlyWood border

	// Military Point Cutouts and Text - Corrected Order and Style
	auto addMilitaryPoints = [&](qreal x, qreal width, const QString& text) {
		QRectF cutoutRect(x, boardY + boardHeight - 70, width, 60);

		QPainterPath cutoutPath;
		cutoutPath.addRoundedRect(cutoutRect, 15, 15);
		m_scene->addPath(cutoutPath, QPen(QColor("#A0522D"), 4), QBrush(QColor("#8B4513"))); // Sienna/SaddleBrown

		QGraphicsTextItem* t = m_scene->addText(text, QFont("Segoe UI", 20, QFont::Bold));
		t->setDefaultTextColor(QColor("#fef08a"));
		QRectF tb = t->boundingRect();
		t->setPos(cutoutRect.center().x() - tb.width() / 2.0, cutoutRect.center().y() - tb.height() / 2.0);
		t->setZValue(2);
	};

	// Correct order from left to right
	addMilitaryPoints(track[1].x() - step / 2, step * 2, "10 MP");
	addMilitaryPoints(track[4].x() - step / 2, step * 2, "5 MP");
	addMilitaryPoints(track[7].x() - step / 2, step, "2 MP");
	addMilitaryPoints(track[10].x() + step / 2, step, "2 MP");
	addMilitaryPoints(track[13].x() + step / 2, step * 2, "5 MP");
	addMilitaryPoints(track[16].x() + step / 2, step * 2, "10 MP");


	// Token bar path
	qreal tokenBarWidth = 1000;
	qreal tokenBarHeight = 140;
	QPainterPath tokenPath;
	tokenPath.addRoundedRect(centerX - tokenBarWidth / 2, boardY - tokenBarHeight + 20, tokenBarWidth, tokenBarHeight, 25, 25);
	QLinearGradient topGrad(0, boardY - tokenBarHeight, 0, boardY);
	topGrad.setColorAt(0, QColor("#67e8f9"));
	topGrad.setColorAt(1, QColor("#0e7490"));
	m_scene->addPath(tokenPath, QPen(QColor("#a5f3fc"), 6), QBrush(topGrad));

	// Draw pawn track ellipses
	for (auto& p : track) {
		QGraphicsEllipseItem* e = m_scene->addEllipse(QRectF(p.x() - 30, p.y() - 30, 60, 60), QPen(QColor("#e0e7ff"), 4), QBrush(QColor(255, 255, 255, 50)));
		e->setZValue(1);
	}

	// Dotted delimiters based on the new diagram
	QPen dottedPen(QColor("#fef08a"), 3, Qt::DotLine);
	for (int i : {0, 3, 6, 8, 9, 11, 14, 17}) {
		qreal x_pos = track[i].x() + step / 2.0;
		m_scene->addLine(x_pos, y - 40, x_pos, y + 40, dottedPen);
	}

	// Pawn
	int pawnPos = static_cast<int>(board.getPawnPos());
	drawPawn(pawnPos, 0, kTrackSlots - 1);

	// Progress tokens
	const auto& tokens = board.getProgressTokens();
	const int count = std::min<int>(static_cast<int>(tokens.size()), 5);
	double circleSize = 100.0;
	double totalWidth = count * circleSize + (count - 1) * 25.0;
	double startTX = centerX - totalWidth / 2.0;
	double ty = boardY - tokenBarHeight;
	for (int i = 0; i < count; ++i) {
		const Models::Token* tok = tokens[i].get();
		if (!tok) continue;
		QRectF cRect(startTX + i * (circleSize + 25.0), ty, circleSize, circleSize);
		QPainterPath circle; circle.addEllipse(cRect);
		auto* ring = m_scene->addPath(circle, QPen(QColor("#6ee7b7"), 8), QBrush(QColor(255, 255, 255, 250)));
		ring->setZValue(3);
		auto* shadow = new QGraphicsDropShadowEffect();
		shadow->setBlurRadius(20);
		shadow->setOffset(0, 4);
		shadow->setColor(QColor(0, 0, 0, 100));
		ring->setGraphicsEffect(shadow);
		QGraphicsTextItem* t = m_scene->addText(QString::fromStdString(tok->getName()), QFont("Segoe UI", 15, QFont::Bold));
		t->setDefaultTextColor(QColor("#047857"));
		QRectF tb = t->boundingRect();
		t->setPos(cRect.center().x() - tb.width() / 2.0, cRect.center().y() - tb.height() / 2.0);
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
	qreal boardWidth = 2200;
	qreal boardHeight = 280;
	qreal centerX = 1200;
	qreal boardY = 250;
	double step = (boardWidth * 0.8) / (19 - 1);
	double startX = centerX - (boardWidth * 0.4);
	double x = startX + position * step;
	double y = boardY + boardHeight / 2;

	QPainterPath pawnPath;
	pawnPath.addEllipse(QPointF(x, y - 30), 22, 22);
	pawnPath.addRect(x - 12, y - 30, 24, 50);
	pawnPath.addEllipse(QPointF(x, y + 28), 14, 14);
	auto* pawn = m_scene->addPath(pawnPath, QPen(QColor("#f87171"), 6), QBrush(QColor("#ef4444")));
	pawn->setZValue(5);
	auto* shadow = new QGraphicsDropShadowEffect();
	shadow->setBlurRadius(25);
	shadow->setOffset(5, 5);
	shadow->setColor(QColor(0, 0, 0, 120));
	pawn->setGraphicsEffect(shadow);
}
