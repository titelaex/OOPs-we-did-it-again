module Core.IGameListener;
import <algorithm>;

namespace Core {
    
    void GameEventNotifier::cleanupExpiredListeners() {
        m_listeners.erase(
            std::remove_if(m_listeners.begin(), m_listeners.end(),
                [](const std::weak_ptr<IGameListener>& wp) { return wp.expired(); }),
            m_listeners.end()
        );
    }
    
    void GameEventNotifier::addListener(std::shared_ptr<IGameListener> listener) {
        if (listener) {
            m_listeners.push_back(listener);
        }
    }
    
    void GameEventNotifier::removeListener(std::shared_ptr<IGameListener> listener) {
        if (!listener) return;
        
        m_listeners.erase(
            std::remove_if(m_listeners.begin(), m_listeners.end(),
                [&listener](const std::weak_ptr<IGameListener>& wp) {
                    auto sp = wp.lock();
                    return sp && sp == listener;
                }),
            m_listeners.end()
        );
    }
    
    void GameEventNotifier::notifyCardBuilt(const CardEvent& event) {
        cleanupExpiredListeners();
        for (auto& weakListener : m_listeners) {
            if (auto listener = weakListener.lock()) {
                listener->onCardBuilt(event);
            }
        }
    }
    
    void GameEventNotifier::notifyCardSold(const CardEvent& event) {
        cleanupExpiredListeners();
        for (auto& weakListener : m_listeners) {
            if (auto listener = weakListener.lock()) {
                listener->onCardSold(event);
            }
        }
    }
    
    void GameEventNotifier::notifyCardDiscarded(const CardEvent& event) {
        cleanupExpiredListeners();
        for (auto& weakListener : m_listeners) {
            if (auto listener = weakListener.lock()) {
                listener->onCardDiscarded(event);
            }
        }
    }
    
    void GameEventNotifier::notifyWonderBuilt(const WonderEvent& event) {
        cleanupExpiredListeners();
        for (auto& weakListener : m_listeners) {
            if (auto listener = weakListener.lock()) {
                listener->onWonderBuilt(event);
            }
        }
    }
    
    void GameEventNotifier::notifyTreeNodeChanged(const TreeNodeEvent& event) {
        cleanupExpiredListeners();
        for (auto& weakListener : m_listeners) {
            if (auto listener = weakListener.lock()) {
                listener->onTreeNodeChanged(event);
            }
        }
    }
    
    void GameEventNotifier::notifyTreeNodeEmptied(const TreeNodeEvent& event) {
        cleanupExpiredListeners();
        for (auto& weakListener : m_listeners) {
            if (auto listener = weakListener.lock()) {
                listener->onTreeNodeEmptied(event);
            }
        }
    }
    
    void GameEventNotifier::notifyResourceGained(const ResourceEvent& event) {
        cleanupExpiredListeners();
        for (auto& weakListener : m_listeners) {
            if (auto listener = weakListener.lock()) {
                listener->onResourceGained(event);
            }
        }
    }
    
    void GameEventNotifier::notifyResourceLost(const ResourceEvent& event) {
        cleanupExpiredListeners();
        for (auto& weakListener : m_listeners) {
            if (auto listener = weakListener.lock()) {
                listener->onResourceLost(event);
            }
        }
    }
    
    void GameEventNotifier::notifyCoinsChanged(const CoinEvent& event) {
        cleanupExpiredListeners();
        for (auto& weakListener : m_listeners) {
            if (auto listener = weakListener.lock()) {
                listener->onCoinsChanged(event);
            }
        }
    }
    
    void GameEventNotifier::notifyTokenAcquired(const TokenEvent& event) {
        cleanupExpiredListeners();
        for (auto& weakListener : m_listeners) {
            if (auto listener = weakListener.lock()) {
                listener->onTokenAcquired(event);
            }
        }
    }
    
    void GameEventNotifier::notifyPawnMoved(const PawnEvent& event) {
        cleanupExpiredListeners();
        for (auto& weakListener : m_listeners) {
            if (auto listener = weakListener.lock()) {
                listener->onPawnMoved(event);
            }
        }
    }
    
    void GameEventNotifier::notifyTurnStarted(const TurnEvent& event) {
        cleanupExpiredListeners();
        for (auto& weakListener : m_listeners) {
            if (auto listener = weakListener.lock()) {
                listener->onTurnStarted(event);
            }
        }
    }
    
    void GameEventNotifier::notifyTurnEnded(const TurnEvent& event) {
        cleanupExpiredListeners();
        for (auto& weakListener : m_listeners) {
            if (auto listener = weakListener.lock()) {
                listener->onTurnEnded(event);
            }
        }
    }
    
    void GameEventNotifier::notifyPhaseChanged(const PhaseEvent& event) {
        cleanupExpiredListeners();
        for (auto& weakListener : m_listeners) {
            if (auto listener = weakListener.lock()) {
                listener->onPhaseChanged(event);
            }
        }
    }
    
    void GameEventNotifier::notifyRoundChanged(int round, int phase) {
        cleanupExpiredListeners();
        for (auto& weakListener : m_listeners) {
            if (auto listener = weakListener.lock()) {
                listener->onRoundChanged(round, phase);
            }
        }
    }
    
    void GameEventNotifier::notifyVictoryAchieved(const VictoryEvent& event) {
        cleanupExpiredListeners();
        for (auto& weakListener : m_listeners) {
            if (auto listener = weakListener.lock()) {
                listener->onVictoryAchieved(event);
            }
        }
    }
    
    void GameEventNotifier::notifyGameStarted(int gameMode, Playstyle p1Style, Playstyle p2Style) {
        cleanupExpiredListeners();
        for (auto& weakListener : m_listeners) {
            if (auto listener = weakListener.lock()) {
                listener->onGameStarted(gameMode, p1Style, p2Style);
            }
        }
    }
    
    void GameEventNotifier::notifyGameEnded() {
        cleanupExpiredListeners();
        for (auto& weakListener : m_listeners) {
            if (auto listener = weakListener.lock()) {
                listener->onGameEnded();
            }
        }
    }
    
    void GameEventNotifier::notifyPointsChanged(const PointsEvent& event) {
        cleanupExpiredListeners();
        for (auto& weakListener : m_listeners) {
            if (auto listener = weakListener.lock()) {
                listener->onPointsChanged(event);
            }
        }
    }
    
    void GameEventNotifier::notifyPlayerStateChanged(int playerID) {
        cleanupExpiredListeners();
        for (auto& weakListener : m_listeners) {
            if (auto listener = weakListener.lock()) {
                listener->onPlayerStateChanged(playerID);
            }
        }
    }
}
