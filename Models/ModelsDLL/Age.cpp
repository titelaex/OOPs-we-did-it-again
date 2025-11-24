module Models.Age;

import <string>;
import <optional>;

std::string ageToString(Models::Age a)
{
	switch (a)
	{
	case Models::Age::AGE_I: return "I";
	case Models::Age::AGE_II: return "II";
	case Models::Age::AGE_III: return "III";
	default: return "Unknown";
	}
}

std::optional<Models::Age> stringToAge(const std::string& str) {
    if (str == "AGE_I") return Models::Age::AGE_I;
    if (str == "AGE_II") return Models::Age::AGE_II;
    if (str == "AGE_III") return Models::Age::AGE_III;
    return std::nullopt;
}
