#pragma once

#include <QtWidgets/QWidget>
#include <QtWidgets/QGraphicsView>
#include <QtWidgets/QGraphicsScene>
#include <QtWidgets/QGraphicsProxyWidget>
#include <QtCore/QObject>
#include <functional>
#include <unordered_map>

class QGraphicsProxyWidget;

class AgeTreeWidget : public QWidget
{
    // No Q_OBJECT here to avoid requiring moc; use callback instead
public:
    explicit AgeTreeWidget(QWidget* parent = nullptr);
    ~AgeTreeWidget();
    void showAgeTree(int age);
    void fitAgeTree();

    // callback invoked when a leaf is clicked
    std::function<void(int,int)> onLeafClicked;

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;

private:
    QGraphicsView* m_view{ nullptr };
    QGraphicsScene* m_scene{ nullptr };
    std::unordered_map<QWidget*, QGraphicsProxyWidget*> m_proxyMap;
};
