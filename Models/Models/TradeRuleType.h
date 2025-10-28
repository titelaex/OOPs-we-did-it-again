#pragma once
#include <cstdint>
#include <string>
namespace Models
{
	enum class TradeRuleType:uint8_t
	{
		CLAY, //raw good
		WOOD, //raw good
		STONE, //raw good
		GLASS, //manufactured good
		PAPYRUS, //manufactured good
	};
	std::string tradeRuleTypeToString(TradeRuleType tradeRule);
}

