module Models.CoinWorthType;
import <optional>;
import <string>;

export __declspec(dllexport) std::optional<CoinWorthType> StringToCoinWorthType(const std::string& str) {
    if (str == "VALUE") return CoinWorthType::VALUE;
    if (str == "WONDER") return CoinWorthType::WONDER;
    if (str == "GREY") return CoinWorthType::GREY;
    if (str == "BROWN") return CoinWorthType::BROWN;
    if (str == "YELLOW") return CoinWorthType::YELLOW;
    if (str == "RED") return CoinWorthType::RED;
    return std::nullopt;
}
