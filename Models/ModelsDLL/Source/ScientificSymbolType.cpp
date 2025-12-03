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

std::string Models::ScientificSymbolTypeToString(Models::ScientificSymbolType s)
{
    switch (s) {
    case Models::ScientificSymbolType::NO_SYMBOL: return "NO_SYMBOL";
    case Models::ScientificSymbolType::GLOBE: return "GLOBE";
    case Models::ScientificSymbolType::WHEEL: return "WHEEL";
    case Models::ScientificSymbolType::CLOCK: return "CLOCK";
    case Models::ScientificSymbolType::MOJAR: return "MOJAR";
    case Models::ScientificSymbolType::PENDULUM: return "PENDULUM";
    case Models::ScientificSymbolType::FEATHER: return "FEATHER";
    case Models::ScientificSymbolType::SCALE: return "SCALE";
    default: return "UNKNOWN";
    }
}
