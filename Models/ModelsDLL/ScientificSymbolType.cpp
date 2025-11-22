module Models.ScientificSymbolType;
import <optional>;
import <string>;

export __declspec(dllexport) std::optional<ScientificSymbolType> StringToScientificSymbolType(const std::string& str) {
    if (str == "NO_SYMBOL") return ScientificSymbolType::NO_SYMBOL;
    if (str == "GLOBE") return ScientificSymbolType::GLOBE;
    if (str == "WHEEL") return ScientificSymbolType::WHEEL;
    if (str == "CLOCK") return ScientificSymbolType::CLOCK;
    if (str == "MOJAR") return ScientificSymbolType::MOJAR;
    if (str == "PENDULUM") return ScientificSymbolType::PENDULUM;
    if (str == "FEATHER") return ScientificSymbolType::FEATHER;
    if (str == "SCALE") return ScientificSymbolType::SCALE;
    return std::nullopt;
}
