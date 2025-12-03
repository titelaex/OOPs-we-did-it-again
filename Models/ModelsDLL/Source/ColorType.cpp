module Models.ColorType;
import <optional>;
import <string>;

std::optional<Models::ColorType> Models::StringToColorType(const std::string& str) {
    if (str == "BROWN") return Models::ColorType::BROWN;
    if (str == "GREY") return Models::ColorType::GREY;
    if (str == "RED") return Models::ColorType::RED;
    if (str == "YELLOW") return Models::ColorType::YELLOW;
    if (str == "GREEN") return Models::ColorType::GREEN;
    if (str == "BLUE") return Models::ColorType::BLUE;
    if (str == "PURPLE") return Models::ColorType::PURPLE;
    return std::nullopt;
}

std::string Models::ColorTypeToString(Models::ColorType c)
{
	switch (c) {
	case Models::ColorType::BROWN: return "BROWN";
	case Models::ColorType::GREY: return "GREY";
	case Models::ColorType::RED: return "RED";
	case Models::ColorType::YELLOW: return "YELLOW";
	case Models::ColorType::GREEN: return "GREEN";
	case Models::ColorType::BLUE: return "BLUE";
	case Models::ColorType::PURPLE: return "PURPLE";
	default: return "UNKNOWN";
	}
}

