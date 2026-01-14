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

    // State
    int m_currentPlayerIndex{ 0 };
};