module Models.CoinWorthType;
import <optional>;
import <string>;

std::optional<Models::CoinWorthType> Models::StringToCoinWorthType(const std::string& str) {
    if (str == "VALUE") return Models::CoinWorthType::VALUE;
    if (str == "WONDER") return Models::CoinWorthType::WONDER;
    if (str == "GREY") return Models::CoinWorthType::GREY;
    if (str == "BROWN") return Models::CoinWorthType::BROWN;
    if (str == "YELLOW") return Models::CoinWorthType::YELLOW;
    if (str == "RED") return Models::CoinWorthType::RED;
    return std::nullopt;
}

std::string Models::CoinWorthTypeToString(Models::CoinWorthType w)
{
	switch (w) {
	case Models::CoinWorthType::VALUE: return "VALUE";
	case Models::CoinWorthType::WONDER: return "WONDER";
	case Models::CoinWorthType::GREY: return "GREY";
	case Models::CoinWorthType::BROWN: return "BROWN";
	case Models::CoinWorthType::YELLOW: return "YELLOW";
	case Models::CoinWorthType::RED: return "RED";
	default: return "UNKNOWN";
	}
}