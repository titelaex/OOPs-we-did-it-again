export module Models.ColorType;

import <cstdint>;
import <string>;
import <optional>;

export namespace Models
{
	export enum class __declspec(dllexport) ColorType : uint8_t
	{
		BROWN,
		GREY,
		RED,
		YELLOW,
		GREEN,
		BLUE,
		PURPLE
	};

	export __declspec(dllexport) std::optional<ColorType> StringToColorType(const std::string& str);
}
