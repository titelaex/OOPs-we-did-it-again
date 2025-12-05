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
import Core.Player;

using namespace Models;

auto payCoins = [](uint8_t amt) {
    return [amt]() {
        Core::Player* cp = Core::GetCurrentPlayer();
        if (!cp) return;
        cp->subtractCoins(amt);
    };
};

auto getCoins = [](uint8_t amt) {
    return [amt]() {
        Core::Player* cp = Core::GetCurrentPlayer();
        if (!cp) return;
        cp->addCoins(amt);
    };
};

auto getResource = [](uint8_t amt, Models::ResourceType res) {
    return [amt, res]() {
        Core::Player* cp = Core::GetCurrentPlayer();
        if (!cp) return;
        cp->m_player.addPermanentResource(res, amt);
    };
};

auto takeResource = [](uint8_t amt, Models::ResourceType res) {
    return [amt, res]() {
        Core::Player* cp = Core::GetCurrentPlayer();
        if (!cp) return;
		auto owned = cp->m_player.getOwnedPermanentResources();
		auto it = owned.find(res);
        if (it != owned.end()) {
            uint8_t currentAmt = it->second;
            if (currentAmt >= amt) {
                cp->m_player.addPermanentResource(res, -static_cast<int8_t>(amt));
            }
            else {
                cp->m_player.addPermanentResource(res, -static_cast<int8_t>(currentAmt));
            }
        }
    };
};

auto getShieldPoints = [](uint8_t amt) {
    return [amt]() {
        Core::Player* cp = Core::GetCurrentPlayer();
        if (!cp) return;
        auto points = cp->m_player.getPoints();
        points.m_militaryVictoryPoints += amt;
        cp->m_player.setPoints(points);
    };
};

auto getVictoryPoints = [](uint8_t amt) {
    return [amt]() {
        Core::Player* cp = Core::GetCurrentPlayer();
        if (!cp) return;
        auto points = cp->m_player.getPoints();
        points.m_buildingVictoryPoints += amt;
        cp->m_player.setPoints(points);
    };
};

auto getScientificSymbol = [](ScientificSymbolType symbol) {
    return [symbol]() {
        Core::Player* cp = Core::GetCurrentPlayer();
        if (!cp) return;
		cp->m_player.addScientificSymbol(symbol, 1);
    };
};

auto getTradeRule = [](TradeRuleType rule) {
    return [rule]() {
        Core::Player* cp = Core::GetCurrentPlayer();
        if (!cp) return;
        auto tradeRules = cp->m_player.getTradeRules();
        tradeRules[rule] = true;
        cp->m_player.setTradeRules(tradeRules);
    };
};

auto applyAgeCoinWorth = [](CoinWorthType type) {
    return [type]() {
        Core::Player* cp = Core::GetCurrentPlayer();
        if (!cp) return;
        switch (type) {
        case CoinWorthType::GREY:
            cp->addCoins(3);
            break;
        case CoinWorthType::BROWN:
            cp->addCoins(2);
            break;
        case CoinWorthType::RED:
            cp->addCoins(1);
            break;
        case CoinWorthType::YELLOW:
            cp->addCoins(1);
            break;
        case CoinWorthType::WONDER:
            cp->addCoins(2);
            break;
        default:
            break;
        };
    };
};

auto applyGuildCointWorth = [](CoinWorthType type) {
    return [type]() {
        Core::Player* cp = Core::GetCurrentPlayer();
        if (!cp) return;
        switch (type) {
        case CoinWorthType::GREYBROWN:
            cp->addCoins(1);
			getVictoryPoints(1)();
            break;
        case CoinWorthType::GREEN:
            cp->addCoins(1);
            getVictoryPoints(1)();
            break;
        case CoinWorthType::RED:
            cp->addCoins(1);
            getVictoryPoints(1)();
            break;
        case CoinWorthType::YELLOW:
            cp->addCoins(1);
            getVictoryPoints(1)();
            break;
        case CoinWorthType::BLUE:
            cp->addCoins(1);
            getVictoryPoints(1)();
            break;
		case CoinWorthType::WONDER:
			getVictoryPoints(2)();
            break;
		default:
			cp->addCoins(3);
            getVictoryPoints(1)();
            break;
        };
    };
};

auto returnCoinsFromOpponent = [](uint8_t amt) {
    return [amt]() {
        Core::Player* cp = Core::GetCurrentPlayer();
        if (!cp) return;
        Core::Player* opponent = Core::GetOpponentPlayer();
        if (!opponent) return;
        opponent->subtractCoins(amt);
        cp->addCoins(amt);
    };
};

auto playAnotherTurn = []() {
    Core::Player* cp = Core::GetCurrentPlayer();
    if (!cp) return;
    cp->setHasAnotherTurn(true);
};

auto discardOpponentCard = [](ColorType type) {
    Core::Player* opponent = Core::GetOpponentPlayer();
    if (!opponent) return;
    opponent->discardCard(type);
};

auto drawToken = []() {
    Core::Player* cp = Core::GetCurrentPlayer();
    if (!cp) return;
    cp->drawToken();
};

auto takeNewCard = []() {
    Core::Player* cp = Core::GetCurrentPlayer();
    if (!cp) return;
    cp->takeNewCard();
};

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
    std::optional<ScientificSymbolType> scientificSymbols;
    if (auto opt = StringToScientificSymbolType(get(6)); opt.has_value()) scientificSymbols = opt;
    std::optional<LinkingSymbolType> hasLinkingSymbol;
    if (auto opt = StringToLinkingSymbolType(get(7)); opt.has_value()) hasLinkingSymbol = opt;
    std::optional<LinkingSymbolType> requiresLinkingSymbol;
    if (auto opt = StringToLinkingSymbolType(get(8)); opt.has_value()) requiresLinkingSymbol = opt;
    std::unordered_map<TradeRuleType, bool> tradeRules = columns.size() > 9 ? ParseTradeRuleMap(get(9)) : std::unordered_map<TradeRuleType, bool>{};
    std::string caption = get(10);
    std::optional<ColorType> color;
    if (auto opt = StringToColorType(get(11)); opt.has_value()) color = opt;
    bool isVisible = parse_bool(12);
    Age age = Age::AGE_I;
    if (auto opt = stringToAge(get(12)); opt.has_value()) age = opt.value();

    auto split_actions = [&](const std::string &s){
        std::vector<std::string> out;
        std::string cur; for (char c : s) { if (c==',') { auto a = cur; // trim
                    size_t a1 = a.find_first_not_of(" \t\r\n"); size_t a2 = a.find_last_not_of(" \t\r\n"); if (a1!=std::string::npos) a = a.substr(a1,a2-a1+1); else a.clear(); if (!a.empty()) out.push_back(a); cur.clear(); }
                else cur.push_back(c);} if (!cur.empty()) { auto a=cur; size_t a1=a.find_first_not_of(" \t\r\n"); size_t a2=a.find_last_not_of(" \t\r\n"); if (a1!=std::string::npos) a=a.substr(a1,a2-a1+1); else a.clear(); if (!a.empty()) out.push_back(a);} return out; };

    AgeCardBuilder b;
    b.setName(get(0)).setResourceCost(resourceCost).setResourceProduction(resourceProduction)
      .setVictoryPoints(victoryPoints).setShieldPoints(shieldPoints).setCaption(caption);
    if (color.has_value()) b.setColor(color.value());
    b.setAge(age);
    if (scientificSymbols.has_value()) b.setScientificSymbols(scientificSymbols);
    if (hasLinkingSymbol.has_value()) b.setHasLinkingSymbol(hasLinkingSymbol);
    if (requiresLinkingSymbol.has_value()) b.setRequiresLinkingSymbol(requiresLinkingSymbol);
    if (!tradeRules.empty()) b.setTradeRules(tradeRules);

    std::string playField = get(13);
    if (!playField.empty()) {
        for (auto &act : split_actions(playField)) {
            if (act == "getResource") {
                if (!resourceProduction.empty()) {
                    auto kv = *resourceProduction.begin();
                    b.addOnPlayAction(getResource(kv.second, kv.first));
                }
            } else if (act == "payCoins") {
                uint8_t amt = coinCost;
                if (amt==0) {
                    std::string num;
                    for (char c : caption) if (isdigit((unsigned char)c)) num.push_back(c);
                    if (!num.empty()) amt = static_cast<uint8_t>(std::stoi(num));
                }
                if (amt>0) b.addOnPlayAction(payCoins(amt));
            } else if (act == "getCoins") {
                uint8_t amt = 0; std::string num; for (char c: caption) if (isdigit((unsigned char)c)) num.push_back(c); if (!num.empty()) amt=static_cast<uint8_t>(std::stoi(num));
                if (amt>0) b.addOnPlayAction(getCoins(amt));
            } else if (act == "getShieldPoints") {
                if (shieldPoints>0) b.addOnPlayAction(getShieldPoints(shieldPoints));
            } else if (act == "getVictoryPoints") {
                if (victoryPoints>0) b.addOnPlayAction(getVictoryPoints(victoryPoints));
            } else if (act == "getScientificSymbol") {
                if (scientificSymbols.has_value()) b.addOnPlayAction(getScientificSymbol(scientificSymbols.value()));
            } else if (act == "getTradeRule") {
                 for (const auto &kv : tradeRules) if (kv.second) b.addOnPlayAction(getTradeRule(kv.first));
             } else if (act.rfind("applyAgeCoinWorth",0)==0) {
                 auto p1 = act.find('('); auto p2 = act.find(')'); if (p1!=std::string::npos && p2!=std::string::npos && p2>p1) {
                     std::string arg = act.substr(p1+1,p2-p1-1);
                     if (auto ct = StringToCoinWorthType(arg); ct.has_value()) {
                        b.addOnPlayAction(applyAgeCoinWorth(ct.value()));
                     }
                 }
             }
         }
     }

    std::string discField = get(14);
    if (!discField.empty()) {
        for (auto &act : split_actions(discField)) {
            if (act == "takeResource") {
                if (!resourceProduction.empty()) { auto kv = *resourceProduction.begin(); b.addOnDiscardAction(takeResource(kv.second, kv.first)); }
            }
        }
    }

    return b.build();
}

GuildCard GuildCardFactory(const std::vector<std::string>& columns) {
    std::string name = columns.size() > 0 ? columns[0] : std::string{};
    GuildCardBuilder b;
    b.setName(name);
    return b.build();
}

Wonder WonderFactory(const std::vector<std::string>& columns) {
    auto get = [&](size_t i) -> const std::string& { static const std::string empty{}; return i < columns.size() ? columns[i] : empty; };
    auto parse_u8 = [&](size_t i) -> uint8_t { return (i < columns.size() && !columns[i].empty()) ? static_cast<uint8_t>(std::stoi(columns[i])) : 0; };
    auto parse_bool = [&](size_t i) -> bool { return (i < columns.size()) && (columns[i] == "true" || columns[i] == "1"); };

    std::unordered_map<ResourceType, uint8_t> resourceCost = columns.size() > 1 ? ParseResourceMap(get(1)) : std::unordered_map<ResourceType, uint8_t>{};
    uint8_t victoryPoints = parse_u8(2);
    CoinWorthType coinWorth = CoinWorthType::WONDER;
    if (auto opt = StringToCoinWorthType(get(3)); opt.has_value()) coinWorth = opt.value();
    uint8_t coinReward = parse_u8(4);
    std::string caption = get(5);
    ColorType color = ColorType::NO_COLOR;
    if (auto opt = StringToColorType(get(6)); opt.has_value()) color = opt.value();
    bool isVisible = parse_bool(7);
    std::string modelPath = get(8);
    ResourceType resourceProduction = ResourceType::NO_RESOURCE;
    if (auto opt = StringToResourceType(get(9)); opt.has_value()) resourceProduction = opt.value();
    uint8_t shieldPoints = parse_u8(10);
    uint8_t opponentLosesMoney = parse_u8(11);
    bool discardCardFromOpponent = parse_bool(12);
    bool playSecondTurn = parse_bool(13);
    bool drawProgressTokens = parse_bool(14);
    bool chooseAndConstructBuilding = parse_bool(15);
    ColorType discardedCardColor = ColorType::NO_COLOR;
    if (auto opt = StringToColorType(get(16)); opt.has_value()) discardedCardColor = opt.value();

    WonderBuilder b;
    b.setName(get(0)).setResourceCost(resourceCost).setVictoryPoints(victoryPoints)
     .setCoinWorth(coinWorth).setCoinReward(coinReward).setCaption(caption).setColor(color)
     .setOpponentLosesMoney(opponentLosesMoney).setShieldPoints(shieldPoints).setResourceProduction(resourceProduction)
     .setConstructed(false);

    if (coinReward > 0) b.addOnPlayAction(payCoins(coinReward));
    return b.build();
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

