module Models.Age;
import <string>;
import <optional>;

export __declspec(dllexport) std::string ageToString(Age a)
{
	switch (a)
	{
	case Age::AGE_I: return "I";
	case Age::AGE_II: return "II";
	case Age::AGE_III: return "III";
	default: return "Unknown";
	}
}

export __declspec(dllexport) std::optional<Age> stringToAge(const std::string& str) {
    if (str == "AGE_I") return Age::AGE_I;
    if (str == "AGE_II") return Age::AGE_II;
    if (str == "AGE_III") return Age::AGE_III;
    return std::nullopt;
}
