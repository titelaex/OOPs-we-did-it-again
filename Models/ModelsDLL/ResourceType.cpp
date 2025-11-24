module Models.ResourceType;
import Models.ResourceType;
import <string>;
import <optional>;

using namespace Models;

std::string ResourceTypeToString(Models::ResourceType resource)
{
	switch (resource)
	{
	case Models::ResourceType::CLAY: return "CLAY";
	case Models::ResourceType::WOOD: return "WOOD";
	case Models::ResourceType::STONE: return "STONE";
	case Models::ResourceType::GLASS: return "GLASS";
	case Models::ResourceType::PAPYRUS: return "PAPYRUS";
	case Models::ResourceType::CONDITION_RAW_GOODS: return "CONDITION_RAW_GOODS";
	case Models::ResourceType::CONDITION_MANUFACTURED_GOODS: return "CONDITION_MANUFACTURED_GOODS";
	default: return "Unknown";
	}
}

std::optional<Models::ResourceType> StringToResourceType(const std::string& str) {
    if (str == "CLAY") return Models::ResourceType::CLAY;
    if (str == "WOOD") return Models::ResourceType::WOOD;
    if (str == "STONE") return Models::ResourceType::STONE;
    if (str == "GLASS") return Models::ResourceType::GLASS;
    if (str == "PAPYRUS") return Models::ResourceType::PAPYRUS;
    if (str == "CONDITION_RAW_GOODS") return Models::ResourceType::CONDITION_RAW_GOODS;
    if (str == "CONDITION_MANUFACTURED_GOODS") return Models::ResourceType::CONDITION_MANUFACTURED_GOODS;
    return std::nullopt;
}
