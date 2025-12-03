module Core.CardCsvParser;

import <vector>;
import <string>;
import <fstream>;
import <sstream>;
import <stdexcept>;
import <algorithm>;
import <unordered_map>;
import <optional>;
import <tuple>;

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
    columns.push_back(cell);
    return columns;
}

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
    while (std::getline(ss, token, ';')) {
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

template<typename T, typename Factory>
std::vector<T> ParseCardsFromCSV(const std::string& path, Factory factory) {
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

    auto trim = [](std::string &s){
        size_t a = s.find_first_not_of(" \t\r\n");
        size_t b = s.find_last_not_of(" \t\r\n");
        if (a == std::string::npos) { s.clear(); return; }
        s = s.substr(a, b - a +1);
    };

    auto parseCoinsTuple = [](const std::string &field) -> std::tuple<uint8_t,uint8_t,uint8_t> {
        if (field.empty()) return {0,0,0};
        // Allow single number (e.g. "6") or a:b:c
        if (field.find(':') == std::string::npos) {
            uint8_t v =0; try { v = static_cast<uint8_t>(std::stoi(field)); } catch (...) {}
            return {v,0,0};
        }
        std::istringstream ss(field);
        std::string a,b,c; uint8_t va=0,vb=0,vc=0;
        if (std::getline(ss,a,':')) { try { va = static_cast<uint8_t>(std::stoi(a)); } catch(...){} }
        if (std::getline(ss,b,':')) { try { vb = static_cast<uint8_t>(std::stoi(b)); } catch(...){} }
        if (std::getline(ss,c)) { try { vc = static_cast<uint8_t>(std::stoi(c)); } catch(...){} }
        return {va,vb,vc};
    };

    std::vector<Token> tokens;
    std::string line;
    while (std::getline(ifs, line)) {
        if (line.empty() || line.find_first_not_of(" \t\r\n") == std::string::npos) continue;
        std::vector<std::string> rawCols = ParseCsvLine(line);
        if (rawCols.size() <6) {
            // Attempt to recover if fewer columns (skip for now)
            continue;
        }
        // If more than6 columns, merge description columns (2..size-4)
        size_t total = rawCols.size();
        size_t coinsIdx = total -3;
        size_t victoryIdx = total -2;
        size_t shieldIdx = total -1;
        std::string typeStr = rawCols[0];
        std::string name = rawCols[1];
        std::string description;
        for (size_t i =2; i < coinsIdx; ++i) {
            if (!description.empty()) description += ','; // reinsert comma lost by splitting
            description += rawCols[i];
        }
        std::string coinsField = rawCols[coinsIdx];
        std::string victoryField = rawCols[victoryIdx];
        std::string shieldField = rawCols[shieldIdx];

        trim(typeStr); trim(name); trim(description); trim(coinsField); trim(victoryField); trim(shieldField);

        TokenType type;
        try { type = tokenTypeFromString(typeStr); }
        catch (...) { continue; }
        // Keep only PROGRESS / MILITARY per current design
        if (type != TokenType::PROGRESS && type != TokenType::MILITARY) continue;

        auto coinsTuple = parseCoinsTuple(coinsField);
        uint8_t victory =0; if (!victoryField.empty()) { try { victory = static_cast<uint8_t>(std::stoi(victoryField)); } catch(...){} }
        uint8_t shield =0; if (!shieldField.empty()) { try { shield = static_cast<uint8_t>(std::stoi(shieldField)); } catch(...){} }

        tokens.emplace_back(type, name, description, coinsTuple, victory, shield);
    }
    return tokens;
}

