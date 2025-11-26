export module Models.ScientificSymbolType;

import <cstdint>;
import <string>;
import <optional>;

namespace Models
{
	export enum class __declspec(dllexport) ScientificSymbolType : uint8_t
	{
		NO_SYMBOL,
		GLOBE,
		WHEEL,
		CLOCK,
		MOJAR,
		PENDULUM,
		FEATHER,
		SCALE
	};

	export __declspec(dllexport) std::optional<ScientificSymbolType> StringToScientificSymbolType(const std::string& str);
	export __declspec(dllexport) std::string ScientificSymbolTypeToString(ScientificSymbolType s);
}
