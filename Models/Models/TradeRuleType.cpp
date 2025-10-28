#include "TradeRuleType.h"

using namespace Models;
std::string Models::tradeRuleTypeToString(TradeRuleType tradeRule)
{
	switch (tradeRule)
	{
	case TradeRuleType::CLAY:
		return "CLAY";
	case TradeRuleType::WOOD:
		return "WOOD";
	case TradeRuleType::STONE:
		return "STONE";
	case TradeRuleType::GLASS:
		return "GLASS";
	case TradeRuleType::PAPYRUS:
		return "PAPYRUS";
	default:
		break;
	}
}