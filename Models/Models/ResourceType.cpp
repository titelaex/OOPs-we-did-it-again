#include "ResourceType.h"

std::string Models::ResourceTypeToString(ResourceType resource)
{
	switch (resource)
	{
	case ResourceType::CLAY:
		return "Clay";
	case ResourceType::WOOD:
		return "Wood";
	case ResourceType::STONE:
		return "Stone";
	case ResourceType::GLASS:
		return "Glass";
	case ResourceType::PAPYRUS:
		return "Papyrus";
	case ResourceType::CONDITION_RAW_GOODS:
		return "Raw Good Condition Placeholder"; //Maybe choose resource
	case ResourceType::CONDITION_MANUFACTURED_GOODS:
		return "Manufactured Good Condition Placeholder"; //Maybe choose resource
	default:
		break;
	};
}
