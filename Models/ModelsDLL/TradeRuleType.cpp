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
