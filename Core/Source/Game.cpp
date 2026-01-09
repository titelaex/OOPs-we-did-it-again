module Core.Game;
import <vector>;
import <string>;
import <random>;
import <iostream>;
import <algorithm>;
import <utility>;
import <fstream>;
import <sstream>;
import <functional>;
import <memory>;
#include <exception>
#include <filesystem>
import Core.PlayerDecisionMaker;
import Core.TrainingLogger;
import Core.MCTS;
import Models.AgeCard;
import Models.GuildCard;
import Models.Wonder;
import Models.ResourceType;
import Models.ScientificSymbolType;
import Models.LinkingSymbolType;
import Models.CoinWorthType;
import Models.ColorType;
import Models.Age;
import Models.TradeRuleType;
import Models.Player;
import Models.Token; 
import Models.Card;
import Core.CardCsvParser;
import Core.Player;
import Core.AgeTree; 
import Core.Node;
import Core.GameState;
namespace Core {
namespace {
    std::unique_ptr<std::vector<std::unique_ptr<Models::Token>>> setupUnusedProgressTokens;
    const int kNrOfRounds = 20;
    const std::vector<int> kMilitaryTokenPositions = {2,5,8,11,14,17};
    bool g_last_active_was_player_one = true;
    std::mt19937_64 make_rng(uint32_t seed) { 
        return std::mt19937_64(seed); 
    }
    template<typename T>
    void ShuffleInplace(std::vector<T>& v, uint32_t seed) {
        auto rng = make_rng(seed);
        std::shuffle(v.begin(), v.end(), rng);
    }
    template<typename T>
    std::vector<T> ShuffleAndMove(std::vector<T>&& src, uint32_t seed) {
        std::vector<T> dest = std::move(src);
        auto rng = make_rng(seed);
        std::shuffle(dest.begin(), dest.end(), rng);
        return dest;
    }
    std::vector<std::string> splitCSVLine(const std::string& line) {
        std::vector<std::string> columns;
        std::stringstream ss(line);
        std::string cell;
        bool in_quotes = false;
        char c;
        while (ss.get(c)) {
            if (c == '"') {
                in_quotes = !in_quotes;
            } else if (c == ',' && !in_quotes) {
                columns.push_back(cell);
                cell.clear();
            } else {
                cell += c;
            }
        }
        columns.push_back(cell);
        return columns;
    }
    void loadGenericFile(const std::string& path, const std::function<void(const std::vector<std::string>&)>& onItem, std::ofstream& log) {
        try {
            std::ifstream ifs(path);
            if (!ifs.is_open()) {
                if (log.is_open()) log << "[Error] Unable to open CSV file: " << path << "\n";
                return;
            }
            std::string header;
            std::getline(ifs, header);
            std::string line;
            size_t lineno = 1;
            while (std::getline(ifs, line)) {
                ++lineno;
                if (line.empty()) continue;
                auto cols = splitCSVLine(line);
                try {
                    onItem(cols);
                } catch (const std::exception& ex) {
                    if (log.is_open()) log << "[Error] Factory error in " << path << " at line " << lineno << ": " << ex.what() << "\n";
                } catch (...) {
                    if (log.is_open()) log << "[Error] Unknown factory error in " << path << " at line " << lineno << "\n";
                }
            }
        } catch (const std::exception& ex) {
            if (log.is_open()) log << "[Exception] Reading " << path << ": " << ex.what() << "\n";
        } catch (...) {
            if (log.is_open()) log << "[Exception] Unknown error reading " << path << "\n";
        }
    }
    std::pair<std::vector<std::unique_ptr<Models::Token>>, std::vector<std::unique_ptr<Models::Token>>> startGameTokens(std::vector<std::unique_ptr<Models::Token>> allTokens) {
        if (!setupUnusedProgressTokens) setupUnusedProgressTokens = std::make_unique<std::vector<std::unique_ptr<Models::Token>>>();
        setupUnusedProgressTokens->clear();
        std::vector<std::unique_ptr<Models::Token>> progress;
        std::vector<std::unique_ptr<Models::Token>> military;
        progress.reserve(allTokens.size()); 
        military.reserve(allTokens.size());
        for (auto &t : allTokens) {
            if (!t) continue;
            if (t->getType() == Models::TokenType::PROGRESS) progress.push_back(std::move(t));
            else if (t->getType() == Models::TokenType::MILITARY) military.push_back(std::move(t));
        }
        std::vector<std::unique_ptr<Models::Token>> selectedProgress;
        constexpr size_t kSelectCount = 5;
        if (progress.size() <= kSelectCount) {
            selectedProgress = std::move(progress);
        } else {
            std::random_device rd; 
            std::mt19937 gen(rd());
            std::vector<size_t> idx(progress.size());
            for (size_t i = 0; i < idx.size(); ++i) idx[i] = i;
            std::shuffle(idx.begin(), idx.end(), gen);
            for (size_t i = 0; i < idx.size(); ++i) {
                if (i < kSelectCount) selectedProgress.push_back(std::move(progress[idx[i]]));
                else setupUnusedProgressTokens->push_back(std::move(progress[idx[i]]));
            }
        }
        return { std::move(selectedProgress), std::move(military) };
    }
    bool determineChooserFromBoardAndLastActive(bool lastActiveWasPlayerOne) {
        auto& board = Board::getInstance();
        int pos = board.getPawnPos();
        if (pos < 9) return true;
        else if (pos > 9) return false;
        else return lastActiveWasPlayerOne;
    }
    int checkImmediateMilitaryVictory() {
        auto& board = Board::getInstance();
        int pos = board.getPawnPos();
        if (pos <= 0) return 0;
        if (pos >= 18) return 1;
        return -1;
    }
    int checkImmediateScientificVictory(Player& p1, Player& p2) {
        auto countDistinct = [](const std::unique_ptr<Models::Player>& mp)->size_t {
            if (!mp) return 0;
            const auto& syms = mp->getOwnedScientificSymbols();
            size_t cnt = 0;
            for (const auto& kv : syms) if (kv.second > 0) ++cnt;
            return cnt;
        };
        size_t s1 = countDistinct(p1.m_player);
        size_t s2 = countDistinct(p2.m_player);
        if (s1 >= 6 && s2 >= 6) return -1;
        if (s1 >= 6) return 0;
        if (s2 >= 6) return 1;
        return -1;
    }
    int determineCivilianWinner(Player& p1, Player& p2) {
        auto safePointsRaw = [](Models::Player* mp)->uint32_t {
            if (!mp) return 0;
            const auto& ptsRef = mp->getPoints();
            uint32_t pts = static_cast<uint32_t>(ptsRef.m_militaryVictoryPoints)
                + static_cast<uint32_t>(ptsRef.m_buildingVictoryPoints)
                + static_cast<uint32_t>(ptsRef.m_wonderVictoryPoints)
                + static_cast<uint32_t>(ptsRef.m_progressVictoryPoints);
            uint8_t totalCoins = mp->totalCoins(mp->getRemainingCoins());
            pts += static_cast<uint32_t>(totalCoins / 3);
            return pts;
        };
        Models::Player* m1 = p1.m_player.get();
        Models::Player* m2 = p2.m_player.get();
        uint32_t total1 = safePointsRaw(m1);
        uint32_t total2 = safePointsRaw(m2);
        if (total1 > total2) return 0;
        if (total2 > total1) return 1;
        auto bluePointsRaw = [](Models::Player* mp)->uint32_t {
            if (!mp) return 0;
            uint32_t sum = 0;
            for (const auto& cptr : mp->getOwnedCards()) {
                if (!cptr) continue;
                if (cptr->getColor() == Models::ColorType::BLUE) sum += cptr->getVictoryPoints();
            }
            return sum;
        };
        uint32_t b1 = bluePointsRaw(m1);
        uint32_t b2 = bluePointsRaw(m2);
        if (b1 > b2) return 0;
        if (b2 > b1) return 1;
        return -1;
    }
    void performCardAction(int action, Player& cur, Player& opp, std::unique_ptr<Models::Card>& cardPtr, Board& board, IPlayerDecisionMaker* decisionMaker = nullptr) {
        if (!cardPtr) return;
        switch (action) {
        case 0: { 
            if (!cur.canAffordCard(cardPtr, opp.m_player)) {
                std::cout << "ERROR: You cannot afford to build this card!\n";
                std::cout << "The card will be returned to the tree.\n";
                return;
            }
            try {
                cur.playCardBuilding(cardPtr, opp.m_player);
            } catch (const std::exception& ex) {
                std::cerr << "Exception in playCardBuilding: " << ex.what() << "\n";
            }
            if (cardPtr) {
                auto& discarded = const_cast<std::vector<std::unique_ptr<Models::Card>>&>(board.getDiscardedCards());
                discarded.push_back(std::move(cardPtr));
            }
            break;
        }
        case 1: { 
            auto& discarded = const_cast<std::vector<std::unique_ptr<Models::Card>>&>(board.getDiscardedCards());
            cur.sellCard(cardPtr, discarded);
            break;
        }
        case 2: { 
            if (Models::Wonder::getWondersBuilt() >= Models::Wonder::MaxWonders) {
                std::cout << "ERROR: Cannot build wonder - maximum of 7 wonders have already been built in the game!\n";
                std::cout << "The card will be returned to the tree.\n";
                return;
            }
            auto& owned = cur.m_player->getOwnedWonders();
            std::vector<size_t> candidates;
            for (size_t i = 0; i < owned.size(); ++i) {
                if (owned[i] && !owned[i]->IsConstructed()) 
                    candidates.push_back(i);
            }
            if (candidates.empty()) {
                std::cout << "No available unbuilt wonders. Moving card to discard.\n";
                auto& discarded = const_cast<std::vector<std::unique_ptr<Models::Card>>&>(board.getDiscardedCards());
                discarded.push_back(std::move(cardPtr));
                break;
            }
            if (!decisionMaker) {
                std::cout << "Choose wonder to construct:\n";
                for (size_t i = 0; i < candidates.size(); ++i) {
                    std::cout << "[" << i << "] " << owned[candidates[i]]->getName() << "\n";
                }
            }
            size_t wchoice = decisionMaker ? decisionMaker->selectWonder(candidates) : 0;
            std::unique_ptr<Models::Wonder>& chosenWonderPtr = owned[candidates[wchoice]];
            if (!cur.canAffordWonder(chosenWonderPtr, opp.m_player)) {
                std::cout << "ERROR: You cannot afford to build this wonder!\n";
                std::cout << "The card will be returned to the tree.\n";
                return;
            }
            std::vector<Models::Token> discardedTokens; 
            auto& discardedCards = const_cast<std::vector<std::unique_ptr<Models::Card>>&>(board.getDiscardedCards());
            cur.playCardWonder(chosenWonderPtr, cardPtr, opp.m_player, discardedTokens, discardedCards);
            break;
        }
        default:
            if (cardPtr) {
                auto& discarded = const_cast<std::vector<std::unique_ptr<Models::Card>>&>(board.getDiscardedCards());
                discarded.push_back(std::move(cardPtr));
            }
            break;
        }
    }
    uint8_t getShieldPointsFromCard(const Models::Card* card) {
        if (!card) return 0;
        if (auto ac = dynamic_cast<const Models::AgeCard*>(card)) {
            return static_cast<uint8_t>(ac->getShieldPoints());
        }
        return 0;
    }
    void displayCardDetails(const Models::Card* card) {
        if (!card) return;
        std::cout << "\n=== CARD DETAILS ===\n";
        std::cout << "Name: " << card->getName() << "\n";
        std::cout << "Color: " << Models::ColorTypeToString(card->getColor()) << "\n";
        const auto& resCost = card->getResourceCost();
        std::cout << "Resource Cost: ";
        if (resCost.empty()) {
            std::cout << "FREE";
        } else {
            bool first = true;
            for (const auto& [res, amt] : resCost) {
                if (!first) std::cout << ", ";
                first = false;
                std::cout << Models::ResourceTypeToString(res) << " x" << static_cast<int>(amt);
            }
        }
        std::cout << "\n";
        if (card->getVictoryPoints() > 0) {
            std::cout << "Victory Points: " << static_cast<int>(card->getVictoryPoints()) << "\n";
        }
        if (auto* ageCard = dynamic_cast<const Models::AgeCard*>(card)) {
            const auto& resProd = ageCard->getResourcesProduction();
            if (!resProd.empty()) {
                std::cout << "Produces: ";
                bool first = true;
                for (const auto& [res, amt] : resProd) {
                    if (!first) std::cout << ", ";
                    first = false;
                    std::cout << Models::ResourceTypeToString(res) << " x" << static_cast<int>(amt);
                }
                std::cout << "\n";
            }
            if (ageCard->getShieldPoints() > 0) {
                std::cout << "Shield Points: " << static_cast<int>(ageCard->getShieldPoints()) << "\n";
            }
            if (ageCard->getScientificSymbols().has_value()) {
                std::cout << "Scientific Symbol: " << Models::ScientificSymbolTypeToString(ageCard->getScientificSymbols().value()) << "\n";
            }
            const auto& tradeRules = ageCard->getTradeRules();
            if (!tradeRules.empty()) {
                std::cout << "Trade Discount: ";
                bool first = true;
                for (const auto& [rule, enabled] : tradeRules) {
                    if (enabled) {
                        if (!first) std::cout << ", ";
                        first = false;
                        std::cout << Models::tradeRuleTypeToString(rule);
                    }
                }
                std::cout << "\n";
            }
        }
        if (auto* wonder = dynamic_cast<const Models::Wonder*>(card)) {
            if (wonder->getShieldPoints() > 0) {
                std::cout << "Shield Points: " << static_cast<int>(wonder->getShieldPoints()) << "\n";
            }
            if (wonder->getResourceProduction() != Models::ResourceType::NO_RESOURCE) {
                std::cout << "Produces: " << Models::ResourceTypeToString(wonder->getResourceProduction()) << "\n";
            }
        }
        std::cout << "====================\n\n";
    }
    void displayPlayerResources(const Player& player, const std::string& label) {
        if (!player.m_player) return;
        std::cout << "\n--- " << label << " RESOURCES ---\n";
        auto coins = player.m_player->getRemainingCoins();
        uint32_t totalCoins = player.m_player->totalCoins(coins);
        std::cout << "Coins: " << totalCoins << "\n";
        const auto& permRes = player.m_player->getOwnedPermanentResources();
        std::cout << "Permanent Resources: ";
        if (permRes.empty()) {
            std::cout << "None";
        } else {
            bool first = true;
            for (const auto& [res, amt] : permRes) {
                if (!first) std::cout << ", ";
                first = false;
                std::cout << Models::ResourceTypeToString(res) << " x" << static_cast<int>(amt);
            }
        }
        std::cout << "\n";
        const auto& tradingRes = player.m_player->getOwnedTradingResources();
        if (!tradingRes.empty()) {
            std::cout << "Trading Resources: ";
            bool first = true;
            for (const auto& [res, amt] : tradingRes) {
                if (!first) std::cout << ", ";
                first = false;
                std::cout << Models::ResourceTypeToString(res) << " x" << static_cast<int>(amt);
            }
            std::cout << "\n";
        }
        std::cout << "----------------------------\n\n";
    }
} 
void Game::preparation() {
    try {
        PrepareBoardCardPools();
        auto& board = Core::Board::getInstance();
        board.setPawnPos(9);
        std::bitset<19> pawnTrack;
        pawnTrack.set(); 
        board.setPawnTrack(pawnTrack);
        auto allTokens = parseTokensFromCSV("Config/Tokens.csv");
        auto [progressSelected, military] = startGameTokens(std::move(allTokens));
        Core::Board::getInstance().setProgressTokens(std::move(progressSelected));
        Core::Board::getInstance().setMilitaryTokens(std::move(military));
        Core::Board::getInstance().setUnusedProgressTokens(std::move(*setupUnusedProgressTokens));
    } catch (const std::exception& ex) {
        std::cerr << "Preparation exception: " << ex.what() << std::endl;
    } catch (...) {
        std::cerr << "Unknown exception during preparation" << std::endl;
    }
}
void Game::PrepareBoardCardPools() {
    auto& board = Board::getInstance();
    board.setupCardPools();
    uint32_t seed = static_cast<uint32_t>(std::random_device{}());
    std::ofstream log("Preparation.log", std::ios::app);
    auto configureRowVisibility = [](std::vector<std::shared_ptr<Node>>& nodes, const std::vector<size_t>& rowPattern) {
        size_t idx = 0;
        bool rowVisible = true;
        for (size_t row = 0; row < rowPattern.size() && idx < nodes.size(); ++row) {
            const bool isLastRow = (row == rowPattern.size() - 1);
            const size_t rowCount = rowPattern[row];
            for (size_t col = 0; col < rowCount && idx < nodes.size(); ++col, ++idx) {
                if (auto node = nodes[idx]) {
                    if (auto* card = node->getCard()) {
                        card->setIsVisible(rowVisible);
                        card->setIsAvailable(isLastRow);
                    }
                }
            }
            rowVisible = !rowVisible;
        }
    };
    try {
        auto cwd = std::filesystem::current_path();
        if (log.is_open()) log << "Current working directory: " << cwd.string() << "\n";
        std::cout << "Current working directory: " << cwd.string() << "\n";
    } catch (...) {
        if (log.is_open()) log << "[Warning] Unable to determine current working directory\n";
    }
    auto findExistingPath = [&](const std::vector<std::string>& candidates) -> std::string {
        for (const auto& p : candidates) {
            try { if (std::filesystem::exists(p)) return p; }
            catch (...) {}
        }
        return std::string{};
    };
    try {
        std::vector<Models::AgeCard> allAgeCards;
        auto loadFn = [&](const std::vector<std::string>& cols) {
            Models::AgeCard card = ageCardFactory(cols);
            allAgeCards.push_back(std::move(card));
        };
        const std::vector<std::string> ageCandidates = {
            "Core/Config/AgeCards.csv", "../Core/Config/AgeCards.csv", "../../Core/Config/AgeCards.csv",
            "AgeCards.csv", "Resources/AgeCards.csv", "Core/Resources/AgeCards.csv", 
            "../Core/Resources/AgeCards.csv", "../ModelsDLL/AgeCards.csv", "ModelsDLL/AgeCards.csv", "../Models/AgeCards.csv"
        };
        std::string agePath = findExistingPath(ageCandidates);
        if (agePath.empty()) {
            if (log.is_open()) log << "[Error] Age file not found in candidates\n";
            std::cout << "[Error] Age file not found in candidates\n";
        } else {
            if (log.is_open()) log << "Using Age file: " << agePath << "\n";
            std::cout << "Using Age file: " << agePath << "\n";
            loadGenericFile(agePath, loadFn, log);
        }
        std::vector<std::unique_ptr<Models::Card>> age1, age2, age3;
        for (size_t i = 0; i < allAgeCards.size(); ++i) {
            auto& c = allAgeCards[i];
            switch (c.getAge()) {
            case Models::Age::AGE_I:
                age1.push_back(std::make_unique<Models::AgeCard>(std::move(c)));
                break;
            case Models::Age::AGE_II:
                age2.push_back(std::make_unique<Models::AgeCard>(std::move(c)));
                break;
            case Models::Age::AGE_III:
            default:
                age3.push_back(std::make_unique<Models::AgeCard>(std::move(c)));
                break;
            }
        }
        ShuffleInplace(age1, seed);
        ShuffleInplace(age2, seed + 1);
        ShuffleInplace(age3, seed + 2);
        board.setUnusedAgeOneCards(std::move(age1));
        board.setUnusedAgeTwoCards(std::move(age2));
        board.setUnusedAgeThreeCards(std::move(age3));
        if (log.is_open()) log << "Loaded ages: I=" << board.getUnusedAgeOneCards().size()
            << " II=" << board.getUnusedAgeTwoCards().size() << " III=" << board.getUnusedAgeThreeCards().size() << "\n";
        std::cout << "Loaded ages: I=" << board.getUnusedAgeOneCards().size()
            << " II=" << board.getUnusedAgeTwoCards().size() << " III=" << board.getUnusedAgeThreeCards().size() << "\n";
    } catch (const std::exception& ex) {
        if (log.is_open()) log << "[Exception] While processing age cards: " << ex.what() << "\n";
    }
    try {
        std::vector<Models::GuildCard> tempGuilds;
        auto loadG = [&](const std::vector<std::string>& cols) {
            Models::GuildCard g = guildCardFactory(cols);
            tempGuilds.push_back(std::move(g));
        };
        const std::vector<std::string> guildCandidates = {
            "Core/Config/Guilds.csv", "../Core/Config/Guilds.csv", "../../Core/Config/Guilds.csv",
            "Guilds.csv", "Resources/Guilds.csv", "Core/Resources/Guilds.csv",
            "../Core/Resources/Guilds.csv", "../ModelsDLL/Guilds.csv", "ModelsDLL/Guilds.csv", "../Models/Guilds.csv"
        };
        std::string guildPath = findExistingPath(guildCandidates);
        if (guildPath.empty()) {
            if (log.is_open()) log << "[Error] Guild file not found in candidates\n";
            std::cout << "[Error] Guild file not found in candidates\n";
        } else {
            if (log.is_open()) log << "Using Guild file: " << guildPath << "\n";
            std::cout << "Using Guild file: " << guildPath << "\n";
            loadGenericFile(guildPath, loadG, log);
        }
        ShuffleInplace(tempGuilds, seed);
        std::vector<std::unique_ptr<Models::Card>> guildPool;
        for (auto& g : tempGuilds) guildPool.push_back(std::make_unique<Models::GuildCard>(std::move(g)));
        board.setUnusedGuildCards(std::move(guildPool));
        if (log.is_open()) log << "Loaded guilds: " << board.getUnusedGuildCards().size() << "\n";
        std::cout << "Loaded guilds: " << board.getUnusedGuildCards().size() << "\n";
    } catch (const std::exception& ex) {
        if (log.is_open()) log << "[Exception] While processing guild cards: " << ex.what() << "\n";
    }
    try {
        std::vector<Models::Wonder> tempWonders;
        auto loadW = [&](const std::vector<std::string>& cols) {
            Models::Wonder w = wonderFactory(cols);
            tempWonders.push_back(std::move(w));
        };
        const std::vector<std::string> wonderCandidates = {
            "Core/Config/Wonders.csv", "../Core/Config/Wonders.csv", "../../Core/Config/Wonders.csv",
            "Wonders.csv", "Resources/Wonders.csv", "Core/Resources/Wonders.csv",
            "../Core/Resources/Wonders.csv", "../ModelsDLL/Wonders.csv", "ModelsDLL/Wonders.csv", "../Models/Wonders.csv"
        };
        std::string wonderPath = findExistingPath(wonderCandidates);
        if (wonderPath.empty()) {
            if (log.is_open()) log << "[Error] Wonder file not found in candidates\n";
            std::cout << "[Error] Wonder file not found in candidates\n";
        } else {
            if (log.is_open()) log << "Using Wonder file: " << wonderPath << "\n";
            std::cout << "Using Wonder file: " << wonderPath << "\n";
            loadGenericFile(wonderPath, loadW, log);
        }
        ShuffleInplace(tempWonders, seed);
        std::vector<std::unique_ptr<Models::Card>> wonderPool;
        for (auto& w : tempWonders) wonderPool.push_back(std::make_unique<Models::Wonder>(std::move(w)));
        board.setUnusedWonders(std::move(wonderPool));
        if (log.is_open()) log << "Loaded wonders: " << board.getUnusedWonders().size() << "\n";
        std::cout << "Loaded wonders: " << board.getUnusedWonders().size() << "\n";
    } catch (const std::exception& ex) {
        if (log.is_open()) log << "[Exception] While processing wonder cards: " << ex.what() << "\n";
    }
    {
        std::vector<std::unique_ptr<Models::Card>> selected;
        auto& pool = const_cast<std::vector<std::unique_ptr<Models::Card>>&>(board.getUnusedAgeOneCards());
        size_t take = std::min<size_t>(20, pool.size());
        size_t i = 0;
        while (i < pool.size() && selected.size() < take) {
            if (!pool[i]) { ++i; continue; }
            if (dynamic_cast<Models::AgeCard*>(pool[i].get())) {
                selected.push_back(std::move(pool[i]));
                pool.erase(pool.begin() + i);
            } else {
                ++i;
            }
        }
        selected = ShuffleAndMove(std::move(selected), seed);
        Core::Age1Tree tree(std::move(selected));
        auto age1Nodes = tree.releaseNodes();
        configureRowVisibility(age1Nodes, { 2,3,4,5,6 });
        board.setAge1Nodes(std::move(age1Nodes));
    }
    {
        std::vector<std::unique_ptr<Models::Card>> selected;
        auto& pool = const_cast<std::vector<std::unique_ptr<Models::Card>>&>(board.getUnusedAgeTwoCards());
        size_t take = std::min<size_t>(20, pool.size());
        size_t i = 0;
        while (i < pool.size() && selected.size() < take) {
            if (!pool[i]) { ++i; continue; }
            if (dynamic_cast<Models::AgeCard*>(pool[i].get())) {
                selected.push_back(std::move(pool[i]));
                pool.erase(pool.begin() + i);
            } else {
                ++i;
            }
        }
        selected = ShuffleAndMove(std::move(selected), seed + 1);
        Core::Age2Tree tree(std::move(selected));
        auto age2Nodes = tree.releaseNodes();
        configureRowVisibility(age2Nodes, { 6,5,4,3,2 });
        board.setAge2Nodes(std::move(age2Nodes));
    }
    {
        std::vector<std::unique_ptr<Models::Card>> selected;
        auto& pool3 = const_cast<std::vector<std::unique_ptr<Models::Card>>&>(board.getUnusedAgeThreeCards());
        auto& poolG = const_cast<std::vector<std::unique_ptr<Models::Card>>&>(board.getUnusedGuildCards());
        size_t take3 = std::min<size_t>(17, pool3.size());
        size_t takeG = std::min<size_t>(3, poolG.size());
        size_t i = 0;
        while (i < pool3.size() && selected.size() < take3) {
            if (!pool3[i]) { ++i; continue; }
            if (dynamic_cast<Models::AgeCard*>(pool3[i].get())) {
                selected.push_back(std::move(pool3[i]));
                pool3.erase(pool3.begin() + i);
            } else {
                ++i;
            }
        }
        i = 0;
        size_t movedG = 0;
        while (i < poolG.size() && movedG < takeG) {
            if (!poolG[i]) { ++i; continue; }
            if (dynamic_cast<Models::GuildCard*>(poolG[i].get())) {
                selected.push_back(std::move(poolG[i]));
                poolG.erase(poolG.begin() + i);
                ++movedG;
            } else {
                ++i;
            }
        }
        selected = ShuffleAndMove(std::move(selected), seed + 2);
        Core::Age3Tree tree(std::move(selected));
        auto age3Nodes = tree.releaseNodes();
        configureRowVisibility(age3Nodes, { 2,3,4,2,4,3,2 });
        board.setAge3Nodes(std::move(age3Nodes));
    }
    auto printNodes = [](const char* title, const std::vector<std::shared_ptr<Node>>& nodes) {
        std::cout << "===== " << title << " (" << nodes.size() << ") =====\n";
        auto idxOf = [&](const Node* ptr) -> std::string {
            if (!ptr) return "-";
            for (size_t j = 0; j < nodes.size(); ++j) {
                if (nodes[j].get() == ptr) return std::to_string(j);
            }
            return "?";
        };
        auto nameOf = [&](const Node* ptr) -> std::string {
            if (!ptr) return "-";
            auto* c = ptr->getCard();
            return c ? c->getName() : std::string{"<none>"};
        };
        for (size_t i = 0; i < nodes.size(); ++i) {
            const auto& n = nodes[i];
            if (!n) continue;
            auto* c = n->getCard();
            std::cout << "[" << i << "] " << (c ? c->getName() : std::string{"<none>"}) << "\n";
            const Node* p1 = n->getParent1().get();
            const Node* p2 = n->getParent2().get();
            const Node* ch1 = n->getChild1().get();
            const Node* ch2 = n->getChild2().get();
            std::cout << "  Parents: (" << idxOf(p1) << ") " << nameOf(p1)
                      << ", (" << idxOf(p2) << ") " << nameOf(p2) << "\n";
            std::cout << "  Children: (" << idxOf(ch1) << ") " << nameOf(ch1)
                      << ", (" << idxOf(ch2) << ") " << nameOf(ch2) << "\n";
        }
        std::cout << std::flush;
    };
    printNodes("Age I", Board::getInstance().getAge1Nodes());
    printNodes("Age II", Board::getInstance().getAge2Nodes());
    printNodes("Age III", Board::getInstance().getAge3Nodes());
}
void Game::wonderSelection(std::shared_ptr<Core::Player>& p1, std::shared_ptr<Core::Player>& p2, IPlayerDecisionMaker* p1Decisions, IPlayerDecisionMaker* p2Decisions) {
    bool deleteP1 = false, deleteP2 = false;
    if (!p1Decisions) {
        p1Decisions = new HumanDecisionMaker();
        deleteP1 = true;
    }
    if (!p2Decisions) {
        p2Decisions = new HumanDecisionMaker();
        deleteP2 = true;
    }
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 1);
    bool player1Starts = (dis(gen) == 0);
    std::vector<std::unique_ptr<Models::Wonder>> availableWonders;
    auto& wondersPool = const_cast<std::vector<std::unique_ptr<Models::Card>>&>(Board::getInstance().getUnusedWonders());
    debugWonders(wondersPool);
    for (size_t sel = 0; sel < 4 && !wondersPool.empty(); ++sel) {
        size_t idx = 0; 
        bool found = false;
        for (; idx < wondersPool.size(); ++idx) {
            if (!wondersPool[idx]) continue;
            if (dynamic_cast<Models::Wonder*>(wondersPool[idx].get())) { found = true; break; }
        }
        if (!found) break;
        std::unique_ptr<Models::Card> cardPtr = std::move(wondersPool[idx]);
        wondersPool.erase(wondersPool.begin() + idx);
        Models::Wonder* raw = static_cast<Models::Wonder*>(cardPtr.release());
        availableWonders.emplace_back(raw);
    }
    auto displayAvailableWonders = [](const std::vector<std::unique_ptr<Models::Wonder>>& wonders) {
        std::cout << "\n=== AVAILABLE WONDERS ===\n";
        for (size_t i = 0; i < wonders.size(); ++i) {
            std::cout << "\n[" << i << "] ";
            wonders[i]->displayCardInfo();
        }
        std::cout << "=========================\n";
    };
    std::cout << "\n+-----------------------------------+\n";
    std::cout << "|   WONDER SELECTION - ROUND 1     |\n";
    std::cout << "+-----------------------------------+\n";
    displayAvailableWonders(availableWonders);
    if (player1Starts) {
        std::cout << "\n> Player 1, choose your first wonder (0-" << (availableWonders.size() - 1) << "): ";
        std::vector<size_t> wonderIndices;
        for (size_t i = 0; i < availableWonders.size(); ++i) wonderIndices.push_back(i);
        size_t choice = p1Decisions->selectCard(wonderIndices);
        p1->chooseWonder(availableWonders, static_cast<uint8_t>(choice));
        if (!availableWonders.empty()) {
            displayAvailableWonders(availableWonders);
            std::cout << "\n> Player 2, choose your first wonder (0-" << (availableWonders.size() - 1) << "): ";
            wonderIndices.clear();
            for (size_t i = 0; i < availableWonders.size(); ++i) wonderIndices.push_back(i);
            choice = p2Decisions->selectCard(wonderIndices);
            p2->chooseWonder(availableWonders, static_cast<uint8_t>(choice));
        }
        if (!availableWonders.empty()) {
            displayAvailableWonders(availableWonders);
            std::cout << "\n> Player 2, choose your second wonder (0-" << (availableWonders.size() - 1) << "): ";
            wonderIndices.clear();
            for (size_t i = 0; i < availableWonders.size(); ++i) wonderIndices.push_back(i);
            choice = p2Decisions->selectCard(wonderIndices);
            p2->chooseWonder(availableWonders, static_cast<uint8_t>(choice));
        }
        if (!availableWonders.empty()) {
            p1->chooseWonder(availableWonders, 0);
        }
    } else {
        std::cout << "\n> Player 2, choose your first wonder (0-" << (availableWonders.size() - 1) << "): ";
        std::vector<size_t> wonderIndices;
        for (size_t i = 0; i < availableWonders.size(); ++i) wonderIndices.push_back(i);
        size_t choice = p2Decisions->selectCard(wonderIndices);
        p2->chooseWonder(availableWonders, static_cast<uint8_t>(choice));
        if (!availableWonders.empty()) {
            displayAvailableWonders(availableWonders);
            std::cout << "\n> Player 1, choose your first wonder (0-" << (availableWonders.size() - 1) << "): ";
            wonderIndices.clear();
            for (size_t i = 0; i < availableWonders.size(); ++i) wonderIndices.push_back(i);
            choice = p1Decisions->selectCard(wonderIndices);
            p1->chooseWonder(availableWonders, static_cast<uint8_t>(choice));
        }
        if (!availableWonders.empty()) {
            displayAvailableWonders(availableWonders);
            std::cout << "\n> Player 1, choose your second wonder (0-" << (availableWonders.size() - 1) << "): ";
            wonderIndices.clear();
            for (size_t i = 0; i < availableWonders.size(); ++i) wonderIndices.push_back(i);
            choice = p1Decisions->selectCard(wonderIndices);
            p1->chooseWonder(availableWonders, static_cast<uint8_t>(choice));
        }
        if (!availableWonders.empty()) {
            p2->chooseWonder(availableWonders, 0);
        }
    }
    for (size_t sel = 0; sel < 4 && !wondersPool.empty(); ++sel) {
        size_t idx = 0; 
        bool found = false;
        for (; idx < wondersPool.size(); ++idx) {
            if (!wondersPool[idx]) continue;
            if (dynamic_cast<Models::Wonder*>(wondersPool[idx].get())) { found = true; break; }
        }
        if (!found) break;
        std::unique_ptr<Models::Card> cardPtr = std::move(wondersPool[idx]);
        wondersPool.erase(wondersPool.begin() + idx);
        Models::Wonder* raw = static_cast<Models::Wonder*>(cardPtr.release());
        availableWonders.emplace_back(raw);
    }
    std::cout << "\n+-----------------------------------+\n";
    std::cout << "|   WONDER SELECTION - ROUND 2     |\n";
    std::cout << "+-----------------------------------+\n";
    displayAvailableWonders(availableWonders);
    if (!player1Starts) {
        std::cout << "\n> Player 1, choose your third wonder (0-" << (availableWonders.size() - 1) << "): ";
        std::vector<size_t> wonderIndices;
        for (size_t i = 0; i < availableWonders.size(); ++i) wonderIndices.push_back(i);
        size_t choice = p1Decisions->selectCard(wonderIndices);
        p1->chooseWonder(availableWonders, static_cast<uint8_t>(choice));
        if (!availableWonders.empty()) {
            displayAvailableWonders(availableWonders);
            std::cout << "\n> Player 2, choose your third wonder (0-" << (availableWonders.size() - 1) << "): ";
            wonderIndices.clear();
            for (size_t i = 0; i < availableWonders.size(); ++i) wonderIndices.push_back(i);
            choice = p2Decisions->selectCard(wonderIndices);
            p2->chooseWonder(availableWonders, static_cast<uint8_t>(choice));
        }
        if (!availableWonders.empty()) {
            displayAvailableWonders(availableWonders);
            std::cout << "\n> Player 2, choose your fourth wonder (0-" << (availableWonders.size() - 1) << "): ";
            wonderIndices.clear();
            for (size_t i = 0; i < availableWonders.size(); ++i) wonderIndices.push_back(i);
            choice = p2Decisions->selectCard(wonderIndices);
            p2->chooseWonder(availableWonders, static_cast<uint8_t>(choice));
        }
        if (!availableWonders.empty()) {
            p1->chooseWonder(availableWonders, 0);
        }
    } else {
        std::cout << "\n> Player 2, choose your third wonder (0-" << (availableWonders.size() - 1) << "): ";
        std::vector<size_t> wonderIndices;
        for (size_t i = 0; i < availableWonders.size(); ++i) wonderIndices.push_back(i);
        size_t choice = p2Decisions->selectCard(wonderIndices);
        p2->chooseWonder(availableWonders, static_cast<uint8_t>(choice));
        if (!availableWonders.empty()) {
            displayAvailableWonders(availableWonders);
            std::cout << "\n> Player 1, choose your third wonder (0-" << (availableWonders.size() - 1) << "): ";
            wonderIndices.clear();
            for (size_t i = 0; i < availableWonders.size(); ++i) wonderIndices.push_back(i);
            choice = p1Decisions->selectCard(wonderIndices);
            p1->chooseWonder(availableWonders, static_cast<uint8_t>(choice));
        }
        if (!availableWonders.empty()) {
            displayAvailableWonders(availableWonders);
            std::cout << "\n> Player 1, choose your fourth wonder (0-" << (availableWonders.size() - 1) << "): ";
            wonderIndices.clear();
            for (size_t i = 0; i < availableWonders.size(); ++i) wonderIndices.push_back(i);
            choice = p1Decisions->selectCard(wonderIndices);
            p1->chooseWonder(availableWonders, static_cast<uint8_t>(choice));
        }
        if (!availableWonders.empty()) {
            p2->chooseWonder(availableWonders, 0);
        }
    }
    std::cout << "\n+-----------------------------------+\n";
    std::cout << "|   WONDER SELECTION COMPLETE!     |\n";
    std::cout << "+-----------------------------------+\n\n";
    if (deleteP1) delete p1Decisions;
    if (deleteP2) delete p2Decisions;
}
void Game::debugWonders(const std::vector<std::unique_ptr<Models::Card>>& pool) {
    std::cout << "WondersPool" << '\n';
    for (const auto& uptr : pool) {
        if (!uptr) continue;
        const auto* w = dynamic_cast<const Models::Wonder*>(uptr.get());
        if (!w) continue;
        std::cout << "- Name: " << w->getName() << '\n';
        std::cout << "  Victory Points: " << static_cast<int>(w->getVictoryPoints()) << '\n';
        std::cout << "  Color: " << Models::ColorTypeToString(w->getColor()) << '\n';
        std::cout << "  Caption: " << w->getCaption() << '\n';
        const auto& cost = w->getResourceCost();
        std::cout << "  Cost: ";
        if (cost.empty()) {
            std::cout << "-";
        } else {
            bool first = true;
            for (const auto& kv : cost) {
                if (!first) std::cout << ", ";
                first = false;
                std::cout << Models::ResourceTypeToString(kv.first) << ':' << static_cast<int>(kv.second);
            }
        }
        std::cout << '\n';
        std::cout << "  Shield Points: " << static_cast<int>(w->getShieldPoints()) << '\n';
        std::cout << "  Resource Production: ";
        if (w->getResourceProduction() == Models::ResourceType::NO_RESOURCE) {
            std::cout << "-";
        } else {
            std::cout << Models::ResourceTypeToString(w->getResourceProduction());
        }
        std::cout << '\n';
        const auto& actions = w->getOnPlayActions();
        std::cout << "  On-Play Actions: ";
        if (actions.empty()) {
            std::cout << "-";
        } else {
            bool firstA = true;
            for (const auto& p : actions) {
                if (!firstA) std::cout << ", ";
                firstA = false;
                std::cout << p.second;
            }
        }
        std::cout << "\n\n";
    }
}
void Game::awardMilitaryTokenIfPresent(Player& receiver) {
    auto& board = Board::getInstance();
    int pos = board.getPawnPos();
    for (int p : kMilitaryTokenPositions) {
        if (p == pos) {
            auto& military = const_cast<std::vector<std::unique_ptr<Models::Token>>&>(board.getMilitaryTokens());
            if (!military.empty()) {
                std::unique_ptr<Models::Token> t = std::move(military.back());
                military.pop_back();
                if (t) {
                    if (receiver.m_player) receiver.m_player->addToken(std::move(t));
                }
            }
            break;
        }
    }
}
void Game::phaseI(Player& p1, Player& p2, IPlayerDecisionMaker* p1Decisions, IPlayerDecisionMaker* p2Decisions, TrainingLogger* logger) {
    if (!p1Decisions) p1Decisions = new HumanDecisionMaker();
    if (!p2Decisions) p2Decisions = new HumanDecisionMaker();
    int nrOfRounds = 1;
    auto& board = Board::getInstance();
    const auto& nodes = board.getAge1Nodes();
    std::random_device rd; 
    std::mt19937 gen(rd());
    bool playerOneTurn = std::uniform_int_distribution<>(0, 1)(gen) == 0;
    while (nrOfRounds <= kNrOfRounds) {
        std::vector<size_t> availableIndex;
        availableIndex.reserve(nodes.size());
        for (size_t i = 0; i < nodes.size(); ++i) {
            if (!nodes[i]) continue;
            if (!nodes[i]->isAvailable()) continue;
            auto* c = nodes[i]->getCard();
            if (!c) continue;
            availableIndex.push_back(i);
        }
        if (availableIndex.empty()) break;
        std::cout << "PhaseI: " << availableIndex.size() << " cards available" << "\n";
        for (int k = 0; k < availableIndex.size(); k++) {
            size_t index = availableIndex[k];
            auto* card = nodes[index]->getCard();
            std::cout << "\n[" << k << "] ";
            if (card) {
                card->displayCardInfo();
                if (auto* ageCard = dynamic_cast<const Models::AgeCard*>(card)) {
                    const auto& resProd = ageCard->getResourcesProduction();
                    if (!resProd.empty()) {
                        std::cout << "    Produces: ";
                        bool first = true;
                        for (const auto& [res, amt] : resProd) {
                            if (!first) std::cout << ", ";
                            first = false;
                            std::cout << Models::ResourceTypeToString(res) << " x" << static_cast<int>(amt);
                        }
                        std::cout << "\n";
                    }
                    if (ageCard->getShieldPoints() > 0) {
                        std::cout << "    Military: +" << static_cast<int>(ageCard->getShieldPoints()) << " shields\n";
                    }
                    if (ageCard->getScientificSymbols().has_value()) {
                        std::cout << "    Science: " << Models::ScientificSymbolTypeToString(ageCard->getScientificSymbols().value()) << "\n";
                    }
                }
            }
        } 
        Player* cur = playerOneTurn ? &p1 : &p2;
        Player* opp = playerOneTurn ? &p2 : &p1;
        IPlayerDecisionMaker* curDecisionMaker = playerOneTurn ? p1Decisions : p2Decisions;
        displayPlayerResources(*cur, playerOneTurn ? "Player 1" : "Player 2");
        std::cout << (playerOneTurn ? "Player 1" : "Player 2") << " choose index (0-" << (availableIndex.size() - 1) << "): ";
        size_t choice = curDecisionMaker->selectCard(availableIndex);
        size_t chosenNodeIndex = availableIndex[choice];
        std::unique_ptr<Models::Card> cardPtr = nodes[chosenNodeIndex]->releaseCard();
        if (!cardPtr) {
            std::cout << "Error: the node doesn't have a card or releaseCard() doesn't work." << "\n";
            playerOneTurn = !playerOneTurn;
            ++nrOfRounds;
            continue;
        }
        std::string cardName = cardPtr->getName();
        displayCardDetails(cardPtr.get());
        displayPlayerResources(*cur, playerOneTurn ? "Player 1" : "Player 2");
        uint8_t shields = getShieldPointsFromCard(cardPtr.get());
        std::cout << " You choose " << cardPtr->getName() << " . Choose the action: '[0]=build, [1]=sell, [2]=use as wonder\n";
        int action = curDecisionMaker->selectCardAction();
        Models::Card* rawCardPtr = cardPtr.get();
        int attemptCount = 0;
        const int maxAttempts = 3;
        while (attemptCount < maxAttempts && cardPtr) {
            performCardAction(action, *cur, *opp, cardPtr, board, curDecisionMaker);
            if (cardPtr) {
                attemptCount++;
                bool isAI = (dynamic_cast<HumanDecisionMaker*>(curDecisionMaker) == nullptr);
                if (isAI) {
                    std::cout << "\n[AI Fallback] Action " << action << " failed. ";
                    if (attemptCount == 1) {
                        action = 1;
                        std::cout << "Trying to sell instead.\n\n";
                    } else if (attemptCount == 2) {
                        action = 1;
                        std::cout << "Forcing sell action.\n\n";
                    } else {
                        std::cout << "All actions failed. Discarding card.\n\n";
                        auto& discarded = const_cast<std::vector<std::unique_ptr<Models::Card>>&>(board.getDiscardedCards());
                        discarded.push_back(std::move(cardPtr));
                        break;
                    }
                } else {
                    std::cout << "\n*** ACTION CANCELLED - Please choose another action ***\n\n";
                    nodes[chosenNodeIndex]->setCard(std::move(cardPtr));
                    continue; 
                }
            }
        }
        if (cardPtr) {
            std::cout << "\n*** ACTION CANCELLED - Please choose another action or card ***\n\n";
            nodes[chosenNodeIndex]->setCard(std::move(cardPtr));
            continue; 
        }
        if (logger) {
            MCTSGameState state = MCTS::captureGameState(1, playerOneTurn);
            MCTSAction mctsAction;
            mctsAction.cardNodeIndex = chosenNodeIndex;
            mctsAction.actionType = action; 
            mctsAction.cardName = cardName;
            mctsAction.wonderIndex = 0;
            TurnRecord turn = createTurnRecord(state, mctsAction, nrOfRounds, 0.5, 0.5);
            logger->logTurn(turn);
        }
        if (shields > 0) {
            Game::movePawn(static_cast<int>(shields));
            awardMilitaryTokenIfPresent(*cur);
            int win = checkImmediateMilitaryVictory();
            if (win != -1) {
                announceVictory(win, "Military Supremacy", p1, p2);
                g_last_active_was_player_one = !playerOneTurn;
                return;
            }
        }
        {
            int sv = checkImmediateScientificVictory(p1, p2);
            if (sv != -1) {
                announceVictory(sv, "Scientific Supremacy", p1, p2);
                g_last_active_was_player_one = !playerOneTurn;
                return;
            }
        }
        displayPlayerHands(p1, p2);
        displayTurnStatus(p1, p2);
        ++nrOfRounds;
        playerOneTurn = !playerOneTurn;
    }
    g_last_active_was_player_one = !playerOneTurn;
    std::cout << "Phase I completed." << "\n";
}
void Game::phaseII(Player& p1, Player& p2, IPlayerDecisionMaker* p1Decisions, IPlayerDecisionMaker* p2Decisions, TrainingLogger* logger) {
    if (!p1Decisions) p1Decisions = new HumanDecisionMaker();
    if (!p2Decisions) p2Decisions = new HumanDecisionMaker();
    int nrOfRounds = 1;
    auto& board = Board::getInstance();
    const auto& nodes = board.getAge2Nodes();
    bool chooserIsPlayer1 = determineChooserFromBoardAndLastActive(g_last_active_was_player_one);
    IPlayerDecisionMaker* chooser = chooserIsPlayer1 ? p1Decisions : p2Decisions;
    std::cout << (chooserIsPlayer1 ? "Player 1" : "Player 2") << " is the chooser for who begins Age II." << "\n";
    std::cout << "Chooser: pick who starts Age II ([0]=Player1, [1]=Player2): ";
    uint8_t starterChoice = chooser->selectStartingPlayer();
    bool playerOneTurn = (starterChoice == 0);
    while (nrOfRounds <= kNrOfRounds) {
        std::vector<size_t> availableIndex;
        availableIndex.reserve(nodes.size());
        for (size_t i = 0; i < nodes.size(); ++i)
            if (nodes[i] && nodes[i]->isAvailable()) availableIndex.push_back(i);
        if (availableIndex.empty()) break;
        std::cout << "PhaseII: " << availableIndex.size() << " cards available" << "\n";
        for (int k = 0; k < availableIndex.size(); k++) {
            size_t index = availableIndex[k];
            auto* card = nodes[index]->getCard();
            std::cout << "\n[" << k << "] ";
            if (card) {
                card->displayCardInfo();
                if (auto* ageCard = dynamic_cast<const Models::AgeCard*>(card)) {
                    const auto& resProd = ageCard->getResourcesProduction();
                    if (!resProd.empty()) {
                        std::cout << "    Produces: ";
                        bool first = true;
                        for (const auto& [res, amt] : resProd) {
                            if (!first) std::cout << ", ";
                            first = false;
                            std::cout << Models::ResourceTypeToString(res) << " x" << static_cast<int>(amt);
                        }
                        std::cout << "\n";
                    }
                    if (ageCard->getShieldPoints() > 0) {
                        std::cout << "    Military: +" << static_cast<int>(ageCard->getShieldPoints()) << " shields\n";
                    }
                    if (ageCard->getScientificSymbols().has_value()) {
                        std::cout << "    Science: " << Models::ScientificSymbolTypeToString(ageCard->getScientificSymbols().value()) << "\n";
                    }
                }
            }
        }
        Player* cur = playerOneTurn ? &p1 : &p2;
        Player* opp = playerOneTurn ? &p2 : &p1;
        IPlayerDecisionMaker* curDecisionMaker = playerOneTurn ? p1Decisions : p2Decisions;
        displayPlayerResources(*cur, playerOneTurn ? "Player 1" : "Player 2");
        std::cout << (playerOneTurn ? "Player 1" : "Player 2") << " choose index (0-" << (availableIndex.size() - 1) << "): ";
        size_t choice = curDecisionMaker->selectCard(availableIndex);
        size_t chosenNodeIndex = availableIndex[choice];
        std::unique_ptr<Models::Card> cardPtr = nodes[chosenNodeIndex]->releaseCard();
        if (!cardPtr) {
            std::cout << "Error: the node doesn't have a card or releaseCard() doesn't work." << "\n";
            playerOneTurn = !playerOneTurn;
            ++nrOfRounds;
            continue;
        }
        std::string cardName = cardPtr->getName();
        displayCardDetails(cardPtr.get());
        displayPlayerResources(*cur, playerOneTurn ? "Player 1" : "Player 2");
        uint8_t shields = getShieldPointsFromCard(cardPtr.get());
        std::cout << " You choose " << cardPtr->getName() << " . Choose the action: '[0]=build, [1]=sell, [2]=use as wonder\n";
        int action = curDecisionMaker->selectCardAction();
        int attemptCount = 0;
        const int maxAttempts = 3;
        while (attemptCount < maxAttempts && cardPtr) {
            performCardAction(action, *cur, *opp, cardPtr, board, curDecisionMaker);
            if (cardPtr) {
                attemptCount++;
                bool isAI = (dynamic_cast<HumanDecisionMaker*>(curDecisionMaker) == nullptr);
                if (isAI) {
                    std::cout << "\n[AI Fallback] Action " << action << " failed. ";
                    if (attemptCount == 1) {
                        action = 1;
                        std::cout << "Trying to sell instead.\n\n";
                    } else if (attemptCount == 2) {
                        action = 1;
                        std::cout << "Forcing sell action.\n\n";
                    } else {
                        std::cout << "All actions failed. Discarding card.\n\n";
                        auto& discarded = const_cast<std::vector<std::unique_ptr<Models::Card>>&>(board.getDiscardedCards());
                        discarded.push_back(std::move(cardPtr));
                        break;
                    }
                } else {
                    std::cout << "\n*** ACTION CANCELLED - Please choose another action ***\n\n";
                    nodes[chosenNodeIndex]->setCard(std::move(cardPtr));
                    continue; 
                }
            }
        }
        if (cardPtr) {
            std::cout << "\n*** ACTION CANCELLED - Please choose another action or card ***\n\n";
            nodes[chosenNodeIndex]->setCard(std::move(cardPtr));
            continue; 
        }
        if (logger) {
            MCTSGameState state = MCTS::captureGameState(2, playerOneTurn);
            MCTSAction mctsAction;
            mctsAction.cardNodeIndex = chosenNodeIndex;
            mctsAction.actionType = action; 
            mctsAction.cardName = cardName;
            mctsAction.wonderIndex = 0;
            TurnRecord turn = createTurnRecord(state, mctsAction, nrOfRounds, 0.5, 0.5);
            logger->logTurn(turn);
        }
        if (shields > 0) {
            Game::movePawn(static_cast<int>(shields));
            awardMilitaryTokenIfPresent(*cur);
            int win = checkImmediateMilitaryVictory();
            if (win != -1) {
                announceVictory(win, "Military Supremacy", p1, p2);
                g_last_active_was_player_one = !playerOneTurn;
                return;
            }
        }
        {
            int sv = checkImmediateScientificVictory(p1, p2);
            if (sv != -1) {
                announceVictory(sv, "Scientific Supremacy", p1, p2);
                g_last_active_was_player_one = !playerOneTurn;
                return;
            }
        }
        displayPlayerHands(p1, p2);
        displayTurnStatus(p1, p2);
        ++nrOfRounds;
        playerOneTurn = !playerOneTurn;
    }
    g_last_active_was_player_one = !playerOneTurn;
    std::cout << "Phase II completed." << "\n";
}
void Game::phaseIII(Player& p1, Player& p2, IPlayerDecisionMaker* p1Decisions, IPlayerDecisionMaker* p2Decisions, TrainingLogger* logger) {
    if (!p1Decisions) p1Decisions = new HumanDecisionMaker();
    if (!p2Decisions) p2Decisions = new HumanDecisionMaker();
    int nrOfRounds = 1;
    auto& board = Board::getInstance();
    const auto& nodes = board.getAge3Nodes();
    bool chooserIsPlayer1 = determineChooserFromBoardAndLastActive(g_last_active_was_player_one);
    IPlayerDecisionMaker* chooser = chooserIsPlayer1 ? p1Decisions : p2Decisions;
    std::cout << (chooserIsPlayer1 ? "Player 1" : "Player 2") << " is the chooser for who begins Age III." << "\n";
    std::cout << "Chooser: pick who starts Age III ([0]=Player1, [1]=Player2): ";
    uint8_t starterChoice = chooser->selectStartingPlayer();
    bool playerOneTurn = (starterChoice == 0);
    while (nrOfRounds <= kNrOfRounds) {
        std::vector<size_t> availableIndex;
        availableIndex.reserve(nodes.size());
        for (size_t i = 0; i < nodes.size(); ++i)
            if (nodes[i] && nodes[i]->isAvailable()) availableIndex.push_back(i);
        if (availableIndex.empty()) break;
        std::cout << "PhaseIII: " << availableIndex.size() << " cards available" << "\n";
        for (int k = 0; k < availableIndex.size(); k++) {
            size_t index = availableIndex[k];
            auto* card = nodes[index]->getCard();
            std::cout << "\n[" << k << "] ";
            if (card) {
                card->displayCardInfo();
                if (auto* ageCard = dynamic_cast<const Models::AgeCard*>(card)) {
                    const auto& resProd = ageCard->getResourcesProduction();
                    if (!resProd.empty()) {
                        std::cout << "    Produces: ";
                        bool first = true;
                        for (const auto& [res, amt] : resProd) {
                            if (!first) std::cout << ", ";
                            first = false;
                            std::cout << Models::ResourceTypeToString(res) << " x" << static_cast<int>(amt);
                        }
                        std::cout << "\n";
                    }
                    if (ageCard->getShieldPoints() > 0) {
                        std::cout << "    Military: +" << static_cast<int>(ageCard->getShieldPoints()) << " shields\n";
                    }
                    if (ageCard->getScientificSymbols().has_value()) {
                        std::cout << "    Science: " << Models::ScientificSymbolTypeToString(ageCard->getScientificSymbols().value()) << "\n";
                    }
                }
            }
        }
        Player* cur = playerOneTurn ? &p1 : &p2;
        Player* opp = playerOneTurn ? &p2 : &p1;
        IPlayerDecisionMaker* curDecisionMaker = playerOneTurn ? p1Decisions : p2Decisions;
        displayPlayerResources(*cur, playerOneTurn ? "Player 1" : "Player 2");
        std::cout << (playerOneTurn ? "Player 1" : "Player 2") << " choose index (0-" << (availableIndex.size() - 1) << "): ";
        size_t choice = curDecisionMaker->selectCard(availableIndex);
        size_t chosenNodeIndex = availableIndex[choice];
        std::unique_ptr<Models::Card> cardPtr = nodes[chosenNodeIndex]->releaseCard();
        if (!cardPtr) {
            std::cout << "Error: the node doesn't have a card or releaseCard() doesn't work." << "\n";
            playerOneTurn = !playerOneTurn;
            ++nrOfRounds;
            continue;
        }
        std::string cardName = cardPtr->getName();
        displayCardDetails(cardPtr.get());
        displayPlayerResources(*cur, playerOneTurn ? "Player 1" : "Player 2");
        uint8_t shields = getShieldPointsFromCard(cardPtr.get());
        std::cout << " You choose " << cardPtr->getName() << " . Choose the action: '[0]=build, [1]=sell, [2]=use as wonder\n";
        int action = curDecisionMaker->selectCardAction();
        int attemptCount = 0;
        const int maxAttempts = 3;
        while (attemptCount < maxAttempts && cardPtr) {
            performCardAction(action, *cur, *opp, cardPtr, board, curDecisionMaker);
            if (cardPtr) {
                attemptCount++;
                bool isAI = (dynamic_cast<HumanDecisionMaker*>(curDecisionMaker) == nullptr);
                if (isAI) {
                    std::cout << "\n[AI Fallback] Action " << action << " failed. ";
                    if (attemptCount == 1) {
                        action = 1;
                        std::cout << "Trying to sell instead.\n\n";
                    } else if (attemptCount == 2) {
                        action = 1;
                        std::cout << "Forcing sell action.\n\n";
                    } else {
                        std::cout << "All actions failed. Discarding card.\n\n";
                        auto& discarded = const_cast<std::vector<std::unique_ptr<Models::Card>>&>(board.getDiscardedCards());
                        discarded.push_back(std::move(cardPtr));
                        break;
                    }
                } else {
                    std::cout << "\n*** ACTION CANCELLED - Please choose another action ***\n\n";
                    nodes[chosenNodeIndex]->setCard(std::move(cardPtr));
                    continue; 
                }
            }
        }
        if (cardPtr) {
            std::cout << "\n*** ACTION CANCELLED - Please choose another action or card ***\n\n";
            nodes[chosenNodeIndex]->setCard(std::move(cardPtr));
            continue; 
        }
        if (logger) {
            MCTSGameState state = MCTS::captureGameState(3, playerOneTurn);
            MCTSAction mctsAction;
            mctsAction.cardNodeIndex = chosenNodeIndex;
            mctsAction.actionType = action; 
            mctsAction.cardName = cardName;
            mctsAction.wonderIndex = 0;
            TurnRecord turn = createTurnRecord(state, mctsAction, nrOfRounds, 0.5, 0.5);
            logger->logTurn(turn);
        }
        if (shields > 0) {
            Game::movePawn(static_cast<int>(shields));
            awardMilitaryTokenIfPresent(*cur);
            int win = checkImmediateMilitaryVictory();
            if (win != -1) {
                announceVictory(win, "Military Supremacy", p1, p2);
                g_last_active_was_player_one = !playerOneTurn;
                return;
            }
        }
        {
            int sv = checkImmediateScientificVictory(p1, p2);
            if (sv != -1) {
                announceVictory(sv, "Scientific Supremacy", p1, p2);
                g_last_active_was_player_one = !playerOneTurn;
                return;
            }
        }
        displayPlayerHands(p1, p2);
        displayTurnStatus(p1, p2);
        ++nrOfRounds;
        playerOneTurn = !playerOneTurn;
    }
    g_last_active_was_player_one = !playerOneTurn;
    int civil = determineCivilianWinner(p1, p2);
    if (civil == -1) {
        Models::Player* m1 = p1.m_player.get();
        Models::Player* m2 = p2.m_player.get();
        uint32_t total1 = 0, total2 = 0;
        if (m1) {
            const auto& ptsRef = m1->getPoints();
            total1 = static_cast<uint32_t>(ptsRef.m_militaryVictoryPoints) + static_cast<uint32_t>(ptsRef.m_buildingVictoryPoints)
                + static_cast<uint32_t>(ptsRef.m_wonderVictoryPoints) + static_cast<uint32_t>(ptsRef.m_progressVictoryPoints)
                + static_cast<uint32_t>(m1->totalCoins(m1->getRemainingCoins()) / 3);
        }
        if (m2) {
            const auto& ptsRef = m2->getPoints();
            total2 = static_cast<uint32_t>(ptsRef.m_militaryVictoryPoints) + static_cast<uint32_t>(ptsRef.m_buildingVictoryPoints)
                + static_cast<uint32_t>(ptsRef.m_wonderVictoryPoints) + static_cast<uint32_t>(ptsRef.m_progressVictoryPoints)
                + static_cast<uint32_t>(m2->totalCoins(m2->getRemainingCoins()) / 3);
        }
        if (total1 == total2) {
            announceVictory(2, "Civilian (TIE)", p1, p2);
        } else {
            announceVictory(total1 > total2 ? 0 : 1, "Civilian", p1, p2);
        }
    } else {
        announceVictory(civil, "Civilian", p1, p2);
    }
    std::cout << "Phase III completed." << "\n";
}
void Game::movePawn(int steps) {
    auto& board = Core::Board::getInstance();
    auto position = board.getPawnPos();
    position = std::clamp(position + steps, 0, 18);
    board.setPawnPos(position);
}
void Game::displayPlayerHands(const Player& p1, const Player& p2) {
    std::cout << "\n========== PLAYER HANDS ==========\n";
    std::cout << "\n--- PLAYER 1: " << (p1.m_player ? p1.m_player->getPlayerUsername() : "Unknown") << " ---\n";
    if (p1.m_player) {
        auto coins = p1.m_player->getRemainingCoins();
        uint32_t totalCoins = p1.m_player->totalCoins(coins);
        std::cout << "Coins: " << totalCoins
            << " (1x" << static_cast<int>(std::get<0>(coins))
            << " + 3x" << static_cast<int>(std::get<1>(coins))
            << " + 6x" << static_cast<int>(std::get<2>(coins)) << ")\n";
        std::cout << "\nOwned Cards (" << p1.m_player->getOwnedCards().size() << "):\n";
        for (const auto& card : p1.m_player->getOwnedCards()) {
            if (card) {
                std::cout << "  - ";
                card->displayCardInfo();
                std::cout << "\n";
            }
        }
        std::cout << "\nWonders (" << p1.m_player->getOwnedWonders().size() << "):\n";
        for (const auto& wonder : p1.m_player->getOwnedWonders()) {
            if (wonder) {
                std::cout << "  - " << wonder->getName()
                    << (wonder->IsConstructed() ? " [CONSTRUCTED]" : " [NOT BUILT]") << "\n";
            }
        }
        std::cout << "\nTokens (" << p1.m_player->getOwnedTokens().size() << "):\n";
        for (const auto& token : p1.m_player->getOwnedTokens()) {
            if (token) {
                std::cout << "  - " << token->getName() << "\n";
            }
        }
    }
    std::cout << "\n--- PLAYER 2: " << (p2.m_player ? p2.m_player->getPlayerUsername() : "Unknown") << " ---\n";
    if (p2.m_player) {
        auto coins = p2.m_player->getRemainingCoins();
        uint32_t totalCoins = p2.m_player->totalCoins(coins);
        std::cout << "Coins: " << totalCoins
            << " (1x" << static_cast<int>(std::get<0>(coins))
            << " + 3x" << static_cast<int>(std::get<1>(coins))
            << " + 6x" << static_cast<int>(std::get<2>(coins)) << ")\n";
        std::cout << "\nOwned Cards (" << p2.m_player->getOwnedCards().size() << "):\n";
        for (const auto& card : p2.m_player->getOwnedCards()) {
            if (card) {
                std::cout << "  - ";
                card->displayCardInfo();
                std::cout << "\n";
            }
        }
        std::cout << "\nWonders (" << p2.m_player->getOwnedWonders().size() << "):\n";
        for (const auto& wonder : p2.m_player->getOwnedWonders()) {
            if (wonder) {
                std::cout << "  - " << wonder->getName()
                    << (wonder->IsConstructed() ? " [CONSTRUCTED]" : " [NOT BUILT]") << "\n";
            }
        }
        std::cout << "\nTokens (" << p2.m_player->getOwnedTokens().size() << "):\n";
        for (const auto& token : p2.m_player->getOwnedTokens()) {
            if (token) {
                std::cout << "  - " << token->getName() << "\n";
            }
        }
    }
    std::cout << "\n==================================\n\n";
}
void Game::displayTurnStatus(const Player& p1, const Player& p2) {
    auto& board = Board::getInstance();
    int pawnPos = board.getPawnPos();
    std::cout << "\n╔════════════════════════════════════════════════════════════════╗\n";
    std::cout << "║                      CURRENT GAME STATUS                       ║\n";
    std::cout << "╠════════════════════════════════════════════════════════════════╣\n";
    std::cout << "║ MILITARY TRACK:                                                ║\n";
    std::cout << "║ P1 [";
    for (int i = 0; i <= 18; ++i) {
        if (i == pawnPos) {
            std::cout << "●";
        }
        else if (i == 9) {
            std::cout << "|";
        }
        else {
            std::cout << "-";
        }
    }
    std::cout << "] P2 ║\n";
    std::cout << "║    0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18    ║\n";
    std::cout << "║ Position: " << pawnPos;
    if (pawnPos < 9) std::cout << " (P1 winning by " << (9 - pawnPos) << ")";
    else if (pawnPos > 9) std::cout << " (P2 winning by " << (pawnPos - 9) << ")";
    else std::cout << " (Neutral)";
    std::cout << std::string(33 - std::to_string(pawnPos).length(), ' ') << "║\n";
    std::cout << "╠════════════════════════════════════════════════════════════════╣\n";
    auto calculateScore = [](const Player& p) -> uint32_t {
        if (!p.m_player) return 0;
        const auto& pts = p.m_player->getPoints();
        uint32_t total = static_cast<uint32_t>(pts.m_militaryVictoryPoints) +
            static_cast<uint32_t>(pts.m_buildingVictoryPoints) +
            static_cast<uint32_t>(pts.m_wonderVictoryPoints) +
            static_cast<uint32_t>(pts.m_progressVictoryPoints);
        total += p.m_player->totalCoins(p.m_player->getRemainingCoins()) / 3;
        return total;
        };
    uint32_t score1 = calculateScore(p1);
    uint32_t score2 = calculateScore(p2);
    std::cout << "║ PLAYER 1: " << (p1.m_player ? p1.m_player->getPlayerUsername() : "Unknown");
    std::cout << std::string(51 - (p1.m_player ? p1.m_player->getPlayerUsername().length() : 7), ' ') << "║\n";
    if (p1.m_player) {
        const auto& pts1 = p1.m_player->getPoints();
        std::cout << "║   Total Score: " << score1 << " VP";
        std::cout << std::string(47 - std::to_string(score1).length(), ' ') << "║\n";
        std::cout << "║   • Military: " << static_cast<int>(pts1.m_militaryVictoryPoints) << " VP";
        std::cout << std::string(45 - std::to_string(pts1.m_militaryVictoryPoints).length(), ' ') << "║\n";
        std::cout << "║   • Buildings: " << static_cast<int>(pts1.m_buildingVictoryPoints) << " VP";
        std::cout << std::string(44 - std::to_string(pts1.m_buildingVictoryPoints).length(), ' ') << "║\n";
        std::cout << "║   • Wonders: " << static_cast<int>(pts1.m_wonderVictoryPoints) << " VP";
        std::cout << std::string(46 - std::to_string(pts1.m_wonderVictoryPoints).length(), ' ') << "║\n";
        std::cout << "║   • Progress: " << static_cast<int>(pts1.m_progressVictoryPoints) << " VP";
        std::cout << std::string(45 - std::to_string(pts1.m_progressVictoryPoints).length(), ' ') << "║\n";
        uint32_t coinVP = p1.m_player->totalCoins(p1.m_player->getRemainingCoins()) / 3;
        std::cout << "║   • Coins: " << coinVP << " VP";
        std::cout << std::string(48 - std::to_string(coinVP).length(), ' ') << "║\n";
    }
    std::cout << "╠════════════════════════════════════════════════════════════════╣\n";
    std::cout << "║ PLAYER 2: " << (p2.m_player ? p2.m_player->getPlayerUsername() : "Unknown");
    std::cout << std::string(51 - (p2.m_player ? p2.m_player->getPlayerUsername().length() : 7), ' ') << "║\n";
    if (p2.m_player) {
        const auto& pts2 = p2.m_player->getPoints();
        std::cout << "║   Total Score: " << score2 << " VP";
        std::cout << std::string(47 - std::to_string(score2).length(), ' ') << "║\n";
        std::cout << "║   • Military: " << static_cast<int>(pts2.m_militaryVictoryPoints) << " VP";
        std::cout << std::string(45 - std::to_string(pts2.m_militaryVictoryPoints).length(), ' ') << "║\n";
        std::cout << "║   • Buildings: " << static_cast<int>(pts2.m_buildingVictoryPoints) << " VP";
        std::cout << std::string(44 - std::to_string(pts2.m_buildingVictoryPoints).length(), ' ') << "║\n";
        std::cout << "║   • Wonders: " << static_cast<int>(pts2.m_wonderVictoryPoints) << " VP";
        std::cout << std::string(46 - std::to_string(pts2.m_wonderVictoryPoints).length(), ' ') << "║\n";
        std::cout << "║   • Progress: " << static_cast<int>(pts2.m_progressVictoryPoints) << " VP";
        std::cout << std::string(45 - std::to_string(pts2.m_progressVictoryPoints).length(), ' ') << "║\n";
        uint32_t coinVP = p2.m_player->totalCoins(p2.m_player->getRemainingCoins()) / 3;
        std::cout << "║   • Coins: " << coinVP << " VP";
        std::cout << std::string(48 - std::to_string(coinVP).length(), ' ') << "║\n";
    }
    std::cout << "╚════════════════════════════════════════════════════════════════╝\n\n";
}
void Game::announceVictory(int winner, const std::string& victoryType, const Player& p1, const Player& p2) {
    std::cout << "\n";
    std::cout << "╔════════════════════════════════════════════════════════════════╗\n";
    std::cout << "║                         GAME OVER!                             ║\n";
    std::cout << "╠════════════════════════════════════════════════════════════════╣\n";
    std::string winnerName;
    if (winner == 0 && p1.m_player) {
        winnerName = p1.m_player->getPlayerUsername();
    }
    else if (winner == 1 && p2.m_player) {
        winnerName = p2.m_player->getPlayerUsername();
    }
    else if (winner == 2) {
        winnerName = "TIE";
    }
    std::cout << "║ Victory Type: " << victoryType;
    std::cout << std::string(49 - victoryType.length(), ' ') << "║\n";
    std::cout << "║ Winner: " << winnerName;
    std::cout << std::string(55 - winnerName.length(), ' ') << "║\n";
    std::cout << "╚════════════════════════════════════════════════════════════════╝\n\n";
    displayTurnStatus(p1, p2);
}
}