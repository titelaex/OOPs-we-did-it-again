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

std::unique_ptr<std::vector<std::unique_ptr<Models::Token>>> setupUnusedProgressTokens;
const int kNrOfRounds =20;
static const std::vector<int> kMilitaryTokenPositions = {2,5,8,11,14,17};

void m_receiveMoneyAction(class Player& player) {}
void m_opponentLosesMoneyAction(class Player& opponent) {}
void m_playSecondTurnAction(class Player& player) {}


//void wonderSelection(std::shared_ptr<Core::Player>& p1, std::shared_ptr<Core::Player>& p2);

std::vector<Models::Token> randomTokenSelector(std::vector<Models::Token>& discardedTokens)
{
	const uint8_t tokensToSelect =3;
	if (discardedTokens.size() <= tokensToSelect) return discardedTokens;
	std::vector<Models::Token> selectedTokens;
	std::random_device seed; std::mt19937 generator(seed());
	std::sample(discardedTokens.begin(), discardedTokens.end(), std::back_inserter(selectedTokens), tokensToSelect, generator);
	return selectedTokens;
}

std::pair<std::vector<std::unique_ptr<Models::Token>>, std::vector<std::unique_ptr<Models::Token>>> startGameTokens(std::vector<std::unique_ptr<Models::Token>> allTokens)
{
  
    if (!setupUnusedProgressTokens) setupUnusedProgressTokens = std::make_unique<std::vector<std::unique_ptr<Models::Token>>>();
    setupUnusedProgressTokens->clear();

    std::vector<std::unique_ptr<Models::Token>> progress;
    std::vector<std::unique_ptr<Models::Token>> military;
    progress.reserve(allTokens.size()); military.reserve(allTokens.size());
    for (auto &t : allTokens) {
        if (!t) continue;
        if (t->getType() == Models::TokenType::PROGRESS) progress.push_back(std::move(t));
        else if (t->getType() == Models::TokenType::MILITARY) military.push_back(std::move(t));
    }

    std::vector<std::unique_ptr<Models::Token>> selectedProgress;
    constexpr size_t kSelectCount = 5;
    if (progress.size() <= kSelectCount) {
        // move all
        selectedProgress = std::move(progress);
    }
    else {
        std::random_device rd; std::mt19937 gen(rd());
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

void m_drawProgressTokenAction(std::vector<Models::Token>& discardedTokens)
{
	auto selectedTokens = randomTokenSelector(discardedTokens);
}
void m_chooseAndConstructBuildingAction(const std::vector<Models::AgeCard>& discardedCards) {}
void m_discardCardFromOpponentAction(class Player& opponent, Models::ColorType color) {}

void movePawn(int steps) {
	auto track= Core::Board::getInstance().getPawnTrack();
	track.reset();
	auto position = Core::Board::getInstance().getPawnPos();
	position = std::clamp(position + steps, 0, 18);
	Core::Board::getInstance().setPawnPos(position);
	track.set(position);
    Core::Board::getInstance().setPawnTrack(track);
}

namespace Core {
    
    
	void preparation()
	{
		try {
			PrepareBoardCardPools();
 auto& gameState = Core::GameState::getInstance();
			std::shared_ptr<Core::Player> player1 = gameState.GetPlayer1();
			std::shared_ptr<Core::Player> player2 = gameState.GetPlayer2();
		//	wonderSelection(player1, player2);
			auto allTokens = parseTokensFromCSV("Config/Tokens.csv");
			auto [progressSelected, military] = startGameTokens(std::move(allTokens));
			Core::Board::getInstance().setProgressTokens(std::move(progressSelected));
			Core::Board::getInstance().setMilitaryTokens(std::move(military));
			Core::Board::getInstance().setUnusedProgressTokens(std::move(*setupUnusedProgressTokens));
		}
		catch (const std::exception& ex) {
			std::cerr << "Preparation exception: " << ex.what() << std::endl;
		}
		catch (...) {
			std::cerr << "Unknown exception during preparation" << std::endl;
		}
	}

    ///O stergem?
    /*void greatLibraryDrawFromSetup()
    {
        Core::Player* cp = Core::getCurrentPlayer();
        if (!cp) return;
        const auto& pool = (setupDiscardedProgressTokens && !setupDiscardedProgressTokens->empty()) ? *setupDiscardedProgressTokens : Core::Board::getInstance().getProgressTokens();
        if (pool.empty()) return;

        std::vector<size_t> indices(pool.size());
        for (size_t i = 0; i < indices.size(); ++i) indices[i] = i;
        std::random_device rd; std::mt19937 gen(rd());
        std::shuffle(indices.begin(), indices.end(), gen);
        size_t pickCount = std::min<size_t>(3, indices.size());
        indices.resize(pickCount);

        std::cout << "The Great Library: choose one of the drawn progress tokens:\n";
        for (size_t i = 0; i < pickCount; ++i) {
            const auto& tptr = pool[indices[i]];
            if (tptr) std::cout << "[" << i << "] " << tptr->getName() << "\n";
            else std::cout << "[" << i << "] <empty>\n";
        }

        size_t choice = 0;
        if (!(std::cin >> choice) || choice >= pickCount) {
            choice = 0;
            if (!std::cin) { std::cin.clear(); std::string discard; std::getline(std::cin, discard); }
        }

        const auto& chosenPtr = pool[indices[choice]];
        if (!chosenPtr) return;

        if (setupDiscardedProgressTokens && !setupDiscardedProgressTokens->empty()) {
            size_t chosenIndex = indices[choice];
            if (chosenIndex < setupDiscardedProgressTokens->size()) {
                std::unique_ptr<Models::Token> movedToken = std::move((*setupDiscardedProgressTokens)[chosenIndex]);
                setupDiscardedProgressTokens->erase(setupDiscardedProgressTokens->begin() + chosenIndex);
                if (movedToken) cp->m_player->addToken(std::move(movedToken));
            }
        }
        else {
            auto moved = removeProgressTokenByName(chosenPtr->getName());
            if (moved) cp->m_player->addToken(std::move(moved));
        }
    }
    */

}

// Debug helpers
namespace {
    void debugPrintAgeTree(const std::vector<std::shared_ptr<Core::Node>>& nodes, int age)
    {
        std::cout << "--- [DEBUG] Age " << age << " tree (all nodes in layout order) ---\n";
        for (size_t i = 0; i < nodes.size(); ++i) {
            if (nodes[i] && nodes[i]->getCard()) std::cout << nodes[i]->getCard()->getName();
            else std::cout << "<empty>";
            if (i + 1 < nodes.size()) std::cout << " | ";
        }
        std::cout << "\n";

        std::cout << "Available (isAvailable==1): ";
        bool first = true;
        for (size_t i = 0; i < nodes.size(); ++i) {
            if (nodes[i] && nodes[i]->getCard() && nodes[i]->isAvailable()) {
                if (!first) std::cout << ", ";
                std::cout << nodes[i]->getCard()->getName();
                first = false;
            }
        }
        if (first) std::cout << "<none>";
        std::cout << "\n\n";
    }

    void debugPrintTreeDetailed(const std::vector<std::shared_ptr<Core::Node>>& nodes, const std::vector<size_t>& rows, int age)
    {
        std::ostringstream oss;
        oss << "--- [DEBUG DETAILED] Age " << age << " tree ---\n";
        // map node ptr to index
        std::unordered_map<Core::Node*, int> idxMap;
        for (size_t i = 0; i < nodes.size(); ++i) if (nodes[i]) idxMap[nodes[i].get()] = static_cast<int>(i);

        // print rows
        size_t idx = 0;
        for (size_t r = 0; r < rows.size(); ++r) {
            size_t cols = rows[r];
            oss << "Row " << r << " (cols=" << cols << "): ";
            for (size_t c = 0; c < cols && idx < nodes.size(); ++c, ++idx) {
                auto& n = nodes[idx];
                if (!n || !n->getCard()) oss << "[" << idx << "]<empty>";
                else oss << "[" << idx << "]" << n->getCard()->getName();
                if (c + 1 < cols) oss << " | ";
            }
            oss << "\n";
        }

        // print full node info
        for (size_t i = 0; i < nodes.size(); ++i) {
            auto& n = nodes[i];
            oss << "node[" << i << "] ";
            if (!n || !n->getCard()) { oss << "<empty>\n"; continue; }
            auto* card = n->getCard();
            oss << "name=\"" << card->getName() << "\"";
            oss << " age=";
            // try to get age if AgeCard
            if (auto ac = dynamic_cast<Models::AgeCard*>(card)) oss << static_cast<int>(ac->getAge()); else oss << "-";
            oss << " vis=" << (card->isVisible() ? "1" : "0") << " avail=" << (n->isAvailable() ? "1" : "0");

            auto p1 = n->getParent1(); auto p2 = n->getParent2();
            oss << " parents=[";
            if (p1) { auto it = idxMap.find(p1.get()); if (it!=idxMap.end()) oss << it->second; else oss << "?"; } else oss << "-";
            oss << ",";
            if (p2) { auto it = idxMap.find(p2.get()); if (it!=idxMap.end()) oss << it->second; else oss << "?"; } else oss << "-";
            oss << "]";

            auto c1 = n->getChild1(); auto c2 = n->getChild2();
            oss << " children=[";
            if (c1) { auto it = idxMap.find(c1.get()); if (it!=idxMap.end()) oss << it->second; else oss << "?"; } else oss << "-";
            oss << ",";
            if (c2) { auto it = idxMap.find(c2.get()); if (it!=idxMap.end()) oss << it->second; else oss << "?"; } else oss << "-";
            oss << "]";

            oss << "\n";
        }
        oss << "--- end detailed age " << age << " ---\n\n";

        // output to console and append to log file
        std::string out = oss.str();
        std::cout << out;
        try {
            std::ofstream fout("TreeDebug.log", std::ios::app);
            if (fout.is_open()) fout << out;
        }
        catch (...) {}
    }

    void debugPrintAllAgeTrees()
    {
        auto& board = Core::Board::getInstance();
        // Detailed prints including row layout and parent/child indices
        debugPrintTreeDetailed(board.getAge1Nodes(), std::vector<size_t>{2,3,4,5,6}, 1);
        debugPrintTreeDetailed(board.getAge2Nodes(), std::vector<size_t>{6,5,4,3,2}, 2);
        debugPrintTreeDetailed(board.getAge3Nodes(), std::vector<size_t>{2,3,4,2,4,3,2}, 3);
    }
}

///CONTINUT GamePhases


namespace Core {

    static std::mt19937_64 make_rng(uint32_t seed) { return std::mt19937_64(seed); }

    template<typename T>
    static void ShuffleInplace(std::vector<T>& v, uint32_t seed)
    {
        auto rng = make_rng(seed);
        std::shuffle(v.begin(), v.end(), rng);
    }

    template<typename T>
    static std::vector<T> ShuffleAndMove(std::vector<T>&& src, uint32_t seed)
    {
        std::vector<T> dest = std::move(src);
        auto rng = make_rng(seed);
        std::shuffle(dest.begin(), dest.end(), rng);
        return dest;
    }

    static std::vector<std::string> splitCSVLine(const std::string& line) {
        std::vector<std::string> columns;
        std::stringstream ss(line);
        std::string cell;
        bool in_quotes = false;
        char c;

        while (ss.get(c)) {
            if (c == '"') {
                in_quotes = !in_quotes;
            }
            else if (c == ',' && !in_quotes) {
                columns.push_back(cell);
                cell.clear();
            }
            else {
                cell += c;
            }
        }
        columns.push_back(cell);
        return columns;
    }

    static void loadGenericFile(const std::string& path, const std::function<void(const std::vector<std::string>&)>& onItem, std::ofstream& log)
    {
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
                }
                catch (const std::exception& ex) {
                    if (log.is_open()) log << "[Error] Factory error in " << path << " at line " << lineno << ": " << ex.what() << "\n";
                }
                catch (...) {
                    if (log.is_open()) log << "[Error] Unknown factory error in " << path << " at line " << lineno << "\n";
                }
            }
        }
        catch (const std::exception& ex) {
            if (log.is_open()) log << "[Exception] Reading " << path << ": " << ex.what() << "\n";
        }
        catch (...) {
            if (log.is_open()) log << "[Exception] Unknown error reading " << path << "\n";
        }
    }

    void PrepareBoardCardPools()
    {
        auto& board = Board::getInstance();
        board.setupCardPools();

        uint32_t seed = static_cast<uint32_t>(std::random_device{}());
        std::ofstream log("Preparation.log", std::ios::app);

        auto configureRowVisibility = [](std::vector<std::shared_ptr<Node>>& nodes, const std::vector<size_t>& rowPattern)
        {
            size_t idx = 0;
            bool rowVisible = true;
            for (size_t row = 0; row < rowPattern.size() && idx < nodes.size(); ++row) {
                const bool isLastRow = (row == rowPattern.size() - 1);
                const size_t rowCount = rowPattern[row];
                for (size_t col = 0; col < rowCount && idx < nodes.size(); ++col, ++idx) {
                    if (auto node = nodes[idx]) {
                        if (auto* card = node->getCard()) {
                            card->setIsVisible(rowVisible);
                            bool noChildren = !(node->getChild1() || node->getChild2());
                            card->setIsAvailable(noChildren);
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
        }
        catch (...) {
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
                "Core/Config/AgeCards.csv",
                "../Core/Config/AgeCards.csv",
                "../../Core/Config/AgeCards.csv",
                "AgeCards.csv",
                "Resources/AgeCards.csv",
                "Core/Resources/AgeCards.csv",
                "../Core/Resources/AgeCards.csv",
                "../ModelsDLL/AgeCards.csv",
                "ModelsDLL/AgeCards.csv",
                "../Models/AgeCards.csv"
            };
            std::string agePath = findExistingPath(ageCandidates);
            if (agePath.empty()) {
                if (log.is_open()) log << "[Error] Age file not found in candidates\n";
                std::cout << "[Error] Age file not found in candidates\n";
            }
            else {
                if (log.is_open()) log << "Using Age file: " << agePath << "\n";
                std::cout << "Using Age file: " << agePath << "\n";
                loadGenericFile(agePath, loadFn, log);
            }

            // Move into board pools and shuffle (preserve logic)
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
        }
        catch (const std::exception& ex) {
            if (log.is_open()) log << "[Exception] While processing age cards: " << ex.what() << "\n";
        }

        try {
            std::vector<Models::GuildCard> tempGuilds;
            auto loadG = [&](const std::vector<std::string>& cols) {
                Models::GuildCard g = guildCardFactory(cols);
                tempGuilds.push_back(std::move(g));
                };
            const std::vector<std::string> guildCandidates = {
                "Core/Config/Guilds.csv",
                "../Core/Config/Guilds.csv",
                "../../Core/Config/Guilds.csv",
                "Guilds.csv",
                "Resources/Guilds.csv",
                "Core/Resources/Guilds.csv",
                "../Core/Resources/Guilds.csv",
                "../ModelsDLL/Guilds.csv",
                "ModelsDLL/Guilds.csv",
                "../Models/Guilds.csv"
            };
            std::string guildPath = findExistingPath(guildCandidates);
            if (guildPath.empty()) {
                if (log.is_open()) log << "[Error] Guild file not found in candidates\n";
                std::cout << "[Error] Guild file not found in candidates\n";
            }
            else {
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
        }
        catch (const std::exception& ex) {
            if (log.is_open()) log << "[Exception] While processing guild cards: " << ex.what() << "\n";
        }

        try {
            std::vector<Models::Wonder> tempWonders;
            auto loadW = [&](const std::vector<std::string>& cols) {
                Models::Wonder w = wonderFactory(cols);
                tempWonders.push_back(std::move(w));
                };
            const std::vector<std::string> wonderCandidates = {
                "Core/Config/Wonders.csv",
                "../Core/Config/Wonders.csv",
                "../../Core/Config/Wonders.csv",
                "Wonders.csv",
                "Resources/Wonders.csv",
                "Core/Resources/Wonders.csv",
                "../Core/Resources/Wonders.csv",
                "../ModelsDLL/Wonders.csv",
                "ModelsDLL/Wonders.csv",
                "../Models/Wonders.csv"
            };
            std::string wonderPath = findExistingPath(wonderCandidates);
            if (wonderPath.empty()) {
                if (log.is_open()) log << "[Error] Wonder file not found in candidates\n";
                std::cout << "[Error] Wonder file not found in candidates\n";
            }
            else {
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
        }
        catch (const std::exception& ex) {
            if (log.is_open()) log << "[Exception] While processing wonder cards: " << ex.what() << "\n";
        }

        // Build node trees
        {
            std::vector<std::unique_ptr<Models::Card>> selected;
            auto& pool = const_cast<std::vector<std::unique_ptr<Models::Card>>&>(board.getUnusedAgeOneCards());
            size_t take = std::min<size_t>(20, pool.size());
            // move matching AgeCard entries out of the pool into selected, removing from pool
            size_t i = 0;
            while (i < pool.size() && selected.size() < take) {
                if (!pool[i]) { ++i; continue; }
                if (dynamic_cast<Models::AgeCard*>(pool[i].get())) {
                    selected.push_back(std::move(pool[i]));
                    pool.erase(pool.begin() + i);
                    // do not increment i, next element shifted into this index
                }
                else {
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
                }
                else {
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
                }
                else {
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
                }
                else {
                    ++i;
                }
            }
            selected = ShuffleAndMove(std::move(selected), seed + 2);
            Core::Age3Tree tree(std::move(selected));
            auto age3Nodes = tree.releaseNodes();
            configureRowVisibility(age3Nodes, { 2,3,4,2,4,3,2 });
            board.setAge3Nodes(std::move(age3Nodes));
        }

        // Debug: print age trees after construction
        debugPrintAllAgeTrees();
    }


    void displayAvailableWonders(const std::vector<std::unique_ptr<Models::Wonder>>& wonders)
    {
        std::cout << "Available Wonders to choose from:\n";
        for (size_t i = 0; i < wonders.size(); ++i)
        {
            std::cout << "[" << i << "] ";
            wonders[i]->displayCardInfo();
            std::cout << "\n";
        }
    }

    static std::vector<std::unique_ptr<Models::Wonder>> takeNextFourWonders()
    {
        auto& wondersPool = const_cast<std::vector<std::unique_ptr<Models::Card>>&>(Board::getInstance().getUnusedWonders());
        std::vector<std::unique_ptr<Models::Wonder>> batch;
        for (size_t sel = 0; sel < 4 && !wondersPool.empty(); ++sel) {
            size_t idx = 0; bool found = false;
            for (; idx < wondersPool.size(); ++idx) {
                if (!wondersPool[idx]) continue;
                if (dynamic_cast<Models::Wonder*>(wondersPool[idx].get())) { found = true; break; }
            }
            if (!found) break;
            std::unique_ptr<Models::Card> cardPtr = std::move(wondersPool[idx]);
            wondersPool.erase(wondersPool.begin() + idx);
            Models::Wonder* raw = static_cast<Models::Wonder*>(cardPtr.release());
            batch.emplace_back(raw);
        }
        return batch;
    }

    static void promptPick(const char* prompt, std::shared_ptr<Core::Player>& player, std::vector<std::unique_ptr<Models::Wonder>>& batch)
    {
        if (batch.empty()) return;
        displayAvailableWonders(batch);
        std::cout << prompt << " (0-" << (batch.size() - 1) << "): ";
        uint8_t idx = 0;
        if (!(std::cin >> idx) || idx >= batch.size()) {
            if (!std::cin) { std::cin.clear(); std::string g; std::getline(std::cin, g); }
            idx = 0;
        }
        player->chooseWonder(batch, idx);
    }

    void wonderSelection(std::shared_ptr<Core::Player>& p1, std::shared_ptr<Core::Player>& p2)
    {
        {
            auto batch = takeNextFourWonders();
            if (batch.empty()) return;
            promptPick("Player1, choose your wonder", p1, batch);
            if (!batch.empty()) promptPick("Player2, choose your wonder", p2, batch);
            if (!batch.empty()) promptPick("Player2, choose your second wonder", p2, batch);
            if (!batch.empty()) p1->chooseWonder(batch, 0);
        }
        {
            auto batch = takeNextFourWonders();
            if (batch.empty()) return;
            promptPick("Player2, choose your wonder", p2, batch);
            if (!batch.empty()) promptPick("Player1, choose your wonder", p1, batch);
            if (!batch.empty()) promptPick("Player1, choose your second wonder", p1, batch);
            if (!batch.empty()) p1->chooseWonder(batch, 0);
        }
    }
}






namespace Core {
    static int g_next_age_starter = -1;

    static bool g_last_active_was_player_one = true;

    static bool determineChooserFromBoardAndLastActive(bool lastActiveWasPlayerOne)
    {
        auto& board = Board::getInstance();
        int pos = board.getPawnPos();
        if (pos < 9)
        {
            return true;
        }
        else if (pos > 9)
        {
            return false;
        }
        else
        {
            return lastActiveWasPlayerOne;
        }
    }

    static int checkImmediateMilitaryVictory()
    {
        auto& board = Board::getInstance();
        int pos = board.getPawnPos();
        if (pos <= 0) return 0;
        if (pos >= 18) return 1;
        return -1;
    }


    static int checkImmediateScientificVictory(Player& p1, Player& p2)
    {
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


    static int determineCivilianWinner(Player& p1, Player& p2)
    {
        auto safePointsRaw = [](Models::Player* mp)->uint32_t {
            if (!mp) 
                0;
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
            if (!mp) 
                return 0;
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

    void awardMilitaryTokenIfPresent(Player& receiver)
    {
        auto& board = Board::getInstance();
        int pos = board.getPawnPos();
        for (int p : kMilitaryTokenPositions) 
        {
            if (p == pos)
            {
                auto& military = const_cast<std::vector<std::unique_ptr<Models::Token>>&>(board.getMilitaryTokens());
                if (!military.empty()) 
                {
                    std::unique_ptr<Models::Token> t = std::move(military.back());
                    military.pop_back();
                    if (t) 
                    {
                        if (receiver.m_player) receiver.m_player->addToken(std::move(t));
                    }
                }
                break;
            }
        }
    }

    static void performCardAction(int action, Player& cur, Player& opp, std::unique_ptr<Models::Card>& cardPtr, Board& board)
    {
        if (!cardPtr) return;
        switch (action)
        {
        case 0: //build
        {
            try {
                cur.playCardBuilding(cardPtr, opp.m_player);
            }
            catch (const std::exception& ex)
            {
                std::cerr << "Exception in playCardBuilding:  " << ex.what() << "\n";
            }

            if (cardPtr)
            {
                auto& discarded = const_cast<std::vector<std::unique_ptr<Models::Card>>&>(board.getDiscardedCards());
                discarded.push_back(std::move(cardPtr));
            }
            break;
        }
        

        case 1: //sell
        {
            auto& discarded = const_cast<std::vector<std::unique_ptr<Models::Card>>&>(board.getDiscardedCards());
            cur.sellCard(cardPtr, discarded);
            break;
        }
        case 2: //use as wonder
        {
            auto& owned = cur.m_player->getOwnedWonders();
            std::vector<size_t> candidates;
            for (size_t i = 0; i < owned.size(); ++i) 
            {
                if (owned[i] && !owned[i]->IsConstructed()) 
                    candidates.push_back(i);
            }

            if (candidates.empty())
            {
                std::cout << "No available unbuilt wonders. Moving card to discard.\n";
                auto& discarded = const_cast<std::vector<std::unique_ptr<Models::Card>>&>(board.getDiscardedCards());
                discarded.push_back(std::move(cardPtr));
                break;
            }

            std::cout << "Choose wonder to construct:\n";
            for (size_t i = 0; i < candidates.size(); ++i) 
            {
                std::cout << "[" << i << "] " << owned[candidates[i]]->getName() << "\n";
            }
            int wchoice = 0;
            if (!(std::cin >> wchoice) || wchoice < 0 || static_cast<size_t>(wchoice) >= candidates.size())
            {
                if (!std::cin) { std::cin.clear(); std::string g; std::getline(std::cin, g); }
                wchoice = 0;
            }
            std::unique_ptr<Models::Wonder>& chosenWonderPtr = owned[candidates[static_cast<size_t>(wchoice)]];
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

    static uint8_t getShieldPointsFromCard(const Models::Card* card)
    {
        if (!card) return 0;
        if (auto ac = dynamic_cast<const Models::AgeCard*>(card)) {
            return static_cast<uint8_t>(ac->getShieldPoints());
        }
        return 0;
    }

    void phaseI(Player& p1, Player& p2)
    {
        int nrOfRounds = 1;
        auto& board = Board::getInstance();
        const auto& nodes = board.getAge1Nodes();
        std::random_device rd; std::mt19937 gen(rd());
        bool playerOneTurn = std::uniform_int_distribution<>(0, 1)(gen) == 0;

        while (nrOfRounds <= kNrOfRounds)
        {
            std::vector<size_t> availableIndex;
            availableIndex.reserve(nodes.size());
            for (size_t i = 0; i < nodes.size(); ++i)
                if (nodes[i] && nodes[i]->isAvailable()) availableIndex.push_back(i);
            if (availableIndex.empty()) break;

            std::cout << "PhaseI: " << availableIndex.size() << " cards available" << "\n";
            for (int k = 0; k < availableIndex.size(); k++)
            {
                size_t index = availableIndex[k];
                auto* card = nodes[index]->getCard();
                std::cout << "[" << k << "] Node index=" << index << " : " << (card ? card->getName() : std::string("<none>")) << "\n";
            } 

            Player* cur = playerOneTurn ? &p1 : &p2;
            Player* opp = playerOneTurn ? &p2 : &p1;

            std::cout << (playerOneTurn ? "Player 1" : "Player 2") << " choose index (0-" << (availableIndex.size() - 1) << "): ";
            int choice = 0;
            if (!(std::cin >> choice) || choice < 0 || static_cast<size_t>(choice) >= availableIndex.size()) 
            {
                if (!std::cin) 
                { 
                    std::cin.clear(); 
                    std::string garbage;
                    std::getline(std::cin, garbage); }
                choice = 0;
            }

            size_t chosenNodeIndex = availableIndex[static_cast<size_t>(choice)];
            std::unique_ptr<Models::Card> cardPtr= nodes[chosenNodeIndex]->releaseCard();
            if (!cardPtr)
            {
                std::cout << "Error: the node doesn't have a card or releaseCard() doesn't work.\n";
                playerOneTurn = !playerOneTurn;
                ++nrOfRounds;
                continue;
            }

            uint8_t shields = getShieldPointsFromCard(cardPtr.get());

            std::cout << " You choose " << cardPtr->getName() << " . Choose the action: '[0]=build, [1]=sell, [2]=use as wonder\n";
            int action = 0;
            if (!(std::cin >> action) || action < 0 || action>2)
            {
                ++nrOfRounds;
                if (!std::cin)
                {
                    std::cin.clear();
                    std::string g;
                    std::getline(std::cin,g);
                }
                action = 0;
            }

            performCardAction(action, *cur, *opp, cardPtr, board);
            if (shields > 0)
            {
                movePawn(static_cast<int>(shields));
                awardMilitaryTokenIfPresent(*cur);
                int win = checkImmediateMilitaryVictory();
                if (win != -1) 
                {
                    std::cout << "Immediate military victory for Player " << (win == 0 ? "1" : "2") << "!\n";
                    g_last_active_was_player_one = !playerOneTurn;
                    return;
                }
            }

            {
                int sv = checkImmediateScientificVictory(p1, p2);
                if (sv != -1) 
                {
                    std::cout << "Immediate scientific victory for Player " << (sv == 0 ? "1" : "2") << "!\n";
                    g_last_active_was_player_one = !playerOneTurn;
                    return;
                }
            }

            ++nrOfRounds;
            playerOneTurn = !playerOneTurn;
        }
        g_last_active_was_player_one = !playerOneTurn;
        std::cout << "Phase I completed.\n";
    }

    void phaseII(Player& p1, Player& p2)
    {
        int nrOfRounds = 1;
        auto& board = Board::getInstance();
        const auto& nodes = board.getAge2Nodes();
        bool chooserIsPlayer1 = determineChooserFromBoardAndLastActive(g_last_active_was_player_one);
        std::cout << (chooserIsPlayer1 ? "Player 1" : "Player 2") << " is the chooser for who begins Age II." << "\n";
        std::cout << "Chooser: pick who starts Age II ([0]=Player1, [1]=Player2): ";
        int starterChoice = 0;
        if (!(std::cin >> starterChoice) || (starterChoice != 0 && starterChoice != 1))
        {
            if (!std::cin) 
            {
                std::cin.clear(); 
                std::string g; 
                std::getline(std::cin, g);
            }
            starterChoice = chooserIsPlayer1 ? 0 : 1;
        }
        bool playerOneTurn = (starterChoice == 0);

        while (nrOfRounds <= kNrOfRounds)
        {
            std::vector<size_t> availableIndex;
            availableIndex.reserve(nodes.size());
            for (size_t i = 0; i < nodes.size(); ++i)
                if (nodes[i] && nodes[i]->isAvailable()) availableIndex.push_back(i);
            if (availableIndex.empty()) break;

            std::cout << "PhaseII: " << availableIndex.size() << " cards available" << "\n";
            for (int k = 0; k < availableIndex.size(); k++)
            {
                size_t index = availableIndex[k];
                auto* card = nodes[index]->getCard();
                std::cout << "[" << k << "] Node index=" << index << " : " << (card ? card->getName() : std::string("<none>")) << "\n";
            }

            Player* cur = playerOneTurn ? &p1 : &p2;
            Player* opp = playerOneTurn ? &p2 : &p1;

            std::cout << (playerOneTurn ? "Player 1" : "Player 2") << " choose index (0-" << (availableIndex.size() - 1) << "): ";
            int choice = 0;
            if (!(std::cin >> choice) || choice < 0 || static_cast<size_t>(choice) >= availableIndex.size())
            {
                if (!std::cin)
                {
                    std::cin.clear();
                    std::string garbage;
                    std::getline(std::cin, garbage);
                }
                choice = 0;
            }

            size_t chosenNodeIndex = availableIndex[static_cast<size_t>(choice)];
            std::unique_ptr<Models::Card> cardPtr = nodes[chosenNodeIndex]->releaseCard();
            if (!cardPtr)
            {
                std::cout << "Error: the node doesn't have a card or releaseCard() doesn't work.\n";
                playerOneTurn = !playerOneTurn;
                ++nrOfRounds;
                continue;
            }

            uint8_t shields = getShieldPointsFromCard(cardPtr.get());

            std::cout << " You choose " << cardPtr->getName() << " . Choose the action: '[0]=build, [1]=sell, [2]=use as wonder\n";
            int action = 0;
            if (!(std::cin >> action) || action < 0 || action>2)
            {
                ++nrOfRounds;
                if (!std::cin)
                {
                    std::cin.clear();
                    std::string g;
                    std::getline(std::cin,g);
                }
                action = 0;
            }

            performCardAction(action, *cur, *opp, cardPtr, board);
            if (shields > 0)
            {
                movePawn(static_cast<int>(shields));
                awardMilitaryTokenIfPresent(*cur);
                int win = checkImmediateMilitaryVictory();
                if (win != -1) 
                {
                    std::cout << "Immediate military victory for Player " << (win == 0 ? "1" : "2") << "!\n";
                    g_last_active_was_player_one = !playerOneTurn;
                    return;
                }
            }

            {
                int sv = checkImmediateScientificVictory(p1, p2);
                if (sv != -1) 
                {
                    std::cout << "Immediate scientific victory for Player " << (sv == 0 ? "1" : "2") << "!\n";
                    g_last_active_was_player_one = !playerOneTurn;
                    return;
                }
            }

            ++nrOfRounds;
            playerOneTurn = !playerOneTurn;
        }
        
        g_last_active_was_player_one = !playerOneTurn;
        std::cout << "Phase II completed.\n";
    }

    void
        phaseIII(Player& p1, Player& p2)
    {
        int nrOfRounds = 1;
        auto& board = Board::getInstance();
        const auto& nodes = board.getAge3Nodes();
        bool chooserIsPlayer1 = determineChooserFromBoardAndLastActive(g_last_active_was_player_one);
        std::cout << (chooserIsPlayer1 ? "Player 1" : "Player 2") << " is the chooser for who begins Age III." << "\n";
        std::cout << "Chooser: pick who starts Age III ([0]=Player1, [1]=Player2): ";
        int starterChoice = 0;
        if (!(std::cin >> starterChoice) || (starterChoice != 0 && starterChoice != 1)) {
            if (!std::cin) 
            { 
                std::cin.clear(); 
                std::string g;
                std::getline(std::cin, g);
            }
            starterChoice = chooserIsPlayer1 ? 0 : 1;
        }
        bool playerOneTurn = (starterChoice == 0);

        while (nrOfRounds <= kNrOfRounds)
        {
            std::vector<size_t> availableIndex;
            availableIndex.reserve(nodes.size());
            for (size_t i = 0; i < nodes.size(); ++i)
                if (nodes[i] && nodes[i]->isAvailable()) 
                    availableIndex.push_back(i);
            if (availableIndex.empty()) 
                break;

            std::cout << "PhaseIII: " << availableIndex.size() << " cards available" << "\n";
            for (int k = 0; k < availableIndex.size(); k++)
            {
                size_t index = availableIndex[k];
                auto* card = nodes[index]->getCard();
                std::cout << "[" << k << "] Node index=" << index << " : " << (card ? card->getName() : std::string("<none>")) << "\n";
            }

            Player* cur = playerOneTurn ? &p1 : &p2;
            Player* opp = playerOneTurn ? &p2 : &p1;

            std::cout << (playerOneTurn ? "Player 1" : "Player 2") << " choose index (0-" << (availableIndex.size() - 1) << "): ";
            int choice = 0;
            if (!(std::cin >> choice) || choice < 0 || static_cast<size_t>(choice) >= availableIndex.size())
            {
                if (!std::cin)
                {
                    std::cin.clear();
                    std::string garbage;
                    std::getline(std::cin, garbage);
                }
                choice = 0;
            }

            size_t chosenNodeIndex = availableIndex[static_cast<size_t>(choice)];
            std::unique_ptr<Models::Card> cardPtr = nodes[chosenNodeIndex]->releaseCard();
            if (!cardPtr)
            {
                std::cout << "Error: the node doesn't have a card or releaseCard() doesn't work." << "\n";
                playerOneTurn = !playerOneTurn;
                ++nrOfRounds;
                continue;
            }

            uint8_t shields = getShieldPointsFromCard(cardPtr.get());

            std::cout << " You choose " << cardPtr->getName() << " . Choose the action: '[0]=build, [1]=sell, [2]=use as wonder\n";
            int action = 0;
            if (!(std::cin >> action) || action < 0 || action>2)
            {
                ++nrOfRounds;
                if (!std::cin)
                {
                    std::cin.clear();
                    std::string g;
                    std::getline(std::cin,g);
                }
                action = 0;
            }

            performCardAction(action, *cur, *opp, cardPtr, board);
            if (shields > 0)
            {
                movePawn(static_cast<int>(shields));
                awardMilitaryTokenIfPresent(*cur);
                int win = checkImmediateMilitaryVictory();
                if (win != -1)
                {
                    std::cout << "Immediate military victory for Player " << (win == 0 ? "1" : "2") << "!\n";
                    g_last_active_was_player_one = !playerOneTurn;
                    return;
                }
            }

            {
                int sv = checkImmediateScientificVictory(p1, p2);
                if (sv != -1)
                {
                    std::cout << "Immediate scientific victory for Player " << (sv == 0 ? "1" : "2") << "!\n";
                    g_last_active_was_player_one = !playerOneTurn;
                    return;
                }
            }

            ++nrOfRounds;
            playerOneTurn = !playerOneTurn;
        }

        g_last_active_was_player_one = !playerOneTurn;
      
        int civil = determineCivilianWinner(p1, p2);
        if (civil == -1) 
        {
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
            if (total1 == total2) 
            {
                std::cout << "No supremacy occurred. Civilian victory: tie/shared. (P1=" << total1 << ", P2=" << total2 << ")\n";
            } else
            {
                std::cout << "No supremacy occurred. Civilian victory for Player " << (total1 > total2 ? "1" : "2")
                    << ". (P1=" << total1 << ", P2=" << total2 << ")\n";
            }
        }
        else
        {
            std::cout << "Civilian victory for Player " << (civil == 0 ? "1" : "2") << "\n";
        }
        std::cout << "Phase III completed." << "\n";
    }
}