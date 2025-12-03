module Models.LinkingSymbolType;
import <optional>;
import <string>;

std::optional<Models::LinkingSymbolType> Models::StringToLinkingSymbolType(const std::string& str) {
    if (str == "NO_SYMBOL") return Models::LinkingSymbolType::NO_SYMBOL;
    if (str == "VESSEL") return Models::LinkingSymbolType::VESSEL;
    if (str == "BARREL") return Models::LinkingSymbolType::BARREL;
    if (str == "DRAMA") return Models::LinkingSymbolType::DRAMA;
    if (str == "TEMPLE") return Models::LinkingSymbolType::TEMPLE;
    if (str == "SUN") return Models::LinkingSymbolType::SUN;
    if (str == "WATER") return Models::LinkingSymbolType::WATER;
    if (str == "PILLAR") return Models::LinkingSymbolType::PILLAR;
    if (str == "MOON") return Models::LinkingSymbolType::MOON;
    if (str == "TARGET") return Models::LinkingSymbolType::TARGET;
    if (str == "SPARTAN") return Models::LinkingSymbolType::SPARTAN;
    if (str == "HORSESHOE") return Models::LinkingSymbolType::HORSESHOE;
    if (str == "DAGGER") return Models::LinkingSymbolType::DAGGER;
    if (str == "TOWER") return Models::LinkingSymbolType::TOWER;
    if (str == "HARP") return Models::LinkingSymbolType::HARP;
    if (str == "COG") return Models::LinkingSymbolType::COG;
    if (str == "BOOK") return Models::LinkingSymbolType::BOOK;
    if (str == "LAMP") return Models::LinkingSymbolType::LAMP;
    return std::nullopt;
}

std::string Models::LinkingSymbolTypeToString(Models::LinkingSymbolType s)
{
    switch (s) {
    case Models::LinkingSymbolType::NO_SYMBOL: return "NO_SYMBOL";
    case Models::LinkingSymbolType::VESSEL: return "VESSEL";
    case Models::LinkingSymbolType::BARREL: return "BARREL";
    case Models::LinkingSymbolType::DRAMA: return "DRAMA";
    case Models::LinkingSymbolType::TEMPLE: return "TEMPLE";
    case Models::LinkingSymbolType::SUN: return "SUN";
    case Models::LinkingSymbolType::WATER: return "WATER";
    case Models::LinkingSymbolType::PILLAR: return "PILLAR";
    case Models::LinkingSymbolType::MOON: return "MOON";
    case Models::LinkingSymbolType::TARGET: return "TARGET";
    case Models::LinkingSymbolType::SPARTAN: return "SPARTAN";
    case Models::LinkingSymbolType::HORSESHOE: return "HORSESHOE";
    case Models::LinkingSymbolType::DAGGER: return "DAGGER";
    case Models::LinkingSymbolType::TOWER: return "TOWER";
    case Models::LinkingSymbolType::HARP: return "HARP";
    case Models::LinkingSymbolType::COG: return "COG";
    case Models::LinkingSymbolType::BOOK: return "BOOK";
    case Models::LinkingSymbolType::LAMP: return "LAMP";
    default: return "UNKNOWN";
    }
}
