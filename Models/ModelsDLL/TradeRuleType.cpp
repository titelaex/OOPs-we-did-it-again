module Models.TradeRuleType;
import Models.TradeRuleType;
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

export __declspec(dllexport) std::optional<TradeRuleType> StringToTradeRuleType(const std::string& str) {
    if (str == "CLAY") return TradeRuleType::CLAY;
    if (str == "WOOD") return TradeRuleType::WOOD;
    if (str == "STONE") return TradeRuleType::STONE;
    if (str == "GLASS") return TradeRuleType::GLASS;
    if (str == "PAPYRUS") return TradeRuleType::PAPYRUS;
    return std::nullopt;
}
