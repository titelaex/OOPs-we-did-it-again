#pragma once
#include <QtWidgets/QMainWindow>
#include "ui_UserInterface.h"
#include <vector>
#include <memory>
#include <unordered_map>

namespace Core { class Player; }
namespace Models { class Wonder; }
class PlayerPanelWidget;
class WonderSelectionWidget; 

import Core.GameState;

class UserInterface : public QMainWindow
{
    Q_OBJECT

public:
    UserInterface(QWidget* parent = nullptr);
    ~UserInterface();

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;

private:
    Ui::UserInterfaceClass ui;

    // Panel-urile
    PlayerPanelWidget* m_leftPanel;
    PlayerPanelWidget* m_rightPanel;
    WonderSelectionWidget* m_centerWidget; 

    // center container panels (so we can color and lock sizes)
    QWidget* m_centerContainer{ nullptr };
    QWidget* m_centerTop{ nullptr };
    QWidget* m_centerMiddle{ nullptr };
    QWidget* m_centerBottom{ nullptr };

    // Starea selectiei Wonder-urilor
    int m_selectionPhase = 0; // 0 = Primele 4, 1 = Următoarele 4, 2 = Gata
    int m_cardsPickedInPhase = 0; // Câte cărți s-au ales în faza curentă (0-3)

    int m_currentPlayerIndex = 0; // 0 = player1, 1 = player2 (used for UI actions)

    std::vector<Models::Wonder*> m_currentBatch;

    // map widget -> proxy for hover scaling
    std::unordered_map<QWidget*, QGraphicsProxyWidget*> m_proxyMap;

    void startWonderSelection();
    void loadNextBatch();
    void onWonderSelected(int index);
    void updatePanels();
    void updateTurnLabel();

    // Show visual representation of age tree inside the center middle panel
    void showAgeTree(int age);

    // Handle click on a leaf node button
    void handleLeafClicked(int nodeIndex, int age);
};