module Core.IGameListener;
import <algorithm>;

namespace Core {
    
    void GameEventNotifier::addListener(std::shared_ptr<IGameListener> listener) {
        listeners.push_back(listener);
    }
    
    void GameEventNotifier::removeListener(std::shared_ptr<IGameListener> listener) {
        listeners.erase(
            std::remove(listeners.begin(), listeners.end(), listener),
            listeners.end()
        );
    }
    
    void GameEventNotifier::notifyCardBuilt(const CardEvent& event) {
        for (auto& listener : listeners) {
            listener->onCardBuilt(event);
        }
    }
    
    void GameEventNotifier::notifyCardSold(const CardEvent& event) {
        for (auto& listener : listeners) {
            listener->onCardSold(event);
        }
    }
    
    void GameEventNotifier::notifyCardDiscarded(const CardEvent& event) {
        for (auto& listener : listeners) {
            listener->onCardDiscarded(event);
        }
    }
    
    void GameEventNotifier::notifyWonderBuilt(const WonderEvent& event) {
        for (auto& listener : listeners) {
            listener->onWonderBuilt(event);
        }
    }
    
    void GameEventNotifier::notifyTreeNodeChanged(const TreeNodeEvent& event) {
        for (auto& listener : listeners) {
            listener->onTreeNodeChanged(event);
        }
    }
    
    void GameEventNotifier::notifyTreeNodeEmptied(const TreeNodeEvent& event) {
        for (auto& listener : listeners) {
            listener->onTreeNodeEmptied(event);
        }
    }
    
    void GameEventNotifier::notifyResourceGained(const ResourceEvent& event) {
        for (auto& listener : listeners) {
            listener->onResourceGained(event);
        }
    }
    
    void GameEventNotifier::notifyResourceLost(const ResourceEvent& event) {
        for (auto& listener : listeners) {
            listener->onResourceLost(event);
        }
    }
    
    void GameEventNotifier::notifyCoinsChanged(const CoinEvent& event) {
        for (auto& listener : listeners) {
            listener->onCoinsChanged(event);
        }
    }
    
    void GameEventNotifier::notifyTokenAcquired(const TokenEvent& event) {
        for (auto& listener : listeners) {
            listener->onTokenAcquired(event);
        }
    }
    
    void GameEventNotifier::notifyPawnMoved(const PawnEvent& event) {
        for (auto& listener : listeners) {
            listener->onPawnMoved(event);
        }
    }
    
    void GameEventNotifier::notifyTurnStarted(const TurnEvent& event) {
        for (auto& listener : listeners) {
            listener->onTurnStarted(event);
        }
    }
    
    void GameEventNotifier::notifyTurnEnded(const TurnEvent& event) {
        for (auto& listener : listeners) {
            listener->onTurnEnded(event);
        }
    }
    
    void GameEventNotifier::notifyPhaseChanged(const PhaseEvent& event) {
        for (auto& listener : listeners) {
            listener->onPhaseChanged(event);
        }
    }
    
    void GameEventNotifier::notifyRoundChanged(int round, int phase) {
        for (auto& listener : listeners) {
            listener->onRoundChanged(round, phase);
        }
    }
    
    void GameEventNotifier::notifyVictoryAchieved(const VictoryEvent& event) {
        for (auto& listener : listeners) {
            listener->onVictoryAchieved(event);
        }
    }
    
    void GameEventNotifier::notifyGameStarted(int gameMode, Playstyle p1Style, Playstyle p2Style) {
        for (auto& listener : listeners) {
            listener->onGameStarted(gameMode, p1Style, p2Style);
        }
    }
    
    void GameEventNotifier::notifyGameEnded() {
        for (auto& listener : listeners) {
            listener->onGameEnded();
        }
    }
    
    void GameEventNotifier::notifyPointsChanged(const PointsEvent& event) {
        for (auto& listener : listeners) {
            listener->onPointsChanged(event);
        }
    }
    
    void GameEventNotifier::notifyPlayerStateChanged(int playerID) {
        for (auto& listener : listeners) {
            listener->onPlayerStateChanged(playerID);
        }
    }
    
    void GameEventNotifier::notifyDisplayRequested(const DisplayRequestEvent& event) {
        for (auto& listener : listeners) {
            listener->onDisplayRequested(event);
        }
    }

    void GameEventNotifier::notifyDisplayCardInfo(const Models::Card& card) {
        for (auto& listener : listeners) {
            listener->displayCardInfo(card);
        }
    }

    void GameEventNotifier::notifyDisplayWonderInfo(const Models::Wonder& wonder) {
        for (auto& listener : listeners) {
            listener->displayWonderInfo(wonder);
        }
    }

    void GameEventNotifier::notifyDisplayAgeCardInfo(const Models::AgeCard& ageCard) {
        for (auto& listener : listeners) {
            listener->displayAgeCardInfo(ageCard);
        }
    }
}
