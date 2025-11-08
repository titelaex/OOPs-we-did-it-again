export module LinkingSymbolType;

#ifdef BUILDING_DLL
#define DLL_API __declspec(dllexport)
#else
#define DLL_API __declspec(dllimport)
#endif

import <cstdint>;

export namespace Models {
	export enum class DLL_API LinkingSymbolType :uint8_t
	{
		NO_SYMBOL,
		VESSEL,
		BARREL,
		DRAMA,
		TEMPLE,
		SUN,
		WATER,
		PILLAR,
		MOON,
		TARGET,
		SPARTAN,
		HORSESHOE,
		DAGGER,
		TOWER,
		HARP,
		COG,
		BOOK,
		LAMP
	};
};
