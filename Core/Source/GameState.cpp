module Core.GameState;
import Core.Board;
import Core.Player;
import Core.ConsoleListener;
import Core.GameStateSerializer;

namespace Core {
    
    GameState& GameState::getInstance() {
        static GameState instance;
        return instance;
    }
    
    GameState::GameState() : m_board(Core::Board::getInstance()) {
        m_player1 = std::make_shared<Core::Player>();
        m_player2 = std::make_shared<Core::Player>();
        
        auto consolePrinter = std::make_shared<ConsolePrinter>();
        m_eventNotifier.addListener(consolePrinter);
    }
    
    std::shared_ptr<Player> GameState::GetPlayer1() const {
        return m_player1;
    }
    
    std::shared_ptr<Player> GameState::GetPlayer2() const {
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
        
        GameStateSerializer::recordLastAction(playerName, actionType, cardName, effects);
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
    
    void GameState::updatePhaseInfo() {
        GameStateSerializer::setCurrentPhase(m_currentPhase, m_currentRound, m_isPlayer1Turn);
    }
    
    void GameState::saveGameState(const std::string& filename) {
        updatePhaseInfo();
        GameStateSerializer::saveGame(false);
    }
    
    void GameState::loadGameState(const std::string&, int saveNumber) {
        if (saveNumber <= 0) {
            saveNumber = GameStateSerializer::getHighestSaveNumber();
        }
        if (saveNumber > 0) {
            GameStateSerializer::loadGame(saveNumber);
        }
    }
    
    int GameState::getCurrentPhase() const {
        return m_currentPhase;
    }

    void GameState::setCurrentPhase(int phase, int round, bool isP1Turn) {
        m_currentPhase = phase;
        m_currentRound = round;
        m_isPlayer1Turn = isP1Turn;
    }

    int GameState::getCurrentRound() const {
        return m_currentRound;
    }

    void GameState::setCurrentRound(int round) {
        m_currentRound = round;
    }

    bool GameState::isPlayer1Turn() const {
        return m_isPlayer1Turn;
    }

    void GameState::setIsPlayer1Turn(bool isP1Turn) {
        m_isPlayer1Turn = isP1Turn;
    }

    int GameState::getGameMode() const {
        return m_gameMode;
    }

    void GameState::setGameMode(int mode, bool training) {
        m_gameMode = mode;
        m_trainingMode = training;
    }

    bool GameState::isTrainingMode() const {
        return m_trainingMode;
    }

    void GameState::setTrainingMode(bool training) {
        m_trainingMode = training;
    }

    Playstyle GameState::getPlayer1Playstyle() const {
        return m_player1Playstyle;
    }

    void GameState::setPlayer1Playstyle(Playstyle style) {
        m_player1Playstyle = style;
    }

    Playstyle GameState::getPlayer2Playstyle() const {
        return m_player2Playstyle;
    }

    void GameState::setPlayer2Playstyle(Playstyle style) {
        m_player2Playstyle = style;
    }

    void GameState::setPlayerPlaystyles(Playstyle p1Style, Playstyle p2Style) {
        m_player1Playstyle = p1Style;
        m_player2Playstyle = p2Style;
    }

    const LastAction& GameState::getLastAction() const {
        return m_lastAction;
    }

    void GameState::setLastAction(const LastAction& action) {
        m_lastAction = action;
    }

    GameEventNotifier& GameState::getEventNotifier() {
        return m_eventNotifier;
    }

    void GameState::setVictory(int winnerId, const std::string& victoryType, int winnerScore, int loserScore)
    {
        m_gameEnded = true;
        m_winnerId = winnerId;
        m_victoryType = victoryType;
        m_winnerScore = winnerScore;
        m_loserScore = loserScore;
    }

    bool GameState::hasEnded() const
    {
        return m_gameEnded;
    }

    int GameState::getWinnerId() const
    {
        return m_winnerId;
    }

    std::string GameState::getVictoryType() const
    {
        return m_victoryType;
    }

    int GameState::getWinnerScore() const
    {
        return m_winnerScore;
    }

    int GameState::getLoserScore() const
    {
        return m_loserScore;
    }

    void GameState::resetVictory()
    {
        m_gameEnded = false;
        m_winnerId = -1;
        m_victoryType.clear();
        m_winnerScore = 0;
        m_loserScore = 0;
    }
}