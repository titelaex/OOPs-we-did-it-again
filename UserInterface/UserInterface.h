#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_UserInterface.h"

class QSplitter;
class QLabel;
class PlayerPanelWidget;
class WonderSelectionWidget;
class WonderSelectionController;
class BoardWidget;
class AgeTreeWidget;

class UserInterface : public QMainWindow
{
    Q_OBJECT

public:
    UserInterface(QWidget* parent = nullptr);
    ~UserInterface();

private:
    void initializePlayers();
    void setupLayout();
    void setupCenterPanel(QSplitter* splitter);
    void startWonderSelection();
    void showPhaseTransitionMessage();
    void showAgeTree(int age);

    Ui::UserInterfaceClass ui;

    // Panels
    PlayerPanelWidget* m_leftPanel{ nullptr };
    PlayerPanelWidget* m_rightPanel{ nullptr };
    WonderSelectionWidget* m_centerWidget{ nullptr };
    WonderSelectionController* m_wonderController{ nullptr };

    // Center container
    QWidget* m_centerContainer{ nullptr };
    QWidget* m_centerTop{ nullptr };
    QWidget* m_centerMiddle{ nullptr };
    QWidget* m_centerBottom{ nullptr };

    // Widgets
  QLabel* m_phaseBanner{ nullptr };
    BoardWidget* m_boardWidget{ nullptr };
    AgeTreeWidget* m_ageTreeWidget{ nullptr };

    // Board widget for bottom panel
    BoardWidget* m_boardWidget { nullptr };

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
    Q_INVOKABLE void showAgeTree(int age);

    // Handle click on a leaf node button
    void handleLeafClicked(int nodeIndex, int age);

private Q_SLOTS:
    // queued slot to finish UI updates after actions (runs in event loop)
    void finishAction(int age, bool parentBecameAvailable);
    // State
    int m_currentPlayerIndex{ 0 };
};