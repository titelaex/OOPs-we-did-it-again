export module Age;
import <string>;
import <cstdint>;

#ifdef BUILDING_DLL
#define DLL_API __declspec(dllexport)
#else
#define DLL_API __declspec(dllimport)
#endif


export namespace Models
{
	export enum class Age : uint8_t
	{
		AGE_I = 1,
		AGE_II = 2,
		AGE_III = 3
	};

	export inline std::string AgeToString(Age a)
	{
		switch (a)
		{
		case Age::AGE_I: return "Age I";
		case Age::AGE_II: return "Age II";
		case Age::AGE_III: return "Age III";
		default: return "Unknown";
		}
	}
	
}
