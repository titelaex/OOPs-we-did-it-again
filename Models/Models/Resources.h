#pragma once
#include <cstdint>
#include <string>
namespace Models
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

	std::string ResourceTypeToString(ResourceType resource);
}
