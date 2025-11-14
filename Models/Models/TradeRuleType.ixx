export module TradeRuleType;

import <cstdint>;
import <string>;

export namespace Models
{
	export enum class __declspec(dllexport) TradeRuleType : uint8_t
	{
		CLAY, //raw good
		WOOD, //raw good
		STONE, //raw good
		GLASS, //manufactured good
		PAPYRUS, //manufactured good
	};

	export __declspec(dllexport) std::string tradeRuleTypeToString(TradeRuleType tradeRule);

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
