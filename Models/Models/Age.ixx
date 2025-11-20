export module Models.Age;
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

	export std::string AgeToString(Age a); // moved implementation to Age.cpp
}
