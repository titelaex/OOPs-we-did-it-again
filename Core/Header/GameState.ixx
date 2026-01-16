export module Core.GameState;
import Core.Board;
import Core.Player;
import Core.AIConfig;
import Core.IGameListener;
import <string>;
import <memory>;
import <vector>;

export namespace Core {
    
    struct LastAction {
        std::string playerName;
        std::string actionType;
        std::string cardName;
        std::vector<std::string> effectsApplied;
        int round = 0;
        int phase = 0;
    };
    
    class GameState {
    public:
        static GameState& getInstance();
        
        GameState(const GameState&) = delete;
        GameState& operator=(const GameState&) = delete;
        
        std::shared_ptr<Player> GetPlayer1();
        std::shared_ptr<Player> GetPlayer2();
        
        int getCurrentPhase() const { return m_currentPhase; }
        void setCurrentPhase(int phase) { m_currentPhase = phase; }
        
        int getCurrentRound() const { return m_currentRound; }
        void setCurrentRound(int round) { m_currentRound = round; }
        
        bool isPlayer1Turn() const { return m_isPlayer1Turn; }
        void setIsPlayer1Turn(bool isP1Turn) { m_isPlayer1Turn = isP1Turn; }
        
        int getGameMode() const { return m_gameMode; }
        void setGameMode(int mode) { m_gameMode = mode; }
        
        bool isTrainingMode() const { return m_trainingMode; }
        void setTrainingMode(bool training) { m_trainingMode = training; }
        
        Playstyle getPlayer1Playstyle() const { return m_player1Playstyle; }
        void setPlayer1Playstyle(Playstyle style) { m_player1Playstyle = style; }
        
        Playstyle getPlayer2Playstyle() const { return m_player2Playstyle; }
        void setPlayer2Playstyle(Playstyle style) { m_player2Playstyle = style; }
        
        const LastAction& getLastAction() const { return m_lastAction; }
        void setLastAction(const LastAction& action) { m_lastAction = action; }
        
        void recordAction(const std::string& playerName, const std::string& actionType,
                         const std::string& cardName, const std::vector<std::string>& effects);
        
        void incrementRound();
        void toggleTurn();
        void advanceToNextPhase();
        
        GameEventNotifier& getEventNotifier() { return m_eventNotifier; }
        
    private:
        GameState();
        ~GameState() = default;
        
        Core::Board& m_board;
        std::shared_ptr<Player> m_player1;
        std::shared_ptr<Player> m_player2;
        
        int m_currentPhase = 1;
        int m_currentRound = 1;
        bool m_isPlayer1Turn = true;
        
        int m_gameMode = 1;
        bool m_trainingMode = false;
        Playstyle m_player1Playstyle = Playstyle::BRITNEY;
        Playstyle m_player2Playstyle = Playstyle::BRITNEY;
        
        LastAction m_lastAction;
        GameEventNotifier m_eventNotifier;
    };
}
