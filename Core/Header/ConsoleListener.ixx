export module Core.ConsoleListener;
import Core.IGameListener;
import Core.Player;
import Models.Card;
import Models.Wonder;
import Models.Token;
import <memory>;
import <string>;
import <vector>;
import <deque>;
import <functional>;

export namespace Core {
    
    export class ConsolePrinter : public IGameListener {
    public:
        ConsolePrinter() = default;
        ~ConsolePrinter() override = default;
        
        void onCardBuilt(const CardEvent& event) override;
        void onCardSold(const CardEvent& event) override;
        void onCardDiscarded(const CardEvent& event) override;
        void onWonderBuilt(const WonderEvent& event) override;
        
        void onTreeNodeChanged(const TreeNodeEvent& event) override;
        void onTreeNodeEmptied(const TreeNodeEvent& event) override;
        
        void onResourceGained(const ResourceEvent& event) override;
        void onResourceLost(const ResourceEvent& event) override;
        void onCoinsChanged(const CoinEvent& event) override;
        
        void onTokenAcquired(const TokenEvent& event) override;
        void onPawnMoved(const PawnEvent& event) override;
        
        void onTurnStarted(const TurnEvent& event) override;
        void onTurnEnded(const TurnEvent& event) override;
        void onPhaseChanged(const PhaseEvent& event) override;
        void onRoundChanged(int round, int phase) override;
        
        void onVictoryAchieved(const VictoryEvent& event) override;
        void onGameStarted(int gameMode, Playstyle p1Style, Playstyle p2Style) override;
        void onGameEnded() override;
        
        void onPointsChanged(const PointsEvent& event) override;
        void onPlayerStateChanged(int playerID) override;
        
        void onDisplayRequested(const DisplayRequestEvent& event) override;
        
    public:
        void displayGameModeMenu() override;
        void displayPlaystyleMenu(const std::string& playerName) override;
        void displayAvailableSaves(const std::vector<int>& saveNumbers) override;
        void displayAvailableCards(const std::deque<std::reference_wrapper<Models::Card>>& cards) override;
        void displayCardSelectionPrompt();
        void displayActionMenu();
        void displayWonderList(const std::vector<std::reference_wrapper<Models::Wonder>>& wonders) override;
        void displayWonderSelectionPrompt(const std::string& playerName);
        void displayProgressTokens(const std::vector<std::reference_wrapper<const Models::Token>>& tokens);
        void displayTokenSelectionPrompt();
        void displayPlayerHands(const Player& p1, const Player& p2) override;
        void displayTurnStatus(const Player& p1, const Player& p2) override;
        void displayBoard() override;
        void displayMessage(const std::string& message) override;
        void displayError(const std::string& error) override;
        void displayWarning(const std::string& warning) override;
        
    private:
        void displayContinueGamePrompt();
        void displaySeparator();
        void displayPhaseHeader(int phase);
    };

    export class ConsoleReader : public IGameListener {
    public:
        ConsoleReader();
        ~ConsoleReader() = default;

        int getIntInput(int min, int max);
        std::string getStringInput();
        size_t selectCardIndex(size_t maxIndex);
        int selectAction();
        int selectGameMode();
        int selectPlaystyle();
        size_t selectWonder(size_t maxIndex);
        int selectSave(const std::vector<int>& saveNumbers);
        void clearInputBuffer();

        void onCardBuilt(const CardEvent& event) override {}
        void onCardSold(const CardEvent& event) override {}
        void onCardDiscarded(const CardEvent& event) override {}
        void onWonderBuilt(const WonderEvent& event) override {}
        void onTreeNodeChanged(const TreeNodeEvent& event) override {}
        void onTreeNodeEmptied(const TreeNodeEvent& event) override {}
        void onResourceGained(const ResourceEvent& event) override {}
        void onResourceLost(const ResourceEvent& event) override {}
        void onCoinsChanged(const CoinEvent& event) override {}
        void onTokenAcquired(const TokenEvent& event) override {}
        void onPawnMoved(const PawnEvent& event) override {}
        void onTurnStarted(const TurnEvent& event) override {}
        void onTurnEnded(const TurnEvent& event) override {}
        void onPhaseChanged(const PhaseEvent& event) override {}
        void onRoundChanged(int round, int phase) override {}
        void onVictoryAchieved(const VictoryEvent& event) override {}
        void onGameStarted(int gameMode, Playstyle p1Style, Playstyle p2Style) override {}
        void onGameEnded() override {}
        void onPointsChanged(const PointsEvent& event) override {}
        void onPlayerStateChanged(int playerID) override {}
        void onDisplayRequested(const DisplayRequestEvent& event) override {}
        void displayGameModeMenu() override {}
        void displayPlaystyleMenu(const std::string& playerName) override {}
        void displayAvailableSaves(const std::vector<int>& saveNumbers) override {}
        void displayAvailableCards(const std::deque<std::reference_wrapper<Models::Card>>& cards) override {}
        void displayWonderList(const std::vector<std::reference_wrapper<Models::Wonder>>& wonders) override {}
        void displayPlayerHands(const Player& p1, const Player& p2) override {}
        void displayTurnStatus(const Player& p1, const Player& p2) override {}
        void displayBoard() override {}
        void displayMessage(const std::string& message) override {}
        void displayError(const std::string& error) override {}
        void displayWarning(const std::string& warning) override {}

    private:
        bool isValidInput(const std::string& input, int min, int max);
    };
}
