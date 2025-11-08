export module TradeRuleType;

import <cstdint>;
import <string>;

export namespace Models
{
	enum class TradeRuleType : uint8_t
	{
		CLAY, //raw good
		WOOD, //raw good
		STONE, //raw good
		GLASS, //manufactured good
		PAPYRUS, //manufactured good
	};

	export std::string tradeRuleTypeToString(TradeRuleType tradeRule)
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
			return {};
		}
	}
}