export module Core.IGameListener;
import Core.Player;
import Core.AIConfig;
import Models.Card;
import Models.Wonder;
import Models.Token;
import <string>;
import <vector>;
import <deque>;
import <functional>;
import <memory>;
import <optional>;

export namespace Core {
    
    struct CardEvent {
        int playerID = 0;
        std::string playerName;
        std::string cardName;
        int nodeIndex = -1;
        std::string cardColor;
        std::vector<std::string> effectsApplied;
    };
    
    struct WonderEvent {
        int playerID = 0;
        std::string playerName;
        std::string wonderName;
        int stageIndex = 0;
        std::string cardUsed;
    };
    
    struct TreeNodeEvent {
        int nodeIndex = -1;
        std::string cardName;
        bool isAvailable = false;
        bool isVisible = false;
        bool isEmpty = false;
        int ageIndex = 0;
    };
    
    struct ResourceEvent {
        int playerID = 0;
        std::string playerName;
        std::string resourceType;
        int amount = 0;
        int newTotal = 0;
    };
    
    struct CoinEvent {
        int playerID = 0;
        std::string playerName;
        int previousTotal = 0;
        int newTotal = 0;
        int change = 0;
    };
    
    struct TokenEvent {
        int playerID = 0;
        std::string playerName;
        std::string tokenName;
        std::string tokenType;
        std::string tokenDescription;
    };
    
    struct PawnEvent {
        int previousPosition = 0;
        int newPosition = 0;
        int steps = 0;
        std::string reason;
    };
    
    struct TurnEvent {
        int playerID = 0;
        std::string playerName;
        int round = 0;
        int phase = 0;
    };
    
    struct PhaseEvent {
        int previousPhase = 0;
        int newPhase = 0;
        std::string phaseName;
    };
    
    struct VictoryEvent {
        int winnerPlayerID = 0;
        std::string winnerName;
        std::string victoryType;
        int winnerScore = 0;
        int loserScore = 0;
    };
    
    struct PointsEvent {
        int playerID = 0;
        std::string playerName;
        int totalPoints = 0;
        int militaryPoints = 0;
        int buildingPoints = 0;
        int wonderPoints = 0;
        int progressPoints = 0;
    };
    
    struct DisplayRequestEvent {
        enum class Type {
            GAME_MODE_MENU,
            PLAYSTYLE_MENU,
            CONTINUE_PROMPT,
            ACTION_MENU,
            CARD_SELECTION_PROMPT,
            AVAILABLE_CARDS,
            WONDER_LIST,
            WONDER_SELECTION_PROMPT,
            PROGRESS_TOKENS,
            TOKEN_SELECTION_PROMPT,
            PLAYER_HANDS,
            TURN_STATUS,
            BOARD,
            MESSAGE,
            ERROR,
            WARNING,
            SEPARATOR,
            PHASE_HEADER
        };
        
        Type displayType;
        std::string context;
        int value = 0;
        
        std::deque<std::reference_wrapper<Models::Card>> cards;
        std::vector<std::reference_wrapper<Models::Wonder>> wonders;
        std::vector<std::reference_wrapper<const Models::Token>> tokens;
        
        std::optional<std::reference_wrapper<const Player>> player1;
        std::optional<std::reference_wrapper<const Player>> player2;
    };
    
    class IGameListener {
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
        
        virtual void onDisplayRequested(const DisplayRequestEvent& event) = 0;
    };
    
    class GameEventNotifier {
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
        
        void notifyDisplayRequested(const DisplayRequestEvent& event);
        
    private:
        std::vector<std::shared_ptr<IGameListener>> listeners;
    };
}
