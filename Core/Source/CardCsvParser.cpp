#include <map>
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
import <utility>;
import <functional>;
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
static std::string trimCopy(const std::string& input)
{
    const auto first = input.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return {};
    const auto last = input.find_last_not_of(" \t\r\n");
    return input.substr(first, last - first + 1);
}
static uint8_t parseUint8Value(const std::string& text)
{
    const std::string trimmed = trimCopy(text);
    if (trimmed.empty()) return 0;
    try {
        int value = std::stoi(trimmed);
        if (value < 0) return 0;
        if (value > 255) value = 255;
        return static_cast<uint8_t>(value);
    }
    catch (...) {
        return 0;
    }
}
static std::vector<std::string> splitActions(const std::string& field)
{
    std::vector<std::string> out;
    std::string cur;
    for (char c : field) {
        if (c == ',') {
            const std::string trimmed = trimCopy(cur);
            if (!trimmed.empty()) out.push_back(trimmed);
            cur.clear();
        }
        else {
            cur.push_back(c);
        }
    }
    const std::string trimmed = trimCopy(cur);
    if (!trimmed.empty()) out.push_back(trimmed);
    return out;
}
auto payCoins = [](uint8_t amt) {
    return [amt]() {
        auto cp = Core::getCurrentPlayer();
        if (!cp) return;
        cp->subtractCoins(amt);
    };
};
auto getCoins = [](uint8_t amt) {
    return [amt]() {
        auto cp = Core::getCurrentPlayer();
        if (!cp) return;
        cp->addCoins(amt);
    };
};
auto getResource = [](uint8_t amt, Models::ResourceType res) {
    return [amt, res]() {
        auto cp = Core::getCurrentPlayer();
        if (!cp) return;
        cp->m_player->addPermanentResource(res, amt);
    };
};
auto takeResource = [](uint8_t amt, Models::ResourceType res) {
    return [amt, res]() {
        auto cp = Core::getCurrentPlayer();
        if (!cp) return;
		auto owned = cp->m_player->getOwnedPermanentResources();
		auto it = owned.find(res);
        if (it != owned.end()) {
            uint8_t currentAmt = it->second;
            if (currentAmt >= amt) {
                cp->m_player->addPermanentResource(res, -static_cast<int8_t>(amt));
            }
            else {
                cp->m_player->addPermanentResource(res, -static_cast<int8_t>(currentAmt));
            }
        }
    };
};
auto getShieldPoints = [](uint8_t amt) {
    return [amt]() {
        auto cp = Core::getCurrentPlayer();
        if (!cp) return;
        auto points = cp->m_player->getPoints();
        points.m_militaryVictoryPoints += amt;
        cp->m_player->setPoints(points);
    };
};
auto getVictoryPoints = [](uint8_t amt) {
    return [amt]() {
        auto cp = Core::getCurrentPlayer();
        if (!cp) return;
        auto points = cp->m_player->getPoints();
        points.m_buildingVictoryPoints += amt;
        cp->m_player->setPoints(points);
    };
};
auto getScientificSymbol = [](ScientificSymbolType symbol) {
    return [symbol]() {
        auto cp = Core::getCurrentPlayer();
        if (!cp) return;
		cp->m_player->addScientificSymbol(symbol, 1);
    };
};
auto getTradeRule = [](TradeRuleType rule) {
    return [rule]() {
        auto cp = Core::getCurrentPlayer();
        if (!cp) return;
        auto tradeRules = cp->m_player->getTradeRules();
        tradeRules[rule] = true;
        cp->m_player->setTradeRules(tradeRules);
    };
};
auto applyAgeCoinWorth = [](CoinWorthType type) {
    return [type]() {
        auto cp = Core::getCurrentPlayer();
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
auto applyGuildCoinWorth = [](CoinWorthType type) {
    return [type]() {
        auto cp = Core::getCurrentPlayer();
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
        auto cp = Core::getCurrentPlayer();
        if (!cp) return;
        auto opponent = Core::getOpponentPlayer();
        if (!opponent) return;
        opponent->subtractCoins(amt);
        cp->addCoins(amt);
    };
};
auto playAnotherTurn = []() {
    auto cp = Core::getCurrentPlayer();
    if (!cp) return;
    cp->setHasAnotherTurn(true);
};
auto discardOpponentCard = [](ColorType type) {
    auto opponent = Core::getOpponentPlayer();
    if (!opponent) return;
    opponent->discardCard(type);
};
auto drawToken = []() {
    auto cp = Core::getCurrentPlayer();
    if (!cp) return;
    cp->drawToken();
};
auto takeNewCard = []() {
    auto cp = Core::getCurrentPlayer();
    if (!cp) return;
    cp->takeNewCard();
};
auto lawTokenEffect = []() {
    auto cp = Core::getCurrentPlayer();
    if (!cp) return;
	cp->m_player->addScientificSymbol(ScientificSymbolType::SCALE, 1);
};
auto applyToken = [](std::string tokenName) {
    return [tokenName]() {
        std::map<int, std::string> tokenDisctionary{
            {1, "Agriculture"},
            {2, "Architecture"},
            {3, "Economy"},
            {4, "Law"},
            {5, "Mansory"},
            {6, "Mathematics"},
            {7, "Philosophy"},
            {8, "Strategy"},
            {9, "Theology"},
            {10, "Urbanism"}
        };
        auto cp = Core::getCurrentPlayer();
        if (!cp || !cp->m_player) return;
        auto& player = *cp->m_player;
        for (const auto& [idx, name] : tokenDisctionary) {
            if (name == tokenName && idx > 0 && static_cast<size_t>(idx) <= player.getTokensOwned().size()) {
                auto tokens = player.getTokensOwned();
                tokens.set(static_cast<size_t>(idx - 1));
                player.setTokensOwned(tokens);
                break;
            }
        }
    };
};
std::vector<std::string> parseCsvLine(const std::string& line) {
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
std::unordered_map<ResourceType, uint8_t> parseResourceMap(const std::string& str) {
    std::unordered_map<ResourceType, uint8_t> map;
    std::istringstream ss(str);
    std::string token;
    while (std::getline(ss, token, ',')) {
        token = trimCopy(token);
        if (token.empty()) continue;
        auto pos = token.find(':');
        if (pos != std::string::npos) {
            std::string resStr = trimCopy(token.substr(0, pos));
            std::string valStr = trimCopy(token.substr(pos + 1));
            if (resStr.empty() || valStr.empty()) continue;
            auto resOpt = StringToResourceType(resStr);
            if (resOpt.has_value()) {
                map[resOpt.value()] = static_cast<uint8_t>(std::stoi(valStr));
            }
        }
    }
    return map;
}
std::unordered_map<TradeRuleType, bool> parseTradeRuleMap(const std::string& str) {
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
E parseEnum(const std::optional<E>& opt, E fallback) {
    return opt.has_value() ? opt.value() : fallback;
}
template<typename T, typename Factory>
std::vector<T> parseCardsFromCSV(const std::string& path, Factory factory) {
    std::ifstream ifs(path);
    if (!ifs.is_open()) throw std::runtime_error("Unable to open CSV file: " + path);
    std::string header;
    std::getline(ifs, header);
    std::vector<T> cards;
    std::string line;
    while (std::getline(ifs, line)) {
        if (line.empty() || line.find_first_not_of(" \t\r\n") == std::string::npos) continue;
        std::vector<std::string> columns = parseCsvLine(line);
        cards.push_back(factory(columns));
    }
    return cards;
}
AgeCard ageCardFactory(const std::vector<std::string>& columns) {
    auto get = [&](size_t i) -> const std::string& { static const std::string empty{}; return i < columns.size() ? columns[i] : empty; };
    auto parse_u8 = [&](size_t i) -> uint8_t { return i < columns.size() ? parseUint8Value(columns[i]) : 0; };
    auto parse_bool = [&](size_t i) -> bool { return (i < columns.size()) && (trimCopy(columns[i]) == "true" || trimCopy(columns[i]) == "1"); };
    auto parseResourceField = [&](size_t idx) {
        if (idx >= columns.size()) return std::unordered_map<ResourceType, uint8_t>{};
        const std::string field = trimCopy(get(idx));
        return field.empty() ? std::unordered_map<ResourceType, uint8_t>{} : parseResourceMap(field);
    };
    std::unordered_map<ResourceType, uint8_t> resourceCost = parseResourceField(1);
    std::unordered_map<ResourceType, uint8_t> resourceProduction = parseResourceField(2);
    uint8_t victoryPoints = parse_u8(3);
    uint8_t shieldPoints = parse_u8(4);
    uint8_t coinCost = parse_u8(5);
    std::optional<ScientificSymbolType> scientificSymbols;
    if (auto opt = StringToScientificSymbolType(trimCopy(get(6))); opt.has_value()) scientificSymbols = opt;
    std::optional<LinkingSymbolType> hasLinkingSymbol;
    if (auto opt = StringToLinkingSymbolType(trimCopy(get(7))); opt.has_value()) hasLinkingSymbol = opt;
    std::optional<LinkingSymbolType> requiresLinkingSymbol;
    if (auto opt = StringToLinkingSymbolType(trimCopy(get(8))); opt.has_value()) requiresLinkingSymbol = opt;
    std::unordered_map<TradeRuleType, bool> tradeRules = columns.size() > 9 ? parseTradeRuleMap(get(9)) : std::unordered_map<TradeRuleType, bool>{};
    std::string caption = trimCopy(get(10));
    std::optional<ColorType> color;
    if (auto opt = StringToColorType(trimCopy(get(11))); opt.has_value()) color = opt;
    bool isVisible = parse_bool(12);
    Age age = Age::AGE_I;
    if (auto opt = stringToAge(trimCopy(get(12))); opt.has_value()) age = opt.value();
    AgeCardBuilder b;
    b.setName(get(0)).setResourceCost(resourceCost).setResourceProduction(resourceProduction)
        .setVictoryPoints(victoryPoints).setShieldPoints(shieldPoints).setCoinCost(coinCost).setCaption(caption);
    if (color.has_value()) b.setColor(color.value());
    b.setAge(age);
    if (scientificSymbols.has_value()) b.setScientificSymbols(scientificSymbols);
    if (hasLinkingSymbol.has_value()) b.setHasLinkingSymbol(hasLinkingSymbol);
    if (requiresLinkingSymbol.has_value()) b.setRequiresLinkingSymbol(requiresLinkingSymbol);
    if (!tradeRules.empty()) b.setTradeRules(tradeRules);
    std::string playField = get(13);
    if (!playField.empty()) {
		for (const auto& act : splitActions(playField)) {
			bool handled = false;
			auto addAction = [&](auto fn) {
				b.addOnPlayAction(fn, act);
				handled = true;
			};
			if (act == "getResource") {
				if (!resourceProduction.empty()) {
					auto kv = *resourceProduction.begin();
					addAction(getResource(kv.second, kv.first));
				}
			}
			else if (act == "payCoins") {
				uint8_t amt = coinCost;
				if (amt == 0) {
					std::string num;
					for (char c : caption) if (isdigit((unsigned char)c)) num.push_back(c);
					if (!num.empty()) amt = static_cast<uint8_t>(std::stoi(num));
				}
				if (amt > 0) addAction(payCoins(amt));
			}
			else if (act == "getCoins") {
				uint8_t amt = 0; std::string num; for (char c : caption) if (isdigit((unsigned char)c)) num.push_back(c); if (!num.empty()) amt = static_cast<uint8_t>(std::stoi(num));
				if (amt > 0) addAction(getCoins(amt));
			}
			else if (act == "getShieldPoints") {
				if (shieldPoints > 0) addAction(getShieldPoints(shieldPoints));
			}
			else if (act == "getVictoryPoints") {
				if (victoryPoints > 0) addAction(getVictoryPoints(victoryPoints));
			}
			else if (act == "getScientificSymbol") {
				if (scientificSymbols.has_value()) addAction(getScientificSymbol(scientificSymbols.value()));
			}
			else if (act == "getTradeRule") {
				for (const auto& kv : tradeRules) if (kv.second) addAction(getTradeRule(kv.first));
			}
			else if (act.rfind("applyAgeCoinWorth", 0) == 0) {
				auto p1 = act.find('('); auto p2 = act.find(')'); if (p1 != std::string::npos && p2 != std::string::npos && p2 > p1) {
					std::string arg = act.substr(p1 + 1, p2 - p1 - 1);
					if (auto ct = StringToCoinWorthType(arg); ct.has_value()) {
						addAction(applyAgeCoinWorth(ct.value()));
					}
				}
			}
			if (!handled) {
				b.addOnPlayAction([]() {}, act);
			}
		}
    }
    std::string discField = get(14);
    if (!discField.empty()) {
        for (const auto& act : splitActions(discField)) {
            if (act == "takeResource") {
                if (!resourceProduction.empty()) { auto kv = *resourceProduction.begin(); b.addOnDiscardAction(takeResource(kv.second, kv.first), act); }
            }
        }
    }
    return b.build();
}
GuildCard guildCardFactory(const std::vector<std::string>&columns) {
    auto get = [&](size_t i) -> const std::string& { static const std::string empty{}; return i < columns.size() ? columns[i] : empty; };
    auto parse_u8 = [&](size_t i) -> uint8_t { return i < columns.size() ? parseUint8Value(columns[i]) : 0; };
    std::string name = get(0);
    std::unordered_map<ResourceType, uint8_t> resourceCost = parseResourceMap(trimCopy(get(1)));
    uint8_t victoryPoints = parse_u8(2);
    std::string caption = get(3);
    ColorType color = ColorType::NO_COLOR;
    if (auto opt = StringToColorType(get(4)); opt.has_value()) color = opt.value();
    GuildCardBuilder b;
    b.setName(name).setResourceCost(resourceCost).setVictoryPoints(victoryPoints).setCaption(caption);
    if (color != ColorType::NO_COLOR) b.setColor(color);
    std::string actionsField = get(5);
    if (!actionsField.empty()) {
		for (const auto& act : splitActions(actionsField)) {
			bool handled = false;
			if (act.rfind("applyGuildCoinWorth", 0) == 0) {
				auto p1 = act.find('('); auto p2 = act.find(')'); if (p1 != std::string::npos && p2 != std::string::npos && p2 > p1) {
					std::string arg = act.substr(p1 + 1, p2 - p1 - 1);
					if (auto ct = StringToCoinWorthType(arg); ct.has_value()) {
						b.addOnPlayAction(applyGuildCoinWorth(ct.value()), act);
						handled = true;
					}
				}
			}
			if (!handled) {
				b.addOnPlayAction([]() {}, act);
			}
		}
    }
    return b.build();
}
Wonder wonderFactory(const std::vector<std::string>& columns) {
    auto get = [&](size_t i) -> const std::string& { static const std::string empty{}; return i < columns.size() ? columns[i] : empty; };
    auto parse_u8 = [&](size_t i) -> uint8_t { return i < columns.size() ? parseUint8Value(columns[i]) : 0; };
    std::unordered_map<ResourceType, uint8_t> resourceCost = parseResourceMap(trimCopy(get(1)));
    uint8_t victoryPoints = parse_u8(2);
    std::string caption = get(3);
    ColorType color = ColorType::NO_COLOR;
    if (auto opt = StringToColorType(get(4)); opt.has_value()) color = opt.value();
    ResourceType resourceProduction = ResourceType::NO_RESOURCE;
    if (auto opt = StringToResourceType(get(5)); opt.has_value()) resourceProduction = opt.value();
    uint8_t shieldPoints = parse_u8(6);
    WonderBuilder b;
    b.setName(get(0)).setResourceCost(resourceCost).setVictoryPoints(victoryPoints)
     .setCaption(caption).setColor(color)
     .setShieldPoints(shieldPoints).setResourceProduction(resourceProduction)
     .setConstructed(false);
    std::string actionsField = get(7);
    if (!actionsField.empty()) {
        for (const auto& act : splitActions(actionsField)) {
            bool handled = false;
            auto addAction = [&](auto fn) {
                b.addOnPlayAction(fn, act);
                handled = true;
            };
			if (act == "getShieldPoints") {
				if (shieldPoints > 0) addAction(getShieldPoints(shieldPoints));
			} else if (act == "getVictoryPoints") {
				if (victoryPoints > 0) addAction(getVictoryPoints(victoryPoints));
			} else if (act == "drawToken") {
				addAction(drawToken);
			} else if (act == "takeNewCard") {
				addAction(takeNewCard);
			} else if (act.rfind("getCoins(", 0) == 0) {
				auto p1 = act.find('('); auto p2 = act.find(')'); if (p1 != std::string::npos && p2 != std::string::npos && p2 > p1) {
					std::string arg = act.substr(p1 + 1, p2 - p1 - 1);
					try { uint8_t v = static_cast<uint8_t>(std::stoi(arg)); addAction(getCoins(v)); } catch(...){ }
				}
			} else if (act.rfind("discardOpponentCard", 0) == 0) {
				auto p1 = act.find('('); auto p2 = act.find(')'); if (p1 != std::string::npos && p2 != std::string::npos && p2 > p1) {
					std::string arg = act.substr(p1 + 1, p2 - p1 - 1);
					if (auto copt = StringToColorType(arg); copt.has_value()) {
						ColorType col = copt.value();
						addAction([col]() { discardOpponentCard(col); });
					}
				}
			} else if (act == "getResource") {
				if (resourceProduction != ResourceType::NO_RESOURCE) addAction(getResource(1, resourceProduction));
			} else if (act.rfind("returnCoinsFromOpponent", 0) == 0) {
				auto p1 = act.find('('); auto p2 = act.find(')'); if (p1 != std::string::npos && p2 != std::string::npos && p2 > p1) {
					std::string arg = act.substr(p1 + 1, p2 - p1 - 1);
					try { uint8_t v = static_cast<uint8_t>(std::stoi(arg)); addAction(returnCoinsFromOpponent(v)); } catch(...){ }
				}
			} else if (act == "playAnotherTurn") {
				addAction(playAnotherTurn);
			}
			if (!handled) {
				b.addOnPlayAction([]() {}, act);
			}
        }
    }
    return b.build();
 }
std::vector<std::unique_ptr<Token>> parseTokensFromCSV(const std::string& path) {
    std::ifstream ifs(path);
    if (!ifs.is_open()) throw std::runtime_error("Unable to open Token CSV file: " + path);
    std::string header;
    std::getline(ifs, header);
    const bool hasActionColumn = header.find("onPlayActions") != std::string::npos;
    auto trim = [](std::string &s){
        size_t a = s.find_first_not_of(" \t\r\n");
        size_t b = s.find_last_not_of(" \t\r\n");
        if (a == std::string::npos) { s.clear(); return; }
        s = s.substr(a, b - a + 1);
    };
    auto parseCoinsTuple = [](const std::string &field) -> std::tuple<uint8_t,uint8_t,uint8_t> {
        if (field.empty()) return {0,0,0};
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
    auto totalCoinValue = [](const std::tuple<uint8_t,uint8_t,uint8_t>& coins) -> uint32_t {
        return static_cast<uint32_t>(std::get<0>(coins)) + static_cast<uint32_t>(std::get<1>(coins)) * 3u + static_cast<uint32_t>(std::get<2>(coins)) * 6u;
    };
    std::vector<std::unique_ptr<Token>> tokens;
    std::string line;
    while (std::getline(ifs, line)) {
        if (line.empty() || line.find_first_not_of(" \t\r\n") == std::string::npos) continue;
        std::vector<std::string> rawCols = parseCsvLine(line);
        if (rawCols.size() < 5) continue;
        size_t total = rawCols.size();
        size_t actionsIdx = hasActionColumn ? total - 1 : total;
        size_t shieldIdx = hasActionColumn ? total - 2 : total - 1;
        size_t victoryIdx = hasActionColumn ? total - 3 : total - 2;
        size_t coinsIdx = hasActionColumn ? total - 4 : total - 3;
        if (coinsIdx < 2 || victoryIdx >= total || shieldIdx >= total) continue;
        std::string typeStr = rawCols[0];
        std::string name = rawCols[1];
        std::string description;
        for (size_t i = 2; i < coinsIdx; ++i) {
            if (!description.empty()) description += ',';
            description += rawCols[i];
        }
        trim(typeStr); trim(name); trim(description);
        if (description.empty()) {
            auto delim = name.find(',');
            if (delim != std::string::npos) {
                description = trimCopy(name.substr(delim + 1));
                name = trimCopy(name.substr(0, delim));
            }
        }
        std::string coinsField = rawCols[coinsIdx];
        std::string victoryField = rawCols[victoryIdx];
        std::string shieldField = rawCols[shieldIdx];
        std::string actionsField = (hasActionColumn && actionsIdx < total) ? rawCols[actionsIdx] : std::string{};
        trim(coinsField); trim(victoryField); trim(shieldField); trim(actionsField);
        TokenType type;
        try { type = tokenTypeFromString(typeStr); }
        catch (...) { continue; }
        if (type != TokenType::PROGRESS && type != TokenType::MILITARY) continue;
        auto coinsTuple = parseCoinsTuple(coinsField);
        uint8_t victory = parseUint8Value(victoryField);
        uint8_t shield = parseUint8Value(shieldField);
        auto token = std::make_unique<Token>(type, name, description, coinsTuple, victory, shield);
        if (!actionsField.empty()) {
            std::vector<std::pair<std::function<void()>, std::string>> actions;
            for (const auto& act : splitActions(actionsField)) {
                bool handled = false;
                auto addAction = [&](const std::function<void()>& fn) {
                    actions.emplace_back(fn, act);
                    handled = true;
                };
                if (act == "getCoins") {
                    uint32_t totalCoins = totalCoinValue(coinsTuple);
                    if (totalCoins > 0) addAction(getCoins(static_cast<uint8_t>(std::min<uint32_t>(totalCoins, 255u))));
                }
                else if (act == "getVictoryPoints") {
                    if (victory > 0) addAction(getVictoryPoints(victory));
                }
                else if (act == "getShieldPoints") {
                    if (shield > 0) addAction(getShieldPoints(shield));
                }
                else if (act == "applyToken") {
                    addAction(applyToken(name));
                }
                else if (act == "lawTokenEffect") {
                    addAction(lawTokenEffect);
                }
                else if (act == "drawToken") {
                    addAction(drawToken);
                }
                else if (act == "takeNewCard") {
                    addAction(takeNewCard);
                }
                if (!handled) {
                    actions.emplace_back([]() {}, act);
                }
            }
            if (!actions.empty()) {
                token->setOnPlayActions(std::move(actions));
            }
        }
        tokens.emplace_back(std::move(token));
    }
    return tokens;
}
