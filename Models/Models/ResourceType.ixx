export module ResourceType;

import <cstdint>;
import <string>;

export namespace Models
{
	enum class ResourceType : uint8_t
	{
		CLAY, //raw good
		WOOD, //raw good
		STONE, //raw good
		GLASS, //manufactured good
		PAPYRUS, //manufactured good
		CONDITION_RAW_GOODS, //player chooses one raw good per round
		CONDITION_MANUFACTURED_GOODS //player chooses one manufactured good per round
	};

	export std::string ResourceTypeToString(ResourceType resource)
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
			return {};
		}
	}
}