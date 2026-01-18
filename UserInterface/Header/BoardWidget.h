#pragma once
#include <QtWidgets/QWidget>
#include <QtWidgets/QGraphicsView>
#include <QtWidgets/QGraphicsScene>
#include <QtWidgets/QGraphicsEllipseItem>
#include <vector>
#include <memory>
#include <functional>

class QResizeEvent;


namespace  Core { class Board; }
namespace Models { class Token; }

class BoardWidget : public QWidget {
	Q_OBJECT
public:
	explicit BoardWidget(QWidget* parent = nullptr);
	void refresh();
	void setPawnPosition(int position);
	
	void enableTokenSelection(std::function<void(int)> onTokenClicked);
	void disableTokenSelection();

signals:
	void tokenClicked(int tokenIndex);

protected:
	void resizeEvent(QResizeEvent* event) override;

private:
	int m_pawnPosition = -1; 
	QGraphicsView* m_view{ nullptr };
	QGraphicsScene* m_scene{ nullptr };
	bool m_tokenSelectionEnabled = false;
	std::function<void(int)> m_onTokenClicked;
	std::vector<QGraphicsEllipseItem*> m_tokenItems;
	
	void drawBoard();
	void drawPawn(int position, int minPos, int maxPos);
};
