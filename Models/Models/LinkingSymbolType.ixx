export module LinkingSymbolType;

import <cstdint>;

export namespace Models {
	export enum class __declspec(dllexport) LinkingSymbolType :uint8_t
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
