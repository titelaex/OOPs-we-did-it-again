module Models.ResourceType;
import Models.ResourceType;
import <string>;

using namespace Models;

std::string Models::ResourceTypeToString(ResourceType resource)
{
	switch (resource)
	{
	case ResourceType::CLAY: return "Clay";
	case ResourceType::WOOD: return "Wood";
	case ResourceType::STONE: return "Stone";
	case ResourceType::GLASS: return "Glass";
	case ResourceType::PAPYRUS: return "Papyrus";
	case ResourceType::CONDITION_RAW_GOODS: return "Raw Goods";
	case ResourceType::CONDITION_MANUFACTURED_GOODS: return "Manufactured Goods";
	default: return "Unknown";
	}
}
