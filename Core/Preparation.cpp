module Core.Preparation;

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
        Core::SetupCardPools();

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
                Models::AgeCard card = AgeCardFactory(cols);
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

            for (size_t i = 0; i < allAgeCards.size(); ++i) {
                auto& c = allAgeCards[i];
                switch (c.getAge()) {
                case Models::Age::AGE_I:
                    Core::unusedAgeOneCards.push_back(std::make_unique<Models::AgeCard>(std::move(c)));
                    break;
                case Models::Age::AGE_II:
                    Core::unusedAgeTwoCards.push_back(std::make_unique<Models::AgeCard>(std::move(c)));
                    break;
                case Models::Age::AGE_III:
                default:
                    Core::unusedAgeThreeCards.push_back(std::make_unique<Models::AgeCard>(std::move(c)));
                    break;
                }
            }

            ShuffleInplace(Core::unusedAgeOneCards, seed);
            ShuffleInplace(Core::unusedAgeTwoCards, seed + 1);
            ShuffleInplace(Core::unusedAgeThreeCards, seed + 2);
            if (log.is_open()) log << "Loaded ages: I=" << Core::unusedAgeOneCards.size()
                << " II=" << Core::unusedAgeTwoCards.size() << " III=" << Core::unusedAgeThreeCards.size() << "\n";
            std::cout << "Loaded ages: I=" << Core::unusedAgeOneCards.size()
                << " II=" << Core::unusedAgeTwoCards.size() << " III=" << Core::unusedAgeThreeCards.size() << "\n";
        }
        catch (const std::exception& ex) {
            if (log.is_open()) log << "[Exception] While processing age cards: " << ex.what() << "\n";
        }

        try {
            std::vector<Models::GuildCard> tempGuilds;
            auto loadG = [&](const std::vector<std::string>& cols) {
                Models::GuildCard g = GuildCardFactory(cols);
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
            for (auto& g : tempGuilds) Core::unusedGuildCards.push_back(std::make_unique<Models::GuildCard>(std::move(g)));
            if (log.is_open()) log << "Loaded guilds: " << Core::unusedGuildCards.size() << "\n";
            std::cout << "Loaded guilds: " << Core::unusedGuildCards.size() << "\n";
        }
        catch (const std::exception& ex) {
            if (log.is_open()) log << "[Exception] While processing guild cards: " << ex.what() << "\n";
        }

        try {
            std::vector<Models::Wonder> tempWonders;
            auto loadW = [&](const std::vector<std::string>& cols) {
                Models::Wonder w = WonderFactory(cols);
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
            for (auto& w : tempWonders) Core::unusedWonders.push_back(std::make_unique<Models::Wonder>(std::move(w)));
            if (log.is_open()) log << "Loaded wonders: " << Core::unusedWonders.size() << "\n";
            std::cout << "Loaded wonders: " << Core::unusedWonders.size() << "\n";
        }
        catch (const std::exception& ex) {
            if (log.is_open()) log << "[Exception] While processing wonder cards: " << ex.what() << "\n";
        }

        {
            std::vector<std::unique_ptr<Models::Card>> selected;
            size_t take = std::min<size_t>(20, Core::unusedAgeOneCards.size());
            for (size_t i = 0; i < take; ++i) selected.push_back(std::move(Core::unusedAgeOneCards[i]));
            Core::unusedAgeOneCards.erase(Core::unusedAgeOneCards.begin(), Core::unusedAgeOneCards.begin() + take);
            selected = ShuffleAndMove(std::move(selected), seed);
            Core::Age1Tree tree(std::move(selected));
            Core::age1Nodes = tree.releaseNodes();
        }


        {
            std::vector<std::unique_ptr<Models::Card>> selected;
            size_t take = std::min<size_t>(20, Core::unusedAgeTwoCards.size());
            for (size_t i = 0; i < take; ++i) selected.push_back(std::move(Core::unusedAgeTwoCards[i]));
            Core::unusedAgeTwoCards.erase(Core::unusedAgeTwoCards.begin(), Core::unusedAgeTwoCards.begin() + take);
            selected = ShuffleAndMove(std::move(selected), seed + 1);
            Core::Age2Tree tree(std::move(selected));
            Core::age2Nodes = tree.releaseNodes();
        }


        {
            std::vector<std::unique_ptr<Models::Card>> selected;
            size_t take = std::min<size_t>(17, Core::unusedAgeThreeCards.size());
            for (size_t i = 0; i < take; ++i) selected.push_back(std::move(Core::unusedAgeThreeCards[i]));
            Core::unusedAgeThreeCards.erase(Core::unusedAgeThreeCards.begin(), Core::unusedAgeThreeCards.begin() + take);

            size_t guildTake = std::min<size_t>(3, Core::unusedGuildCards.size());
            for (size_t i = 0; i < guildTake; ++i) selected.push_back(std::move(Core::unusedGuildCards[i]));
            Core::unusedGuildCards.erase(Core::unusedGuildCards.begin(), Core::unusedGuildCards.begin() + guildTake);

            selected = ShuffleAndMove(std::move(selected), seed + 2);
            Core::Age3Tree tree(std::move(selected));
            Core::age3Nodes = tree.releaseNodes();
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
        for (size_t i = 0; i <= 3; i++)
        {
            availableWonders.push_back(
                std::unique_ptr<Models::Wonder>(static_cast<Models::Wonder*>(Core::unusedWonders[i].release()))
            );
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
        std::vector<std::unique_ptr<Models::Wonder>> availableWonders;
        for (size_t i = 0; i <= 3; i++)
        {
            availableWonders.push_back(
                std::unique_ptr<Models::Wonder>(static_cast<Models::Wonder*>(Core::unusedWonders[i].release()))
            );
        }


		displayAvailableWonders(availableWonders);


        uint8_t chosenIndex = 0;
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
