export module Models.TradeRuleType;

import <cstdint>;
import <string>;
import <optional>;

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

	export __declspec(dllexport) std::string tradeRuleTypeToString(TradeRuleType tradeRule); 
	export __declspec(dllexport) std::optional<TradeRuleType> StringToTradeRuleType(const std::string& str);
}
