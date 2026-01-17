#pragma once

#include <QtWidgets/QWidget>
#include <QtWidgets/QGraphicsView>
#include <QtWidgets/QGraphicsScene>
#include <QtWidgets/QGraphicsProxyWidget>
#include <QtCore/QObject>
#include <functional>
#include <unordered_map>

class QGraphicsProxyWidget;
class PlayerPanelWidget;

class AgeTreeWidget : public QWidget
{
public:
    explicit AgeTreeWidget(QWidget* parent = nullptr);
    ~AgeTreeWidget();
    void showAgeTree(int age);
    void fitAgeTree();

    int getCurrentAge() const { return m_currentAge; }

    // Set player panels for refreshing after actions
    void setPlayerPanels(PlayerPanelWidget* left, PlayerPanelWidget* right);

    // Set current player index (0 or 1)
    void setCurrentPlayerIndex(int index);
    int getCurrentPlayerIndex() const { return m_currentPlayerIndex; }

    // Callbacks for UI coordination
    std::function<void(int nodeIndex, int age)> onLeafClicked; // Called when user clicks a card
    std::function<void(int newPlayerIndex, const QString& playerName)> onPlayerTurnChanged; // Called after successful action

    // Callback for requesting token selection from the board
    std::function<void(std::function<void(int)>)> onRequestTokenSelection;
    std::function<void()> onDisableTokenSelection;

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;

private:
    void handleLeafClicked(int nodeIndex, int age);
    void refreshPanels();

    QGraphicsView* m_view{ nullptr };
    QGraphicsScene* m_scene{ nullptr };
    std::unordered_map<QWidget*, QGraphicsProxyWidget*> m_proxyMap;

    PlayerPanelWidget* m_leftPanel{ nullptr };
    PlayerPanelWidget* m_rightPanel{ nullptr };
    int m_currentPlayerIndex{ 0 };
    int m_currentAge{ 1 };
};
