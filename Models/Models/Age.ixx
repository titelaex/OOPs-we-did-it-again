export module Age;
import <string>;
import <cstdint>;


export namespace  Models
{
	export enum class __declspec(dllexport) Age : uint8_t
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
