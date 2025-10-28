#include "tradeRule.h"

using namespace Models;
std::string Models::tradeRuleTypeToString(tradeRuleType tradeRule)
{
	switch (tradeRule)
	{
	case tradeRuleType::CLAY:
		return "CLAY";
	case tradeRuleType::WOOD:
		return "WOOD";
	case tradeRuleType::STONE:
		return "STONE";
	case tradeRuleType::GLASS:
		return "GLASS";
	case tradeRuleType::PAPYRUS:
		return "PAPYRUS";
	default:
		break;
	}
}