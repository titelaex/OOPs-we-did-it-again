#pragma once
#include <QtWidgets/QMainWindow>
#include "ui_UserInterface.h"
#include <vector>
#include <memory>

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

private:
    Ui::UserInterfaceClass ui;

    // Panel-urile
    PlayerPanelWidget* m_leftPanel;
    PlayerPanelWidget* m_rightPanel;
    WonderSelectionWidget* m_centerWidget; 

    // Starea selectiei Wonder-urilor
    int m_selectionPhase = 0; // 0 = Primele 4, 1 = Următoarele 4, 2 = Gata
    int m_cardsPickedInPhase = 0; // Câte cărți s-au ales în faza curentă (0-3)


    std::vector<Models::Wonder*> m_currentBatch;

    void startWonderSelection();
    void loadNextBatch();
    void onWonderSelected(int index);
    void updatePanels();
    void updateTurnLabel();
};