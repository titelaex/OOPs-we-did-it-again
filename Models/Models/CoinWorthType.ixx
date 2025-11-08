export module CoinWorthType;

#ifdef BUILDING_DLL
#define DLL_API __declspec(dllexport)
#else
#define DLL_API __declspec(dllimport)
#endif

import <cstdint>;

export namespace Models
{
	export enum class DLL_API CoinWorthType : uint8_t
	{
		VALUE,
		WONDER,
		GREY,
		BROWN,
		YELLOW,
		RED,
	};
}
