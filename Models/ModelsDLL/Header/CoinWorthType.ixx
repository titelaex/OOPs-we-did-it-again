export module Models.CoinWorthType;

import <cstdint>;
import <string>;
import <optional>;

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
		GREYBROWN,
		BLUE,
		GREEN
	};

	export __declspec(dllexport) std::optional<CoinWorthType> StringToCoinWorthType(const std::string& str);
	export __declspec(dllexport) std::string CoinWorthTypeToString(Models::CoinWorthType w);
}
