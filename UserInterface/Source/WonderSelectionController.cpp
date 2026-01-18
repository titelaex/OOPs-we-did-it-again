#include "Header/WonderSelectionController.h"
#include "Header/PlayerPanelWidget.h"
#include "Header/WonderSelectionWidget.h"

import Core.Board;
import Core.Player;
import Core.GameState;
import Models.Wonder;
import Models.Card;

WonderSelectionController::WonderSelectionController(
    WonderSelectionWidget* widget,
    PlayerPanelWidget* leftPanel,
    PlayerPanelWidget* rightPanel,
    QObject* parent)
    : QObject(parent)
    , m_widget(widget)
    , m_leftPanel(leftPanel)
    , m_rightPanel(rightPanel)
{
    if (m_widget) {
        m_widget->setOnWonderClicked([this](int index) {
            this->onWonderSelected(index);
            });
    }
}

void WonderSelectionController::start()
{
    m_selectionPhase = 0;
    m_cardsPickedInPhase = 0;
    loadNextBatch();
}

void WonderSelectionController::loadNextBatch()
{
    auto& board = Core::Board::getInstance();
    auto& unusedWonders = const_cast<std::vector<std::unique_ptr<Models::Card>>&>(board.getUnusedWonders());

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
        if (m_widget) m_widget->hide();
        return;
    }

    if (m_widget) {
        m_widget->show();
        m_widget->loadWonders(m_currentBatch);
    }
    updateTurnLabel();
}

void WonderSelectionController::onWonderSelected(int index)
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

    if (m_widget) m_widget->loadWonders(m_currentBatch);
    if (m_leftPanel) m_leftPanel->refreshWonders();
    if (m_rightPanel) m_rightPanel->refreshWonders();

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

                if (m_leftPanel) {
                    m_leftPanel->refreshStats();
                    m_leftPanel->refreshCards();
                    m_leftPanel->refreshWonders();
                }
                if (m_rightPanel) {
                    m_rightPanel->refreshStats();
                    m_rightPanel->refreshCards();
                    m_rightPanel->refreshWonders();
                }
                break;
            }
        }

        m_currentBatch.clear();
        if (m_widget) m_widget->loadWonders(m_currentBatch);

        m_selectionPhase++;
        m_cardsPickedInPhase = 0;

        if (m_selectionPhase < 2) {
            loadNextBatch();
        }
        else {
            if (m_widget) {
                m_widget->setTurnMessage("");
                m_widget->hide();
            }
            if (onSelectionComplete) {
                onSelectionComplete();
            }
        }
    }
    else if (m_currentBatch.size() > 1) {
        updateTurnLabel();
    }
}

void WonderSelectionController::updateTurnLabel()
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

    if (m_widget) {
        m_widget->setTurnMessage("Este randul lui: " + currentPlayerName);
    }
}
