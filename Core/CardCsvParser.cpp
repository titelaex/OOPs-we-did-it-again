module Core.CardCsvParser;

import <vector>;
import <string>;
import <fstream>;
import <sstream>;
import <stdexcept>;
#include <functional>
import <algorithm>;
#include <unordered_map>
#include <optional>;

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

// Helper function to parse a single line of CSV, handling quoted fields
std::vector<std::string> ParseCsvLine(const std::string& line) {
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
    columns.push_back(cell); // Add the last cell
    return columns;
}

std::unordered_map<ResourceType, uint8_t> ParseResourceMap(const std::string& str) {
    std::unordered_map<ResourceType, uint8_t> map;
    std::istringstream ss(str);
    std::string token;
    // Revert to comma as the delimiter for resource lists.
    // This ensures it works for both AgeCards and Wonders.
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
    while (std::getline(ss, token, ';')) { // Use semicolon as a safer delimiter
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
        if (line.empty() || line.find_first_not_of(" \t\r\n") == std::string::npos) continue;
        std::vector<std::string> columns = ParseCsvLine(line);
        cards.push_back(factory(columns));
    }
    return cards;
}

AgeCard AgeCardFactory(const std::vector<std::string>& columns) {
    auto get = [&](size_t i) -> const std::string& { static const std::string empty{}; return i < columns.size() ? columns[i] : empty; };
    auto parse_u8 = [&](size_t i) -> uint8_t { return (i < columns.size() && !columns[i].empty()) ? static_cast<uint8_t>(std::stoi(columns[i])) : 0; };
    auto parse_bool = [&](size_t i) -> bool { return (i < columns.size()) && (columns[i] == "true" || columns[i] == "1"); };

    std::unordered_map<ResourceType, uint8_t> resourceCost = columns.size() > 1 ? ParseResourceMap(get(1)) : std::unordered_map<ResourceType, uint8_t>{};
    std::unordered_map<ResourceType, uint8_t> resourceProduction = columns.size() > 2 ? ParseResourceMap(get(2)) : std::unordered_map<ResourceType, uint8_t>{};
    uint8_t victoryPoints = parse_u8(3);
    uint8_t shieldPoints = parse_u8(4);
    uint8_t coinCost = parse_u8(5);
    ScientificSymbolType scientificSymbols = ParseEnum(StringToScientificSymbolType(get(6)), ScientificSymbolType::NO_SYMBOL);
    LinkingSymbolType hasLinkingSymbol = ParseEnum(StringToLinkingSymbolType(get(7)), LinkingSymbolType::NO_SYMBOL);
    LinkingSymbolType requiresLinkingSymbol = ParseEnum(StringToLinkingSymbolType(get(8)), LinkingSymbolType::NO_SYMBOL);
    CoinWorthType coinWorth = ParseEnum(StringToCoinWorthType(get(9)), CoinWorthType::VALUE);
    uint8_t coinReward = parse_u8(10);
    std::unordered_map<TradeRuleType, bool> tradeRules = columns.size() > 11 ? ParseTradeRuleMap(get(11)) : std::unordered_map<TradeRuleType, bool>{};
    std::string caption = get(12);
    ColorType color = ParseEnum(StringToColorType(get(13)), ColorType::BROWN);
    bool isVisible = parse_bool(14);
    std::string modelPath = get(15);
    Age age = ParseEnum(stringToAge(get(16)), Age::AGE_I);

    return AgeCard(
        get(0),
        resourceCost,
        resourceProduction,
        victoryPoints,
        shieldPoints,
        coinCost,
        scientificSymbols,
        hasLinkingSymbol,
        requiresLinkingSymbol,
        coinWorth,
        coinReward,
        tradeRules,
        caption,
        color,
        isVisible,
        modelPath,
        age
    );
}

GuildCard GuildCardFactory(const std::vector<std::string>& columns) {
    std::vector<std::string> scoringRules;
    if (columns.size() > 1) {
        std::istringstream rs(columns[1]);
        std::string token;
        while (std::getline(rs, token, ';')) {
            if (!token.empty()) scoringRules.push_back(token);
        }
    }
    std::string name = columns.size() > 0 ? columns[0] : std::string{};
    return GuildCard(name, scoringRules);
}

Wonder WonderFactory(const std::vector<std::string>& columns) {
    auto get = [&](size_t i) -> const std::string& { static const std::string empty{}; return i < columns.size() ? columns[i] : empty; };
    auto parse_u8 = [&](size_t i) -> uint8_t { return (i < columns.size() && !columns[i].empty()) ? static_cast<uint8_t>(std::stoi(columns[i])) : 0; };
    auto parse_bool = [&](size_t i) -> bool { return (i < columns.size()) && (columns[i] == "true" || columns[i] == "1"); };

    std::unordered_map<ResourceType, uint8_t> resourceCost = columns.size() > 1 ? ParseResourceMap(get(1)) : std::unordered_map<ResourceType, uint8_t>{};
    uint8_t victoryPoints = parse_u8(2);
    CoinWorthType coinWorth = ParseEnum(StringToCoinWorthType(get(3)), CoinWorthType::VALUE);
    uint8_t coinReward = parse_u8(4);
    std::string caption = get(5);
    ColorType color = ParseEnum(StringToColorType(get(6)), ColorType::NO_COLOR);
    bool isVisible = parse_bool(7);
    std::string modelPath = get(8);
    ResourceType resourceProduction = ParseEnum(StringToResourceType(get(9)), ResourceType::NO_RESOURCE);
    uint8_t shieldPoints = parse_u8(10);
    uint8_t opponentLosesMoney = parse_u8(11);
    bool discardCardFromOpponent = parse_bool(12);
    bool playSecondTurn = parse_bool(13);
    bool drawProgressTokens = parse_bool(14);
    bool chooseAndConstructBuilding = parse_bool(15);
    ColorType discardedCardColor = ParseEnum(StringToColorType(get(16)), ColorType::NO_COLOR);

    // Note: Models::Wonder ctor does not support playerReceivesMoney; ignore that column and pass args in correct order
    return Wonder(
        get(0),
        resourceCost,
        victoryPoints,
        coinWorth,
        coinReward,
        caption,
        color,
        isVisible,
        modelPath,
        opponentLosesMoney,
        shieldPoints,
        resourceProduction,
        playSecondTurn,
        drawProgressTokens,
        discardCardFromOpponent,
        chooseAndConstructBuilding,
        discardedCardColor
    );
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
        std::vector<std::string> columns = ParseCsvLine(line);
        TokenType type = TokenType::PROGRESS;
        try { if (columns.size() > 0) type = tokenTypeFromString(columns[0]); }
        catch (...) {}
        std::string name = columns.size() > 1 ? columns[1] : "";
        std::string description = columns.size() > 2 ? columns[2] : "";
        uint8_t coins = (columns.size() > 3 && !columns[3].empty()) ? static_cast<uint8_t>(std::stoi(columns[3])) : 0;
        uint8_t victory = (columns.size() > 4 && !columns[4].empty()) ? static_cast<uint8_t>(std::stoi(columns[4])) : 0;
        uint8_t shield = (columns.size() > 5 && !columns[5].empty()) ? static_cast<uint8_t>(std::stoi(columns[5])) : 0;
        tokens.emplace_back(type, name, description, std::make_tuple(coins, 0, 0), victory, shield);
    }
    return tokens;
}

