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
