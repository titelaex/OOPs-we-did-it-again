#include "UserInterface.h"
#include "PlayerPanelWidget.h"
#include "WonderSelectionWidget.h" 
#include "Preparation.h"

#include <QtWidgets/QSplitter>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QWidget> 
#include <algorithm> 
#include <vector>  
#include <memory>  


import Core.Board;
import Models.Wonder;
import Models.Card;   
import Core.Player;
import Core.GameState;

UserInterface::UserInterface(QWidget* parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

    auto splitter = new QSplitter(this);

    // Obtinem jucătorii din GameState
    auto& gameState = Core::GameState::getInstance();
    std::shared_ptr<Core::Player> p1 = gameState.GetPlayer1();
    std::shared_ptr<Core::Player> p2 = gameState.GetPlayer2();

    
    if (p1 && !p1->m_player) {
        p1->m_player = std::make_unique<Models::Player>(1, std::string("Player1"));
    }
    if (p2 && !p2->m_player) {
        p2->m_player = std::make_unique<Models::Player>(2, std::string("Player2"));
    }

 
    PreparationDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted)
    {
        QString n1 = dlg.player1Name();
        QString n2 = dlg.player2Name();

        if (p1 && p1->m_player && !n1.isEmpty())
            p1->m_player->setPlayerUsername(n1.toStdString());

        if (p2 && p2->m_player && !n2.isEmpty())
            p2->m_player->setPlayerUsername(n2.toStdString());
    }

    m_leftPanel = new PlayerPanelWidget(p1, splitter, true);
    auto leftScroll = new QScrollArea(splitter);
    leftScroll->setWidgetResizable(true);
    leftScroll->setWidget(m_leftPanel);
    splitter->addWidget(leftScroll);

    m_centerWidget = new WonderSelectionWidget(this);
    m_centerWidget->setOnWonderClicked([this](int index) {
        this->onWonderSelected(index);
        });
    splitter->addWidget(m_centerWidget);

    m_rightPanel = new PlayerPanelWidget(p2, splitter, false);
    auto rightScroll = new QScrollArea(splitter);
    rightScroll->setWidgetResizable(true);
    rightScroll->setWidget(m_rightPanel);
    splitter->addWidget(rightScroll);

    splitter->setStretchFactor(0, 2);
    splitter->setStretchFactor(1, 1);
    splitter->setStretchFactor(2, 2);

    setCentralWidget(splitter);

    startWonderSelection();
}

void UserInterface::startWonderSelection()
{
    m_selectionPhase = 0;
    m_cardsPickedInPhase = 0;
    loadNextBatch();
}

void UserInterface::loadNextBatch()
{
    auto& boardInstance = Core::Board::getInstance();

    auto& unusedWonders = const_cast<std::vector<std::unique_ptr<Models::Card>>&>(boardInstance.getUnusedWonders());

    m_currentBatch.clear();

    int count = 0;
    for (const auto& cardUniquePtr : unusedWonders) {

        if (!cardUniquePtr) continue;

        if (auto w = dynamic_cast<Models::Wonder*>(cardUniquePtr.get())) {
            m_currentBatch.push_back(w);
            count++;
        }

        if (count == 4) break;
    }

    if (m_currentBatch.empty()) {
        m_centerWidget->hide();
        return;
    }

    m_centerWidget->loadWonders(m_currentBatch);

	updateTurnLabel();
}

void UserInterface::onWonderSelected(int index)
{
    if (index < 0 || static_cast<size_t>(index) >= m_currentBatch.size()) return;

    auto& gameState = Core::GameState::getInstance();
    auto p1 = gameState.GetPlayer1();
    auto p2 = gameState.GetPlayer2();
    auto& board = Core::Board::getInstance();

    std::shared_ptr<Core::Player> currentPlayer;

    if (m_selectionPhase == 0) {
        if (m_cardsPickedInPhase == 0) currentPlayer = p1;
        else if (m_cardsPickedInPhase == 1) currentPlayer = p2;
        else if (m_cardsPickedInPhase == 2) currentPlayer = p2;
        else currentPlayer = p1;
    }
    else {
        if (m_cardsPickedInPhase == 0) currentPlayer = p2;
        else if (m_cardsPickedInPhase == 1) currentPlayer = p1;
        else if (m_cardsPickedInPhase == 2) currentPlayer = p1;
        else currentPlayer = p2;
    }

    Models::Wonder* selectedRawPtr = m_currentBatch[index];

    auto& unusedPool = const_cast<std::vector<std::unique_ptr<Models::Card>>&>(board.getUnusedWonders());

    for (auto it = unusedPool.begin(); it != unusedPool.end(); ++it) {
        if (it->get() == selectedRawPtr) {
            std::unique_ptr<Models::Card> cardPtr = std::move(*it);

            unusedPool.erase(it);

            Models::Wonder* rawW = static_cast<Models::Wonder*>(cardPtr.release());
            std::unique_ptr<Models::Wonder> wonderPtr(rawW);

            currentPlayer->m_player->addWonder(std::move(wonderPtr));
            break;
        }
    }

    m_cardsPickedInPhase++;

    m_currentBatch.erase(m_currentBatch.begin() + index);
    m_centerWidget->loadWonders(m_currentBatch);

     m_leftPanel->refreshWonders();
     m_rightPanel->refreshWonders();

    if (m_currentBatch.size() == 1) {
        if (m_selectionPhase == 0) currentPlayer = p1; 
        else currentPlayer = p2;                       

        Models::Wonder* lastRawPtr = m_currentBatch[0];

        for (auto it = unusedPool.begin(); it != unusedPool.end(); ++it) {
            if (it->get() == lastRawPtr) {
                std::unique_ptr<Models::Card> cardPtr = std::move(*it);
                unusedPool.erase(it);
                Models::Wonder* rawW = static_cast<Models::Wonder*>(cardPtr.release());
                std::unique_ptr<Models::Wonder> wonderPtr(rawW);
                currentPlayer->m_player->addWonder(std::move(wonderPtr));

                m_leftPanel->refreshWonders();
                m_rightPanel->refreshWonders();

                break;
            }
        }

        m_currentBatch.clear();
        m_centerWidget->loadWonders(m_currentBatch);

        m_selectionPhase++;
        m_cardsPickedInPhase = 0;

        if (m_selectionPhase < 2) {
            loadNextBatch(); 
        }
        else {
            m_centerWidget->setTurnMessage("");
        }
    }
    if (m_currentBatch.size() > 1)
    {
        updateTurnLabel();
    }
}

UserInterface::~UserInterface()
{
}

void UserInterface::updateTurnLabel()
{
    auto& gameState = Core::GameState::getInstance();
    auto p1 = gameState.GetPlayer1();
    auto p2 = gameState.GetPlayer2();

    QString p1Name = QString::fromStdString(p1->m_player->getPlayerUsername());
    QString p2Name = QString::fromStdString(p2->m_player->getPlayerUsername());

    QString currentPlayerName;

    if (m_selectionPhase == 0) {
        if (m_cardsPickedInPhase == 0) currentPlayerName = p1Name;
        else if (m_cardsPickedInPhase == 1) currentPlayerName = p2Name;
        else if (m_cardsPickedInPhase == 2) currentPlayerName = p2Name;
        else currentPlayerName = p1Name;
    }
    else {
        if (m_cardsPickedInPhase == 0) currentPlayerName = p2Name;
        else if (m_cardsPickedInPhase == 1) currentPlayerName = p1Name;
        else if (m_cardsPickedInPhase == 2) currentPlayerName = p1Name;
        else currentPlayerName = p2Name;
    }

    m_centerWidget->setTurnMessage("Este randul lui: " + currentPlayerName);
}