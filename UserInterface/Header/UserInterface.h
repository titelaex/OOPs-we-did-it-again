#pragma once

#include <QtWidgets/QMainWindow>
#include <vector>
#include <unordered_map>

#include "ui_UserInterface.h"

class QSplitter;
class QLabel;
class QGraphicsProxyWidget;
class QWidget;

class PlayerPanelWidget;
class WonderSelectionWidget;
class WonderSelectionController;
class BoardWidget;
class AgeTreeWidget;

namespace Models { class Wonder; }

class UserInterface : public QMainWindow
{
    Q_OBJECT

public:
    explicit  UserInterface(QWidget* parent = nullptr);
    ~UserInterface();

private:
    void initializePlayers();
    void setupLayout();
    void setupCenterPanel(QSplitter* splitter);
    void startWonderSelection();
    void showPhaseTransitionMessage();
    void loadNextBatch();
    void updatePanels();
    void handleLeafClicked(int nodeIndex, int age);

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

    // Wonder selection state
    int m_selectionPhase = 0; // 0 = first round, 1 = second, 2 = done
    int m_cardsPickedInPhase = 0; // cards picked in current phase (0-3)

    int m_currentPlayerIndex = 0; // 0 = player1, 1 = player2 (used for UI actions)

    std::vector<Models::Wonder*> m_currentBatch;

    // map widget -> proxy for hover scaling
    std::unordered_map<QWidget*, QGraphicsProxyWidget*> m_proxyMap;

    void updateTurnLabel();

    // Show visual representation of age tree inside the center middle panel
    Q_INVOKABLE void showAgeTree(int age);

private Q_SLOTS:
    // queued slot to finish UI updates after actions (runs in event loop)
    void finishAction(int age, bool parentBecameAvailable);
    void onWonderSelected(int index);
};