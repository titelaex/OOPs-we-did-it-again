export module ColorType;

#ifdef BUILDING_DLL
#define DLL_API __declspec(dllexport)
#else
#define DLL_API __declspec(dllimport)
#endif

import <cstdint>;

export namespace Models
{
	export enum class DLL_API ColorType : uint8_t
	{
		BROWN,
		GREY,
		RED,
		YELLOW,
		GREEN,
		BLUE,
		PURPLE
	};
}
