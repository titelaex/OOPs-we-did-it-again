export module Models.Age;
import <string>;
import <optional>;

export namespace  Models
{
	export enum class __declspec(dllexport) Age : uint8_t
	{
		AGE_I = 1,
		AGE_II = 2,
		AGE_III = 3
	};

	export __declspec(dllexport) std::string AgeToString(Age a);
	export __declspec(dllexport) std::optional<Age> StringToAge(const std::string& str);
}
