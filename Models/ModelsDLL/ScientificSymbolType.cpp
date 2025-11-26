module Models.ScientificSymbolType;
import <optional>;
import <string>;

std::optional<Models::ScientificSymbolType> Models::StringToScientificSymbolType(const std::string& str) {
    if (str == "NO_SYMBOL") return Models::ScientificSymbolType::NO_SYMBOL;
    if (str == "GLOBE") return Models::ScientificSymbolType::GLOBE;
    if (str == "WHEEL") return Models::ScientificSymbolType::WHEEL;
    if (str == "CLOCK") return Models::ScientificSymbolType::CLOCK;
    if (str == "MOJAR") return Models::ScientificSymbolType::MOJAR;
    if (str == "PENDULUM") return Models::ScientificSymbolType::PENDULUM;
    if (str == "FEATHER") return Models::ScientificSymbolType::FEATHER;
    if (str == "SCALE") return Models::ScientificSymbolType::SCALE;
    return std::nullopt;
}
