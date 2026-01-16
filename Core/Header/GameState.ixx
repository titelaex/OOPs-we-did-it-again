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
        
        std::shared_ptr<Player> GetPlayer1() const;
        std::shared_ptr<Player> GetPlayer2() const;
        
        int getCurrentPhase() const;
        void setCurrentPhase(int phase, int round, bool isP1Turn);
        
        int getCurrentRound() const;
        void setCurrentRound(int round);
        
        bool isPlayer1Turn() const;
        void setIsPlayer1Turn(bool isP1Turn);
        
        int getGameMode() const;
        void setGameMode(int mode, bool training);
        
        bool isTrainingMode() const;
        void setTrainingMode(bool training);
        
        Playstyle getPlayer1Playstyle() const;
        void setPlayer1Playstyle(Playstyle style);
        
        Playstyle getPlayer2Playstyle() const;
        void setPlayer2Playstyle(Playstyle style);
        
        void setPlayerPlaystyles(Playstyle p1Style, Playstyle p2Style);
        
        const LastAction& getLastAction() const;
        void setLastAction(const LastAction& action);
        
        void recordAction(const std::string& playerName, const std::string& actionType,
                         const std::string& cardName, const std::vector<std::string>& effects);
        
        void incrementRound();
        void toggleTurn();
        void advanceToNextPhase();
        
        void setVictory(int winnerId, const std::string& victoryType, int winnerScore, int loserScore);
        bool hasEnded() const;
        int getWinnerId() const;
        std::string getVictoryType() const;
        int getWinnerScore() const;
        int getLoserScore() const;
        void resetVictory();
        
        void saveGameState(const std::string& filename);
        void loadGameState(const std::string& filename);
        void updatePhaseInfo();
        
        GameEventNotifier& getEventNotifier();
        
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
        
        bool m_gameEnded = false;
        int m_winnerId = -1;
        std::string m_victoryType;
        int m_winnerScore = 0;
        int m_loserScore = 0;
        
        LastAction m_lastAction;
        GameEventNotifier m_eventNotifier;
    };
}
