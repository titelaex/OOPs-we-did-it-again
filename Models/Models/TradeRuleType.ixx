export module Models.TradeRuleType;

import <cstdint>;
import <string>;

export namespace Models
{
	export enum class __declspec(dllexport) TradeRuleType : uint8_t
	{
		CLAY,
		WOOD,
		STONE,
		GLASS,
		PAPYRUS,
	};

	export __declspec(dllexport) std::string tradeRuleTypeToString(TradeRuleType tradeRule); // implementation in TradeRuleType.cpp
}
