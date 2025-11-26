export module Models.LinkingSymbolType;

import <cstdint>;
import <string>;
import <optional>;

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

	export __declspec(dllexport) std::optional<LinkingSymbolType> StringToLinkingSymbolType(const std::string& str);
	export __declspec(dllexport) std::string LinkingSymbolTypeToString(Models::LinkingSymbolType s);
};
