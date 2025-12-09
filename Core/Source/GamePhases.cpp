module Core.GamePhases;

#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <functional>
#include <algorithm>
#include <exception>
#include <random>
#include <filesystem>
#include <memory>

import Core.Board;
import Core.CardCsvParser;
import Core.Node;
import Core.AgeTree;
import Core.Player;

import Models.AgeCard;
import Models.GuildCard;
import Models.Wonder;
import Models.Age;
import Models.Card;
import Models.Player;

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
            for (size_t i =0; i < allAgeCards.size(); ++i) {
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
            ShuffleInplace(age2, seed +1);
            ShuffleInplace(age3, seed +2);
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
                } else {
                    ++i;
                }
            }
            selected = ShuffleAndMove(std::move(selected), seed);
            Core::Age1Tree tree(std::move(selected));
            board.setAge1Nodes(tree.releaseNodes());
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
            selected = ShuffleAndMove(std::move(selected), seed +1);
            Core::Age2Tree tree(std::move(selected));
            board.setAge2Nodes(tree.releaseNodes());
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
            selected = ShuffleAndMove(std::move(selected), seed +2);
            Core::Age3Tree tree(std::move(selected));
            board.setAge3Nodes(tree.releaseNodes());
        }
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

    void WonderSelection(std::unique_ptr<Core::Player> p1, std::unique_ptr<Core::Player> p2)
    {
		bool playerOneTurn = true; // true -> p1's turn, false -> p2's turn
		///random selection of the player to start the wonder selection
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<> dis(0, 1);
		playerOneTurn = (dis(gen) == 0) ? true : false;

		// get the first 4 wonders from the unusedWonders pool
		std::vector<std::unique_ptr<Models::Wonder>> availableWonders;
        const auto& wondersPool = Board::getInstance().getUnusedWonders();
        for (size_t i =0; i <=3 && i < wondersPool.size(); i++)
        {
            if (const auto* w = dynamic_cast<const Models::Wonder*>(wondersPool[i].get()))
                availableWonders.push_back(std::make_unique<Models::Wonder>(*w));
        }

		displayAvailableWonders(availableWonders);

        uint8_t chosenIndex = 0;
        playerOneTurn ? 
			(std::cout << "Player 1, choose your wonder (0-" << availableWonders.size() - 1 << "): ",
			std::cin >> chosenIndex,
            p1->chooseWonder(availableWonders, chosenIndex),
		    displayAvailableWonders(availableWonders),
            std::cout << "Player 2, choose your wonder (0-" << availableWonders.size() - 1 << "): ",
            std::cin >> chosenIndex,
            p2->chooseWonder(availableWonders, chosenIndex),
            std::cout << "Player 2, choose your wonder (0-" << availableWonders.size() - 1 << "): ",
            std::cin >> chosenIndex,
            p2->chooseWonder(availableWonders, chosenIndex),
            p1->chooseWonder(availableWonders, 0)) // ssa o fac automat din chooseWonder
        :
            (std::cout << "Player 2, choose your wonder (0-" << availableWonders.size() - 1 << "): ",
            std::cin >> chosenIndex,
            p2->chooseWonder(availableWonders, chosenIndex),
		    displayAvailableWonders(availableWonders),
            std::cout << "Player 1, choose your wonder (0-" << availableWonders.size() - 1 << "): ",
            std::cin >> chosenIndex,
            p1->chooseWonder(availableWonders, chosenIndex),
            std::cout << "Player 1, choose your wonder (0-" << availableWonders.size() - 1 << "): ",
            std::cin >> chosenIndex,
            p1->chooseWonder(availableWonders, chosenIndex),
            p2->chooseWonder(availableWonders, 0));


        // get the next 4 wonders from the unusedWonders pool
        for (size_t i =0; i <=3 && i < wondersPool.size(); i++)
        {
            if (const auto* w = dynamic_cast<const Models::Wonder*>(wondersPool[i].get()))
                availableWonders.push_back(std::make_unique<Models::Wonder>(*w));
        }


		displayAvailableWonders(availableWonders);

        !playerOneTurn ? 
			(std::cout << "Player 1, choose your wonder (0-" << availableWonders.size() - 1 << "): ",
			std::cin >> chosenIndex,
            p1->chooseWonder(availableWonders, chosenIndex),
		    displayAvailableWonders(availableWonders),
            std::cout << "Player 2, choose your wonder (0-" << availableWonders.size() - 1 << "): ",
            std::cin >> chosenIndex,
            p2->chooseWonder(availableWonders, chosenIndex),
            std::cout << "Player 2, choose your wonder (0-" << availableWonders.size() - 1 << "): ",
            std::cin >> chosenIndex,
            p2->chooseWonder(availableWonders, chosenIndex),
            p1->chooseWonder(availableWonders, 0)) // ssa o fac automat din chooseWonder
        :
            (std::cout << "Player 2, choose your wonder (0-" << availableWonders.size() - 1 << "): ",
            std::cin >> chosenIndex,
            p2->chooseWonder(availableWonders, chosenIndex),
		    displayAvailableWonders(availableWonders),
            std::cout << "Player 1, choose your wonder (0-" << availableWonders.size() - 1 << "): ",
            std::cin >> chosenIndex,
            p1->chooseWonder(availableWonders, chosenIndex),
            std::cout << "Player 1, choose your wonder (0-" << availableWonders.size() - 1 << "): ",
            std::cin >> chosenIndex,
            p1->chooseWonder(availableWonders, chosenIndex),
            p2->chooseWonder(availableWonders, 0));
    }
}
