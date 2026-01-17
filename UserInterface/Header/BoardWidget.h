#pragma once
#include <QtWidgets/QWidget>
#include <QtWidgets/QGraphicsView>
#include <QtWidgets/QGraphicsScene>
#include <vector>
#include <memory>

class QResizeEvent;


namespace  Core { class Board; }
namespace Models { class Token; }

class BoardWidget : public QWidget {
	Q_OBJECT
public:
	explicit BoardWidget(QWidget* parent = nullptr);
	void refresh();
	void setPawnPosition(int position);

protected:
	void resizeEvent(QResizeEvent* event) override;

private:
	int m_pawnPosition = 0;
	QGraphicsView* m_view{ nullptr };
	QGraphicsScene* m_scene{ nullptr };
	void drawBoard();
	void drawPawn(int position, int minPos, int maxPos);
	void drawProgressTokens(const std::vector<std::unique_ptr<Models::Token>>& tokens);
};
