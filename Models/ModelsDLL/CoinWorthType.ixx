export module Models.CoinWorthType;

import <cstdint>;

export namespace Models
{
	export enum class __declspec(dllexport) CoinWorthType : uint8_t
	{
		VALUE,
		WONDER,
		GREY,
		BROWN,
		YELLOW,
		RED,
	};
}
