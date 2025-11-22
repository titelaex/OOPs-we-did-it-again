module Models.LinkingSymbolType;
import <optional>;
import <string>;

export __declspec(dllexport) std::optional<LinkingSymbolType> StringToLinkingSymbolType(const std::string& str) {
    if (str == "NO_SYMBOL") return LinkingSymbolType::NO_SYMBOL;
    if (str == "VESSEL") return LinkingSymbolType::VESSEL;
    if (str == "BARREL") return LinkingSymbolType::BARREL;
    if (str == "DRAMA") return LinkingSymbolType::DRAMA;
    if (str == "TEMPLE") return LinkingSymbolType::TEMPLE;
    if (str == "SUN") return LinkingSymbolType::SUN;
    if (str == "WATER") return LinkingSymbolType::WATER;
    if (str == "PILLAR") return LinkingSymbolType::PILLAR;
    if (str == "MOON") return LinkingSymbolType::MOON;
    if (str == "TARGET") return LinkingSymbolType::TARGET;
    if (str == "SPARTAN") return LinkingSymbolType::SPARTAN;
    if (str == "HORSESHOE") return LinkingSymbolType::HORSESHOE;
    if (str == "DAGGER") return LinkingSymbolType::DAGGER;
    if (str == "TOWER") return LinkingSymbolType::TOWER;
    if (str == "HARP") return LinkingSymbolType::HARP;
    if (str == "COG") return LinkingSymbolType::COG;
    if (str == "BOOK") return LinkingSymbolType::BOOK;
    if (str == "LAMP") return LinkingSymbolType::LAMP;
    return std::nullopt;
}
