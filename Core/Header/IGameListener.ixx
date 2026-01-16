export module Core.IGameListener;
import <string>;
import <vector>;
import <memory>;
import Core.AIConfig;

export namespace Core {
    
    struct CardEvent {
        int playerID;
        std::string playerName;
        std::string cardName;
        int nodeIndex;
        std::string cardColor;
        std::vector<std::string> effectsApplied;
    };
    
    struct ResourceEvent {
        int playerID;
        std::string playerName;
        std::string resourceType;
        int amount;
        int newTotal;
    };
    
    struct CoinEvent {
        int playerID;
        std::string playerName;
        int amount;
        int newTotal;
    };
    
    struct TokenEvent {
        int playerID;
        std::string playerName;
        std::string tokenName;
        std::string tokenDescription;
    };
    
    struct WonderEvent {
        int playerID;
        std::string playerName;
        std::string wonderName;
        int stageIndex;
        std::string cardUsed;
    };
    
    struct TreeNodeEvent {
        int nodeIndex;
        std::string cardName;
        bool isAvailable;
        bool isVisible;
        bool isEmpty;
        int ageIndex;
    };
    
    struct PawnEvent {
        int previousPosition;
        int newPosition;
        int steps;
        std::string reason;
    };
    
    struct TurnEvent {
        int playerID;
        std::string playerName;
        int round;
        int phase;
    };
    
    struct PhaseEvent {
        int previousPhase;
        int newPhase;
        std::string phaseName;
    };
    
    struct VictoryEvent {
        int winnerPlayerID;
        std::string winnerName;
        std::string victoryType;
        int winnerScore;
        int loserScore;
    };
    
    struct PointsEvent {
        int playerID;
        std::string playerName;
        int militaryPoints;
        int buildingPoints;
        int wonderPoints;
        int progressPoints;
        int totalPoints;
    };
    
    export class IGameListener {
    public:
        virtual ~IGameListener() = default;
        
        virtual void onCardBuilt(const CardEvent& event) = 0;
        virtual void onCardSold(const CardEvent& event) = 0;
        virtual void onCardDiscarded(const CardEvent& event) = 0;
        virtual void onWonderBuilt(const WonderEvent& event) = 0;
        
        virtual void onTreeNodeChanged(const TreeNodeEvent& event) = 0;
        virtual void onTreeNodeEmptied(const TreeNodeEvent& event) = 0;
        
        virtual void onResourceGained(const ResourceEvent& event) = 0;
        virtual void onResourceLost(const ResourceEvent& event) = 0;
        virtual void onCoinsChanged(const CoinEvent& event) = 0;
        
        virtual void onTokenAcquired(const TokenEvent& event) = 0;
        virtual void onPawnMoved(const PawnEvent& event) = 0;
        
        virtual void onTurnStarted(const TurnEvent& event) = 0;
        virtual void onTurnEnded(const TurnEvent& event) = 0;
        virtual void onPhaseChanged(const PhaseEvent& event) = 0;
        virtual void onRoundChanged(int round, int phase) = 0;
        
        virtual void onVictoryAchieved(const VictoryEvent& event) = 0;
        virtual void onGameStarted(int gameMode, Playstyle p1Style, Playstyle p2Style) = 0;
        virtual void onGameEnded() = 0;
        
        virtual void onPointsChanged(const PointsEvent& event) = 0;
        virtual void onPlayerStateChanged(int playerID) = 0;
    };
    
    export class GameEventNotifier {
    public:
        void addListener(std::shared_ptr<IGameListener> listener);
        void removeListener(std::shared_ptr<IGameListener> listener);
        
        void notifyCardBuilt(const CardEvent& event);
        void notifyCardSold(const CardEvent& event);
        void notifyCardDiscarded(const CardEvent& event);
        void notifyWonderBuilt(const WonderEvent& event);
        
        void notifyTreeNodeChanged(const TreeNodeEvent& event);
        void notifyTreeNodeEmptied(const TreeNodeEvent& event);
        
        void notifyResourceGained(const ResourceEvent& event);
        void notifyResourceLost(const ResourceEvent& event);
        void notifyCoinsChanged(const CoinEvent& event);
        
        void notifyTokenAcquired(const TokenEvent& event);
        void notifyPawnMoved(const PawnEvent& event);
        
        void notifyTurnStarted(const TurnEvent& event);
        void notifyTurnEnded(const TurnEvent& event);
        void notifyPhaseChanged(const PhaseEvent& event);
        void notifyRoundChanged(int round, int phase);
        
        void notifyVictoryAchieved(const VictoryEvent& event);
        void notifyGameStarted(int gameMode, Playstyle p1Style, Playstyle p2Style);
        void notifyGameEnded();
        
        void notifyPointsChanged(const PointsEvent& event);
        void notifyPlayerStateChanged(int playerID);
        
    private:
        std::vector<std::weak_ptr<IGameListener>> m_listeners;
        void cleanupExpiredListeners();
    };
}
