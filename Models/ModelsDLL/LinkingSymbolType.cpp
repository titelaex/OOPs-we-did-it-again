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
