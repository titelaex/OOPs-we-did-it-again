module Models.TradeRuleType;
import Models.TradeRuleType;
import <optional>;
import <string>;

using namespace Models;

std::string Models::tradeRuleTypeToString(TradeRuleType tradeRule)
{
	switch (tradeRule)
	{
	case TradeRuleType::CLAY: return "Clay";
	case TradeRuleType::WOOD: return "Wood";
	case TradeRuleType::STONE: return "Stone";
	case TradeRuleType::GLASS: return "Glass";
	case TradeRuleType::PAPYRUS: return "Papyrus";
	default: return "Unknown";
	}
}

std::optional<Models::TradeRuleType> StringToTradeRuleType(const std::string& str) {
    if (str == "CLAY") return Models::TradeRuleType::CLAY;
    if (str == "WOOD") return Models::TradeRuleType::WOOD;
    if (str == "STONE") return Models::TradeRuleType::STONE;
    if (str == "GLASS") return Models::TradeRuleType::GLASS;
    if (str == "PAPYRUS") return Models::TradeRuleType::PAPYRUS;
    return std::nullopt;
}
