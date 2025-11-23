#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <functional>
#include <algorithm>
#include <unordered_map>
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
import Models.Token;

using namespace Models;

std::unordered_map<ResourceType, uint8_t> ParseResourceMap(const std::string& str) {
    std::unordered_map<ResourceType, uint8_t> map;
    std::istringstream ss(str);
    std::string token;
    while (std::getline(ss, token, ',')) {
        auto pos = token.find(':');
        if (pos != std::string::npos) {
            std::string resStr = token.substr(0, pos);
            std::string valStr = token.substr(pos + 1);
            auto resOpt = StringToResourceType(resStr);
            if (resOpt.has_value()) {
                map[resOpt.value()] = static_cast<uint8_t>(std::stoi(valStr));
            }
        }
    }
    return map;
}

std::unordered_map<TradeRuleType, bool> ParseTradeRuleMap(const std::string& str) {
    std::unordered_map<TradeRuleType, bool> map;
    std::istringstream ss(str);
    std::string token;
    while (std::getline(ss, token, ',')) {
        auto pos = token.find(':');
        if (pos != std::string::npos) {
            std::string ruleStr = token.substr(0, pos);
            std::string valStr = token.substr(pos + 1);
            auto ruleOpt = StringToTradeRuleType(ruleStr);
            if (ruleOpt.has_value()) {
                map[ruleOpt.value()] = (valStr == "true" || valStr == "1");
            }
        }
    }
    return map;
}

template<typename E>
E ParseEnum(const std::optional<E>& opt, E fallback) {
    return opt.has_value() ? opt.value() : fallback;
}

template<typename T>
std::vector<T> ParseCardsFromCSV(const std::string& path, std::function<T(const std::vector<std::string>&)> factory) {
    std::ifstream ifs(path);
    if (!ifs.is_open()) throw std::runtime_error("Unable to open CSV file: " + path);
    std::string header;
    std::getline(ifs, header);
    std::vector<T> cards;
    std::string line;
    while (std::getline(ifs, line)) {
        if (line.empty()) continue;
        std::vector<std::string> columns;
        std::istringstream ss(line);
        std::string cell;
        while (std::getline(ss, cell, ',')) columns.push_back(cell);
        cards.push_back(factory(columns));
    }
    return cards;
}

AgeCard AgeCardFactory(const std::vector<std::string>& columns) {
    return AgeCard(
        columns[0],
        ParseResourceMap(columns[1]),
        ParseResourceMap(columns[2]),
        static_cast<uint8_t>(std::stoi(columns[3])),
        static_cast<uint8_t>(std::stoi(columns[4])),
        static_cast<uint8_t>(std::stoi(columns[5])),
        ParseEnum(StringToScientificSymbolType(columns[6]), ScientificSymbolType::NO_SYMBOL),
        ParseEnum(StringToLinkingSymbolType(columns[7]), LinkingSymbolType::NO_SYMBOL),
        ParseEnum(StringToLinkingSymbolType(columns[8]), LinkingSymbolType::NO_SYMBOL),
        ParseEnum(StringToCoinWorthType(columns[9]), CoinWorthType::VALUE),
        static_cast<uint8_t>(std::stoi(columns[10])),
        ParseTradeRuleMap(columns[11]),
        columns[12],
        ParseEnum(StringToColorType(columns[13]), ColorType::BROWN),
        columns[14] == "true",
        columns[15],
        ParseEnum(StringToAge(columns[16]), Age::AGE_I)
    );
}

GuildCard GuildCardFactory(const std::vector<std::string>& columns) {
    std::vector<std::string> scoringRules;
    return GuildCard(scoringRules);
}

Wonder WonderFactory(const std::vector<std::string>& columns) {
    return Wonder();
}

std::vector<Token> ParseTokensFromCSV(const std::string& path) {
    std::ifstream ifs(path);
    if (!ifs.is_open()) throw std::runtime_error("Unable to open Token CSV file: " + path);
    std::string header;
    std::getline(ifs, header); 
    std::vector<Token> tokens;
    std::string line;
    while (std::getline(ifs, line)) {
        if (line.empty()) continue;
        std::vector<std::string> columns;
        std::istringstream ss(line);
        std::string cell;
        while (std::getline(ss, cell, ',')) columns.push_back(cell);
        TokenType type = TokenType::PROGRESS;
        try { if (columns.size() > 0) type = tokenTypeFromString(columns[0]); } catch (...) {}
        std::string name = columns.size() > 1 ? columns[1] : "";
        std::string description = columns.size() > 2 ? columns[2] : "";
        uint8_t coins = (columns.size() > 3 && !columns[3].empty()) ? static_cast<uint8_t>(std::stoi(columns[3])) : 0;
        uint8_t victory = (columns.size() > 4 && !columns[4].empty()) ? static_cast<uint8_t>(std::stoi(columns[4])) : 0;
        uint8_t shield = (columns.size() > 5 && !columns[5].empty()) ? static_cast<uint8_t>(std::stoi(columns[5])) : 0;
        tokens.emplace_back(type, name, description, std::make_tuple(coins, 0, 0), victory, shield);
    }
    return tokens;
}

