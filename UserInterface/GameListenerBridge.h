#pragma once
#include <QObject>
#include <memory>

import Models.Player;
import Core.Player;

import Core.IGameListener; 

class GameListenerBridge : public QObject, public Core::IGameListener {
    Q_OBJECT

public:
    explicit GameListenerBridge(QObject* parent = nullptr) : QObject(parent) {}

    void onPawnMoved(const Core::PawnEvent& event) override {
        emit pawnMovedSignal(event.newPosition);
    }

    void onCardBuilt(const Core::CardEvent& e) override {}
    void onCardSold(const Core::CardEvent& e) override {}
    void onCardDiscarded(const Core::CardEvent& e) override {}
    void onWonderBuilt(const Core::WonderEvent& e) override {}
    void onTreeNodeChanged(const Core::TreeNodeEvent& e) override {
        emit treeNodeChangedSignal(e.ageIndex, e.nodeIndex, e.isAvailable, e.isEmpty);
    }
    void onTreeNodeEmptied(const Core::TreeNodeEvent& e) override {
        emit treeNodeEmptiedSignal(e.ageIndex, e.nodeIndex);
    }
    void onResourceGained(const Core::ResourceEvent& e) override {}
    void onResourceLost(const Core::ResourceEvent& e) override {}
    void onCoinsChanged(const Core::CoinEvent& e) override {}
    void onTokenAcquired(const Core::TokenEvent& e) override {}
    void onTurnStarted(const Core::TurnEvent& e) override {}
    void onTurnEnded(const Core::TurnEvent& e) override {}
    void onPhaseChanged(const Core::PhaseEvent& e) override {}
    void onRoundChanged(int r, int p) override {}
    void onVictoryAchieved(const Core::VictoryEvent& e) override {}
    void onGameStarted(int gm, Core::Playstyle p1, Core::Playstyle p2) override {}
    void onGameEnded() override {}
    void onPointsChanged(const Core::PointsEvent& e) override {}
    void onPlayerStateChanged(int id) override {}
    void onDisplayRequested(const Core::DisplayRequestEvent& e) override {}
    void displayGameModeMenu() override {}
    void displayPlaystyleMenu(const std::string& s) override {}
    void displayAvailableSaves(const std::vector<int>& v) override {}
    void displayAvailableCards(const std::deque<std::reference_wrapper<Models::Card>>& c) override {}
    void displayWonderList(const std::vector<std::reference_wrapper<Models::Wonder>>& w) override {}
    void displayPlayerHands(const Core::Player& p1, const Core::Player& p2) override {}
    void displayTurnStatus(const Core::Player& p1, const Core::Player& p2) override {}
    void displayBoard() override {}
    void displayMessage(const std::string& m) override {}
    void displayError(const std::string& e) override {}
    void displayWarning(const std::string& w) override {}
private:
    std::shared_ptr<GameListenerBridge> m_gameListener;
signals:
    void pawnMovedSignal(int newPosition);
    void treeNodeChangedSignal(int ageIndex, int nodeIndex, bool isAvailable, bool isEmpty);
    void treeNodeEmptiedSignal(int ageIndex, int nodeIndex);
};