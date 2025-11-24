module Core.Preparation;

#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <functional>
#include <algorithm>
#include <unordered_map>
#include <exception>
#include <random>

import Core.Board;
import Core.CardCsvParser;
import Core.Node;

import Models.AgeCard;
import Models.GuildCard;
import Models.Wonder;
import Models.Age;
import Models.Card;

namespace Core {

static std::mt19937_64 make_rng(uint32_t seed) { return std::mt19937_64(seed); }

template<typename T>
void ShuffleInplace(std::vector<T>& v, uint32_t seed)
{
    auto rng = make_rng(seed);
    std::shuffle(v.begin(), v.end(), rng);
}

static std::vector<std::string> splitCSVLine(const std::string& line) {
    std::vector<std::string> cols;
    std::istringstream ss(line);
    std::string cell;
    while (std::getline(ss, cell, ',')) cols.push_back(cell);
    return cols;
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

void PrepareBoardCardPools()
{
    Core::SetupCardPools();

    uint32_t seed = static_cast<uint32_t>(std::random_device{}());
    std::ofstream log("Preparation.log", std::ios::app);

    try {
        std::vector<Models::AgeCard> allAgeCards;
        auto loadFn = [&](const std::vector<std::string>& cols){
            Models::AgeCard card = AgeCardFactory(cols);
            allAgeCards.push_back(std::move(card));
        };
        loadGenericFile("ModelsDLL/AgeCards.csv", loadFn, log);

        ShuffleInplace(allAgeCards, seed);

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

    } catch (const std::exception& ex) {
        if (log.is_open()) log << "[Exception] While processing age cards: " << ex.what() << "\n";
    }

    try {
        std::vector<Models::GuildCard> tempGuilds;
        auto loadG = [&](const std::vector<std::string>& cols){
            Models::GuildCard g = GuildCardFactory(cols);
            tempGuilds.push_back(std::move(g));
        };
        loadGenericFile("ModelsDLL/Guilds.csv", loadG, log);
        ShuffleInplace(tempGuilds, seed);
        for (auto& g : tempGuilds) Core::unusedGuildCards.push_back(std::make_unique<Models::GuildCard>(std::move(g)));
    } catch (const std::exception& ex) {
        if (log.is_open()) log << "[Exception] While processing guild cards: " << ex.what() << "\n";
    }

    try {
        std::vector<Models::Wonder> tempWonders;
        auto loadW = [&](const std::vector<std::string>& cols){
            Models::Wonder w = WonderFactory(cols);
            tempWonders.push_back(std::move(w));
        };
        loadGenericFile("ModelsDLL/Wonders.csv", loadW, log);
        ShuffleInplace(tempWonders, seed);
        for (auto& w : tempWonders) Core::unusedWonders.push_back(std::make_unique<Models::Wonder>(std::move(w)));
    } catch (const std::exception& ex) {
        if (log.is_open()) log << "[Exception] While processing wonder cards: " << ex.what() << "\n";
    }

    Core::BuildAge1Tree(seed);
    Core::BuildAge2Tree(seed + 1);
    Core::BuildAge3Tree(seed + 2);
}

} // namespace Core
