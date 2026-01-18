module Core.ConsoleListener;
import <iostream>;
import <iomanip>;
import <sstream>;
import <string>;
import <vector>;
import Core.Player;
import Core.Board;
import Models.Player;
import Models.Card;
import Models.AgeCard;
import Models.Wonder;
import Models.Token;

namespace Core {
    
    void ConsolePrinter::onCardBuilt(const CardEvent& event) {
        std::cout << "[CARD] " << event.playerName << " built " << event.cardName << "\n";
        if (!event.effectsApplied.empty()) {
            for (const auto& effect : event.effectsApplied) {
                std::cout << "       -> " << effect << "\n";
            }
        }
    }
    
    void ConsolePrinter::onCardSold(const CardEvent& event) {
        std::cout << "[CARD] " << event.playerName << " sold " << event.cardName << "\n";
    }
    
    void ConsolePrinter::onCardDiscarded(const CardEvent& event) {
        std::cout << "[CARD] " << event.cardName << " discarded\n";
    }
    
    void ConsolePrinter::onWonderBuilt(const WonderEvent& event) {
        std::cout << "[WONDER] " << event.playerName << " built " << event.wonderName 
                  << " stage " << event.stageIndex << "\n";
    }
    
    void ConsolePrinter::onTreeNodeChanged(const TreeNodeEvent& event) {
        if (!event.isEmpty) {
            std::cout << "[TREE] Node " << event.nodeIndex << " changed - "
                      << (event.isAvailable ? "AVAILABLE" : "UNAVAILABLE") << "\n";
        }
    }
    
    void ConsolePrinter::onTreeNodeEmptied(const TreeNodeEvent& event) {
        std::cout << "[TREE] Node " << event.nodeIndex << " is now empty\n";
    }
    
    void ConsolePrinter::onResourceGained(const ResourceEvent& event) {
        std::cout << "[RESOURCE] " << event.playerName << " +" << event.amount 
                  << " " << event.resourceType << " (total: " << event.newTotal << ")\n";
    }
    
    void ConsolePrinter::onResourceLost(const ResourceEvent& event) {
        std::cout << "[RESOURCE] " << event.playerName << " -" << event.amount 
                  << " " << event.resourceType << " (total: " << event.newTotal << ")\n";
    }
    
    void ConsolePrinter::onCoinsChanged(const CoinEvent& event) {
        std::cout << "[COINS] " << event.playerName << " coins: " << event.newTotal << "\n";
    }
    
    void ConsolePrinter::onTokenAcquired(const TokenEvent& event) {
        std::cout << "[TOKEN] " << event.playerName << " acquired: " << event.tokenName << "\n";
    }
    
    void ConsolePrinter::onPawnMoved(const PawnEvent& event) {
        std::cout << "[PAWN] Moved " << event.steps << " steps: " << event.previousPosition 
                  << " -> " << event.newPosition << " (" << event.reason << ")\n";
    }
    
    void ConsolePrinter::onTurnStarted(const TurnEvent& event) {
        std::cout << "\n>>> TURN: " << event.playerName << " (Phase " << event.phase 
                  << ", Round " << event.round << ")\n";
    }
    
    void ConsolePrinter::onTurnEnded(const TurnEvent& event) {
        std::cout << "<<< END TURN: " << event.playerName << "\n\n";
    }
    
    void ConsolePrinter::onPhaseChanged(const PhaseEvent& event) {
        std::cout << "\n========== PHASE " << event.newPhase << " BEGINS ==========\n";
        std::cout << "(" << event.phaseName << ")\n";
        std::cout << "==============================================\n\n";
    }
    
    void ConsolePrinter::onRoundChanged(int round, int phase) {
        std::cout << "[ROUND] Round " << round << " starts (Phase " << phase << ")\n";
    }
    
    void ConsolePrinter::onVictoryAchieved(const VictoryEvent& event) {
        std::cout << "\n!!!!!! VICTORY !!!!!!\n";
        std::cout << "Winner: " << event.winnerName << "\n";
        std::cout << "Type: " << event.victoryType << "\n";
        std::cout << "Score: " << event.winnerScore << " vs " << event.loserScore << "\n";
        std::cout << "!!!!!!!!!!!!!!!!!!!!!\n\n";
    }
    
    void ConsolePrinter::onGameStarted(int gameMode, Playstyle p1Style, Playstyle p2Style) {
        std::cout << "\n========== GAME STARTED ==========\n";
        std::cout << "==================================\n\n";
    }
    
    void ConsolePrinter::onGameEnded() {
        std::cout << "\n========== GAME ENDED ==========\n";
        std::cout << "================================\n\n";
    }
    
    void ConsolePrinter::onPointsChanged(const PointsEvent& event) {
        std::cout << "[POINTS] " << event.playerName << " = " << event.totalPoints 
                  << " (M:" << event.militaryPoints << " B:" << event.buildingPoints 
                  << " W:" << event.wonderPoints << " P:" << event.progressPoints << ")\n";
    }
    
    void ConsolePrinter::onPlayerStateChanged(int playerID) {
        std::cout << "[STATE] Player " << playerID << " updated\n";
    }
    
    void ConsolePrinter::onDisplayRequested(const DisplayRequestEvent& event) {
        using Type = DisplayRequestEvent::Type;
        
        switch (event.displayType) {
            case Type::GAME_MODE_MENU:
                displayGameModeMenu();
                break;
                
            case Type::PLAYSTYLE_MENU:
                displayPlaystyleMenu(event.context);
                break;
                
            case Type::CONTINUE_PROMPT:
                displayContinueGamePrompt();
                break;
                
            case Type::ACTION_MENU:
                displayActionMenu();
                break;
                
            case Type::CARD_SELECTION_PROMPT:
                displayCardSelectionPrompt();
                break;
                
            case Type::AVAILABLE_CARDS:
                displayAvailableCards(event.cards);
                break;
                
            case Type::AVAILABLE_SAVES:
                displayAvailableSaves(event.saveNumbers);
                break;
                
            case Type::WONDER_LIST:
                displayWonderList(event.wonders);
                break;
                
            case Type::WONDER_SELECTION_PROMPT:
                displayWonderSelectionPrompt(event.context);
                break;
                
            case Type::PROGRESS_TOKENS:
                displayProgressTokens(event.tokens);
                break;
                
            case Type::TOKEN_SELECTION_PROMPT:
                displayTokenSelectionPrompt();
                break;
                
            case Type::CARD_DISCARD_SELECTION_PROMPT:
                std::cout << "\n" << event.context << "\n";
                {
                    size_t i = 0;
                    for (const auto& cardRef : event.cards) {
                        std::cout << "[" << i << "] " << cardRef.get().getName() << "\n";
                        ++i;
                    }
                }
                break;
                
            case Type::PLAYER_HANDS:
                if (event.player1 && event.player2) {
                    displayPlayerHands(event.player1->get(), event.player2->get());
                }
                break;
                
            case Type::TURN_STATUS:
                if (event.player1 && event.player2) {
                    displayTurnStatus(event.player1->get(), event.player2->get());
                }
                break;
                
            case Type::BOARD:
                displayBoard();
                break;
                
            case Type::MESSAGE:
                displayMessage(event.context);
                break;
                
            case Type::ERROR:
                displayError(event.context);
                break;
                
            case Type::WARNING:
                displayWarning(event.context);
                break;
                
            case Type::SEPARATOR:
                displaySeparator();
                break;
                
            case Type::PHASE_HEADER:
                displayPhaseHeader(event.value);
                break;
        }
    }
    
    void ConsolePrinter::displayGameModeMenu() {
        std::cout << "\n=== CHOOSE GAME MODE ===\n";
        std::cout << "[1] Human vs Human\n";
        std::cout << "[2] Human vs AI\n";
        std::cout << "[3] AI vs AI (Training Mode)\n";
        std::cout << "[4] Human with AI Suggestions\n";
        std::cout << "Choice: ";
    }
    
    void ConsolePrinter::displayPlaystyleMenu(const std::string& playerName) {
        std::cout << "\n=== CHOOSE PLAYSTYLE FOR " << playerName << " ===\n";
        std::cout << "[1] Britney (Peaceful/Long-game)\n";
        std::cout << "[2] Spears (Aggressive/Military)\n";
        std::cout << "Choice: ";
    }
    
    void ConsolePrinter::displayContinueGamePrompt() {
        std::cout << "\n=== NEW GAME OR CONTINUE? ===\n";
        std::cout << "[0] New Game\n";
        std::cout << "[1] Continue Previous Game\n";
        std::cout << "Choice: ";
    }

    static const char* colorAnsi(const Models::ColorType c) {
        switch (c) {
        case Models::ColorType::BROWN: return "\x1b[38;5;94m"; 
        case Models::ColorType::GREY: return "\x1b[90m";
        case Models::ColorType::RED: return "\x1b[31m";
        case Models::ColorType::GREEN: return "\x1b[32m";
        case Models::ColorType::YELLOW: return "\x1b[33m";
        case Models::ColorType::BLUE: return "\x1b[34m";
        case Models::ColorType::PURPLE: return "\x1b[35m";
        default: return "\x1b[0m";
        }
    }
    static const char* resetAnsi() { return "\x1b[0m"; }

    static std::string costAbbrev(const std::unordered_map<Models::ResourceType, uint8_t>& rc) {
        auto abbrev = [](Models::ResourceType r) -> std::string {
            switch (r) {
            case Models::ResourceType::WOOD: return "W";
            case Models::ResourceType::STONE: return "S";
            case Models::ResourceType::CLAY: return "C";
            case Models::ResourceType::PAPYRUS: return "P";
            case Models::ResourceType::GLASS: return "G";
            default: return "?";
            }
        };
        std::string s;
        bool first = true;
        for (auto& kv : rc) {
            if (!first) s += ','; first = false;
            s += abbrev(kv.first);
            s += ':';
            s += std::to_string(kv.second);
        }
        return s;
    }

    static void displayBasicCardLine(const Models::Card& card) {
        const char* cAnsi = colorAnsi(card.getColor());
        const char* rAnsi = resetAnsi();
        std::cout << cAnsi << card.getName() << rAnsi
            << " | VP=" << static_cast<int>(card.getVictoryPoints())
            << " | Cost=" << (!card.getResourceCost().empty() ? costAbbrev(card.getResourceCost()) : "FREE")
            << " | Vis=" << (card.isVisible() ? 'Y' : 'N')
            << " | Avl=" << (card.isAvailable() ? 'Y' : 'N')
            << "\n";
    }

    void ConsolePrinter::displayCardInfo(const Models::Card& card) {
        if (const auto* age = dynamic_cast<const Models::AgeCard*>(&card)) {
            displayAgeCardInfo(*age);
            return;
        }
        if (const auto* wonder = dynamic_cast<const Models::Wonder*>(&card)) {
            displayWonderInfo(*wonder);
            return;
        }
        displayBasicCardLine(card);
    }

    void ConsolePrinter::displayAgeCardInfo(const Models::AgeCard& ageCard) {
        std::cout << "\n";
        displayBasicCardLine(static_cast<const Models::Card&>(ageCard));

        bool firstField = true;
        auto sep = [&]() { if (!firstField) std::cout << " | "; firstField = false; };

        const auto& prod = ageCard.getResourcesProduction();
        if (!prod.empty()) {
            sep();
            std::cout << "Prod=";
            bool first = true;
            auto abbrev = [&](Models::ResourceType r) {
                switch (r) {
                case Models::ResourceType::WOOD: return 'W';
                case Models::ResourceType::STONE: return 'S';
                case Models::ResourceType::CLAY: return 'C';
                case Models::ResourceType::PAPYRUS: return 'P';
                case Models::ResourceType::GLASS: return 'G';
                default: return '?';
                }
            };
            for (const auto& kv : prod) {
                if (!first) std::cout << ','; first = false;
                std::cout << abbrev(kv.first) << ':' << static_cast<int>(kv.second);
            }
        }

        if (ageCard.getShieldPoints() > 0) { sep(); std::cout << "SP=" << static_cast<int>(ageCard.getShieldPoints()); }

        if (ageCard.getScientificSymbols().has_value()) { sep(); std::cout << "Sci=" << Models::ScientificSymbolTypeToString(ageCard.getScientificSymbols().value()); }

        if (ageCard.getHasLinkingSymbol().has_value()) { sep(); std::cout << "HasLS=" << Models::LinkingSymbolTypeToString(ageCard.getHasLinkingSymbol().value()); }
        if (ageCard.getRequiresLinkingSymbol().has_value()) { sep(); std::cout << "ReqLS=" << Models::LinkingSymbolTypeToString(ageCard.getRequiresLinkingSymbol().value()); }

        const auto& trade = ageCard.getTradeRules();
        if (!trade.empty()) {
            bool any = false;
            std::string tr;
            for (const auto& kv : trade) {
                if (kv.second) {
                    if (any) tr.push_back(',');
                    tr += Models::ResourceTypeToString(static_cast<Models::ResourceType>(kv.first));
                    any = true;
                }
            }
            if (any) { sep(); std::cout << "TR=" << tr; }
        }

        std::cout << "\n";
    }

    void ConsolePrinter::displayWonderInfo(const Models::Wonder& wonder) {
        displayBasicCardLine(static_cast<const Models::Card&>(wonder));
        std::cout << " Resource Production: " << Models::ResourceTypeToString(wonder.getResourceProduction()) << "\n";
        std::cout << " Shield Points: " << static_cast<int>(wonder.getShieldPoints()) << "\n";
        std::cout << " Constructed: " << (wonder.IsConstructed() ? "Yes" : "No") << "\n";
    }
    
    void ConsolePrinter::displayAvailableCards(const std::deque<std::reference_wrapper<Models::Card>>& cards) {
        std::cout << "\n=== AVAILABLE CARDS ===\n";
        for (size_t i = 0; i < cards.size(); ++i) {
            std::cout << "[" << i << "] ";
            displayCardInfo(cards[i].get());
        }
    }
    
    void ConsolePrinter::displayCardSelectionPrompt() {
        std::cout << "\nSelect a card (enter index): ";
    }
    
    void ConsolePrinter::displayActionMenu() {
        std::cout << "\n=== CHOOSE ACTION ===\n";
        std::cout << "[0] Build Card\n";
        std::cout << "[1] Sell Card for Coins\n";
        std::cout << "[2] Use for Wonder Stage\n";
        std::cout << "Choice: ";
    }
    
    void ConsolePrinter::displayWonderList(const std::vector<std::reference_wrapper<Models::Wonder>>& wonders) {
        std::cout << "\n=== AVAILABLE WONDERS ===\n";
        for (size_t i = 0; i < wonders.size(); ++i) {
            std::cout << "[" << i << "] ";
            displayWonderInfo(wonders[i].get());
        }
    }
    
    void ConsolePrinter::displayWonderSelectionPrompt(const std::string& playerName) {
        std::cout << "\n" << playerName << ", choose a wonder (enter index): ";
    }
    
    void ConsolePrinter::displayProgressTokens(const std::vector<std::reference_wrapper<const Models::Token>>& tokens) {
        std::cout << "\n=== PROGRESS TOKENS ===\n";
        for (size_t i = 0; i < tokens.size(); ++i) {
            std::cout << "[" << i << "] " << tokens[i].get().getName() << "\n";
        }
    }
    
    void ConsolePrinter::displayTokenSelectionPrompt() {
        std::cout << "\nChoose a progress token (enter index): ";
    }
    
    void ConsolePrinter::displayPlayerHands(const Player& p1, const Player& p2) {
        std::cout << "\n========== PLAYER HANDS ==========\n";
        
        auto displayPlayer = [](const Player& p, const std::string& label) {
            std::cout << "\n" << label << ": " 
                      << (p.m_player ? p.m_player->getPlayerUsername() : "Unknown") << "\n";
            
            if (!p.m_player) return;
            
            auto coins = p.m_player->getRemainingCoins();
            uint32_t totalCoins = p.m_player->totalCoins(coins);
            std::cout << "  Coins: " << totalCoins << " (G:" << static_cast<int>(std::get<0>(coins))
                      << " S:" << static_cast<int>(std::get<1>(coins))
                      << " B:" << static_cast<int>(std::get<2>(coins)) << ")\n";
            
            const auto& cards = p.m_player->getOwnedCards();
            std::cout << "  Cards: " << cards.size() << "\n";
            
            const auto& wonders = p.m_player->getOwnedWonders();
            std::cout << "  Wonders: " << wonders.size();
            int built = 0;
            for (const auto& w : wonders) {
                if (w && w->IsConstructed()) built++;
            }
            std::cout << " (" << built << " built)\n";
            
            const auto& tokens = p.m_player->getOwnedTokens();
            std::cout << "  Tokens: " << tokens.size() << "\n";
        };
        
        displayPlayer(p1, "PLAYER 1");
        displayPlayer(p2, "PLAYER 2");
        
        std::cout << "==================================\n\n";
    }
    
    void ConsolePrinter::displayTurnStatus(const Player& p1, const Player& p2) {
        auto& board = Board::getInstance();
        int pawnPos = board.getPawnPos();
        
        std::cout << "\n========== TURN STATUS ==========\n";
        
        std::cout << "Military Track: P1 [";
        for (int i = 0; i <= 18; ++i) {
            if (i == pawnPos) std::cout << "?";
            else if (i == 9) std::cout << "|";
            else std::cout << "-";
        }
        std::cout << "] P2\n";
        std::cout << "Position: " << pawnPos;
        if (pawnPos < 9) std::cout << " (P1 +" << (9 - pawnPos) << ")";
        else if (pawnPos > 9) std::cout << " (P2 +" << (pawnPos - 9) << ")";
        else std::cout << " (Neutral)";
        std::cout << "\n\n";
        
        auto displayScore = [](const Player& p, const std::string& name) {
            if (!p.m_player) return;
            const auto& pts = p.m_player->getPoints();
            uint32_t total = static_cast<uint32_t>(pts.m_militaryVictoryPoints) +
                            static_cast<uint32_t>(pts.m_buildingVictoryPoints) +
                            static_cast<uint32_t>(pts.m_wonderVictoryPoints) +
                            static_cast<uint32_t>(pts.m_progressVictoryPoints);
            total += p.m_player->totalCoins(p.m_player->getRemainingCoins()) / 3;
            
            std::cout << name << ": " << p.m_player->getPlayerUsername() << "\n";
            std::cout << "  Total: " << total << " VP\n";
            std::cout << "  Military: " << static_cast<int>(pts.m_militaryVictoryPoints) << " VP\n";
            std::cout << "  Buildings: " << static_cast<int>(pts.m_buildingVictoryPoints) << " VP\n";
            std::cout << "  Wonders: " << static_cast<int>(pts.m_wonderVictoryPoints) << " VP\n";
            std::cout << "  Progress: " << static_cast<int>(pts.m_progressVictoryPoints) << " VP\n\n";
        };
        
        displayScore(p1, "PLAYER 1");
        displayScore(p2, "PLAYER 2");
        
        std::cout << "=================================\n\n";
    }
    
    void ConsolePrinter::displayBoard() {
        auto& board = Board::getInstance();
        
        std::cout << "\n========== BOARD ==========\n";
        
        const auto& progressTokens = board.getProgressTokens();
        std::cout << "Progress Tokens: ";
        for (const auto& t : progressTokens) {
            if (t) std::cout << t->getName() << " ";
        }
        std::cout << "\n";
        
        int pawnPos = board.getPawnPos();
        std::cout << "\nMilitary: P1 [";
        for (int i = 0; i <= 18; ++i) {
            if (i == pawnPos) std::cout << "?";
            else if (i == 9) std::cout << "|";
            else std::cout << "-";
        }
        std::cout << "] P2\n";
        
        const auto& militaryTokens = board.getMilitaryTokens();
        std::cout << "Military Tokens: ";
        for (const auto& t : militaryTokens) {
            if (t) std::cout << t->getName() << " ";
        }
        std::cout << "\n";
        
        std::cout << "===========================\n\n";
    }
    
    void ConsolePrinter::displayMessage(const std::string& message) {
        std::cout << "[INFO] " << message << "\n";
    }
    
    void ConsolePrinter::displayError(const std::string& error) {
        std::cout << "[ERROR] " << error << "\n";
    }
    
    void ConsolePrinter::displayWarning(const std::string& warning) {
        std::cout << "[WARNING] " << warning << "\n";
    }
    
    void ConsolePrinter::displaySeparator() {
        std::cout << "\n========================================\n\n";
    }
    
    void ConsolePrinter::displayPhaseHeader(int phase) {
        std::cout << "\n========== PHASE " << phase << " ==========\n\n";
    }
    
    void ConsolePrinter::displayAvailableSaves(const std::vector<int>& saveNumbers) {
        std::cout << "\n=== AVAILABLE SAVES ===\n";
        for (int saveNum : saveNumbers) {
            std::cout << "[" << saveNum << "] Save #" << saveNum << "\n";
        }
        std::cout << "[0] Start New Game\n";
        std::cout << "Choose save number or 0 for new game: ";
    }
}

namespace Core {

ConsoleReader::ConsoleReader() = default;

void ConsoleReader::clearInputBuffer()
{
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

bool ConsoleReader::isValidInput(const std::string& input, int min, int max)
{
    try {
        int value = std::stoi(input);
        return value >= min && value <= max;
    } catch (...) {
        return false;
    }
}

int ConsoleReader::getIntInput(int min, int max)
{
    std::string input;
    while (true) {
        std::getline(std::cin, input);
        
        if (isValidInput(input, min, max)) {
            return std::stoi(input);
        }
    }
}

std::string ConsoleReader::getStringInput()
{
    std::string input;
    std::getline(std::cin, input);
    if (!input.empty() && input.back() == '\r') {
        input.pop_back();
    }
    return input;
}

size_t ConsoleReader::selectCardIndex(size_t maxIndex)
{
    int choice = getIntInput(0, static_cast<int>(maxIndex));
    return static_cast<size_t>(choice);
}

int ConsoleReader::selectAction()
{
    return getIntInput(0, 2);
}

int ConsoleReader::selectGameMode()
{
    return getIntInput(1, 4);
}

int ConsoleReader::selectPlaystyle()
{
    return getIntInput(1, 2);
}

size_t ConsoleReader::selectWonder(size_t maxIndex)
{
    int choice = getIntInput(0, static_cast<int>(maxIndex));
    return static_cast<size_t>(choice);
}

int ConsoleReader::selectSave(const std::vector<int>& saveNumbers)
{
    int maxSave = saveNumbers.empty() ? 0 : saveNumbers.back();
    return getIntInput(0, maxSave);
}

}
