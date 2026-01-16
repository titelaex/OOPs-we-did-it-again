module Core.GameState;
import Core.Board;
import Core.Player;

namespace Core {
    
    GameState& GameState::getInstance() {
        static GameState instance;
        return instance;
    }
    
    GameState::GameState() : m_board(Core::Board::getInstance()) {
        m_player1 = std::make_shared<Core::Player>();
        m_player2 = std::make_shared<Core::Player>();
    }
    
    std::shared_ptr<Player> GameState::GetPlayer1() {
        return m_player1;
    }
    
    std::shared_ptr<Player> GameState::GetPlayer2() {
        return m_player2;
    }
    
    void GameState::recordAction(const std::string& playerName, const std::string& actionType,
                                 const std::string& cardName, const std::vector<std::string>& effects) {
        m_lastAction.playerName = playerName;
        m_lastAction.actionType = actionType;
        m_lastAction.cardName = cardName;
        m_lastAction.effectsApplied = effects;
        m_lastAction.round = m_currentRound;
        m_lastAction.phase = m_currentPhase;
    }
    
    void GameState::incrementRound() {
        m_currentRound++;
    }
    
    void GameState::toggleTurn() {
        m_isPlayer1Turn = !m_isPlayer1Turn;
    }
    
    void GameState::advanceToNextPhase() {
        if (m_currentPhase < 3) {
            m_currentPhase++;
            m_currentRound = 1;
        }
    }
}