export module Models.ScientificSymbolType;

import <cstdint>;

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
}
