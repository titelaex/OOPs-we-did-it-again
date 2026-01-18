module Models.ResourceType;
import Models.ResourceType;
import <string>;
import <optional>;

namespace Models {

std::string ResourceTypeToString(ResourceType resource)
{
    switch (resource)
    {
    case ResourceType::CLAY: return "CLAY";
    case ResourceType::WOOD: return "WOOD";
    case ResourceType::STONE: return "STONE";
    case ResourceType::GLASS: return "GLASS";
    case ResourceType::PAPYRUS: return "PAPYRUS";
    case ResourceType::CONDITION_RAW_GOODS: return "CONDITION_RAW_GOODS";
    case ResourceType::CONDITION_MANUFACTURED_GOODS: return "CONDITION_MANUFACTURED_GOODS";
    default: return "Unknown";
    }
}

std::optional<ResourceType> StringToResourceType(const std::string& str) {
    if (str == "CLAY") return ResourceType::CLAY;
    if (str == "WOOD") return ResourceType::WOOD;
    if (str == "STONE") return ResourceType::STONE;
    if (str == "GLASS") return ResourceType::GLASS;
    if (str == "PAPYRUS") return ResourceType::PAPYRUS;
    if (str == "CONDITION_RAW_GOODS") return ResourceType::CONDITION_RAW_GOODS;
    if (str == "CONDITION_MANUFACTURED_GOODS") return ResourceType::CONDITION_MANUFACTURED_GOODS;
    return std::nullopt;
}


}
